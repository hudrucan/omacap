#include "capture-handoff-smoke.hpp"
#include "capture-handoff-test.hpp"
#include "capture.hpp"
#include "editor.hpp"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QPromise>
#include <QScopeGuard>
#include <QSemaphore>
#include <QTemporaryDir>
#include <QtConcurrentRun>
#include <QtTest/QTest>
#include <memory>

namespace {
bool checkBackdrop(QApplication &application, const CaptureData &capture,
                   QString &error) {
  CaptureEditor editor(capture, CaptureEditor::CaptureMode::Fullscreen);
  editor.resize(800, 600);
  editor.show();
  if (!editor.waitForSnapshot()) {
    error = QStringLiteral("Could not persist backdrop fixture");
    return false;
  }
  QPromise<QImage> pending;
  pending.start();
  editor.setBackdropFutureForTest(pending.future());
  const auto initialLog = editor.operationLog();
  editor.setProcessLauncherForTest([](const QString &, const QStringList &) { return true; });
  QTest::keyClick(&editor, Qt::Key_S, Qt::ControlModifier);
  const bool started =
      editor.statusForTest().contains(QStringLiteral("Saving screenshot"));
  // Finish the held decode before any failure return destroys its promise.
  pending.addResult(capture.source);
  pending.finish();
  application.processEvents();
  if (started || !editor.isVisible() || editor.operationLog() == initialLog) {
    editor.waitForExport();
    error = QStringLiteral("Output did not wait for the configured backdrop");
    return false;
  }
  const auto ready = editor.operationLog();
  if (ready.isEmpty() || ready.first().type != Operation::Type::Background ||
      ready.first().background != BackgroundStyle::Custom) {
    error = QStringLiteral("Ready backdrop was not included in document history");
    return false;
  }
  return true;
}

bool checkDestruction(const CaptureData &capture, QString &error) {
  auto editor = std::make_unique<CaptureEditor>(
      capture, CaptureEditor::CaptureMode::Fullscreen);
  if (!editor->waitForSnapshot()) {
    error = QStringLiteral("Could not persist destruction fixture");
    return false;
  }
  const QString source = temporarySnapshotPath();
  const QString log = operationLogPath(source);
  QSemaphore started;
  QSemaphore destroyed;
  // The old destructor unlinks the files and wakes this worker. A destructor
  // that drains output must wait for the bounded worker instead, retaining
  // both files throughout that interval. No GUI event loop releases the wait.
  auto pending = QtConcurrent::run([&] {
    started.release();
    static_cast<void>(destroyed.tryAcquire(1, 100));
    return QFile::exists(source) && QFile::exists(log);
  });
  QObject::connect(editor.get(), &QObject::destroyed,
                    [&] { destroyed.release(); });
  editor->setFinishFutureForTest(pending);
  started.acquire();
  editor.reset();
  pending.waitForFinished();
  if (!pending.result()) {
    error = QStringLiteral("Destruction removed files still owned by output");
    return false;
  }
  if (QFile::exists(source) || QFile::exists(log)) {
    error = QStringLiteral("Destruction did not clean drained working files");
    return false;
  }
  return true;
}

} // namespace

bool runCaptureHandoffSmoke(QApplication &application, QString &error) {
  QTemporaryDir runtime;
  QTemporaryDir state;
  if (!runtime.isValid() || !state.isValid())
    return false;
  const auto previousRuntime = qgetenv("XDG_RUNTIME_DIR");
  const auto previousState = qgetenv("XDG_STATE_HOME");
  const auto previousHandoff = qgetenv("OMACAP_HANDOFF_DIR");
  const auto restore = qScopeGuard([&] {
    qputenv("XDG_RUNTIME_DIR", previousRuntime);
    qputenv("XDG_STATE_HOME", previousState);
    qputenv("OMACAP_HANDOFF_DIR", previousHandoff);
  });
  qputenv("XDG_RUNTIME_DIR", runtime.path().toUtf8());
  qputenv("XDG_STATE_HOME", state.path().toUtf8());
  qunsetenv("OMACAP_HANDOFF_DIR");
  const QString directory = captureHandoffDirectory();
  if (!directory.startsWith(runtime.path() + '/')) {
    error = QStringLiteral("Action data escaped the runtime directory");
    return false;
  }

  CaptureData capture;
  capture.monitor.name = QStringLiteral("TEST");
  capture.monitor.geometry = {0, 0, 800, 600};
  capture.monitor.pixelSize = {800, 600};
  capture.monitor.scale = 1.0;
  capture.source = QImage(800, 600, QImage::Format_ARGB32_Premultiplied);
  capture.source.fill(Qt::white);
  capture.previewSize = capture.source.size();
  {
    CaptureEditor editor(capture, CaptureEditor::CaptureMode::File);
    editor.setSuppressSnapshots(true);
    editor.show();
    QTest::keyClick(&editor, Qt::Key_Escape);
    if (editor.isVisible() || !listCaptureHandoffsForTest().isEmpty()) {
      error = QStringLiteral("Closing an editor retained a screenshot");
      return false;
    }
  }

  OperationLog log;
  log.captureId = QString(32, QLatin1Char('a'));
  log.previewSize = capture.previewSize;
  CaptureHandoff published;
  auto writer = std::make_unique<CaptureHandoffWriter>(log.captureId);
  QString failure;
  if (writer->record({}, log, capture.source, failure) || failure.isEmpty() ||
      !listCaptureHandoffsForTest().isEmpty()) {
    error = QStringLiteral("An incomplete handoff was published");
    return false;
  }
  auto opening = QtConcurrent::run([id = log.captureId] {
    return findCaptureHandoff(id);
  });
  QTest::qWait(30);
  const bool openedEarly = opening.isFinished();
  const bool recorded = writer->record(capture.source, log, capture.source,
                                        error, &published);
  writer.reset();
  opening.waitForFinished();
  const auto handoff = opening.result();
  if (openedEarly || !recorded || !handoff ||
      QImage(handoff->sourcePath).convertToFormat(capture.source.format()) != capture.source ||
      QImage(handoff->thumbPath).isNull()) {
    error = QStringLiteral("Immediate Edit failed: early=%1 recorded=%2 found=%3 (%4)")
                .arg(openedEarly).arg(recorded).arg(handoff.has_value()).arg(error);
    return false;
  }
  OperationLog restored;
  if (!loadOperationLog(handoff->logPath, restored, error) ||
      restored.captureId != log.captureId)
    return false;

  // Repeated edits replace their temporary document, without a history list.
  if (!recordCaptureHandoff(capture.source, log, capture.source, error) ||
      listCaptureHandoffsForTest().size() != 1)
    return false;
  const auto latest = findCaptureHandoff(log.captureId);
  QFile thumbnail(latest->thumbPath);
  if (!thumbnail.open(QIODevice::ReadWrite) ||
      !thumbnail.setFileTime(QDateTime::currentDateTime().addSecs(-16 * 60),
                             QFileDevice::FileModificationTime))
    return false;
  thumbnail.close();
  log.captureId = QString(32, QLatin1Char('b'));
  if (!recordCaptureHandoff(capture.source, log, capture.source, error) ||
      findCaptureHandoff(restored.captureId) ||
      listCaptureHandoffsForTest().size() != 1) {
    error = QStringLiteral("Expired action data was retained");
    return false;
  }
  if (!QDir(state.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
    error = QStringLiteral("Capture actions created persistent history");
    return false;
  }
  return checkDestruction(capture, error) &&
         checkBackdrop(application, capture, error);
}

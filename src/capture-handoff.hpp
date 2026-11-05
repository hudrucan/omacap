/** Temporary runtime documents shared by notification, preview, and editor. */
#pragma once

#include <QImage>
#include <QString>
#include <memory>
#include <optional>

struct OperationLog;
class QLockFile;

struct CaptureHandoff {
  QString sourcePath;
  QString logPath;
  QString thumbPath;
};

constexpr int kHandoffThumbEdge = 320;

/// Private runtime directory. OMARCHY_CAPTURE_HANDOFF_DIR isolates test fixtures.
[[nodiscard]] QString captureHandoffDirectory();

/// Reserve before showing a preview; its Edit action waits for publication.
class CaptureHandoffWriter final {
public:
  explicit CaptureHandoffWriter(QString captureId);
  ~CaptureHandoffWriter();
  [[nodiscard]] bool record(const QImage &source, const OperationLog &log,
                            const QImage &rendered, QString &error,
                            CaptureHandoff *published = nullptr);

private:
  QString captureId_;
  QString root_;
  QString stem_;
  QString error_;
  std::unique_ptr<QLockFile> pending_;
};

/// Worker-only. Waits for a pending source/log write before reopening it.
[[nodiscard]] std::optional<CaptureHandoff> findCaptureHandoff(
    const QString &captureId, QString *error = nullptr);
[[nodiscard]] bool recordCaptureHandoff(const QImage &source,
                                       const OperationLog &log,
                                       const QImage &rendered, QString &error);
void removeCaptureHandoff(const CaptureHandoff &handoff);

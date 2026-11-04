#pragma once
#include "capture-handoff.hpp"
#include <QDir>
#include <QVector>

inline QVector<CaptureHandoff> listCaptureHandoffsForTest() {
  QVector<CaptureHandoff> result;
  const QDir dir(captureHandoffDirectory());
  const auto names = dir.entryList({QStringLiteral("*.thumb.png")},
                                   QDir::Files, QDir::Name | QDir::Reversed);
  for (const QString &name : names) {
    const QString id = name.chopped(10).section(QLatin1Char('-'), -1);
    if (auto handoff = findCaptureHandoff(id))
      result.push_back(*handoff);
  }
  return result;
}

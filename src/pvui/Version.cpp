#include "Version.h"
#include <QString>
#include <QCoreApplication>

namespace pvui {

QString versionString() noexcept {
#ifdef PVIEW_NOVERSION
  return QCoreApplication::translate("Version", "(No Version Available)");
#else
  QString version = QString("%1.%2.%3").arg(QString::number(PVIEW_VERSION_MAJOR), QString::number(PVIEW_VERSION_MINOR), QString::number(PVIEW_VERSION_PATCH));
#ifndef NDEBUG
  version += " " + QCoreApplication::translate("Version", "(Development Build)");
#endif
  return version;
#endif
}

}

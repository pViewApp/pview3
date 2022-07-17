#ifndef PVUI_VERSION_H
#define PVUI_VERSION_H

#include <QString>

#ifndef PVIEW_VERSION_MAJOR
#define PVIEW_VERSION_MAJOR 3
#define PVIEW_VERSION_MINOR 0
#define PVIEW_VERSION_PATCH 0

#define PVIEW_NOVERSION
#endif

namespace pvui {

QString versionString() noexcept;

}

#endif // PVUI_VERSION_H

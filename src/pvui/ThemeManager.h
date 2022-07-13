#ifndef PVUI_THEMEMANAGER_H
#define PVUI_THEMEMANAGER_H

#include <QByteArray>
#include <QObject>

namespace pvui {
struct ThemeManager {
public:
  ThemeManager() = delete;
  enum class Theme : unsigned int {
    System,
    FusionLight,
    FusionDark,
#ifdef Q_OS_WINDOWS
    Auto,
#endif
  };

  static void initialize();
  static void setTheme(Theme theme);
  static bool handleNativeEvent(const QByteArray& eventType, void* message, long* result);
  static Theme currentTheme();
  static Theme defaultTheme();
};
} // namespace pvui

#endif // PVUI_THEMEMANAGER_H

#include "ThemeManager.h"
#include <QApplication>
#include <QColor>
#include <QDate>
#include <QDoubleSpinBox>
#include <QItemEditorFactory>
#include <QPalette>
#include <QSettings>
#include <QStandardItemEditorCreator>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>
#include <Qt>
#ifdef Q_OS_WIN
#include <cstring>
#include <windows.h>
#include <winreg.h>
#endif

namespace pvui {

namespace {

constexpr QColor brandColor = QColor(0x33, 0xaa, 0x00);
constexpr QColor translucentBrandColor = QColor(0x33, 0xaa, 0x00, 0x40);

QColor withAlpha(const QColor& base, int alpha) {
  QColor alphaColor = base;
  alphaColor.setAlpha(alpha);
  return alphaColor;
}

QPalette lightPalette() {
  QPalette palette = QPalette(QColor(0xe0, 0xe0, 0xe0), QColor(0xe9, 0xe9, 0xe9));
  palette.setColor(QPalette::Base, QColor(0xf8, 0xf8, 0xf8));
  palette.setColor(QPalette::Highlight, brandColor);
  palette.setColor(QPalette::Inactive, QPalette::Highlight, translucentBrandColor);
  palette.setColor(QPalette::Inactive, QPalette::ButtonText, withAlpha(palette.color(QPalette::ButtonText), 0x80));

  palette.setColor(QPalette::Disabled, QPalette::Base, Qt::transparent);
  palette.setColor(QPalette::Disabled, QPalette::Text, withAlpha(palette.color(QPalette::Text), 0x80));
  palette.setColor(QPalette::Disabled, QPalette::PlaceholderText,
                   withAlpha(palette.color(QPalette::PlaceholderText), 0x80));
  palette.setColor(QPalette::Disabled, QPalette::WindowText, withAlpha(palette.color(QPalette::WindowText), 0x80));
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, withAlpha(palette.color(QPalette::ButtonText), 0x80));
  palette.setColor(QPalette::Disabled, QPalette::Button, withAlpha(palette.color(QPalette::Button), 0x80));
  palette.setColor(QPalette::Disabled, QPalette::Window, Qt::transparent);
  palette.setColor(QPalette::Disabled, QPalette::Dark, Qt::transparent);
  palette.setColor(QPalette::Disabled, QPalette::Shadow, Qt::transparent);
  palette.setColor(QPalette::Disabled, QPalette::Midlight, Qt::transparent);
  palette.setColor(QPalette::Disabled, QPalette::Mid, Qt::transparent);
  palette.setColor(QPalette::Disabled, QPalette::Light, Qt::transparent);
  return palette;
}

QPalette darkPalette() {
  QPalette palette = QPalette(QColor(0x20, 0x23, 0x1f), QColor(0x3b, 0x3f, 0x3a));
  palette.setColor(QPalette::Base, palette.color(QPalette::Button));
  palette.setColor(QPalette::Highlight, brandColor.lighter(75));
  palette.setColor(QPalette::Inactive, QPalette::Highlight, palette.color(QPalette::Window));
  palette.setColor(QPalette::Inactive, QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
  palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, palette.color(QPalette::PlaceholderText));
  palette.setColor(QPalette::Inactive, QPalette::ButtonText, withAlpha(palette.color(QPalette::ButtonText), 0x80));
  palette.setColor(QPalette::Disabled, QPalette::Base, withAlpha(palette.color(QPalette::Base), 0x40));

  return palette;
}
} // namespace

} // namespace pvui
#ifdef Q_OS_WIN

namespace {

void reloadWindowsAutoTheme() {
  char result[4]; // reg key is a DWORD
  auto size = static_cast<DWORD>(sizeof(result));
  auto res = RegGetValueA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                          "AppsUseLighttheme", RRF_RT_REG_DWORD, nullptr, &result, &size);
  auto value = int(result[3] << 24 | result[2] << 16 | result[1] << 8 | result[0]);

  if (res == ERROR_SUCCESS && value == 0) {
    QApplication::setPalette(pvui::darkPalette());
  } else {
    QApplication::setPalette(pvui::lightPalette());
  }
}

// Handles a native windows event
// This implements ThemeManager::Theme::Auto changing if the user changes the system color scheme.
bool windowsHandleNativeEvent(MSG* msg, /* out */ long* result) {
  if (msg->message == WM_SETTINGCHANGE && pvui::ThemeManager::currentTheme() == pvui::ThemeManager::Theme::Auto &&
      msg->lParam && lstrcmpW(reinterpret_cast<wchar_t*>(msg->lParam), L"ImmersiveColorSet") == 0) {
    reloadWindowsAutoTheme();
    *result = 0;
    return true;
  } else {
    return false;
  }
}
} // namespace

#endif // Q_OS_WIN

namespace pvui {

void ThemeManager::initialize() { setTheme(currentTheme()); }

void ThemeManager::setTheme(Theme theme) {
  QSettings settings;
  settings.setValue(QStringLiteral("Theme"), static_cast<int>(theme));
  if (theme == Theme::System) {
    QStyle* style = QStyleFactory::create("windowsvista");
    QApplication::setStyle(style);
    QApplication::setPalette(style->standardPalette());
    return;
  } else if (theme == Theme::FusionLight || theme == Theme::FusionDark) {
    QApplication::setStyle("fusion");
    if (theme == Theme::FusionLight) {
      QApplication::setPalette(lightPalette());
    } else {
      QApplication::setPalette(darkPalette());
    }
  } else if (theme == Theme::Auto) {
    QApplication::setStyle("fusion");
    reloadWindowsAutoTheme();
  }
}

ThemeManager::Theme ThemeManager::currentTheme() {
  QSettings settings;
  return static_cast<Theme>(settings.value(QStringLiteral("Theme"), static_cast<int>(defaultTheme())).toInt());
}

bool ThemeManager::handleNativeEvent(const QByteArray& eventType, void* message, long* result) {
  if (eventType == QStringLiteral("windows_generic_MSG")) {
    return windowsHandleNativeEvent(static_cast<MSG*>(message), result);
  }
  return false;
}

ThemeManager::Theme ThemeManager::defaultTheme() {
#ifdef Q_OS_WIN
  return Theme::Auto;
#else
  return Theme::System;
#endif
}

} // namespace pvui

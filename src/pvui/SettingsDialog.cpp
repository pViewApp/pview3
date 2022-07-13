#include "SettingsDialog.h"
#include "ThemeManager.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedLayout>
#include <QString>
#include <QStringLiteral>
#include <QtGlobal>

namespace pvui {
namespace dialogs {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::RestoreDefaults) {
#ifdef Q_OS_MACOS
  setWindowTitle(tr("Preferences"));
#else
  setWindowTitle(tr("Settings"));
#endif

  setupWidgets();
  resize(400, 240); // default size
}

void SettingsDialog::setupWidgets() {
  QVBoxLayout* root = new QVBoxLayout();
  setLayout(root);

  // Heading
#ifdef Q_OS_MACOS
  constexpr char title[] = "<html><h1>Preferences</html></h1>";
#else
  constexpr char title[] = "<html><h1>Settings</html></h1>";
#endif
  root->addWidget(new QLabel(tr(title)));

  QScrollArea* container = new QScrollArea();
root->addWidget(container, 1); // make sure to stretch it out
  container->setFrameStyle(QFrame::NoFrame);
  container->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
  container->setMinimumHeight(180);

  QWidget* content = new QWidget();
  container->setWidget(content);
  container->setWidgetResizable(true);
  content->setLayout(&mainLayout);

  // APPEARANCE SECTION
  appearanceGroupBox = new QGroupBox(tr("&Appearance"));
  mainLayout.addWidget(appearanceGroupBox);
  appearanceLayout = new QFormLayout(appearanceGroupBox);
  theme = new QComboBox();
#ifdef Q_OS_WIN
  theme->addItem(tr("Auto"), static_cast<int>(ThemeManager::Theme::Auto));
#endif
  theme->addItem(tr("Dark"), static_cast<int>(ThemeManager::Theme::FusionDark));
  theme->addItem(tr("Light"), static_cast<int>(ThemeManager::Theme::FusionLight));
  theme->addItem(tr("System"), static_cast<int>(ThemeManager::Theme::System));
  QObject::connect(theme, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
    ThemeManager::setTheme(static_cast<ThemeManager::Theme>(theme->currentData().toInt()));
  });
  appearanceLayout->addRow(new QLabel(tr("Theme:")), theme);

  // WARNINGS SECTION
  warningsGroupBox = new QGroupBox(tr("&Warnings"));
  mainLayout.addWidget(warningsGroupBox);
  warningsLayout = new QVBoxLayout(warningsGroupBox);
  warnOnTransactionDeletion = new QCheckBox(tr("Warn me when I delete a &transaction"));
  warnOnSecurityDeletion = new QCheckBox(tr("Warn me when I delete a &security"));
  warnOnSecurityPriceDownloadFailure = new QCheckBox(tr("Warn me when a security price &update fails"));
  warnOnSecurityPriceDeletion = new QCheckBox(tr("Warn me when I delete a security &price"));
  QObject::connect(warnOnTransactionDeletion, &QCheckBox::toggled, this, [this](bool toggled) {
                     set(QStringLiteral("AccountPage/WarnOnTransactionDeletion"), toggled);
                   });
  QObject::connect(warnOnSecurityDeletion, &QCheckBox::toggled, this, [this](bool toggled) {
                     set(QStringLiteral("SecurityPage/WarnOnSecurityDeletion"), toggled);
                   });
  QObject::connect(warnOnSecurityPriceDownloadFailure, &QCheckBox::toggled, this, [this](bool toggled) {
                     set(QStringLiteral("SecurityPage/WarnOnSecurityPriceDownloadFailure"), toggled);
                   });
  QObject::connect(warnOnSecurityPriceDeletion, &QCheckBox::toggled, this, [this](bool toggled) {
                     set(QStringLiteral("SecurityPriceDialog/WarnOnSecurityPriceDeletion"), toggled);
                   });
  warningsLayout->addWidget(warnOnTransactionDeletion);
  warningsLayout->addWidget(warnOnSecurityDeletion);
  warningsLayout->addWidget(warnOnSecurityPriceDownloadFailure);
  warningsLayout->addWidget(warnOnSecurityPriceDeletion);
  warningsLayout->addStretch(); // Push previous widgets to top

  // BUTTONBOX
  root->addWidget(&buttonBox);
  QObject::connect(&buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* button) {
                     if (buttonBox.buttonRole(button) == QDialogButtonBox::AcceptRole) {
                       this->accept();
                     } else if (buttonBox.buttonRole(button) == QDialogButtonBox::ResetRole) {
                       this->resetToDefaults();
                     }
                   });
  // FINAL INIT
  refresh();
}

void SettingsDialog::set(const QString& key, const QVariant& value) {
  settings.setValue(key, value);
  emit settingChanged(key, value);
}

void SettingsDialog::refresh() {
  theme->setCurrentIndex(theme->findData(static_cast<int>(ThemeManager::currentTheme())));
  warnOnTransactionDeletion->setChecked(settings.value(QStringLiteral("AccountPage/WarnOnTransactionDeletion"), true).toBool());
  warnOnSecurityDeletion->setChecked(settings.value(QStringLiteral("SecurityPage/WarnOnSecurityDeletion"), true).toBool());
  warnOnSecurityPriceDownloadFailure->setChecked(settings.value(QStringLiteral("SecurityPage/WarnOnSecurityPriceDownloadFailure"), true).toBool());
  warnOnSecurityPriceDeletion->setChecked(settings.value(QStringLiteral("SecurityPriceDialog/WarnOnSecurityPriceDeletion"), true).toBool());
}

void SettingsDialog::resetToDefaults() {
  ThemeManager::setTheme(ThemeManager::defaultTheme());
  set(QStringLiteral("AccountPage/WarnOnTransactionDeletion"), true);
  set(QStringLiteral("SecurityPage/WarnOnSecurityDeletion"), true);
  set(QStringLiteral("SecurityPage/WarnOnSecurityPriceDownloadFailure"), true);
  set(QStringLiteral("SecurityPriceDialog/WarnOnSecurityPriceDeletion"), true);
  refresh();
}

}
}

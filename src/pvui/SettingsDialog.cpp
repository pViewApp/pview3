#include "SettingsDialog.h"
#include <QGroupBox>
#include <QtGlobal>
#include <QLabel>
#include <QFrame>
#include <QCheckBox>
#include <QStringLiteral>
#include <QDialogButtonBox>
#include <QStackedLayout>
#include <QScrollArea>
#include <QSizePolicy>

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
  warnOnTransactionDeletion->setChecked(settings.value(QStringLiteral("AccountPage/WarnOnTransactionDeletion"), true).toBool());
  warnOnSecurityDeletion->setChecked(settings.value(QStringLiteral("SecurityPage/WarnOnSecurityDeletion"), true).toBool());
  warnOnSecurityPriceDownloadFailure->setChecked(settings.value(QStringLiteral("SecurityPage/WarnOnSecurityPriceDownloadFailure"), true).toBool());
  warnOnSecurityPriceDeletion->setChecked((QStringLiteral("SecurityPriceDialog/WarnOnSecurityPriceDeletion"), true));
}

void SettingsDialog::resetToDefaults() {
  set(QStringLiteral("AccountPage/WarnOnTransactionDeletion"), true);
  set(QStringLiteral("SecurityPage/WarnOnSecurityDeletion"), true);
  set(QStringLiteral("SecurityPage/WarnOnSecurityPriceDownloadFailure"), true);
  set(QStringLiteral("SecurityPriceDialog/WarnOnSecurityPriceDeletion"), true);
  refresh();
}

}
}

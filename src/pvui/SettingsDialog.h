#ifndef PVUI_SETTINGSDIALOG_H
#define PVUI_SETTINGSDIALOG_H

#include <QObject>
#include <QDialog>
#include <QString>
#include <QDialogButtonBox>
#include <QVariant>
#include <QSettings>
#include <QVBoxLayout>

class QGroupBox;
class QCheckBox;
class QFormLayout;
class QComboBox;

namespace pvui {
namespace dialogs {

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget* parent = nullptr);
public slots:
  void refresh();
  void resetToDefaults();
private:
  void setupWidgets();
private slots:
  void set(const QString& key, const QVariant& value);
private:
  QSettings settings;

  QVBoxLayout mainLayout;

  QGroupBox* appearanceGroupBox;
  QFormLayout* appearanceLayout;
  QComboBox* theme;

  QGroupBox* warningsGroupBox;
  QVBoxLayout* warningsLayout;
  QCheckBox* warnOnTransactionDeletion;
  QCheckBox* warnOnSecurityDeletion;
  QCheckBox* warnOnSecurityPriceDownloadFailure;
  QCheckBox* warnOnSecurityPriceDeletion;

  QDialogButtonBox buttonBox;
signals:
  void settingChanged(const QString& key, const QVariant& value); // note: not emitted when the theme is changed
};

}
}

#endif // PVUI_SETTINGSDIALOG_H

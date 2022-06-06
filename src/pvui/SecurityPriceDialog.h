#ifndef PVUI_DIALOGS_SECURITYPRICEDIALOG_H
#define PVUI_DIALOGS_SECURITYPRICEDIALOG_H

#include "SecurityPriceInsertionWidget.h"
#include "SecurityPriceModel.h"
#include "pv/Security.h"
#include "pv/Signals.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QWidget>
#include <memory>

namespace pvui {
namespace dialogs {

/// \brief A dialog which allows you to view and edit a security's prices.
class SecurityPriceDialog : public QDialog {
  Q_OBJECT
private:
  pv::Security* security_ = nullptr;

  QVBoxLayout* layout = new QVBoxLayout(this);
  QTableView* table = new QTableView;
  controls::SecurityPriceInsertionWidget* insertionWidget;
  QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(table);
  std::unique_ptr<models::SecurityPriceModel> model = nullptr;

  pv::ScopedConnection securityNameChangeConnection;

  void setupTableContextMenu();

  void updateTitle();

public:
  SecurityPriceDialog(pv::Security& security, QWidget* parent = nullptr);
  void showEvent(QShowEvent*) override { insertionWidget->setFocus(); }
public slots:
  void setSecurity(pv::Security& security);

signals:
  void securityNameChanged();
};

} // namespace dialogs
} // namespace pvui

#endif // PVUI_DIALOGS_SECURITYPRICEDIALOG_H

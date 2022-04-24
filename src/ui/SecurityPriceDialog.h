#ifndef PVUI_DIALOGS_SECURITYPRICEDIALOG_H
#define PVUI_DIALOGS_SECURITYPRICEDIALOG_H

#include "DataFile.h"
#include "SecurityPriceInsertionWidget.h"
#include "SecurityPriceModel.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QWidget>
#include <boost/signals2/connection.hpp>
#include <memory>

namespace pvui {
namespace dialogs {
class SecurityPriceDialog : public QDialog {
  Q_OBJECT
private:
  pv::SecurityPtr security_;

  QVBoxLayout* layout = new QVBoxLayout(this);
  QTableView* table = new QTableView;
  controls::SecurityPriceInsertionWidget* insertionWidget = new controls::SecurityPriceInsertionWidget;
  QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(table);
  std::unique_ptr<models::SecurityPriceModel> model = nullptr;

  boost::signals2::scoped_connection securityNameChangeConnection;

  void setupTableContextMenu();
  void updateTitle();

public:
  SecurityPriceDialog(pv::SecurityPtr security = nullptr, QWidget* parent = nullptr);
  void showEvent(QShowEvent*) override { insertionWidget->setFocus(); }
public slots:
  void setSecurity(pv::SecurityPtr security);
};

} // namespace dialogs
} // namespace pvui

#endif // PVUI_DIALOGS_SECURITYPRICEDIALOG_H

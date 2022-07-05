#ifndef PVUI_DIALOGS_SECURITYPRICEDIALOG_H
#define PVUI_DIALOGS_SECURITYPRICEDIALOG_H

#include "SecurityPriceInsertionWidget.h"
#include "SecurityPriceModel.h"
#include "pv/Security.h"
#include "pv/Signals.h"
#include "pvui/DataFileManager.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QWidget>
#include <memory>
#include <optional>

namespace pvui {
namespace dialogs {

/// \brief A dialog which allows you to view and edit a security's prices.
class SecurityPriceDialog : public QDialog {
  Q_OBJECT
private:
  DataFileManager& dataFileManager;
  pv::i64 security_;

  QVBoxLayout* layout = new QVBoxLayout(this);
  QTableView* table = new QTableView;
  controls::SecurityPriceInsertionWidget* insertionWidget;
  QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(table);
  std::unique_ptr<models::SecurityPriceModel> model = nullptr;

  pv::ScopedConnection securityUpdatedConnection;
  pv::ScopedConnection resetConnection;

  void setupTableContextMenu();

  void updateTitle();
private slots:
  void onSubmit(QDate date);

  void handleDataFileChanged();
public:
  SecurityPriceDialog(DataFileManager& dataFileManager, pv::i64 security, QWidget* parent = nullptr);
public slots:
  void setSecurity(pv::i64 security);

signals:
  void securityUpdated();
};

} // namespace dialogs
} // namespace pvui

#endif // PVUI_DIALOGS_SECURITYPRICEDIALOG_H

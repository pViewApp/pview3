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

namespace pvui {
namespace dialogs {
class SecurityPriceDialog : public QDialog {
  Q_OBJECT
private:
  pv::SecurityPtr security_;

  QSortFilterProxyModel* model = new QSortFilterProxyModel;

  QVBoxLayout* layout = new QVBoxLayout(this);
  QTableView* table = new QTableView;
  controls::SecurityPriceInsertionWidget* insertionBar = new controls::SecurityPriceInsertionWidget;
  QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

public:
  SecurityPriceDialog(pv::SecurityPtr security = nullptr, QWidget* parent = nullptr);
public slots:
  void setSecurity(pv::SecurityPtr security);
};

} // namespace dialogs
} // namespace pvui

#endif // PVUI_DIALOGS_SECURITYPRICEDIALOG_H

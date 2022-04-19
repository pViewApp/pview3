#ifndef PVUI_ACCOUNTPAGE_H
#define PVUI_ACCOUNTPAGE_H

#include "DataFile.h"
#include "ExtendedSpinBox.h"
#include "Page.h"
#include "TransactionInsertionWidget.h"
#include "TransactionModel.h"
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVariant>
#include <QWidget>
#include <boost/signals2.hpp>
#include <optional>

namespace pvui {

class AccountPageWidget : public PageWidget {
  Q_OBJECT
private:
  pv::AccountPtr account_ = nullptr;
  QTableView *table;
  controls::TransactionInsertionWidget *insertWidget;
  QSortFilterProxyModel *model;

public:
  AccountPageWidget(QWidget *parent = nullptr);
public slots:
  inline void setAccount(pv::AccountPtr account) {
    account_ = account;
    setTitle(QString::fromStdString(account->name()));
    model->setSourceModel(new models::TransactionModel(account_));
    insertWidget->setAccount(account);
    table->scrollToBottom();
  }
};
} // namespace pvui

#endif // PVUI_ACCOUNT_PAGE_H

#include "AccountPage.h"
#include "Actions.h"
#include "SecurityPage.h"
#include "TransactionInsertionWidget.h"
#include <QDate>
#include <QHeaderView>
#include <QLineEdit>
#include <QTreeView>
#include <algorithm>
#include <date/date.h>
#include <fmt/format.h>
#include <string>

pvui::AccountPageWidget::AccountPageWidget(pv::AccountPtr account, QWidget* parent) : PageWidget(parent) {
  setTitle("No Account Open");

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(table, 1);
  layout->addWidget(insertWidget);
  setContent(layout);

  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->hide();

  table->setSortingEnabled(true);
  table->sortByColumn(0, Qt::AscendingOrder);

  table->setSelectionBehavior(QTableView::SelectRows);

  table->setModel(proxyModel);
  table->scrollToBottom();

  setAccount(account);
}

void pvui::AccountPageWidget::setAccount(pv::AccountPtr account) {
  account_ = account;

  setDisabled(account_ == nullptr);

  if (account_ == nullptr) {
  } else {
    setTitle(QString::fromStdString(account_->name()));
  }

  model = account_ == nullptr ? nullptr : std::make_unique<models::TransactionModel>(account_);
  proxyModel->setSourceModel(model.get());
  insertWidget->setAccount(account_);

  if (!isEnabled())
    return;

  table->scrollToBottom();

  accountNameChangedConnection = account->nameChanged().connect(
      [&](std::string newName, std::string) { setTitle(QString::fromStdString(newName)); });
}

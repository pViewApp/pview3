#include "AccountPage.h"
#include "SecurityPage.h"
#include "TransactionInsertionWidget.h"
#include "pv/Actions.h"
#include <QDate>
#include <QHeaderView>
#include <QLineEdit>
#include <QTreeView>
#include <algorithm>
#include <date/date.h>
#include <fmt/format.h>
#include <string>

pvui::AccountPageWidget::AccountPageWidget(std::optional<pv::Account> account, QWidget* parent) : PageWidget(parent) {
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

void pvui::AccountPageWidget::setAccount(std::optional<pv::Account> account) {
  account_ = account;

  setEnabled(account_.has_value());

  if (!account_.has_value()) {
    setTitle("No Account Open");
  } else {
    setTitle(QString::fromStdString(account_->name()));
  }

  model = account_.has_value() ? std::make_unique<models::TransactionModel>(*account_) : nullptr;
  proxyModel->setSourceModel(model.get());
  insertWidget->setAccount(account_);

  if (!isEnabled())
    return;

  table->scrollToBottom();

  accountNameChangedConnection = account->nameChanged().connect(
      [&](std::string newName, std::string) { setTitle(QString::fromStdString(newName)); });
}

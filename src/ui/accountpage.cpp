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

pvui::AccountPageWidget::AccountPageWidget(QWidget* parent)
    : PageWidget(parent), table(new QTableView), insertWidget(new controls::TransactionInsertionWidget),
      model(new QSortFilterProxyModel) {
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

  table->setModel(model);
  table->scrollToBottom();
}

#include "SecurityPriceDialog.h"
#include <QAction>
#include <QHeaderView>
#include <vector>

namespace pvui {
namespace dialogs {

// Implementation detail
namespace SecurityPriceDialog_ {
namespace {

constexpr int dialogWidth = 800;
constexpr int dialogHeight = 600;

} // namespace
} // namespace SecurityPriceDialog_

SecurityPriceDialog::SecurityPriceDialog(pv::Security& security, QWidget* parent)
    : QDialog(parent), insertionWidget(new controls::SecurityPriceInsertionWidget(security)) {
  setWindowFlag(Qt::WindowContextHelpButtonHint, true); // Enable the What's This? button
  // Setup dialog
  resize(SecurityPriceDialog_::dialogWidth, SecurityPriceDialog_::dialogHeight);

  layout->addWidget(table);
  layout->addWidget(insertionWidget);
  layout->addWidget(dialogButtonBox);

  setFocusPolicy(Qt::TabFocus);
  setFocusProxy(insertionWidget);

  // Setup buttons
  QObject::connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &SecurityPriceDialog::accept);

  // Setup table
  table->verticalHeader()->hide();
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setModel(proxyModel);
  table->setSortingEnabled(true);
  table->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  table->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
  table->setAlternatingRowColors(true);
  setupTableContextMenu();

  table->setWhatsThis(
      tr("This table shows the current security prices for <b>%1</b>.").arg(QString::fromStdString(security.name())));
  insertionWidget->setWhatsThis(tr(
      R"(
<html>Enter new security prices in this form:
  <ul>
    <li><b>Date</b>: the date of this security price</li>
    <li><b>Price</b>: the price in dollars</li>
  </ul>
</html>
)"));

  QObject::connect(this, &SecurityPriceDialog::securityNameChanged, this,
                   [&]() { updateTitle(); }); // Update the title when name changed
  setSecurity(security);
  QObject::connect(insertionWidget, &controls::SecurityPriceInsertionWidget::submitted, this,
                   &SecurityPriceDialog::onSubmit);
}

void SecurityPriceDialog::onSubmit(QDate date) {
  std::optional<QModelIndex> sourceIndex = model->mapFromDate(
      pv::Date(pv::YearMonthDay(pv::Year(date.year()), pv::Month(date.month()), pv::Day(date.day()))));
  if (sourceIndex.has_value()) {
    table->selectRow(proxyModel->mapFromSource(*sourceIndex).row());
  }
}

void SecurityPriceDialog::setupTableContextMenu() {
  table->setContextMenuPolicy(Qt::ActionsContextMenu);

  QAction* deleteAction = new QAction(tr("Delete"));
  deleteAction->setShortcut(QKeySequence::Delete);
  deleteAction->setDisabled(true);

  QObject::connect(deleteAction, &QAction::triggered, this, [&] {
    // Get selected rows
    auto selected = proxyModel->mapSelectionToSource(table->selectionModel()->selection());

    std::vector<pv::Date> dates; // Vector stores dates of all deleted security prices
    dates.reserve(selected.size());

    for (const auto& selectionRange : selected) {
      dates.push_back(model->mapToDate(selectionRange.topLeft()));
    }

    std::for_each(dates.cbegin(), dates.cend(), [&](const pv::Date& date) { security_->removePrice(date); });
  });

  QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                   [=]() { deleteAction->setDisabled(table->selectionModel()->selectedRows().isEmpty()); });

  table->addAction(deleteAction);
}

void SecurityPriceDialog::updateTitle() {
  setWindowTitle(tr("Editing Security Prices for %1").arg(QString::fromStdString(security_->name())));
}

void SecurityPriceDialog::setSecurity(pv::Security& security) {
  security_ = &security;

  model = std::make_unique<models::SecurityPriceModel>(&security, proxyModel);
  proxyModel->setSourceModel(model.get());
  insertionWidget->setSecurity(security);
  table->scrollToBottom();

  securityNameChangeConnection.disconnect();

  updateTitle();
  securityNameChangeConnection =
      security.listenNameChanged([&](std::string, std::string) { emit securityNameChanged(); });
}

} // namespace dialogs
} // namespace pvui

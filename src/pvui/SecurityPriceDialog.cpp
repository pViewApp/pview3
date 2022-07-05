#include "SecurityPriceDialog.h"
#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include <QAction>
#include "DateUtils.h"
#include "pv/Security.h"
#include "pvui/DataFileManager.h"
#include <QHeaderView>
#include <optional>
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

SecurityPriceDialog::SecurityPriceDialog(DataFileManager& dataFileManager, pv::i64 security, QWidget* parent)
    : QDialog(parent), dataFileManager(dataFileManager),
      insertionWidget(new controls::SecurityPriceInsertionWidget(dataFileManager)) {
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
  setupTableContextMenu();

  insertionWidget->setWhatsThis(tr(
      R"(
<html>Enter new security prices in this form:
  <ul>
    <li><b>Date</b>: the date of this security price</li>
    <li><b>Price</b>: the price in dollars</li>
  </ul>
</html>
)"));

  QObject::connect(this, &SecurityPriceDialog::securityUpdated, this,
                   [&]() { updateTitle(); }); // Update the title when name changed
  setSecurity(security);
  QObject::connect(insertionWidget, &controls::SecurityPriceInsertionWidget::submitted, this,
                   &SecurityPriceDialog::onSubmit);

  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this,
                   &SecurityPriceDialog::handleDataFileChanged);

  handleDataFileChanged();
}

void SecurityPriceDialog::handleDataFileChanged() {
  resetConnection.disconnect();
  securityUpdatedConnection.disconnect();

  if (!dataFileManager.has()) {
    return;
  }

  securityUpdatedConnection = dataFileManager->onSecurityUpdated([&](pv::i64 security) {
    if (security == this->security_) {
      emit securityUpdated();
    }
  });

  resetConnection = dataFileManager->onRollback([&] { setSecurity(this->security_); });
}

void SecurityPriceDialog::onSubmit(QDate date) {
  std::optional<QModelIndex> sourceIndex = model->mapFromDate(date);
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

    std::vector<pv::i64> dates; // Vector stores dates of all deleted security prices
    dates.reserve(selected.size());

    for (const auto& selectionRange : selected) {
      dates.push_back(toEpochDate(model->mapToDate(selectionRange.topLeft())));
    }

    // Create transaction for improved performance on bulk update
    bool transaction = dataFileManager->beginTransaction() == pv::ResultCode::OK;

    std::for_each(dates.cbegin(), dates.cend(),
                  [&](const pv::i64 date) { dataFileManager->removeSecurityPrice(security_, date); });
    if (transaction) {
      dataFileManager->commitTransaction();
    }
  });

  QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                   [=]() { deleteAction->setDisabled(table->selectionModel()->selectedRows().isEmpty()); });

  table->addAction(deleteAction);
}

void SecurityPriceDialog::updateTitle() {
  setWindowTitle(tr("Editing Security Prices for %1")
                     .arg(QString::fromStdString(pv::security::name(*dataFileManager, security_))));
}

void SecurityPriceDialog::setSecurity(pv::i64 security) {
  this->security_ = security;

  model = std::make_unique<models::SecurityPriceModel>(*dataFileManager, security, proxyModel);
  proxyModel->setSourceModel(model.get());
  insertionWidget->setSecurity(security);
  table->scrollToBottom();

  table->setWhatsThis(tr("This table shows the current security prices for <b>%1</b>.")
                          .arg(QString::fromStdString(pv::security::name(*dataFileManager, security)).toHtmlEscaped()));

  insertionWidget->setSecurity(security);

  updateTitle();
}

} // namespace dialogs
} // namespace pvui

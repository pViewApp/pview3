#include "SecurityPriceDialog.h"
#include <QAction>
#include <QHeaderView>
#include <vector>

constexpr int dialogWidth = 800;
constexpr int dialogHeight = 600;

namespace pvui {
namespace dialogs {
SecurityPriceDialog::SecurityPriceDialog(pv::Security security, QWidget* parent)
    : QDialog(parent), security_(security), insertionWidget(new controls::SecurityPriceInsertionWidget(security_)) {
  // Setup dialog
  resize(dialogWidth, dialogHeight);

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

  setSecurity(security);
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

    std::for_each(dates.cbegin(), dates.cend(), [&](const pv::Date& date) { security_.removePrice(date); });
  });

  QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                   [=]() { deleteAction->setDisabled(table->selectionModel()->selectedRows().isEmpty()); });

  table->addAction(deleteAction);
}

void SecurityPriceDialog::updateTitle() {
  setWindowTitle(tr("Editing Security Prices for %1").arg(QString::fromStdString(security_.name())));
}

void SecurityPriceDialog::setSecurity(pv::Security security) {
  security_ = security;
  setEnabled(security_.valid());

  model = std::make_unique<models::SecurityPriceModel>(security, proxyModel);
  proxyModel->setSourceModel(model.get());
  insertionWidget->setSecurity(security);
  table->scrollToBottom();

  securityNameChangeConnection.disconnect();

  if (!security_.valid())
    return;

  updateTitle();
  securityNameChangeConnection = security_.nameChanged().connect([&](std::string, std::string) { updateTitle(); });
}

} // namespace dialogs
} // namespace pvui

#include "SecurityPriceDialog.h"
#include <QHeaderView>

constexpr int dialogWidth = 800;
constexpr int dialogHeight = 600;

namespace pvui {
namespace dialogs {
SecurityPriceDialog::SecurityPriceDialog(pv::SecurityPtr security,
                                         QWidget *parent)
    : QDialog(parent), security_(security) {
  // Setup dialog
  setSizeGripEnabled(true);
  resize(dialogWidth, dialogHeight);

  layout->addWidget(table);
  layout->addWidget(insertionBar);
  layout->addWidget(dialogButtonBox);

  // Setup buttons
  QObject::connect(dialogButtonBox, &QDialogButtonBox::accepted, this,
                   &SecurityPriceDialog::accept);

  // Setup table
  table->verticalHeader()->hide();
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setModel(model);
  table->setSortingEnabled(true);
  table->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  table->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);

  setSecurity(security);
}

void SecurityPriceDialog::setSecurity(pv::SecurityPtr security) {
  security_ = security;

  model->setSourceModel(new models::SecurityPriceModel(security));
  insertionBar->setSecurity(security);
  table->scrollToBottom();
}

} // namespace dialogs
} // namespace pvui

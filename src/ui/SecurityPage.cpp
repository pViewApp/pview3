#include "SecurityPage.h"
#include "SecurityModel.h"
#include "SecurityPriceDialog.h"
#include "SecurityUtils.h"
#include <QHeaderView>

pvui::SecurityPageWidget::SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent)
    : PageWidget(parent), dataFileManager_(dataFileManager) {
  setTitle(tr("Securities"));

  // Setup layout
  auto* mainLayout = new QVBoxLayout();
  mainLayout->addWidget(toolBar);
  mainLayout->addWidget(table);
  mainLayout->addWidget(insertionWidget);
  setContent(mainLayout);

  // Setup toolbar
  setupActions();

  // Setup table
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this,
                   [this](pv::DataFile& dataFile) { setDataFile(dataFile); });
  proxyModel.sort(0, Qt::AscendingOrder);
  table->setSortingEnabled(true);
  table->setModel(&proxyModel);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->hide();
  table->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
  QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&] {
    bool enabled = currentSelectedSecurity() != nullptr;
    securityInfoAction.setEnabled(enabled);
    deleteSecurityAction.setEnabled(enabled);
  });

  setDataFile(*dataFileManager);
}

void pvui::SecurityPageWidget::setupActions() {
  toolBar->addAction(&securityInfoAction);
  toolBar->addAction(&deleteSecurityAction);

  securityInfoAction.setEnabled(false);
  deleteSecurityAction.setEnabled(false);
  deleteSecurityAction.setShortcut(QKeySequence::Delete);

  QObject::connect(&securityInfoAction, &QAction::triggered, this, [&]() {
    pv::SecurityPtr security = currentSelectedSecurity();
    if (security == nullptr)
      return;

    dialogs::SecurityPriceDialog* dialog = new dialogs::SecurityPriceDialog(security, this);
    dialog->exec();
  });

  QObject::connect(&deleteSecurityAction, &QAction::triggered, this, [&]() {
    pv::SecurityPtr security = currentSelectedSecurity();
    if (security == nullptr)
      return;

    dataFileManager_->removeSecurity(security);
  });

  table->addAction(&deleteSecurityAction);
  table->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void pvui::SecurityPageWidget::setDataFile(pv::DataFile& dataFile) {
  model = std::make_unique<models::SecurityModel>(dataFile);
  proxyModel.setSourceModel(new models::SecurityModel(dataFile));
  table->scrollToBottom();
}

pv::SecurityPtr pvui::SecurityPageWidget::currentSelectedSecurity() {
  QItemSelection selection = table->selectionModel()->selection();
  if (selection.isEmpty())
    return nullptr;

  QItemSelectionRange range = selection.first();

  if (range.isEmpty())
    return nullptr;

  QModelIndex index = range.topLeft();
  return model->mapFromIndex(proxyModel.mapToSource(index));
}

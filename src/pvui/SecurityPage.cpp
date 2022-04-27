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
    bool enabled = currentSelectedSecurity().has_value();
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
    std::optional<pv::Security> security = currentSelectedSecurity();
    if (!security.has_value())
      return;

    dialogs::SecurityPriceDialog* dialog = new dialogs::SecurityPriceDialog(*security, this);
    dialog->exec();
  });

  QObject::connect(&deleteSecurityAction, &QAction::triggered, this, [&]() {
    std::optional<pv::Security> security = currentSelectedSecurity();
    if (!security.has_value())
      return;

    dataFileManager_->removeSecurity(*security);
  });

  table->addAction(&deleteSecurityAction);
  table->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void pvui::SecurityPageWidget::setDataFile(pv::DataFile& dataFile) {
  model = std::make_unique<models::SecurityModel>(dataFile);
  proxyModel.setSourceModel(model.get());
  table->scrollToBottom();
}

std::optional<pv::Security> pvui::SecurityPageWidget::currentSelectedSecurity() {
  QItemSelection selection = table->selectionModel()->selection();
  if (selection.isEmpty())
    return std::nullopt;

  QItemSelectionRange range = selection.first();

  if (range.isEmpty())
    return std::nullopt;

  QModelIndex index = range.topLeft();
  return model->mapFromIndex(proxyModel.mapToSource(index));
}

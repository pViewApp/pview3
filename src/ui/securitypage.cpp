#include "SecurityPage.h"
#include "SecurityModel.h"
#include "SecurityPriceDialog.h"
#include "SecurityUtils.h"
#include <QHeaderView>

void pvui::SecurityPageWidget::setupToolbar() {
  toolBar->addAction(securityInfoAction);
  securityInfoAction->setEnabled(false);

  QObject::connect(securityInfoAction, &QAction::triggered, this, [&]() {
    QItemSelectionRange range = table->selectionModel()->selection().first();
    if (range.isEmpty())
      return;

    QString symbol = table->model()->data(range.topLeft()).toString();
    pv::SecurityPtr security = dataFileManager_.dataFile().securityForSymbol(symbol.toStdString());
    dialogs::SecurityPriceDialog* dialog = new dialogs::SecurityPriceDialog(security, this);
    dialog->setWindowTitle(tr("Editing Security Prices for ") + symbol);
    dialog->exec();
  });
}

void pvui::SecurityPageWidget::setDataFile(pv::DataFile& dataFile) {
  model = std::make_unique<models::SecurityModel>(dataFile);
  proxyModel->setSourceModel(new models::SecurityModel(dataFile));
  table->scrollToBottom();
}

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
  setupToolbar();

  // Setup table
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this,
                   [this](pv::DataFile& dataFile) { setDataFile(dataFile); });
  proxyModel->sort(0, Qt::AscendingOrder);
  table->setSortingEnabled(true);
  table->setModel(proxyModel);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->hide();
  table->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
  QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                   [&](QItemSelection current) { securityInfoAction->setEnabled(!current.isEmpty()); });

  setDataFile(*dataFileManager);
}

#include "SecurityPage.h"
#include "SecurityModel.h"
#include "SecurityPriceDialog.h"
#include "SecurityUtils.h"
#include <QHeaderView>

pvui::SecurityPageWidget::SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent)
    : PageWidget(parent), dataFileManager_(dataFileManager) {
  setTitle(tr("Securities"));

  // Setup layout
  layout()->addWidget(table);
  layout()->addWidget(insertionWidget);

  // Setup toolbar & table
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
    pv::Security* security = currentSelectedSecurity();
    bool enabled = security != nullptr;
    securityInfoAction.setEnabled(enabled);
    deleteSecurityAction.setEnabled(enabled);
    setToolBarLabel(security);
  });

  setDataFile(*dataFileManager);
}

void pvui::SecurityPageWidget::setupActions() {
  toolBar_->setObjectName(QString::fromUtf8("securityPageToolBar"));
  toolBar_->setWindowTitle(tr("Securities"));

  toolBar_->addWidget(toolBarTitleLabel);
  toolBarTitleLabel->setTextFormat(Qt::TextFormat::PlainText); // Disable HTML
  setToolBarLabel(nullptr);

  toolBar_->addAction(&securityInfoAction);
  toolBar_->addAction(&deleteSecurityAction);

  securityInfoAction.setEnabled(false);
  deleteSecurityAction.setEnabled(false);
  deleteSecurityAction.setShortcut(QKeySequence::Delete);

  QObject::connect(&securityInfoAction, &QAction::triggered, this, [&]() {
    pv::Security* security = currentSelectedSecurity();
    if (security == nullptr)
      return;

    dialogs::SecurityPriceDialog* dialog = new dialogs::SecurityPriceDialog(*security, this);
    dialog->exec();
  });

  QObject::connect(&deleteSecurityAction, &QAction::triggered, this, [&]() {
    pv::Security* security = currentSelectedSecurity();
    if (security == nullptr)
      return;

    dataFileManager_->removeSecurity(*security);
  });

  table->addAction(&securityInfoAction);
  table->addAction(&deleteSecurityAction);
  table->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void pvui::SecurityPageWidget::setDataFile(pv::DataFile& dataFile) {
  model = std::make_unique<models::SecurityModel>(dataFile);
  proxyModel.setSourceModel(model.get());
  table->scrollToBottom();
}

pv::Security* pvui::SecurityPageWidget::currentSelectedSecurity() {
  QItemSelection selection = table->selectionModel()->selection();
  if (selection.isEmpty())
    return nullptr;

  QItemSelectionRange range = selection.first();

  if (range.isEmpty())
    return nullptr;

  QModelIndex index = range.topLeft();
  return model->mapFromIndex(proxyModel.mapToSource(index));
}

void pvui::SecurityPageWidget::setToolBarLabel(pv::Security* security) {
  static QString format = QString::fromUtf8("%1: ");
  if (security != nullptr) {
    toolBarTitleLabel->setText(format.arg(QString::fromStdString(security->name())));
    toolBarTitleLabel->setEnabled(true);
    QFont bold = font();
    bold.setBold(true);
    toolBarTitleLabel->setFont(bold); // Use bold font
  } else {
    toolBarTitleLabel->setText(format.arg(QString::fromUtf8("(No Security Selected)")));
    toolBarTitleLabel->setDisabled(true);
    toolBarTitleLabel->setFont(font()); // Use normal font
  }
}

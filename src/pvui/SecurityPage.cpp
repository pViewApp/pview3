#include "SecurityPage.h"
#include "SecurityModel.h"
#include "SecurityPriceDialog.h"
#include "SecurityUtils.h"
#include <QCheckBox>
#include <QHeaderView>
#include <QMessageBox>

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
  if (currentPriceDownload != nullptr) {
    currentPriceDownload->abort();
  }

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

void pvui::SecurityPageWidget::resetSecurityPriceUpdateDialog() {
  securityPriceUpdateDialog.setText("");
  securityPriceUpdateDialog.setCheckBox(nullptr);
}

void pvui::SecurityPageWidget::setupActions() {
  toolBar_->setObjectName(QString::fromUtf8("securityPageToolBar"));
  toolBar_->setWindowTitle(tr("Securities"));

  toolBar_->addWidget(toolBarTitleLabel);
  toolBarTitleLabel->setTextFormat(Qt::TextFormat::PlainText); // Disable HTML
  setToolBarLabel(nullptr);

  toolBar_->addAction(&securityInfoAction);
  toolBar_->addAction(&deleteSecurityAction);
  toolBar_->addAction(&updateSecurityPriceAction);

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

  QObject::connect(&updateSecurityPriceAction, &QAction::triggered, this,
                   &SecurityPageWidget::beginUpdateSecurityPrices);

  table->addAction(&securityInfoAction);
  table->addAction(&deleteSecurityAction);
  table->addAction(&updateSecurityPriceAction);
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

void pvui::SecurityPageWidget::beginUpdateSecurityPrices() {
  if (currentPriceDownload != nullptr) {
    return; // Only 1 download at a time
  }

  auto* currentSecurity = currentSelectedSecurity();

  auto endDate = QDate::currentDate().addDays(1);
  auto beginDate = endDate.addDays(-(3 * 31)); // 3 months

  QStringList symbols;

  if (currentSecurity != nullptr) {
    symbols.append(QString::fromStdString(currentSecurity->symbol()));
  } else {
    symbols.reserve(static_cast<int>(dataFileManager_->securities().size()));
    for (const auto* security : dataFileManager_->securities()) {
      symbols.append(QString::fromStdString(security->symbol()));
    }
  }

  currentPriceDownload = priceDownloader_.download(symbols, beginDate, endDate);
  currentPriceDownload->setParent(this);

  QObject::connect(currentPriceDownload, &SecurityPriceDownload::success, this,
                   &SecurityPageWidget::updateSecurityPrices);
  QObject::connect(currentPriceDownload, &SecurityPriceDownload::error, this,
                   &SecurityPageWidget::updateSecurityPricesError);
  QObject::connect(currentPriceDownload, &SecurityPriceDownload::complete, this,
                   &SecurityPageWidget::endUpdateSecurityPrices);
}

void pvui::SecurityPageWidget::updateSecurityPrices(std::map<QDate, pv::Decimal> prices, QString symbol) {
  auto* security = dataFileManager_->securityForSymbol(symbol.toStdString());
  for (const auto& pair : prices) {
    auto pvDate = pv::Date(
        pv::YearMonthDay(pv::Year(pair.first.year()), pv::Month(pair.first.month()), pv::Day(pair.first.day())));
    if (security->prices().find(pvDate) == security->prices().cend()) {
      // Only do it if no existing price on pvDate
      security->setPrice(pvDate, pair.second);
    }
  }
}

void pvui::SecurityPageWidget::updateSecurityPricesError(QNetworkReply::NetworkError err, QString symbol) {
  assert(err != QNetworkReply::NoError && "No error occured, but still called error slot?");

  if (err == QNetworkReply::ContentNotFoundError) {
    failedSecurityDownloadsSymbols += symbol;
    return;
  }

  currentPriceDownload->abort();
  resetSecurityPriceUpdateDialog();
  if (err == QNetworkReply::TimeoutError) {
    securityPriceUpdateDialog.setText(tr("The connection timed out."));
  } else if (err == QNetworkReply::HostNotFoundError || err == QNetworkReply::UnknownNetworkError) {
    securityPriceUpdateDialog.setText(tr("pView could not connect to the internet."));
  } else {
    securityPriceUpdateDialog.setText(tr("Please try again later. (Error Code 0x%1)").arg(err, 0, 16));
  }
  securityPriceUpdateDialog.show();
}

void pvui::SecurityPageWidget::endUpdateSecurityPrices() {
  delete currentPriceDownload;
  currentPriceDownload = nullptr;

  constexpr char failedSecurityDownloadSettingKey[] = "pv/SecurityPage/suppressFailedSecurityPriceDownloadDialog";
  bool suppressFailedDownloadDialog = settings.value(failedSecurityDownloadSettingKey, false).toBool();

  if (!failedSecurityDownloadsSymbols.empty() && !suppressFailedDownloadDialog) {
    resetSecurityPriceUpdateDialog();
    QString base = tr("<html>pView couldn't download prices for the following security(s):<ul>", nullptr,
                      failedSecurityDownloadsSymbols.size());
    const auto& failedDownloads = failedSecurityDownloadsSymbols; // const reference to avoid clazy complaining
    for (const auto& symbol : failedDownloads) {
      base.append("<li>");
      base.append(symbol);
      base.append("</li>");
    }
    base.append("</ul></html>");

    QCheckBox* checkBox = new QCheckBox(tr("&Don't show this again"));
    QObject::connect(checkBox, &QCheckBox::toggled, this,
                     [=](bool toggled) { settings.setValue(failedSecurityDownloadSettingKey, toggled); });

    securityPriceUpdateDialog.setCheckBox(checkBox);
    securityPriceUpdateDialog.setText(base);
    securityPriceUpdateDialog.show();
  }

  failedSecurityDownloadsSymbols.clear();
}

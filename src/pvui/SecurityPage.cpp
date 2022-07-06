#include "SecurityPage.h"
#include "SecurityModel.h"
#include <Qt>
#include "SecurityPriceDialog.h"
#include "DateUtils.h"
#include <cassert>
#include <sqlite3.h>
#include <QMetaObject>
#include "SecurityUtils.h"
#include "pv/Security.h"
#include "pvui/SecurityInsertionWidget.h"
#include <QCheckBox>
#include <QHeaderView>
#include <QMessageBox>
#include <optional>
#include <QStringLiteral>

pvui::SecurityPageWidget::SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent)
    : PageWidget(parent), dataFileManager_(dataFileManager) {
  settings.beginGroup(QStringLiteral("SecurityPage"));
  setTitle(tr("Securities"));

  // Setup layout
  layout()->addWidget(table);
  layout()->addWidget(insertionWidget);

  // Setup toolbar & table
  setupActions();

  // Setup table
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, &SecurityPageWidget::handleDataFileChanged);
  if (currentPriceDownload != nullptr) {
    currentPriceDownload->abort();
  }
  QObject::connect(insertionWidget, &controls::SecurityInsertionWidget::submitted, this, &SecurityPageWidget::handleSecuritySubmitted);
  QObject::connect(&proxyModel, &QSortFilterProxyModel::sourceModelChanged, this, [this]{
                     auto rc = table->model()->rowCount();
                     (void) rc;
                     table->update();
                     table->updateGeometry();
                     table->scrollToTop();
                     table->scrollToBottom();
                   }, Qt::QueuedConnection);

  proxyModel.sort(0, Qt::AscendingOrder);
  table->setSortingEnabled(true);
  table->setModel(&proxyModel);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->hide();
  table->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
  QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&] {
    std::optional<pv::i64> security = currentSelectedSecurity();
    bool enabled = security.has_value();
    securityInfoAction.setEnabled(enabled);
    deleteSecurityAction.setEnabled(enabled);
    setToolBarLabel(security);
  });

  handleDataFileChanged();
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
  setToolBarLabel(std::nullopt);

  toolBar_->addAction(&securityInfoAction);
  toolBar_->addAction(&deleteSecurityAction);
  toolBar_->addAction(&updateSecurityPriceAction);

  securityInfoAction.setEnabled(false);
  deleteSecurityAction.setEnabled(false);
  deleteSecurityAction.setShortcut(QKeySequence::Delete);

  QObject::connect(&securityInfoAction, &QAction::triggered, this, [&]() {
    assert(dataFileManager_.has());
    std::optional<pv::i64> security = currentSelectedSecurity();
    if (!security.has_value())
      return;

    dialogs::SecurityPriceDialog* dialog = new dialogs::SecurityPriceDialog(dataFileManager_, *security);
    dialog->exec();
  });

  QObject::connect(&deleteSecurityAction, &QAction::triggered, this, [&]() {
    assert(dataFileManager_.has());
    std::optional<pv::i64> security = currentSelectedSecurity();
    if (!security.has_value())
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

void pvui::SecurityPageWidget::handleDataFileChanged() {
  model = dataFileManager_.has() ? std::make_unique<models::SecurityModel>(*dataFileManager_) : nullptr;
  proxyModel.setSourceModel(model.get());
  securityInfoAction.setEnabled(dataFileManager_.has());
  deleteSecurityAction.setEnabled(dataFileManager_.has());
  updateSecurityPriceAction.setEnabled(dataFileManager_.has());

  setToolBarLabel(std::nullopt);
}

void pvui::SecurityPageWidget::handleSecuritySubmitted(pv::i64 security) {
  // Select the new security in the table
  QModelIndex index = proxyModel.mapFromSource(model->index(model->rowOfSecurity(security), 0));
  table->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  table->scrollTo(index);
}

std::optional<pv::i64> pvui::SecurityPageWidget::currentSelectedSecurity() {
  QItemSelection selection = table->selectionModel()->selection();
  if (selection.isEmpty())
    return std::nullopt;

  QItemSelectionRange range = selection.first();

  if (range.isEmpty())
    return std::nullopt;

  QModelIndex index = range.topLeft();
  return model->securityOfRow(proxyModel.mapToSource(index).row());
}

void pvui::SecurityPageWidget::setToolBarLabel(std::optional<pv::i64> security) {
  static QString format = QString::fromUtf8("%1: ");
  if (security.has_value()) {
    toolBarTitleLabel->setText(format.arg(QString::fromStdString(pv::security::name(*dataFileManager_, *security))));
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

  std::optional<pv::i64> currentSecurity = currentSelectedSecurity();

  auto endDate = QDate::currentDate().addDays(1);
  auto beginDate = endDate.addDays(-(3 * 31)); // 3 months

  QStringList symbols;

  if (currentSecurity.has_value()) {
    symbols.append(QString::fromStdString(pv::security::symbol(*dataFileManager_, *currentSecurity)));
  } else {
    auto listSecuritiesQuery = dataFileManager_->query("SELECT Id From Securities");
    while (sqlite3_step(&*listSecuritiesQuery) == SQLITE_ROW) {
      std::string symbol = pv::security::symbol(*dataFileManager_, sqlite3_column_int64(&*listSecuritiesQuery, 0));
      symbols += QString::fromStdString(symbol);
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

void pvui::SecurityPageWidget::updateSecurityPrices(std::map<QDate, pv::i64> prices, QString symbol) {
  if (!dataFileManager_.has()) {
    return;
  }

  pv::i64 security = *pv::security::securityForSymbol(*dataFileManager_, symbol.toStdString());

  bool transactionCreated = dataFileManager_->beginTransaction() == pv::ResultCode::OK;
  for (const auto& pair : prices) {
    pv::i64 date = toEpochDate(pair.first);
    if (!pv::security::price(*dataFileManager_, security, date).has_value()) {
      // Only do it if no existing price on pvDate
      dataFileManager_->setSecurityPrice(security, date, pair.second);
    }
  }
  if (transactionCreated) {
    dataFileManager_->commitTransaction();
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
    securityPriceUpdateDialog.setText(tr("Sorry, a network error occured."));
    securityPriceUpdateDialog.setDetailedText(tr("Error Code: 0x%1").arg(err, 0, 16));
  }
  securityPriceUpdateDialog.show();
}

void pvui::SecurityPageWidget::endUpdateSecurityPrices() {
  delete currentPriceDownload;
  currentPriceDownload = nullptr;

  bool suppressFailedDownloadDialog = settings.value(QStringLiteral("SuppressFailedSecurityPriceDownloadWarning"), false).toBool();

  if (!failedSecurityDownloadsSymbols.empty() && !suppressFailedDownloadDialog) {
    resetSecurityPriceUpdateDialog();
    QString base = tr("<html>pView couldn't download prices for the following security(s):<ul>", nullptr,
                      failedSecurityDownloadsSymbols.size());
    const auto& failedDownloads = failedSecurityDownloadsSymbols; // const reference to avoid clazy complaining
    for (const auto& symbol : failedDownloads) {
      base.append("<li>");
      base.append(symbol.toHtmlEscaped());
      base.append("</li>");
    }
    base.append("</ul></html>");

    QCheckBox* checkBox = new QCheckBox(tr("&Don't show this again"));
    QObject::connect(checkBox, &QCheckBox::toggled, this,
                     [=](bool toggled) { settings.setValue(QStringLiteral("SuppressFailedSecurityPriceDownloadWarning"), toggled); });

    securityPriceUpdateDialog.setCheckBox(checkBox);
    securityPriceUpdateDialog.setText(base);
    securityPriceUpdateDialog.show();
  }

  failedSecurityDownloadsSymbols.clear();
}

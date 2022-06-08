#ifndef PVUI_SECURITY_PAGE_H
#define PVUI_SECURITY_PAGE_H

#include "DataFileManager.h"
#include "Page.h"
#include "SecurityInsertionWidget.h"
#include "SecurityModel.h"
#include "SecurityPriceDownloader.h"
#include "pv/Security.h"
#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QQueue>
#include <QRegularExpression>
#include <QSettings>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QToolBar>
#include <QValidator>
#include <memory>
#include <optional>

namespace pvui {

class SecurityPageWidget : public PageWidget {
  Q_OBJECT
private:
  pvui::DataFileManager& dataFileManager_;

  pvui::SecurityPriceDownloader priceDownloader_;
  pvui::SecurityPriceDownload* currentPriceDownload = nullptr;

  QStringList failedSecurityDownloadsSymbols;

  QLabel* toolBarTitleLabel = new QLabel();
  QToolBar* toolBar_ = new QToolBar(this);
  QTableView* table = new QTableView;
  controls::SecurityInsertionWidget* insertionWidget = new controls::SecurityInsertionWidget(dataFileManager_);

  QSortFilterProxyModel proxyModel = QSortFilterProxyModel(table);
  std::unique_ptr<models::SecurityModel> model = nullptr;

  QAction securityInfoAction = QAction(tr("Edit Security Prices..."));
  QAction deleteSecurityAction = QAction(tr("Delete Security"));
  QAction updateSecurityPriceAction = QAction(tr("Update Security Prices"));

  QMessageBox securityPriceUpdateDialog =
      QMessageBox(QMessageBox::Warning, tr("Failed to Download Security Prices"), "", QMessageBox::Ok, this);
  void resetSecurityPriceUpdateDialog();

  QSettings settings;

  void setupActions();
  void setDataFile(pv::DataFile& dataFile);

  pv::Security* currentSelectedSecurity();

  void setToolBarLabel(pv::Security* security);

private slots:
  void beginUpdateSecurityPrices();
  void updateSecurityPrices(std::map<QDate, pv::Decimal> data, QString symbol);
  void updateSecurityPricesError(QNetworkReply::NetworkError err, QString symbol);
  void endUpdateSecurityPrices();

public:
  SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent = nullptr);

  QToolBar* toolBar() override { return toolBar_; }
};

} // namespace pvui

#endif // PVUI_SECURITY_PAGE_H

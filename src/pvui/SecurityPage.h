#ifndef PVUI_SECURITY_PAGE_H
#define PVUI_SECURITY_PAGE_H

#include "DataFileManager.h"
#include "Page.h"
#include "SecurityInsertionWidget.h"
#include "SecurityModel.h"
#include "SecurityPriceDownloader.h"
#include "pv/Integer64.h"
#include "pv/Security.h"
#include <QAction>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <memory>
#include <optional>
#include <QList>

class QToolBar;

namespace pvui {

class SecurityPageWidget : public PageWidget {
  Q_OBJECT
private:
  pvui::DataFileManager& dataFileManager_;

  pvui::SecurityPriceDownloader priceDownloader_;
  pvui::SecurityPriceDownload* currentPriceDownload = nullptr;

  QStringList failedSecurityDownloadsSymbols;

  QToolBar* toolBar_;
  QTableView* table = new QTableView;
  controls::SecurityInsertionWidget* insertionWidget = new controls::SecurityInsertionWidget(dataFileManager_);

  QSortFilterProxyModel proxyModel = QSortFilterProxyModel(table);
  std::unique_ptr<models::SecurityModel> model = nullptr;

  QAction securityPriceAction;
  QAction deleteSecurityAction;
  QAction updateSecurityPriceAction;
  QAction advancedUpdateSecurityPriceAction;

  QMessageBox securityPriceUpdateDialog =
      QMessageBox(QMessageBox::Warning, tr("Failed to Download Security Prices"), "", QMessageBox::Ok, this);
  void resetSecurityPriceUpdateDialog();

  QSettings settings;

  void setupActions();

  QList<pv::i64> selectedSecurities();

  void setToolBarLabel(std::optional<pv::i64> security);
private slots:
  void updateActions();

  void showSecurityPriceDialog(pv::i64 security);

  void handleDataFileChanged();

  bool canDeleteSecurities();
  void deleteSelectedSecurities();

  void handleSecuritySubmitted(pv::i64 security);

  void beginUpdateSecurityPrices(QDate begin, int onConflictBehaviour);
  void beginBasicUpdateSecurityPrices();
  void beginAdvancedUpdateSecurityPrices();
  void updateSecurityPrices(const std::map<QDate, pv::i64>& data, QString symbol, int onConflictBehaviour);
  void updateSecurityPricesError(QNetworkReply::NetworkError err, QString symbol);
  void endUpdateSecurityPrices();

public:
  SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent = nullptr);
};

} // namespace pvui

#endif // PVUI_SECURITY_PAGE_H

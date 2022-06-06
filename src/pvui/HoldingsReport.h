#ifndef PVUI_REPORTS_HOLDINGSREPORT_H
#define PVUI_REPORTS_HOLDINGSREPORT_H

#include "HoldingsModel.h"
#include "Report.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>
#include <memory>

namespace pvui {
namespace reports {

class HoldingsReport : public pvui::Report {
private:
  QSettings settings;

  std::unique_ptr<models::HoldingsModel> model = nullptr;
  QSortFilterProxyModel proxyModel;
  QTableView* table = new QTableView;

  QGroupBox* summaryGroupBox = new QGroupBox;
  QHBoxLayout* summaryLayout = new QHBoxLayout;

  QLabel* summaryCostBasisLabel = new QLabel();
  QLabel* summaryMarketValueLabel = new QLabel();
  QLabel* summaryIncomeLabel = new QLabel();

  void populateSummary();

private slots:
  void setDataFile(pv::DataFile& dataFile);

public:
  HoldingsReport(DataFileManager& manager, QWidget* parent = nullptr);

  void reload() noexcept override;
};

} // namespace reports
} // namespace pvui

#endif // PVUI_REPORTS_HOLDINGSREPORT_H

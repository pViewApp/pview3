#include "StandardReportFactory.h"
#include "AssetAllocationReport.h"
#include "HoldingsReport.h"
#include "MarketValueReport.h"

namespace {

enum class DayOfWeek : int {
  MONDAY = 1,
  TUESDAY = 2,
  WEDNESDAY = 3,
  THURSDAY = 4,
  FRIDAY = 5,
  SATURDAY = 6,
  SUNDAY = 7,
};

QDate nextDayOfWeek(DayOfWeek dayOfWeek) {
  auto today = QDate::currentDate();
  auto output = today.addDays(static_cast<int>(dayOfWeek) - today.dayOfWeek());

  if (output < today) {
    output = output.addDays(7);
  }

  return output;
}

} // namespace

namespace pvui {

StandardReportFactory::StandardReportFactory() {}

std::vector<Report*> StandardReportFactory::createReports(DataFileManager& dataFileManager) const noexcept {
  using namespace pvui::reports;

  auto* currentMarketValueReport =
      new MarketValueReport(MarketValueReport::tr("Market Value (Current)"), dataFileManager);
  // Use default settings for currentMarketValueReport

  auto* past52WeeksMarketValueReport =
      new MarketValueReport(MarketValueReport::tr("Market Value (Past 52 Weeks)"), dataFileManager);

  // Customize past52WeeksMarketValueReport
  past52WeeksMarketValueReport->start = []() {
    return nextDayOfWeek(DayOfWeek::FRIDAY).addDays(-(52 * 7)); // 52 weeks ago
  };

  past52WeeksMarketValueReport->end = []() { return nextDayOfWeek(DayOfWeek::FRIDAY); };
  past52WeeksMarketValueReport->interval = 7;

  return {
      new HoldingsReport(dataFileManager),
      new AssetAllocationReport(dataFileManager),
      currentMarketValueReport,
      past52WeeksMarketValueReport,
  };
}

} // namespace pvui

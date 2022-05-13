#include "StandardReportFactory.h"
#include "HoldingsReport.h"

namespace pvui {

StandardReportFactory::StandardReportFactory() {}

std::vector<Report*> StandardReportFactory::createReports(const DataFileManager& dataFileManager) const noexcept {
  return {
      new reports::HoldingsReport(dataFileManager),
  };
}

} // namespace pvui

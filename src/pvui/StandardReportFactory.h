#ifndef PVUI_STANDARDREPORTFACTORY_H
#define PVUI_STANDARDREPORTFACTORY_H

#include "ReportFactory.h"

namespace pvui {

class StandardReportFactory : public pvui::ReportFactory {
public:
  StandardReportFactory();

  std::vector<Report*> createReports(const DataFileManager& dataFileManager) const noexcept override;
};

} // namespace pvui

#endif // PVUI_STANDARDREPORTFACTORY_H

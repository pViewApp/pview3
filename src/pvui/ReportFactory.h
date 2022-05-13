#ifndef PVUI_REPORTFACTORY_H
#define PVUI_REPORTFACTORY_H

#include "Report.h"
#include <vector>

namespace pvui {

class ReportFactory {
public:
  ReportFactory() {}
  virtual ~ReportFactory() {}

  virtual std::vector<Report*> createReports(const DataFileManager& dataFileMangager) const noexcept = 0;
};

} // namespace pvui

#endif // PVUI_REPORTFACTORY_H

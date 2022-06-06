#ifndef PVUI_REPORTGROUPERS_REPORTGROUPERS_H
#define PVUI_REPORTGROUPERS_REPORTGROUPERS_H

#include "ReportGrouper.h"
#include "pv/DataFile.h"
#include <chrono>

namespace pvui {
namespace reportgroupers {

class SecurityValueGrouper : public ReportGrouper {
  const pv::DataFile& dataFile;

public:
  SecurityValueGrouper(const pv::DataFile& dataFile);

  QVector<QString> keys() const noexcept override;

  double value(QString key, pv::Date date) const noexcept override;
};

} // namespace reportgroupers
} // namespace pvui

#endif // PVUI_REPORTGROUPERS_REPORTGROUPERS_H

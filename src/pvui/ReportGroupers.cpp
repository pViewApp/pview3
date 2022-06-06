#include "ReportGroupers.h"
#include "pv/Algorithms.h"
#include <algorithm>
#include <iterator>

namespace pvui {
namespace reportgroupers {

SecurityValueGrouper::SecurityValueGrouper(const pv::DataFile& dataFile) : dataFile(dataFile) {}

QVector<QString> SecurityValueGrouper::keys() const noexcept {
  const auto& securities = dataFile.securities();
  QVector<QString> output;
  output.reserve(static_cast<int>(securities.size()));
  std::transform(securities.begin(), securities.end(), std::back_inserter(output),
                 [](const pv::Security* security) { return QString::fromStdString(security->symbol()); });
  return output;
}

double SecurityValueGrouper::value(QString key, pv::Date date) const noexcept {
  const pv::Security* security = dataFile.securityForSymbol(key.toStdString());
  if (security == nullptr) {
    return 0;
  }

  return static_cast<double>(pv::algorithms::marketValue(*security, date).value_or(0));
}

} // namespace reportgroupers
} // namespace pvui

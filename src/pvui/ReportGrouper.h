#ifndef PVUI_REPORTGROUPER_H
#define PVUI_REPORTGROUPER_H

#include "pv/Date.h"
#include <QMap>
#include <QVector>

namespace pvui {

class ReportGrouper {
public:
  ReportGrouper() {}
  virtual ~ReportGrouper() {}

  virtual QVector<QString> keys() const noexcept = 0;
  virtual double value(QString key, pv::Date date) const noexcept = 0;
};

} // namespace pvui

#endif // PVUI_REPORTGROUPER_H

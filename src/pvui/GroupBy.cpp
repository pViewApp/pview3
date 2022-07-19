#include "GroupBy.h"
#include <QSettings>
#include <QVariant>
#include "pv/Security.h"

namespace pvui {

GroupBy currentGroupBy() {
  constexpr GroupBy defaultGroupBy = pvui::GroupBy::Symbol;
  return static_cast<GroupBy>(QSettings().value(QStringLiteral("ReportsGroupBy"), static_cast<int>(defaultGroupBy)).toInt());
}

void setGroupBy(GroupBy groupBy) {
  QSettings().setValue(QStringLiteral("ReportsGroupBy"), static_cast<int>(groupBy));
}

QString group(pv::DataFile& dataFile, pv::i64 security, GroupBy groupBy) {
  switch (groupBy) {
    case GroupBy::AssetClass: return QString::fromStdString(pv::security::assetClass(dataFile, security));
    case GroupBy::Sector: return QString::fromStdString(pv::security::sector(dataFile, security));
    case GroupBy::Symbol: return QString::fromStdString(pv::security::symbol(dataFile, security));
    default: return QString();
  }
}

}



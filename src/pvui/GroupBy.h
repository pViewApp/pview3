#ifndef PVUI_GROUPBY_H
#define PVUI_GROUPBY_H
#include <QObject>
#include <QString>
#include "pv/DataFile.h"

namespace pvui {
enum class GroupBy {
  AssetClass, Sector, Symbol
};
GroupBy currentGroupBy();
void setGroupBy(GroupBy groupBy);
QString group(pv::DataFile& dataFile, pv::i64 security, GroupBy groupBy);
}
#endif // PVUI_GROUPBY_H


#include "AssetAllocationReport.h"
#include "DateUtils.h"
#include "pv/Algorithms.h"
#include "pv/DataFile.h"
#include "pv/Security.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPalette>
#include <QString>
#include <QwtLegend>
#include <QwtText>
#include <sqlite3.h>
#include <utility>

namespace {
enum class GroupBy { Symbol, AssetClass, Sector };

QString group(GroupBy groupBy, pv::DataFile& dataFile, pv::i64 security) {
  switch (groupBy) {
  case GroupBy::AssetClass:
    return QString::fromStdString(pv::security::assetClass(dataFile, security));
  case GroupBy::Sector:
    return QString::fromStdString(pv::security::sector(dataFile, security));
  case GroupBy::Symbol:
    return QString::fromStdString(pv::security::symbol(dataFile, security));
  default:
    return QString();
  }
}
} // namespace

namespace pvui {
namespace reports {

AssetAllocationReport::AssetAllocationReport(DataFileManager& dataFileManager, QWidget* parent)
    : pvui::Report(tr("Asset Allocation"), dataFileManager, parent), plot(pvui::Report::createPlot()) {
  titleLabel()->hide();
  // Disable axes, we are drawing a pie chart not an xy chart
  plot->enableAxis(QwtAxis::XBottom, false);
  plot->enableAxis(QwtAxis::YLeft, false);
  pie.attach(plot);
  pie.setLegendIconSize(QSize(16, 16));
  plot->setTitle(tr("Asset Allocation"));

  // Setup groupby
  QHBoxLayout* groupByLayout = new QHBoxLayout;
  QLabel* label = new QLabel(tr("&Group By:"));
  groupBy = new QComboBox;
  label->setBuddy(groupBy);
  layout()->addLayout(groupByLayout);
  groupByLayout->addStretch();
  groupByLayout->addWidget(label);
  groupByLayout->addWidget(groupBy);
  groupBy->addItem(tr("Asset Class"), static_cast<int>(GroupBy::AssetClass));
  groupBy->addItem(tr("Sector"), static_cast<int>(GroupBy::Sector));
  groupBy->addItem(tr("Security"), static_cast<int>(GroupBy::Symbol));
  QObject::connect(groupBy, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
    settings.setValue("ReportsGroupBy", groupBy->currentData());
    if (this->dataFileManager.has()) {
      reload();
    }
  });
  groupBy->setCurrentIndex(groupBy->findData(settings.value("ReportsGroupBy", static_cast<int>(GroupBy::Symbol))));
  layout()->addWidget(plot);
}

void AssetAllocationReport::reload() {
  auto* stmt = dataFileManager->cachedQuery("SELECT Id FROM Securities");
  QList<double> data;
  QList<QwtText> titles;
  QList<QColor> colors;
  std::map<QString, pv::i64> values;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    pv::i64 security = sqlite3_column_int64(stmt, 0);
    values[group(static_cast<GroupBy>(groupBy->currentData().toInt()), *dataFileManager, security)] +=
        pv::algorithms::marketValue(*dataFileManager, security, currentEpochDate()).value_or(0) / 100.;
  }

  int i = 0;
  for (const auto& pair : values) {
    data += pair.second;
    titles += QwtText(pair.first);
    colors += pvui::Report::plotPalette(i).color(QPalette::Button);
    ++i;
  }

  int cashBalance = 0;
  stmt = dataFileManager->cachedQuery("SELECT Id FROM Accounts");
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    cashBalance +=
        pv::algorithms::cashBalance(*dataFileManager, sqlite3_column_int64(stmt, 0), currentEpochDate()) / 100.;
  }
  titles += QwtText(tr("Cash Balance"));
  data += cashBalance;
  colors += pvui::Report::plotPalette(i).color(QPalette::Button);

  pie.setSamples(std::move(data));
  pie.setPieTitles(std::move(titles));
  pie.setPieColors(std::move(colors));

  plot->insertLegend(new QwtLegend);
  plot->replot();
}
} // namespace reports

} // namespace pvui

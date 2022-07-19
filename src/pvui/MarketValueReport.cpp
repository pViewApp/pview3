#include "MarketValueReport.h"
#include "DateUtils.h"
#include "pv/Algorithms.h"
#include "pv/Integer64.h"
#include "GroupBy.h"
#include "pv/Security.h"
#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QLabel>
#include <QList>
#include <QLocale>
#include <QwtAxisId>
#include <QwtColumnSymbol>
#include <QwtDateScaleDraw>
#include <QwtLegend>
#include <QwtPlotCanvas>
#include <QwtPlotLayout>
#include <QwtScaleWidget>
#include <QwtSetSample>
#include <QwtSymbol>
#include <QwtText>
#include <algorithm>
#include <cassert>
#include <sqlite3.h>
#include <vector>

namespace pvui {
namespace reports {

MarketValueReport::MarketValueReport(QString name, DataFileManager& dataFileManager, QWidget* parent)
    : Report(name, dataFileManager, parent), div(new QwtScaleDiv) {
  titleLabel()->hide();
  layout()->addLayout(groupBySelectorLayout);
  layout()->addWidget(plot);

  costBasisCurve.attach(plot);
  marketValueCurve.attach(plot);
  chart.attach(plot);
  grid.attach(plot);

  // Setup grid
  grid.setPen(palette().color(QPalette::Button));
  // Setup cost basis curve
  costBasisCurve.setStyle(QwtPlotCurve::CurveStyle::Lines);
  auto* costBasisSymbol = new QwtSymbol(QwtSymbol::Style::Ellipse);
  costBasisSymbol->setSize(QSize(12, 12));
  costBasisCurve.setSymbol(costBasisSymbol);
  costBasisCurve.setLegendAttributes(QwtPlotCurve::LegendShowLine | QwtPlotCurve::LegendShowSymbol);
  costBasisCurve.setLegendIconSize(QSize(16, 16));
  costBasisCurve.setTitle(tr("Cost Basis"));
  costBasisCurve.setPen(QPen(Qt::PenStyle::SolidLine));

  auto* marketValueSymbol = new QwtSymbol(QwtSymbol::Style::Diamond); // use pView brand color
  marketValueSymbol->setColor(QColor(0x33, 0xaa, 0x00));
  marketValueSymbol->setSize(QSize(12, 12));
  marketValueCurve.setSymbol(marketValueSymbol);
  marketValueCurve.setLegendAttributes(QwtPlotCurve::LegendShowLine | QwtPlotCurve::LegendShowSymbol);
  marketValueCurve.setLegendIconSize(QSize(16, 16));
  marketValueCurve.setTitle(tr("Market Value"));
  marketValueCurve.setPen(QPen(Qt::PenStyle::SolidLine));

  chart.setStyle(QwtPlotMultiBarChart::Stacked);
  chart.setLegendIconSize(QSize(16, 16));

  auto* scaleDraw = new QwtDateScaleDraw;
  plot->setAxisScaleDraw(QwtAxis::XBottom, scaleDraw);
  scaleDraw->setLabelRotation(90);
  scaleDraw->setSpacing(10);
  scaleDraw->setLabelAlignment(Qt::AlignRight);

  plot->setAxisTitle(QwtAxis::XBottom, tr("Date"));
  plot->setAxisTitle(QwtAxis::YLeft, tr("Market Value ($)"));
  plot->setTitle(this->name());

  QObject::connect(this, &MarketValueReport::nameChanged, this, [&](QString newName) { plot->setTitle(newName); });

  setupGroupBySelection();
}

void MarketValueReport::setupGroupBySelection() {
  groupBySelectorLayout->addStretch(1);
  auto* groupByLabel = new QLabel(tr("&Group By:"));
  groupByLabel->setBuddy(groupBySelector);
  groupBySelectorLayout->addWidget(groupByLabel);
  groupBySelectorLayout->addWidget(groupBySelector);
  groupBySelector->setEditable(false);
  groupBySelector->addItem(tr("Asset Class"), static_cast<int>(GroupBy::AssetClass));
  groupBySelector->addItem(tr("Sector"), static_cast<int>(GroupBy::Sector));
  groupBySelector->addItem(tr("Security"), static_cast<int>(GroupBy::Symbol));

  QObject::connect(groupBySelector, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
                     pvui::setGroupBy(static_cast<GroupBy>(groupBySelector->currentData().toInt()));
                     if (dataFileManager.has()) {
                       reload();
                     }
                   });
}

void MarketValueReport::drawPlot() noexcept {
  assert(dataFileManager.has());
  // todo improve efficiency of this function
  std::map<QString, QMap<QDate, double>> values;
  //        ^ group       ^ date   ^ market value
  // Populate cashBalances and values

  { // Begin new scope because we declare variables here that are not needed later
    QVector<double> costBasisXData;
    QVector<double> costBasisYData;

    QVector<double> marketValueXData;
    QVector<double> marketValueYData;

    auto* securityListStmt = dataFileManager->cachedQuery("SELECT Id FROM Securities");
    std::vector<pv::i64> securities;

    while (sqlite3_step(securityListStmt) == SQLITE_ROW) {
      securities.push_back(sqlite3_column_int64(securityListStmt, 0));
    }

    for (QDate date = start(), endDate = end(); date <= endDate; date = date.addDays(interval)) {
      pv::i64 costBasis = 0;
      pv::i64 marketValue = 0;
      pv::i64 epochDay = toEpochDate(date);
      for (const auto security : securities) {
        QString group = pvui::group(*dataFileManager, security, currentGroupBy());

        pv::i64 marketValueForSecurity = pv::algorithms::marketValue(*dataFileManager, security, epochDay).value_or(0);
        marketValue += marketValueForSecurity;

        auto iter = values[group].find(date); // Automatically create values[sector] if needed
        if (iter == values[group].end()) {
          values[group][date] = marketValueForSecurity;
        } else {
          // Add to existing value
          iter.value() += marketValueForSecurity;
        }

        costBasis += pv::algorithms::costBasis(*dataFileManager, security, epochDay);
      }

      double qwtDate = QwtDate::toDouble(QDateTime(date, QTime(0, 0, 0)));

      costBasisXData += qwtDate;
      costBasisYData += costBasis / 100.;

      marketValueXData += qwtDate;
      marketValueYData += marketValue / 100.;
    }

    costBasisCurve.setSamples(costBasisXData, costBasisYData);
    marketValueCurve.setSamples(marketValueXData, marketValueYData);
  }

  QList<QwtText> titles;
  QVector<QwtSetSample> samples;

  for (const auto& pair : values) {
    titles += QwtText(pair.first);
  }

  titles += QwtText(tr("Cash Balance"));

  for (auto date = start(); date <= end(); date = date.addDays(interval)) {
    auto pvDate = toEpochDate(date); 
    QVector<double> samplesForDate;
    samplesForDate.reserve(titles.size() + 1);
    for (const auto& pair : values) {
      assert(pair.second.contains(date));
      samplesForDate += pair.second.value(date) / 100.; 
    }

    pv::i64 cashBalance = 0;

    auto accountQuery = dataFileManager->query("SELECT Id FROM Accounts");
    while (sqlite3_step(accountQuery.get()) == SQLITE_ROW) {
      cashBalance += pv::algorithms::cashBalance(*dataFileManager, sqlite3_column_int64(accountQuery.get(), 0), pvDate);
    }

    samplesForDate += cashBalance / 100.;

    samples += QwtSetSample(QwtDate::toDouble(QDateTime(date, QTime(0, 0, 0))), samplesForDate);
  }

  for (std::size_t i = 0; i < values.size() + 1 + 1;
       i++) { // values.size + 1 because there is also the cash balance series

    auto* symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    symbol->setFrameStyle(QwtColumnSymbol::NoFrame);
    symbol->setPalette(Report::plotPalette(i));
    chart.setSymbol(static_cast<int>(i), symbol);
  }
  chart.setBarTitles(titles);
  chart.setSamples(samples);
}

QwtScaleDiv MarketValueReport::createScaleDiv() const noexcept {
  constexpr int scaleLeftPadding = 3;  // Amount of padding (in days) to add to the beginning of the axis
  constexpr int scaleRightPadding = 3; // Amount of padding (in days) to add to the beginning of the axis
  auto scaleDiv = QwtScaleDiv(QwtDate::toDouble(QDateTime(start().addDays(-scaleLeftPadding), QTime(0, 0, 0))),
                              QwtDate::toDouble(QDateTime(end().addDays(scaleRightPadding), QTime(0, 0, 0))));

  QList<double> ticks;
  for (std::size_t i = 0; i < chart.dataSize(); ++i) {
    ticks += chart.data()->sample(i).value;
  }

  scaleDiv.setTicks(QwtScaleDiv::MajorTick, ticks);
  return scaleDiv;
}
void MarketValueReport::reload() noexcept {
  if (currentGroupBy() != static_cast<GroupBy>(groupBySelector->currentData().toInt())) {
    groupBySelector->setCurrentIndex(groupBySelector->findData(static_cast<int>(pvui::currentGroupBy())));
  }
  drawPlot();

  plot->setAxisScaleDiv(QwtAxis::XBottom, createScaleDiv());

  // Workaround, ensure that the bar size is large enough for single-sample charts
  if (chart.data()->size() == 1) {
    chart.setLayoutHint(200);
  } else {
    chart.setLayoutHint(0);
  }

  plot->insertLegend(new QwtLegend);
  plot->replot();
}

} // namespace reports
} // namespace pvui

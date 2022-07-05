#include "MarketValueReport.h"
#include "pv/Algorithms.h"
#include "DateUtils.h"
#include <sqlite3.h>
#include "pv/Integer64.h"
#include "pv/Security.h"
#include <QDate>
#include <QDateTime>
#include <QLabel>
#include <QList>
#include <QLocale>
#include <QwtAxisId>
#include <QwtColumnSymbol>
#include <cassert>
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
#include <vector>

namespace pvui {
namespace reports {

MarketValueReport::MarketValueReport(QString name, DataFileManager& dataFileManager, QWidget* parent)
    : Report(name, dataFileManager, parent), div(new QwtScaleDiv) {
  titleLabel()->hide();
  layout()->addLayout(groupBySelectorLayout);
  layout()->addWidget(plot);

  costBasisCurve.attach(plot);
  chart.attach(plot);
  grid.attach(plot);

  // Setup grid
  grid.setPen(palette().color(QPalette::Button));
  // Setup cost basis curve
  costBasisCurve.setStyle(QwtPlotCurve::CurveStyle::Lines);
  auto* symbol = new QwtSymbol(QwtSymbol::Style::Ellipse);
  symbol->setSize(QSize(12, 12));
  costBasisCurve.setSymbol(symbol);
  costBasisCurve.setLegendAttributes(QwtPlotCurve::LegendShowLine | QwtPlotCurve::LegendShowSymbol);
  costBasisCurve.setLegendIconSize(QSize(16, 16));
  costBasisCurve.setTitle(tr("Cost Basis"));
  costBasisCurve.setPen(QPen(Qt::PenStyle::SolidLine));

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
  groupBySelector->addItems({tr("Security"), tr("Asset Class"), tr("Sector")});
  groupBySelector->setEditable(false);
  groupBySelector->setCurrentText(tr("Security"));

  QObject::connect(groupBySelector, &QComboBox::currentTextChanged, this, [this]() { reload(); });
}

void MarketValueReport::drawPlot(std::function<QString(const pv::i64)> grouper) noexcept {
  assert(dataFileManager.has());
  // todo improve efficiency of this function
  std::map<QString, QMap<QDate, double>> values;
  //        ^ group       ^ date   ^ market value
  // Populate cashBalances and values

  { // Begin new scope because we declare variables here that are not needed later
    QVector<double> costBasisXData;
    QVector<double> costBasisYData;

    auto securityListStmt = dataFileManager->query("SELECT Id FROM Securities");
    std::vector<pv::i64> securities;

    while (sqlite3_step(securityListStmt.get()) == SQLITE_ROW) {
      securities.push_back(sqlite3_column_int64(securityListStmt.get(), 0));
    }

    for (QDate date = start(), endDate = end(); date <= endDate; date = date.addDays(interval)) {
      pv::i64 costBasis = 0;
      pv::i64 epochDay = toEpochDate(date);
      for (const auto security : securities) {
        QString group = grouper(security);

        auto iter = values[group].find(date); // Automatically create values[sector] if needed
        if (iter == values[group].end()) {
          values[group][date] = pv::algorithms::marketValue(*dataFileManager, security, epochDay).value_or(0);
        } else {
          // Add to existing value
          iter.value() += pv::algorithms::marketValue(*dataFileManager, security, epochDay).value_or(0);
        }

        costBasis += pv::algorithms::costBasis(*dataFileManager, security, epochDay);
      }

      costBasisXData += QwtDate::toDouble(QDateTime(date, QTime(0, 0, 0)));
      costBasisYData += costBasis / 100.;
    }

    costBasisCurve.setSamples(costBasisXData, costBasisYData);
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
  if (groupBySelector->currentText() == tr("Security")) {
    drawPlot([this](pv::i64 security) { return QString::fromStdString(pv::security::symbol(*dataFileManager, security)); });
  } else if (groupBySelector->currentText() == tr("Asset Class")) {
    drawPlot([this](pv::i64 security) { return QString::fromStdString(pv::security::assetClass(*dataFileManager, security)); });
  } else if (groupBySelector->currentText() == tr("Sector")) {
    drawPlot([this](pv::i64 security) { return QString::fromStdString(pv::security::sector(*dataFileManager, security)); });
  }

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

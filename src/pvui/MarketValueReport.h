#ifndef PVUI_REPORTS_MARKETVALUEREPORT_H
#define PVUI_REPORTS_MARKETVALUEREPORT_H

#include "Report.h"
#include <QComboBox>
#include <QDate>
#include <QHBoxLayout>
#include <QObject>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotGrid>
#include <QwtPlotMultiBarChart>
#include <QwtScaleDiv>
#include <functional>
#include <memory>

namespace pvui {
namespace reports {

class MarketValueReport : public Report {
  Q_OBJECT
private:
  void setupGroupBySelection();

  QHBoxLayout* groupBySelectorLayout = new QHBoxLayout;

  QwtPlot* const plot = createPlot();
  QwtPlotMultiBarChart chart;
  QwtPlotCurve costBasisCurve;
  QwtPlotGrid grid;

  QComboBox* const groupBySelector = new QComboBox;

  QwtScaleDiv* div;

  void drawPlot(std::function<QString(pv::i64)> grouper) noexcept;

  QwtScaleDiv createScaleDiv() const noexcept;

public:
  using DateSupplier = std::function<QDate()>;
  using IntervalSupplier = std::function<int /* days */ ()>;
  MarketValueReport(QString name, DataFileManager& dataFileManager, QWidget* parent = nullptr);

  DateSupplier start = []() { return QDate::currentDate(); };
  DateSupplier end = []() { return QDate::currentDate(); };
  unsigned int interval = 1;

  void reload() noexcept override;
};

} // namespace reports
} // namespace pvui

#endif // PVUI_REPORTS_MARKETVALUEREPORT_H

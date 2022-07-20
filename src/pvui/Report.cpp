#include "Report.h"
#include <Qt>
#include <QColor>
#include <QwtPlotCanvas>
#include <QwtPlotLayout>
#include <array>
#include <QwtScaleWidget>

QwtPlot* pvui::Report::createPlot(QWidget* parent) noexcept {
  QwtPlot* plot = new QwtPlot(parent);

  // Make sure plot doesn't take too much space
  plot->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));

  // Disable old-fashioned frame
  QwtPlotCanvas* canvas = new QwtPlotCanvas;
  canvas->setLineWidth(0); // Disable the border, we're going to use the axes as borders instead
  plot->setCanvas(canvas);
  plot->plotLayout()->setAlignCanvasToScales(true); // Make the axes take up the entire width/height of the chart
  for (int axis = 0; axis < QwtPlot::axisCnt; ++axis) {
    // For each axis, make it's margin 0
    plot->axisWidget(axis)->setMargin(0);
  }

  // Set canvas background (needs to be done after previous step, since that changes the canvas)
  plot->canvas()->setBackgroundRole(QPalette::Base); // Use a different background for the chart

  // Unset the cross-shaped cursor
  plot->canvas()->setCursor(QCursor(Qt::ArrowCursor));

  return plot;
}

const QColor pvui::Report::plotColor(std::size_t index) noexcept {
  using namespace QColorConstants::Svg;
  constexpr std::array colors = {
    red, dodgerblue, gold, limegreen, darkorange, darkviolet, maroon, darkturquoise,
    salmon, olivedrab, orangered, steelblue, khaki, darkslateblue, palevioletred, lightseagreen
  };
  return colors.at(index % colors.size());
}

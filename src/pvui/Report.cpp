#include "Report.h"
#include <QPalette>
#include <QwtPlotCanvas>
#include <QwtPlotLayout>
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

const QList<QPalette>& pvui::Report::plotPalettes() noexcept {
  static QList<QPalette> list = {
      QColor(0x33, 0x91, 0x0b), QColor(0x00, 0x60, 0xc6), QColor(0x91, 0x0b, 0x33), QColor(0x69, 0x0b, 0x91),
      QColor(0xc6, 0x66, 0x00), QColor(0xfc, 0x00, 0x00), QColor(0x6e, 0x9b, 0x1a), QColor(0x00, 0xcc, 0xc1),
  };
  return list;
}

const QPalette& pvui::Report::plotPalette(size_t index) noexcept {
  return Report::plotPalettes().at(index % Report::plotPalettes().size());
}

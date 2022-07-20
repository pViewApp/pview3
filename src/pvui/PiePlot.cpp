#include "PiePlot.h"
#include <QPainter>
#include <QSizeF>
#include <Qt>
#include <QwtGraphic>
#include <QwtLegend>
#include <QwtPlot>
#include <QwtText>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace pvui {

namespace {
constexpr auto defaultFillColor = Qt::gray;
} // namespace

PiePlot::PiePlot() {
  setItemAttribute(QwtPlotItem::Legend, true);
  setRenderHint(QwtPlotItem::RenderAntialiased, true);
}

void PiePlot::draw(QPainter* painter, const QwtScaleMap&, const QwtScaleMap&, const QRectF& canvasRect) const {
  constexpr int margin = 16;

  QRectF pieRect =
      (canvasRect.width() > canvasRect.height())
          ? QRectF((canvasRect.left() + margin * 2 + canvasRect.width() - canvasRect.height()) / 2,
                  canvasRect.top() + margin, canvasRect.height() - margin * 2, canvasRect.height() - margin * 2)
          : QRectF(canvasRect.left() + margin,
                   (canvasRect.top() + margin * 2 + canvasRect.height() - canvasRect.width()) / 2,
                   canvasRect.width() - margin * 2,
                   canvasRect.width() - margin * 2); // Create the largest possible centered square with margin
  double sum = 0;
  for (double sample : samples_) {
    assert(sample >= 0 && "Pie sample must be >= 0!");
    sum += sample;
  }

  constexpr int fullCircle = 360 * 16;
  int angleDrawnSoFar = 0;
  for (int i = 0; i < samples_.size(); ++i) {
    painter->setBrush(pieColors_.size() > i ? pieColors_.at(i) : defaultFillColor);
    int span = std::ceil(fullCircle * (samples_.at(i) / sum)); // ceil to avoid gaps in the pie
    painter->drawPie(pieRect, angleDrawnSoFar, -span);         // we negate s
    angleDrawnSoFar -= span;
  }
}

void PiePlot::setSamples(QList<double> samples) {
  samples_ = samples;
  itemChanged();
}
void PiePlot::setPieColors(QList<QColor> pieColors) {
  pieColors_ = pieColors;
  itemChanged();
}
void PiePlot::setPieTitles(QList<QwtText> pieTitles) {
  pieTitles_ = pieTitles;
  itemChanged();
}

QList<QwtLegendData> PiePlot::legendData() const {
  QList<QwtLegendData> legend;
  for (int i = 0; i < pieTitles_.size(); ++i) {
    QwtLegendData data;
    data.setValue(QwtLegendData::TitleRole, QVariant::fromValue(pieTitles_.at(i)));
    data.setValue(QwtLegendData::IconRole, QVariant::fromValue(legendIcon(i, legendIconSize())));
    legend += data;
  }
  return legend;
}

QwtGraphic PiePlot::legendIcon(int index, const QSizeF& size) const {
  QwtGraphic graphic;
  QPainter painter(&graphic);
  graphic.setDefaultSize(size);
  painter.fillRect(0, 0, size.width(), size.height(),
                   pieColors_.size() > index ? pieColors_.at(index) : defaultFillColor);
  return graphic;
}

} // namespace pvui

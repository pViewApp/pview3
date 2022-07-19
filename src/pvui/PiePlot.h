#ifndef PVUI_PIE_PLOT_H
#define PVUI_PIE_PLOT_H

#include <QColor>
#include <QList>
#include <QwtGraphic>
#include <QwtLegendData>
#include <QwtPlotItem>
#include <QwtText>

namespace pvui {

class PiePlot : public QwtPlotItem {
private:
  QList<double> samples_;
  QList<QColor> pieColors_;
  QList<QwtText> pieTitles_;

public:
  static constexpr int rttiValue = QwtPlotItem::Rtti_PlotUserItem + 0x2008;
  PiePlot();

  void draw(QPainter* painter, const QwtScaleMap&, const QwtScaleMap&, const QRectF& canvasRect) const override;

  int rtti() const override { return rttiValue; }

  void setSamples(QList<double> samples);
  void setPieColors(QList<QColor> pieColors);
  void setPieTitles(QList<QwtText> pieTitles);

  QList<QwtLegendData> legendData() const override;

  QwtGraphic legendIcon(int index, const QSizeF& size) const override;
};

} // namespace pvui

#endif // PVUI_PIE_PLOT_H

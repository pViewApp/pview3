#ifndef PVUI_ASSETALLOCATIONREPORT_H
#define PVUI_ASSETALLOCATIONREPORT_H

#include "DataFileManager.h"
#include "PiePlot.h"
#include "Report.h"
#include <QSettings>
#include <QwtPlot>

class QComboBox;

namespace pvui {
namespace reports {

class AssetAllocationReport : public pvui::Report {
  Q_OBJECT
private:
  QSettings settings;
  QComboBox* groupBy;
  QwtPlot* plot;
  PiePlot pie;

public:
  AssetAllocationReport(DataFileManager& dataFileManager, QWidget* parent = nullptr);
  void reload() override;
};

} // namespace reports
} // namespace pvui

#endif // PVUI_ASSETALLOCATIONREPORT_H

#ifndef PVUI_REPORT_H
#define PVUI_REPORT_H

#include "DataFileManager.h"
#include <QWidget>

namespace pvui {

class Report : public QWidget {
  Q_OBJECT
private:
  const DataFileManager& dataFileManager;

protected:
  const pv::DataFile& dataFile() const noexcept { return *dataFileManager; }

public:
  Report(const DataFileManger& dataFileManager, QWidget* parent = nullptr)
      : QWidget(parent), dataFileManager(dataFileManager) {}

  virtual QString name() const noexcept = 0;
};

} // namespace pvui

#endif // PVUI_REPORT_H

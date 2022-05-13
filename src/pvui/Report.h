#ifndef PVUI_REPORT_H
#define PVUI_REPORT_H

#include "DataFileManager.h"
#include "Page.h"

namespace pvui {

class Report : public PageWidget {
  Q_OBJECT
private:
  const DataFileManager& dataFileManager;

  QString name_;

protected:
  const pv::DataFile& dataFile() const noexcept { return *dataFileManager; }

public:
  Report(QString name, const DataFileManager& dataFileManager, QWidget* parent = nullptr)
      : PageWidget(parent), dataFileManager(dataFileManager), name_(name) {
    setTitle(name);
  }

  QString name() const noexcept { return name_; }

  virtual void reload() noexcept {}
};

} // namespace pvui

#endif // PVUI_REPORT_H

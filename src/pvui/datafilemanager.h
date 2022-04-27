#ifndef PVUI_DATAFILEMANAGER_H
#define PVUI_DATAFILEMANAGER_H

#include "pv/DataFile.h"
#include <QObject>

namespace pvui {
class DataFileManager : public QObject {
  Q_OBJECT
private:
  pv::DataFile dataFile_;

public:
  DataFileManager() = default;

  pv::DataFile& dataFile() noexcept { return dataFile_; }

  const pv::DataFile& constDataFile() const noexcept { return dataFile_; }

  const pv::DataFile& operator*() const noexcept { return dataFile_; }

  pv::DataFile& operator*() noexcept { return dataFile_; }

  const pv::DataFile* operator->() const noexcept { return &dataFile_; }

  pv::DataFile* operator->() noexcept { return &dataFile_; }
signals:
  void dataFileChanged(pv::DataFile& newDataFile);
};
} // namespace pvui

#endif // PVUI_DATA_FILE_MANAGER_H

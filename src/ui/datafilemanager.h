#ifndef PVUI_DATAFILEMANAGER_H
#define PVUI_DATAFILEMANAGER_H

#include "DataFile.h"
#include <QObject>

namespace pvui {
class DataFileManager : public QObject {
  Q_OBJECT
private:
  pv::DataFile m_dataFile;

public:
  inline DataFileManager() = default;

  inline pv::DataFile& dataFile() noexcept { return m_dataFile; }

  inline const pv::DataFile& constDataFile() const noexcept { return m_dataFile; }
signals:
  void dataFileChanged(pv::DataFile& newDataFile);
};
} // namespace pvui

#endif // PVUI_DATA_FILE_MANAGER_H

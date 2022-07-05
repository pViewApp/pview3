#ifndef PVUI_DATA_FILE_MANAGER_H
#define PVUI_DATA_FILE_MANAGER_H

#include "pv/DataFile.h"
#include "pv/Signals.h"
#include <QObject>
#include <filesystem>
#include <utility>
#include <string>
#include <optional>

namespace pvui {
class DataFileManager : public QObject {
  Q_OBJECT
private:
  std::optional<pv::DataFile> dataFile_ = std::nullopt;
public:
  explicit DataFileManager(std::optional<pv::DataFile> file = std::nullopt);

  DataFileManager& operator=(pv::DataFile&& dataFile) noexcept {
    setDataFile(std::move(dataFile));
    return *this;
  }

  DataFileManager& operator=(std::optional<pv::DataFile> dataFile) noexcept {
    setDataFile(std::move(dataFile));
    return *this;
  } 

  pv::DataFile& get() noexcept { return *dataFile_; }

  const pv::DataFile& cget() const noexcept { return *dataFile_; }

  pv::DataFile& operator*() noexcept { return *dataFile_; }

  const pv::DataFile& operator*() const noexcept { return *dataFile_; }

  const pv::DataFile* operator->() const noexcept { return &*dataFile_; }

  pv::DataFile* operator->() noexcept { return &*dataFile_; }

  operator bool() const noexcept { return dataFile_.has_value(); }
  
  bool has() { return dataFile_.has_value(); }

  void setDataFile(std::optional<pv::DataFile> dataFile) noexcept;
signals:
  void dataFileChanged();
};
} // namespace pvui

#endif // PVUI_DATA_FILE_MANAGER_H

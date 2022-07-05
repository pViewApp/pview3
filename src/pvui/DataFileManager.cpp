#include "DataFileManager.h"
#include "pv/DataFile.h"
#include <utility>
#include <filesystem>
#include <QObject>

namespace pvui {

DataFileManager::DataFileManager(std::optional<pv::DataFile> dataFile) : dataFile_(std::move(dataFile)) {}

void DataFileManager::setDataFile(std::optional<pv::DataFile> dataFile) noexcept {
  dataFile_ = std::move(dataFile);
  emit dataFileChanged();
}

}

#pragma once
#ifndef UI_DATAFILEMANAGER_H
#define UI_DATAFILE_MANAGER_H

#include "datafile.h"

namespace pvui {
	using pv::DataFile;
	using pv::Signal;

	class DataFileManager {
	private:
		DataFile m_dataFile;
	public:
		inline DataFileManager() = default;

		inline DataFile& dataFile() noexcept {
			return m_dataFile;
		}

		inline const DataFile& constDataFile() const noexcept {
			return m_dataFile;
		}
	signals:
		void dataFileChanged(DataFile& newDataFile);
	};
}

#endif // UI_DATAFILE_MANAGER_H

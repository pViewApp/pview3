#ifndef UI_DATA_FILE_MANAGER_H
#define UI_DATA_FILE_MANAGER_H

#include <QObject>
#include "DataFile.h"

namespace pvui {
	class DataFileManager : public QObject {
		Q_OBJECT
	private:
		pv::DataFile m_dataFile;
	public:
		inline DataFileManager() = default;

		inline pv::DataFile& dataFile() noexcept {
			return m_dataFile;
		}

		inline const pv::DataFile& constDataFile() const noexcept {
			return m_dataFile;
		}
	signals:
		void dataFileChanged(pv::DataFile& newDataFile);
	};
}

#endif // UI_DATA_FILE_MANAGER_H

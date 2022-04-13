#ifndef PVUI_SECURITYMODEL_H
#define PVUI_SECURITYMODEL_H

#include <QAbstractItemModel>
#include "datafile.h"

namespace pvui::models {
	class SecurityModel : public QAbstractItemModel {
		Q_OBJECT
	private:
		pv::DataFile& dataFile_;
	public:
		inline SecurityModel(pv::DataFile& dataFile) : dataFile_(dataFile) {};

		inline int rowCount(const QModelIndex& parent = QModelIndex()) const override {
			if (parent.isValid()) return 0;
			return static_cast<int>(dataFile_.securities().size());
		}

		inline QModelIndex parent(const QModelIndex& index = QModelIndex()) const override {
			return QModelIndex();
		}

		inline int columnCount(const QModelIndex& index = QModelIndex()) const override {
			return 4; // Symbol, name, asset class, sector
		}

		inline QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
			return createIndex(row, column, row > dataFile_.securities().size() - 1 ? nullptr : dataFile_.securities().at(row - 1));
		}

		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	};
}
#endif // PVUI_SECURITYMODEL_H

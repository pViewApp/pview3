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
		 SecurityModel(pv::DataFile& dataFile);

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
			return createIndex(row, column, row > dataFile_.securities().size() - 1 ? nullptr : dataFile_.securities().at(row));
		}

		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	signals:
		void beforeSecurityAdded(const QModelIndex& index, int first, int last);
		void afterSecurityAdded();
	};
}
#endif // PVUI_SECURITYMODEL_H

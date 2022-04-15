#ifndef PVUI_TRANSACTION_MODEL_H
#define PVUI_TRANSACTION_MODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include "DataFile.h"

namespace pvui::models {
	class TransactionModel : public QAbstractItemModel {
		Q_OBJECT
	private:
		const pv::Account* account_;
	public:
		TransactionModel(const pv::Account* account, QObject* parent = nullptr);

		inline QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
			if (parent.isValid()) return QModelIndex(); // Only top-level parents are 
			return createIndex(row, column, row > rowCount() - 1 ? nullptr : account_->transactions().at(row)); // Use nullptr if this row is greater than the number of transactions
		}

		inline int rowCount(const QModelIndex& parent = QModelIndex()) const override {
			if (parent.isValid()) return 0; // Only top-level parents are allowed
			return static_cast<int>(account_->transactions().size());
		}

		inline int columnCount(const QModelIndex& parent = QModelIndex()) const override {
			if (parent.isValid()) return 0; // Only top-level parents are allowed
			return 7; // The columns are Date, Action, Security, Number of Shares, Share Price, Commission, Total Amount
		}

		inline QModelIndex parent(const QModelIndex& index) const override {
			return QModelIndex(); // There cannot be any parents since there will never be children
		}

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	signals:
		void beforeTransactionAdded(const QModelIndex& index, int first, int last);
		void afterTransactionAdded();
	};
}

#endif // PVUI_TRANSACTION_MODEL_H

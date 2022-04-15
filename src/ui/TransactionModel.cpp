#include <QDate>
#include "TransactionModel.h"

pvui::models::TransactionModel::TransactionModel(const pv::Account* account, QObject* parent) : QAbstractItemModel(parent),
	account_(account)
{
	// Listen for before transaction added

	QObject::connect(this, &TransactionModel::beforeTransactionAdded, this, &TransactionModel::beginInsertRows);

	auto beforeTransactionAddedSlot = [&]() {
		const auto index = QModelIndex();
		emit this->beforeTransactionAdded(index, rowCount(), rowCount());
	};

	auto beforeTransactionAddedConnection = account_->beforeTransactionAdded().connect(beforeTransactionAddedSlot);

	QObject::connect(this, &QObject::destroyed, this, [&]() {
		beforeTransactionAddedConnection.disconnect();
	});

	// Listen for after transaction added

	QObject::connect(this, &TransactionModel::afterTransactionAdded, this, &TransactionModel::endInsertRows);

	auto afterTransactionAddedSlot = [&](const pv::Transaction*) {
		emit this->afterTransactionAdded();
	};

	auto afterTransactionAddedConnection = account_->transactionAdded().connect(afterTransactionAddedSlot);

	QObject::connect(this, &QObject::destroyed, this, [&]() {
		afterTransactionAddedConnection.disconnect();
	});
}


QVariant pvui::models::TransactionModel::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DisplayRole || !index.isValid() || index.row() > rowCount() - 1) return QVariant();

	pv::Transaction& transaction = *static_cast<pv::Transaction*>(index.internalPointer());
	switch (index.column()) {
	case 0:
		date::year_month_day ymd{ transaction.date() };
		return QDate(static_cast<int>(ymd.year()), static_cast<unsigned int>(ymd.month()), static_cast<unsigned int>(ymd.day()));
	case 1:
		return QString::fromStdString(transaction.action().name());
	case 2:
		return transaction.security() == pv::Security::NONE ? "" : QString::fromStdString(transaction.security()->symbol());
	case 3:
		return QString::fromStdString(transaction.numberOfShares().str());
	case 4:
		return QString::fromStdString(transaction.sharePrice().str());
	case 5:
		return QString::fromStdString(transaction.commission().str());
	case 6:
		return QString::fromStdString(transaction.totalAmount().str());
	default:
		return QVariant();
	}
}

QVariant pvui::models::TransactionModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Orientation::Vertical || role != Qt::DisplayRole) return QVariant();
	switch (section) {
	case 0:
		return tr("Date");
	case 1:
		return tr("Action");
	case 2:
		return tr("Security");
	case 3:
		return tr("# Of Shares");
	case 4:
		return tr("Share Price");
	case 5:
		return tr("Commission");
	case 6:
		return tr("Total Amount");
	default:
		return QVariant();
	}
}

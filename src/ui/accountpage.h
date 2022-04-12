#ifndef UI_ACCOUNTPAGE_H
#define UI_ACCOUNTPAGE_H

#include <QWidget>
#include <QVariant>
#include <QTableView>
#include <QDate>
#include <QDateEdit>
#include <QComboBox>
#include <QSortFilterProxyModel>

#include "page.h"
#include "datafile.h"

namespace pvui {
	class TransactionItemModel : public QAbstractItemModel {
		Q_OBJECT
	private:
		const pv::Account* account_;
	public:
		TransactionItemModel(const pv::Account* account, QObject* parent = nullptr);

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

		inline QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
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

		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	signals:
		void beforeTransactionAdded(const QModelIndex& index, int start, int end);
		void afterTransactionAdded();
	};

	class TransactionInsertionWidget : public QWidget {
		Q_OBJECT
	private:
		pv::Account* account_;
		QDateEdit* dateEditor;
		QComboBox* actionEditor;
		QComboBox* securityEditor;
		QLineEdit* numberOfSharesEditor;
		QLineEdit* sharePriceEditor;
		QLineEdit* commissionEditor;
		QLineEdit* totalAmountEditor;
		QHBoxLayout* layout;
		void reset();
	public:
		TransactionInsertionWidget(pv::Account* account = nullptr, QWidget* parent = nullptr);
	public slots:
		void submit();
		void setAccount(pv::Account* account);
	};

	class AccountPageWidget : public PageWidget {
		Q_OBJECT
	private:
		pv::Account* account_;
		QTableView* table;
		TransactionInsertionWidget* insertWidget;
		QSortFilterProxyModel* model;

	public:
		AccountPageWidget(QWidget* parent = nullptr);
	public slots:
		inline void setAccount(pv::Account* account) {
			account_ = account;
			setTitle(QString::fromStdString(account->name()));
			model->setSourceModel(new TransactionItemModel(account_));
			insertWidget->setAccount(account);
		}
	};
}

#endif // UI_ACCOUNTPAGE_H

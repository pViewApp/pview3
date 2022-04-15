#ifndef PVUI_ACCOUNT_PAGE_H
#define PVUI_ACCOUNT_PAGE_H

#include <optional>
#include <boost/signals2.hpp>
#include <QWidget>
#include <QVariant>
#include <QTableView>
#include <QDate>
#include <QDateEdit>
#include <QComboBox>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include "Page.h"
#include "DataFile.h"
#include "TransactionModel.h"

namespace pvui {
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

		std::optional<boost::signals2::connection> dataFileSecurityConnection = std::nullopt; // Connection to the current DataFile's securityAdded() signal
		void reset();
	protected:
		inline void showEvent(QShowEvent* showEvent) override {
			if (!showEvent->spontaneous()) {
				dateEditor->setFocus();
			}
		}
	public:
		TransactionInsertionWidget(pv::Account* account = nullptr, QWidget* parent = nullptr);
	protected slots:
		void setupSecurityList();
	public slots:
		void submit();
		void setAccount(pv::Account* account);
	};

	class AccountPageWidget : public PageWidget {
		Q_OBJECT
	private:
		pv::Account* account_ = nullptr;
		QTableView* table;
		TransactionInsertionWidget* insertWidget;
		QSortFilterProxyModel* model;

	public:
		AccountPageWidget(QWidget* parent = nullptr);
	public slots:
		inline void setAccount(pv::Account* account) {
			account_ = account;
			setTitle(QString::fromStdString(account->name()));
			model->setSourceModel(new models::TransactionModel(account_));
			insertWidget->setAccount(account);
		}
	};
}

#endif // PVUI_ACCOUNT_PAGE_H

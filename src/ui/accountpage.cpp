#include <date/date.h>
#include <fmt/format.h>
#include <algorithm>
#include <QTreeView>
#include <QDate>
#include <QHeaderView>
#include <QLineEdit>
#include "util.h"
#include "accountpage.h"
#include "actions.h"

void pvui::TransactionInsertionWidget::reset()
{
	dateEditor->setDate(QDate::currentDate());
	actionEditor->setCurrentIndex(-1);
	securityEditor->setCurrentIndex(-1);
	numberOfSharesEditor->setText("");
	sharePriceEditor->setText("");
	numberOfSharesEditor->setText("");
	commissionEditor->setText("");
	totalAmountEditor->setText("");
}

pvui::AccountPageWidget::AccountPageWidget(QWidget* parent) : PageWidget(parent),
	table(new QTableView),
	insertWidget(new TransactionInsertionWidget),
	model(new QSortFilterProxyModel)
{
	setTitle("No Account Open");

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(table, 1);
	layout->addWidget(insertWidget);
	setContent(layout);

	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table->verticalHeader()->hide();

	table->setSortingEnabled(true);
	table->sortByColumn(0, Qt::AscendingOrder);

	table->setSelectionBehavior(QTableView::SelectRows);

	table->setModel(model);
}

pvui::TransactionItemModel::TransactionItemModel(const pv::Account* account, QObject* parent) : QAbstractItemModel(parent),
	account_(account)
{
	// Listen for before transaction added

	QObject::connect(this, &TransactionItemModel::beforeTransactionAdded, this, &TransactionItemModel::beginInsertRows);

	auto beforeTransactionAddedSlot = [&]() {
		const auto index = QModelIndex();
		emit this->beforeTransactionAdded(index, rowCount(), rowCount());
	};

	auto beforeTransactionAddedConnection = account_->beforeTransactionAdded().connect(beforeTransactionAddedSlot);

	QObject::connect(this, &QObject::destroyed, this, [&]() {
		beforeTransactionAddedConnection.disconnect();
	});

	// Listen for after transaction added

	QObject::connect(this, &TransactionItemModel::afterTransactionAdded, this, &TransactionItemModel::endInsertRows);

	auto afterTransactionAddedSlot = [&](const pv::Transaction*) {
		emit this->afterTransactionAdded();
	};

	auto afterTransactionAddedConnection = account_->transactionAdded().connect(afterTransactionAddedSlot);

	QObject::connect(this, &QObject::destroyed, this, [&]() {
		afterTransactionAddedConnection.disconnect();
	});
}

QVariant pvui::TransactionItemModel::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DisplayRole || !index.isValid() || index.row() > rowCount() - 1) return QVariant();

	pv::Transaction& transaction = *static_cast<pv::Transaction*>(index.internalPointer());
	switch (index.column()) {
	case 0:
		date::year_month_day ymd{ transaction.date() };
		return QDate(static_cast<int>(ymd.year()), static_cast<unsigned int>(ymd.month()), static_cast<unsigned int>(ymd.day()));
	case 1:
		return QString::fromStdString(transaction.action().name());
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

pvui::TransactionInsertionWidget::TransactionInsertionWidget(pv::Account* account, QWidget* parent) : QWidget(parent),
	layout(new QHBoxLayout(this)),
	dateEditor(new QDateEdit(QDate::currentDate())),
	actionEditor(new QComboBox),
	securityEditor(new QComboBox),
	numberOfSharesEditor(new QLineEdit),
	sharePriceEditor(new QLineEdit),
	commissionEditor(new QLineEdit),
	totalAmountEditor(new QLineEdit)
{
	dateEditor->setCalendarPopup(true);
	actionEditor->setEditable(true);
	securityEditor->setEditable(true);

	QStringList actions{pv::ACTIONS.size()};
	std::transform(pv::ACTIONS.cbegin(), pv::ACTIONS.cend(), actions.begin(), [](pv::Action* action) {
		return tr(action->name().c_str());
	});

	actionEditor->addItems(actions);

	actionEditor->lineEdit()->setPlaceholderText(tr("Action"));
	securityEditor->lineEdit()->setPlaceholderText(tr("Security"));
	numberOfSharesEditor->setPlaceholderText(tr("# of Shares"));
	sharePriceEditor->setPlaceholderText(tr("Share Price"));
	commissionEditor->setPlaceholderText(tr("Commission"));
	totalAmountEditor->setPlaceholderText(tr("Total Amount"));

	layout->addWidget(dateEditor, 1);
	layout->addWidget(actionEditor, 1);
	layout->addWidget(securityEditor, 1);
	layout->addWidget(numberOfSharesEditor, 1);
	layout->addWidget(sharePriceEditor, 1);
	layout->addWidget(commissionEditor, 1);
	layout->addWidget(totalAmountEditor, 1);

	QObject::connect(actionEditor->lineEdit(), &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(securityEditor->lineEdit(), &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(numberOfSharesEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(sharePriceEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(commissionEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(totalAmountEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);

	setAccount(account);
}

void pvui::TransactionInsertionWidget::submit() {
	using namespace date;
	if (account_ != nullptr) {
		auto date = dateEditor->date();

		account_->addTransaction(
			local_days(year_month_day(year(date.year()), month(date.month()), day(date.daysInMonth()))),
			*pv::actionFromName(actionEditor->itemText(actionEditor->currentIndex()).toStdString()),
			pv::Security::NONE,
			pv::Decimal(numberOfSharesEditor->text().toStdString()),
			pv::Decimal(sharePriceEditor->text().toStdString()),
			pv::Decimal(commissionEditor->text().toStdString()),
			pv::Decimal(totalAmountEditor->text().toStdString())
		);
	}
}

void pvui::TransactionInsertionWidget::setAccount(pv::Account* account) {
	account_ = account;
	bool enabled = account_ != nullptr;

	reset();

	dateEditor->setEnabled(enabled);
	actionEditor->setEnabled(enabled);
	securityEditor->setEnabled(enabled);
	numberOfSharesEditor->setEnabled(enabled);
	sharePriceEditor->setEnabled(enabled);
	commissionEditor->setEnabled(enabled);
	totalAmountEditor->setEnabled(enabled);
}

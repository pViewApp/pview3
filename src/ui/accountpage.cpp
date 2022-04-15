#include <algorithm>
#include <string>
#include <QTreeView>
#include <QDate>
#include <QHeaderView>
#include <QLineEdit>
#include <date/date.h>
#include <fmt/format.h>
#include "AccountPage.h"
#include "Actions.h"
#include "SecurityPage.h"

const std::string defaultSecurityAssetClass = "Equities";
const std::string defaultSecuritySector = "Other";

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

	dateEditor->setFocus();
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

	static const QSizePolicy sizePolicy = { QSizePolicy::Ignored, QSizePolicy::Preferred };

	dateEditor->setSizePolicy(sizePolicy);
	actionEditor->setSizePolicy(sizePolicy);
	securityEditor->setSizePolicy(sizePolicy);
	numberOfSharesEditor->setSizePolicy(sizePolicy);
	sharePriceEditor->setSizePolicy(sizePolicy);
	commissionEditor->setSizePolicy(sizePolicy);
	totalAmountEditor->setSizePolicy(sizePolicy);

	pvui::SecuritySymbolValidator* securityValidator = new SecuritySymbolValidator;
	securityEditor->setValidator(securityValidator);
	securityValidator->setParent(securityEditor);

	QObject::connect(actionEditor->lineEdit(), &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(securityEditor->lineEdit(), &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(numberOfSharesEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(sharePriceEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(commissionEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);
	QObject::connect(totalAmountEditor, &QLineEdit::returnPressed, this, &TransactionInsertionWidget::submit);

	setAccount(account);
}

void pvui::TransactionInsertionWidget::setupSecurityList() {
	if (dataFileSecurityConnection.has_value()) {
		dataFileSecurityConnection->disconnect();
	}

	if (account_ == nullptr) {
		dataFileSecurityConnection = std::nullopt;
		securityEditor->clear();
		return;
	}

	dataFileSecurityConnection = account_->dataFile().securityAdded().connect([&](pv::Security* security) {
		QString text = securityEditor->currentText();
		securityEditor->addItem(QString::fromStdString(security->symbol()));
		securityEditor->model()->sort(0);
		securityEditor->setCurrentText(text);
	});

	securityEditor->clear();

	for (auto* security : account_->dataFile().securities()) {
		securityEditor->addItem(QString::fromStdString(security->symbol()));
	}

	securityEditor->model()->sort(0);
	
}

void pvui::TransactionInsertionWidget::submit() {
	using namespace date;

	std::string securitySymbol = securityEditor->currentText().trimmed().toStdString();

	pv::Security* security;
	
	if (securitySymbol.empty()) {
		security = pv::Security::NONE;
	}
	else {
		security = account_->dataFile().securityForSymbol(securitySymbol);

		if (security == nullptr) {
			security = account_->dataFile().addSecurity(
				securitySymbol,
				securitySymbol,
				defaultSecurityAssetClass,
				defaultSecuritySector
			);
		}
	}
	
	if (account_ != nullptr) {
		auto date = dateEditor->date();

		account_->addTransaction(
			local_days(year_month_day(year(date.year()), month(date.month()), day(date.daysInMonth()))),
			*pv::actionFromName(actionEditor->itemText(actionEditor->currentIndex()).toStdString()),
			security,
			pv::Decimal(numberOfSharesEditor->text().toStdString()),
			pv::Decimal(sharePriceEditor->text().toStdString()),
			pv::Decimal(commissionEditor->text().toStdString()),
			pv::Decimal(totalAmountEditor->text().toStdString())
		);

		reset();
	}
}

void pvui::TransactionInsertionWidget::setAccount(pv::Account* account) {
	if (account_ == account) return;
	account_ = account;
	bool enabled = account_ != nullptr;

	setupSecurityList();
	reset();

	dateEditor->setEnabled(enabled);
	actionEditor->setEnabled(enabled);
	securityEditor->setEnabled(enabled);
	numberOfSharesEditor->setEnabled(enabled);
	sharePriceEditor->setEnabled(enabled);
	commissionEditor->setEnabled(enabled);
	totalAmountEditor->setEnabled(enabled);
}

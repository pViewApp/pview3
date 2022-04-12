#include "datafile.h"

using pv::Action;
using pv::Transaction;
using pv::TransactionBase;
using pv::Account;

using pv::Date;
using pv::Decimal;

Transaction* pv::Account::addTransaction(Date date, const Action& action, const Security* security, pv::Decimal numberOfShares, Decimal sharePrice, Decimal commission, Decimal totalAmount) {
	TransactionBase base(date, security, numberOfShares, sharePrice, commission, totalAmount);
	action.processTransaction(base);

	signal_beforeTransactionAdded();

	unsigned int id = nextTransactionId.fetch_add(1);
	transactions_.try_emplace(id, *this, id, action, base);

	auto* transaction = transactionForId(id);

	validTransactions.push_back(transaction);

	signal_transactionAdded(transaction);

	return transaction;
}

Account* pv::DataFile::addAccount(string name)  {
	unsigned int id = nextAccountId.fetch_add(1);
	accounts_.try_emplace(id, *this, id, name);

	auto* account = accountForId(id);
	validAccounts.push_back(account);

	signal_accountAdded(account);
	signal_accountsChanged(accounts());

	return account;
}

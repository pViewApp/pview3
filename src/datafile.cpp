#include "datafile.h"

pv::Transaction* pv::Account::addTransaction(pv::Date date, const pv::Action& action, const pv::Security* security, pv::Decimal numberOfShares, pv::Decimal sharePrice, pv::Decimal commission, pv::Decimal totalAmount) {
	pv::TransactionBase base(date, security, numberOfShares, sharePrice, commission, totalAmount);
	action.processTransaction(base);

	signal_beforeTransactionAdded();

	unsigned int id = nextTransactionId.fetch_add(1);
	transactions_.try_emplace(id, *this, id, action, base);

	auto* transaction = transactionForId(id);

	validTransactions.push_back(transaction);

	signal_transactionAdded(transaction);

	return transaction;
}

pv::Account* pv::DataFile::addAccount(std::string name)  {
	unsigned int id = nextAccountId.fetch_add(1);
	accounts_.try_emplace(id, *this, id, name);
	auto* account = accountForId(id);
	validAccounts.push_back(account);

	signal_accountAdded(account);

	return account;
}

pv::Security* pv::DataFile::addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector)
{
	signal_beforeSecurityAdded();

	securities_.try_emplace(symbol, symbol, name, assetClass, sector);
	auto* security = &securities_.at(symbol);
	validSecurities_.push_back(security);

	signal_securityAdded(security);

	return security;
}

pv::Security::Security(std::string symbol, std::string name, std::string assetClass, std::string sector) : symbol_(symbol),
	name_(name),
	assetClass_(assetClass),
	sector_(sector)
{}

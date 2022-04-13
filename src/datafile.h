#ifndef PV_DATAFILE_H
#define PV_DATAFILE_H

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <utility>
#include <functional>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/signals2.hpp>


#include "types.h"

namespace pv {
	namespace util {
		class NonCopyable {
		public:
			NonCopyable() = default;
			NonCopyable(const NonCopyable&) = delete;
			NonCopyable(NonCopyable&&) = default;
			NonCopyable& operator=(NonCopyable&&) = default;
		};
	}

	class Account;
	class DataFile;
	class Action;
	
	class Security : util::NonCopyable {
	private:
		std::string symbol_;
		std::string name_;
		std::string assetClass_;
		std::string sector_;
	public:
		inline static Security* const NONE = nullptr;

		Security(std::string symbol, std::string name, std::string assetClass, std::string sector);

		Security(Security&&) = default;

		inline std::string symbol() {
			return symbol_;
		}

		inline std::string name() {
			return name_;
		}

		inline std::string assetClass() {
			return assetClass_;
		}

		inline std::string sector() {
			return sector_;
		}
	};

	// This class should only be used by Action implementations
	class TransactionBase {
	public:
		Date date;
		const Security* security;
		Decimal numberOfShares;
		Decimal sharePrice;
		Decimal commission;
		Decimal totalAmount;
	public:
		inline TransactionBase(pv::Date date, const Security* security, Decimal numberOfShares, Decimal sharePrice, Decimal commission, Decimal totalAmount)
			: date(date), security(security), numberOfShares(numberOfShares), sharePrice(sharePrice), commission(commission), totalAmount(totalAmount) {}

	};

	class Action {
	public:
		Action() = default;

		virtual void processTransaction(TransactionBase& in) const = 0;

		virtual std::string name() const noexcept = 0;
	};

	class Transaction : util::NonCopyable {
	private:
		Account& account_;
		unsigned int id_;
		Date date_;
		const Action& action_;
		const Security* security_;
		Decimal numberOfShares_;
		Decimal sharePrice_;
		Decimal commission_;
		Decimal totalAmount_;
	public:
		inline Transaction(Account& account, unsigned int id, const Action& action, TransactionBase base) : account_(account),
			id_(id),
			date_(base.date), action_(action),
			security_(base.security),
			numberOfShares_(base.numberOfShares),
			sharePrice_(base.sharePrice),
			commission_(base.commission),
			totalAmount_(base.totalAmount) {}

		Transaction(Transaction&&) = default;

		inline Account& account() const noexcept {
			return account_;
		}
		inline Date date() const noexcept {
			return date_;
		}

		inline const Action& action() const noexcept {
			return action_;
		}

		inline const Security* security() const noexcept {
			return security_;
		}

		inline Decimal numberOfShares() const noexcept {
			return numberOfShares_;
		}

		inline Decimal sharePrice() const noexcept {
			return sharePrice_;
		}

		inline Decimal commission() const noexcept {
			return commission_;
		}
		
		inline Decimal totalAmount() const noexcept {
			return totalAmount_;
		}
	};

	class Account : public util::NonCopyable {
	private:
		DataFile& dataFile_;
		std::string name_;
		unsigned int id_;

		std::atomic_uint nextTransactionId;
		std::vector<Transaction*> validTransactions;
		std::map<unsigned int, Transaction> transactions_;

		mutable Signal<void()> signal_beforeTransactionAdded;
		mutable Signal<void(const Transaction*)> signal_transactionAdded;
		mutable Signal<void(const std::string&, const std::string&)> signal_nameChanged;
	public:
		inline Account(DataFile& dataFile, unsigned int id, std::string name): id_(id), name_(name), dataFile_(dataFile) {

		}

		inline Transaction* transactionForId(unsigned int id) {
			return transactions_.find(id) == transactions_.cend() ? nullptr : &transactions_.at(id);
		}

		Account(Account&&) = default;

		inline DataFile& dataFile() const noexcept {
			return dataFile_;
		}

		inline std::string name() const noexcept {
			return name_;
		}

		inline void setName(std::string name) noexcept {
			std::string oldName = name_;
			name_ = name;
			signal_nameChanged(name, name_);
		}

		inline const std::vector<Transaction*>& transactions() const noexcept{
			return validTransactions;
		}

		inline Signal<void()>& beforeTransactionAdded() const noexcept {
			return signal_beforeTransactionAdded;
		}
		
		inline Signal<void(const Transaction*)>& transactionAdded() const noexcept {
			return signal_transactionAdded;
		}

		inline Signal<void(const std::string&, const std::string&)>& nameChanged() const noexcept {
			return signal_nameChanged;
		}

		Transaction* addTransaction(
			pv::Date date,
			const pv::Action& action,
			const Security* security,
			pv::Decimal numberOfShares,
			pv::Decimal sharePrice,
			pv::Decimal commission,
			pv::Decimal totalAmount
		);
	};
	class DataFile : util::NonCopyable {
	private:
		std::atomic_uint nextAccountId;

		std::map<unsigned int, Account> accounts_;
		std::map<std::string, Security> securities_;

		std::vector<Account*> validAccounts;
		std::vector <Security*> validSecurities_;

		mutable Signal<void(Account*)> signal_accountAdded;
		mutable Signal<void(Security*)> signal_securityAdded;
		mutable Signal<void()> signal_beforeSecurityAdded;
	public:
		DataFile() = default;

		inline const std::vector<Account*>& accounts() const noexcept {
			return validAccounts;
		}

		inline const std::vector<Security*>& securities() const noexcept {
			return validSecurities_;
		}

		inline Signal<void(Account*)>& accountAdded() const noexcept {
			return signal_accountAdded;
		}

		inline Signal<void(Security*)>& securityAdded() {
			return signal_securityAdded;
		}

		inline Signal<void()>& beforeSecurityAdded() {
			return signal_beforeSecurityAdded;
		}

		inline Account* accountForId(unsigned int id) {
			return accounts_.find(id) == accounts_.cend() ? nullptr : &accounts_.at(id);
		}

		inline Security* securityForSymbol(std::string symbol) {
			return securities_.find(symbol) == securities_.cend() ? nullptr : &securities_.at(symbol);
		}

		Account* addAccount(std::string name);
		Security* addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector);
	};
}

#endif // PV_DATAFILE_H

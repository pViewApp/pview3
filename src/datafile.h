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
	using std::string;
	using std::vector;


	class Account;
	class DataFile;
	class Action;
	class Security {
	public:
		inline static Security* const NONE = nullptr;
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
		inline TransactionBase(Date date, const Security* security, Decimal numberOfShares, Decimal sharePrice, Decimal commission, Decimal totalAmount)
			: date(date), security(security), numberOfShares(numberOfShares), sharePrice(sharePrice), commission(commission), totalAmount(totalAmount) {}

	};

	class Action {
	public:
		Action() = default;

		virtual void processTransaction(TransactionBase& in) const = 0;

		virtual string name() const noexcept = 0;
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
		inline Transaction(Account& account, unsigned int id, const Action& action, TransactionBase base)
			: account_(account), id_(id), action_(action),
			date_(base.date), security_(base.security), numberOfShares_(base.numberOfShares), sharePrice_(base.sharePrice), commission_(base.commission), totalAmount_(base.totalAmount) {}

		Transaction(Transaction&&) = default;
		Transaction& operator=(Transaction&&) = default;

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
		string name_;
		unsigned int id_;

		std::atomic_uint nextTransactionId;
		vector<Transaction*> validTransactions;
		std::map<unsigned int, Transaction> transactions_;

		mutable Signal<void()> signal_beforeTransactionAdded;
		mutable Signal<void(const Transaction*)> signal_transactionAdded;
		mutable Signal<void(const string&, const string&)> signal_nameChanged;
	public:
		inline Account(DataFile& dataFile, unsigned int id, string name): id_(id), name_(name), dataFile_(dataFile) {

		}

		inline Transaction* transactionForId(unsigned int id) {
			return &transactions_.at(id);
		}

		Account(Account&&) = default;
		Account& operator=(Account&&) = default;

		inline DataFile& dataFile() const noexcept {
			return dataFile_;
		}

		inline string name() const noexcept {
			return name_;
		}

		inline void setName(string name) noexcept {
			string oldName = name_;
			name_ = name;
			signal_nameChanged(name, name_);
		}

		inline const vector<Transaction*>& transactions() const noexcept{
			return validTransactions;
		}

		inline Signal<void()>& beforeTransactionAdded() const noexcept {
			return signal_beforeTransactionAdded;
		}
		
		inline Signal<void(const Transaction*)>& transactionAdded() const noexcept {
			return signal_transactionAdded;
		}

		inline Signal<void(const string&, const string&)>& nameChanged() const noexcept {
			return signal_nameChanged;
		}

		Transaction* addTransaction(
			Date date,
			const Action& action,
			const Security* security,
			Decimal numberOfShares,
			Decimal sharePrice,
			Decimal commission,
			Decimal totalAmount
		);
	};
	
	class DataFile : util::NonCopyable {
	private:
		std::atomic_uint nextAccountId;
		std::vector<Account*> validAccounts;
		std::map<unsigned int, Account> accounts_;
		mutable Signal<void(Account*)> signal_accountAdded;
		mutable Signal<void(const std::vector<Account*>&)> signal_accountsChanged;
	public:
		DataFile() = default;

		inline const vector <Account*>& accounts() const noexcept {
			return validAccounts;
		}

		inline Signal<void(Account*)>& accountAdded() const noexcept {
			return signal_accountAdded;
		}

		inline Account* accountForId(unsigned int id) {
			return &accounts_.at(id);
		}

		inline Signal<void(const std::vector<Account*>&)>& accountsChanged() const noexcept {
			return signal_accountsChanged;
		}

		Account* addAccount(string name);
	};
}

#endif // PV_DATAFILE_H

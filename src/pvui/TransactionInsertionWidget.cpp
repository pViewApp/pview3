#include "TransactionInsertionWidget.h"
#include "ActionData.h"
#include "SecurityUtils.h"
#include "pv/DataFile.h"
#include "pv/Security.h"
#include <QShortcut>
#include <pv/Account.h>

namespace pvui {
namespace controls {
TransactionInsertionWidget::TransactionInsertionWidget(pv::DataFile* dataFile, pv::Account* account, QWidget* parent)
    : QWidget(parent) {
  dateEditor->setCalendarPopup(true);
  actionEditor->setEditable(true);
  securityEditor->setEditable(true);

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

  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(dateEditor);

  static const QSizePolicy sizePolicy = {QSizePolicy::Ignored, QSizePolicy::Preferred};

  dateEditor->setSizePolicy(sizePolicy);
  actionEditor->setSizePolicy(sizePolicy);
  securityEditor->setSizePolicy(sizePolicy);
  numberOfSharesEditor->setSizePolicy(sizePolicy);
  sharePriceEditor->setSizePolicy(sizePolicy);
  commissionEditor->setSizePolicy(sizePolicy);
  totalAmountEditor->setSizePolicy(sizePolicy);

  auto* securityValidator = new util::SecuritySymbolValidator;
  securityEditor->setValidator(securityValidator);
  securityValidator->setParent(securityEditor);

  // Setup submit on enter key press
  QShortcut* returnShortcut = new QShortcut(Qt::Key_Return, this);
  QShortcut* enterShortcut = new QShortcut(Qt::Key_Enter, this);
  QObject::connect(returnShortcut, &QShortcut::activated, this, &TransactionInsertionWidget::submit);
  QObject::connect(enterShortcut, &QShortcut::activated, this, &TransactionInsertionWidget::submit);

  setAccount(dataFile, account);
  setupActionList();

  setFocusProxy(dateEditor);
}

void TransactionInsertionWidget::setupSecurityList() {
  if (dataFileSecurityConnection.has_value()) {
    dataFileSecurityConnection->disconnect();
  }

  if (dataFile_ == nullptr || account_ == nullptr) {
    dataFileSecurityConnection = std::nullopt;
    securityEditor->clear();
    return;
  }

  dataFileSecurityConnection = dataFile_->listenSecurityAdded([this](pv::Security* security) {
    QString text = securityEditor->currentText();
    securityEditor->addItem(QString::fromStdString(security->symbol()));
    securityEditor->model()->sort(0);
    securityEditor->setCurrentText(text);
  });

  securityEditor->clear();

  for (const auto* security : dataFile_->securities()) {
    securityEditor->addItem(QString::fromStdString(security->symbol()));
  }

  securityEditor->model()->sort(0);
}

bool TransactionInsertionWidget::submit() {
  std::string securitySymbol = securityEditor->currentText().trimmed().toStdString();

  pv::Security* security = nullptr;

  QDate qDate = dateEditor->date();
  auto date = pv::Date(pv::YearMonthDay(pv::Year(qDate.year()), pv::Month(qDate.month()), pv::Day(qDate.day())));

  if (!securitySymbol.empty()) {
    // User requested this transaction to have a security
    security = dataFile_->securityForSymbol(securitySymbol);

    if (security == nullptr) {
      // Security doesn't exist, create one
      static const std::string defaultSecurityAssetClass = "Equities";
      static const std::string defaultSecuritySector = "Other";

      security =
          dataFile_->addSecurity(securitySymbol, securitySymbol, defaultSecurityAssetClass, defaultSecuritySector);
    }
  }

  if (account_ == nullptr || dataFile_ == nullptr)
    return false;

  const pvui::ActionData* actionData = pvui::actionData(actionEditor->currentText());
  if (actionData == nullptr) {
    return false; // Invalid action
  }

  pv::Action action = actionData->action;

  bool success = false;

  switch (action) {
  case pv::Action::BUY: {
    success = account_->addTransaction(pv::BuyTransaction(
               date, security, numberOfSharesEditor->decimalValue(), sharePriceEditor->decimalValue(),
               commissionEditor->decimalValue())) == pv::TransactionOperationResult::SUCCESS;
    break;
  }
  case pv::Action::SELL: {
    success = account_->addTransaction(pv::SellTransaction(
               date, security, numberOfSharesEditor->decimalValue(), sharePriceEditor->decimalValue(),
               commissionEditor->decimalValue())) == pv::TransactionOperationResult::SUCCESS;
    break;
  }
  case pv::Action::DEPOSIT: {
    success = account_->addTransaction(pv::DepositTransaction(
                 date, security, totalAmountEditor->decimalValue()
               )) == pv::TransactionOperationResult::SUCCESS;
    break;
  }
  case pv::Action::WITHDRAW: {
    success = account_->addTransaction(pv::WithdrawTransaction(
                 date, security, totalAmountEditor->decimalValue()
               )) == pv::TransactionOperationResult::SUCCESS;
    break;
  }
  case pv::Action::DIVIDEND: {
    success = account_->addTransaction(pv::DividendTransaction(
                 date, security, totalAmountEditor->decimalValue()
               )) == pv::TransactionOperationResult::SUCCESS;
    break;
  }
  }

  reset();
  dateEditor->setFocus();

  return success;
}

void TransactionInsertionWidget::reset() {
  dateEditor->setDate(QDate::currentDate());
  dateEditor->clear();
  actionEditor->setCurrentIndex(-1);
  securityEditor->setCurrentIndex(-1);
  numberOfSharesEditor->setBlank();
  sharePriceEditor->setBlank();
  numberOfSharesEditor->setBlank();
  commissionEditor->setBlank();
  totalAmountEditor->setBlank();
}

void TransactionInsertionWidget::setupActionList() {
  for (const pvui::ActionData& action : pvui::actionData()) {
    actionEditor->addItem(action.name);
  }
}

void TransactionInsertionWidget::setAccount(pv::DataFile* dataFile, pv::Account* account) {
  if (account_ == account)
    return;

  account_ = account;
  dataFile_ = dataFile;
  bool enabled = account != nullptr;

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

} // namespace controls
} // namespace pvui

#include "TransactionInsertionWidget.h"
#include "DateUtils.h"
#include "SecurityModel.h"
#include "SecurityUtils.h"
#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include "pv/Security.h"
#include "pvui/DataFileManager.h"
#include "pvui/SecurityModel.h"
#include <QShortcut>
#include <cmath>
#include <memory>
#include <optional>
#include <sqlite3.h>

namespace pvui {
namespace controls {
TransactionInsertionWidget::TransactionInsertionWidget(DataFileManager& dataFileManager, QWidget* parent)
    : QWidget(parent), dataFileManager(dataFileManager) {
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
  layout->setContentsMargins(0, 0, 0, 0);

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

  QObject::connect(actionEditor, qOverload<int>(&QComboBox::currentIndexChanged), this, &TransactionInsertionWidget::handleActionChanged);
  QObject::connect(numberOfSharesEditor, qOverload<int>(&ExtendedSpinBox::valueChanged), this, &TransactionInsertionWidget::updateValues);
  QObject::connect(sharePriceEditor, qOverload<double>(&ExtendedDoubleSpinBox::valueChanged), this, &TransactionInsertionWidget::updateValues);
  QObject::connect(commissionEditor, qOverload<double>(&ExtendedDoubleSpinBox::valueChanged), this, &TransactionInsertionWidget::updateValues);
  QObject::connect(totalAmountEditor, qOverload<double>(&ExtendedDoubleSpinBox::valueChanged), this, &TransactionInsertionWidget::updateValues);


  securityProxy.sort(0, Qt::AscendingOrder);
  securityEditor->setModelColumn(0);
  securityEditor->setModel(&securityProxy);

  setFocusProxy(dateEditor);

  setupActionList();
  handleDataFileChanged(); // Initialize in consistent state
  // Handle dataFile
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, &TransactionInsertionWidget::handleDataFileChanged);
}

void TransactionInsertionWidget::handleDataFileChanged() {
  setAccount(std::nullopt);
  setupSecurityList();
  reset();
}

void TransactionInsertionWidget::repopulateSecurityList() {
  securityEditor->clear();
  auto query = dataFileManager->query("SELECT Id FROM Securities");
  if (!query) {
    return;
  }
  while (sqlite3_step(&*query) == SQLITE_ROW) {
    securityEditor->addItem(
        QString::fromStdString(pv::security::symbol(*dataFileManager, sqlite3_column_int64(&*query, 0))));
  }
}

void TransactionInsertionWidget::setupSecurityList() {
  securityModel = dataFileManager.has() ? std::make_unique<models::SecurityModel>(*dataFileManager) : nullptr;
  securityProxy.setSourceModel(&*securityModel);
}

bool TransactionInsertionWidget::submit() {
  if (!account_.has_value()) {
    return false;
  }
  std::string securitySymbol = securityEditor->currentText().trimmed().toStdString();

  QDate qDate = dateEditor->date();
  pv::i64 date = toEpochDate(qDate);
  std::optional<pv::i64> security = std::nullopt;
  pv::i64 numberOfShares = numberOfSharesEditor->value();
  pv::i64 sharePrice = static_cast<pv::i64>(std::llround(sharePriceEditor->value() * 100));
  pv::i64 commission = static_cast<pv::i64>(std::llround(commissionEditor->value() * 100));
  pv::i64 amount = static_cast<pv::i64>(std::llround(totalAmountEditor->value() * 100));

  if (!securitySymbol.empty()) {
    // User requested this transaction to have a security
    security = pv::security::securityForSymbol(*dataFileManager, securitySymbol);

    if (!security.has_value()) {
      // Security doesn't exist, create one
      static const std::string defaultSecurityAssetClass = "Equities";
      static const std::string defaultSecuritySector = "Other";

      dataFileManager->addSecurity(securitySymbol, securitySymbol, defaultSecurityAssetClass, defaultSecuritySector);
      security = dataFileManager->lastInsertedId();
    }
  }

  QVariant actionData = actionEditor->currentData();
  if (!actionData.isValid()) {
    return false; // No action selected
  }
  pv::Action action = static_cast<pv::Action>(actionData.toInt());

  pv::ResultCode result = pv::ResultCode::DbError; // Use some error by default, overrwrite later if needed

  switch (action) {
  case pv::Action::BUY: {
    if (!security.has_value()) {
      return false;
    }
    result = dataFileManager->addBuyTransaction(*account_, date, *security, numberOfShares, sharePrice, commission);
    break;
  }
  case pv::Action::SELL: {
    if (!security.has_value()) {
      return false;
    }
    result = dataFileManager->addSellTransaction(*account_, date, *security, numberOfShares, sharePrice, commission);
    break;
  }
  case pv::Action::DEPOSIT: {
    result = dataFileManager->addDepositTransaction(*account_, date, security, amount);
    break;
  }
  case pv::Action::WITHDRAW: {
    result = dataFileManager->addWithdrawTransaction(*account_, date, security, amount);
    break;
  }
  case pv::Action::DIVIDEND: {
    if (!security.has_value()) {
      return false;
    }
    result = dataFileManager->addDividendTransaction(*account_, date, *security, amount);
    break;
  }
  case pv::Action::INTEREST: {
    if (!security.has_value()) {
      return false;
    }
    result = dataFileManager->addInterestTransaction(*account_, date, *security, amount);
    break;
  }
  }

  if (result != pv::ResultCode::Ok) {
    return false;
  }

  reset();
  dateEditor->setFocus();

  emit submitted(dataFileManager->lastInsertedId());

  return true;
}

void TransactionInsertionWidget::reset() {
  dateEditor->setDate(QDate::currentDate());
  actionEditor->setCurrentIndex(-1);
  securityEditor->setCurrentIndex(-1);
  numberOfSharesEditor->setBlank();
  sharePriceEditor->setBlank();
  numberOfSharesEditor->setBlank();
  commissionEditor->setBlank();
  totalAmountEditor->setBlank();
}

void TransactionInsertionWidget::handleActionChanged() {
  // first enable all
  numberOfSharesEditor->setEnabled(true);
  sharePriceEditor->setEnabled(true);
  commissionEditor->setEnabled(true);
  totalAmountEditor->setEnabled(true);

  QVariant actionData = actionEditor->currentData();
  if (!actionData.isValid()) {
    return; // Enable all fields
  }
  pv::Action action = static_cast<pv::Action>(actionData.toInt());
  // Disable unneeded fields
  if (action == pv::Action::BUY || action == pv::Action::SELL) {
    totalAmountEditor->setEnabled(false);
  } else if (action == pv::Action::DEPOSIT || action == pv::Action::WITHDRAW || action == pv::Action::DIVIDEND || action == pv::Action::INTEREST) {
    commissionEditor->setEnabled(false);
    numberOfSharesEditor->setEnabled(false);
    sharePriceEditor->setEnabled(false);
  }

  updateValues();
}

void TransactionInsertionWidget::updateValues() {
  QVariant actionData = actionEditor->currentData();
  if (actionData.isNull()) {
    return;
  }
  pv::Action action = static_cast<pv::Action>(actionData.toInt());
  switch (action) {
    case pv::Action::DEPOSIT:
    case pv::Action::WITHDRAW:
    case pv::Action::DIVIDEND:
    case pv::Action::INTEREST:
      numberOfSharesEditor->setBlank();
      sharePriceEditor->setBlank();
      commissionEditor->setBlank();
      break;
    case pv::Action::SELL:
      totalAmountEditor->setValue(((std::llround(sharePriceEditor->value() * 100) * numberOfSharesEditor->value()) - std::llround(commissionEditor->value() * 100)) / 100.);
      break;
    case pv::Action::BUY:
      totalAmountEditor->setValue(((std::llround(sharePriceEditor->value() * 100) * numberOfSharesEditor->value()) + std::llround(commissionEditor->value() * 100)) / 100.);
      break;
  }
}

void TransactionInsertionWidget::setupActionList() {
  actionEditor->addItem(tr("In"), static_cast<int>(pv::Action::DEPOSIT));
  actionEditor->addItem(tr("Out"), static_cast<int>(pv::Action::WITHDRAW));
  actionEditor->addItem(tr("Buy"), static_cast<int>(pv::Action::BUY));
  actionEditor->addItem(tr("Sell"), static_cast<int>(pv::Action::SELL));
  actionEditor->addItem(tr("Dividend"), static_cast<int>(pv::Action::DIVIDEND));
  actionEditor->addItem(tr("Interest"), static_cast<int>(pv::Action::DIVIDEND));
}

void TransactionInsertionWidget::setAccount(std::optional<pv::i64> account) {
  if (account_ == account)
    return;

  account_ = account;
  bool enabled = account_.has_value();

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

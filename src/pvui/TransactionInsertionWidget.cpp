#include "TransactionInsertionWidget.h"
#include "ActionMappings.h"
#include "SecurityUtils.h"
#include "pv/Actions.h"
#include "pv/DataFile.h"
#include "pv/Security.h"
#include <QShortcut>

namespace pvui {
namespace controls {
TransactionInsertionWidget::TransactionInsertionWidget(std::optional<pv::Account> account, QWidget* parent)
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

  setAccount(account);
  setupActionList();
}

void TransactionInsertionWidget::setupSecurityList() {
  if (dataFileSecurityConnection.has_value()) {
    dataFileSecurityConnection->disconnect();
  }

  if (!account_.has_value()) {
    dataFileSecurityConnection = std::nullopt;
    securityEditor->clear();
    return;
  }

  dataFileSecurityConnection = account_->dataFile()->securityAdded().connect([&](pv::Security& security) {
    QString text = securityEditor->currentText();
    securityEditor->addItem(QString::fromStdString(security.symbol()));
    securityEditor->model()->sort(0);
    securityEditor->setCurrentText(text);
  });

  securityEditor->clear();

  for (const auto& security : account_->dataFile()->securities()) {
    securityEditor->addItem(QString::fromStdString(security.symbol()));
  }

  securityEditor->model()->sort(0);
}

bool TransactionInsertionWidget::submit() {
  using namespace date;

  std::string securitySymbol = securityEditor->currentText().trimmed().toStdString();

  std::optional<pv::Security> security;

  if (securitySymbol.empty()) {
    security = std::nullopt;
  } else {
    security = account_->dataFile()->securityForSymbol(securitySymbol);

    if (!security.has_value()) {
      // Security doesn't exist, create one
      static const std::string defaultSecurityAssetClass = "Equities";
      static const std::string defaultSecuritySector = "Other";

      security = account_->dataFile()->addSecurity(securitySymbol, securitySymbol, defaultSecurityAssetClass,
                                                   defaultSecuritySector);
    }
  }

  if (!account_.has_value())
    return false;

  auto date = dateEditor->date();

  if (!actionEditor->currentData().canConvert<QString>()) {
    return false;
  }

  // The currentData() holds the untranslated name of the action, defined in ActionMappings.h
  const auto actionIter =
      actionmappings::nameToActionMappings.find(actionEditor->currentData().value<QString>().toStdString());
  if (actionIter == actionmappings::nameToActionMappings.cend()) {
    return false; // Invalid action
  }

  const pv::Action& action = *(actionIter->second);

  std::variant<pv::Transaction, pv::TransactionAdditionError> result =
      account_->addTransaction(local_days(year_month_day(year(date.year()), month(date.month()), day(date.day()))),
                               action, security, numberOfSharesEditor->decimalValue(), sharePriceEditor->decimalValue(),
                               commissionEditor->decimalValue(), totalAmountEditor->decimalValue());

  if (std::holds_alternative<pv::TransactionAdditionError>(result)) {
    // Invalid transaction
    return false;
  }

  reset();

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

  dateEditor->setFocus();
}

void TransactionInsertionWidget::setupActionList() {
  for (const pv::Action* action : actionmappings::actions) {
    std::string name = actionmappings::actionToNameMappings.at(action);
    QString translatedName = tr(name.c_str());

    actionEditor->addItem(std::move(translatedName), QString::fromStdString(name));
  }
}

void TransactionInsertionWidget::setAccount(std::optional<pv::Account> account) {
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

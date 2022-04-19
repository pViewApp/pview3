#include "TransactionInsertionWidget.h"
#include "Actions.h"
#include "SecurityUtils.h"
#include <QShortcut>

const std::string defaultSecurityAssetClass = "Equities";
const std::string defaultSecuritySector = "Other";

namespace pvui {
namespace controls {
TransactionInsertionWidget::TransactionInsertionWidget(pv::AccountPtr account,
                                                       QWidget *parent)
    : QWidget(parent) {
  dateEditor->setCalendarPopup(true);
  actionEditor->setEditable(true);
  securityEditor->setEditable(true);

  QStringList actions{pv::ACTIONS.size()};
  std::transform(pv::ACTIONS.cbegin(), pv::ACTIONS.cend(), actions.begin(),
                 [](pv::Action *action) { return tr(action->name().c_str()); });

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

  static const QSizePolicy sizePolicy = {QSizePolicy::Ignored,
                                         QSizePolicy::Preferred};

  dateEditor->setSizePolicy(sizePolicy);
  actionEditor->setSizePolicy(sizePolicy);
  securityEditor->setSizePolicy(sizePolicy);
  numberOfSharesEditor->setSizePolicy(sizePolicy);
  sharePriceEditor->setSizePolicy(sizePolicy);
  commissionEditor->setSizePolicy(sizePolicy);
  totalAmountEditor->setSizePolicy(sizePolicy);

  auto *securityValidator = new util::SecuritySymbolValidator;
  securityEditor->setValidator(securityValidator);
  securityValidator->setParent(securityEditor);

  // Setup submit on enter key press
  QShortcut *returnShortcut = new QShortcut(Qt::Key_Return, this);
  QShortcut *enterShortcut = new QShortcut(Qt::Key_Enter, this);
  QObject::connect(returnShortcut, &QShortcut::activated, this,
                   &TransactionInsertionWidget::submit);
  QObject::connect(enterShortcut, &QShortcut::activated, this,
                   &TransactionInsertionWidget::submit);

  setAccount(account);
}

void TransactionInsertionWidget::setupSecurityList() {
  if (dataFileSecurityConnection.has_value()) {
    dataFileSecurityConnection->disconnect();
  }

  if (account_ == nullptr) {
    dataFileSecurityConnection = std::nullopt;
    securityEditor->clear();
    return;
  }

  dataFileSecurityConnection = account_->dataFile().securityAdded().connect(
      [&](pv::SecurityPtr security) {
        QString text = securityEditor->currentText();
        securityEditor->addItem(QString::fromStdString(security->symbol()));
        securityEditor->model()->sort(0);
        securityEditor->setCurrentText(text);
      });

  securityEditor->clear();

  for (auto security : account_->dataFile().securities()) {
    securityEditor->addItem(QString::fromStdString(security->symbol()));
  }

  securityEditor->model()->sort(0);
}

void TransactionInsertionWidget::submit() {
  using namespace date;

  std::string securitySymbol =
      securityEditor->currentText().trimmed().toStdString();

  pv::SecurityPtr security;

  if (securitySymbol.empty()) {
    security = pv::Security::None;
  } else {
    security = account_->dataFile().securityForSymbol(securitySymbol);

    if (security == nullptr) {
      security = account_->dataFile().addSecurity(
          securitySymbol, securitySymbol, defaultSecurityAssetClass,
          defaultSecuritySector);
    }
  }

  if (account_ != nullptr) {
    auto date = dateEditor->date();

    auto *action = pv::actionFromName(
        actionEditor->itemText(actionEditor->currentIndex()).toStdString());
    if (action == nullptr)
      return;

    account_->addTransaction(
        local_days(year_month_day(year(date.year()), month(date.month()),
                                  day(date.day()))),
        *action, security, numberOfSharesEditor->decimalValue(),
        sharePriceEditor->decimalValue(), commissionEditor->decimalValue(),
        totalAmountEditor->decimalValue());

    reset();
  }
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

void TransactionInsertionWidget::setAccount(pv::AccountPtr account) {
  if (account_ == account)
    return;
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

} // namespace controls
} // namespace pvui

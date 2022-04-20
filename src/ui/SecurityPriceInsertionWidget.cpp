#include "SecurityPriceInsertionWidget.h"
#include <QShortcut>

pvui::controls::SecurityPriceInsertionWidget::SecurityPriceInsertionWidget(pv::SecurityPtr security, QWidget* parent)
    : QWidget(parent) {
  layout->addWidget(dateEditor, 1);
  layout->addWidget(priceEditor, 1);

  dateEditor->setCalendarPopup(true);
  priceEditor->setPlaceholderText(tr("Price"));

  // Set Size Policy
  static const QSizePolicy sizePolicy = {QSizePolicy::Ignored, QSizePolicy::Preferred};
  dateEditor->setSizePolicy(sizePolicy);
  priceEditor->setSizePolicy(sizePolicy);

  QShortcut* submitShortcutReturn = new QShortcut(Qt::Key_Return, this);
  QShortcut* submitShortcutEnter = new QShortcut(Qt::Key_Enter, this);

  QObject::connect(submitShortcutEnter, &QShortcut::activated, this, &SecurityPriceInsertionWidget::submit);
  QObject::connect(submitShortcutReturn, &QShortcut::activated, this, &SecurityPriceInsertionWidget::submit);

  setSecurity(security);
}

void pvui::controls::SecurityPriceInsertionWidget::reset() {
  bool enable = security_ == nullptr;

  dateEditor->setEnabled(enable);
  dateEditor->setEnabled(priceEditor);

  dateEditor->setDate(QDate::currentDate());
  priceEditor->setBlank();
}

bool pvui::controls::SecurityPriceInsertionWidget::submit() {
  using namespace pv;

  auto qDate = dateEditor->date();
  Date date(YearMonthDay(Year(qDate.year()), Month(qDate.month()), Day(qDate.day())));

  if (security_->prices().find(date) != security_->prices().cend())
    return false;

  security_->setPrice(date, priceEditor->decimalValue());

  reset();
  dateEditor->setFocus();

  return true;
}

void pvui::controls::SecurityPriceInsertionWidget::setSecurity(pv::SecurityPtr security) {
  security_ = security;
  reset();
}

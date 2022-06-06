#include "SecurityPriceInsertionWidget.h"
#include <QShortcut>

pvui::controls::SecurityPriceInsertionWidget::SecurityPriceInsertionWidget(pv::Security& security, QWidget* parent)
    : QWidget(parent), security_(&security) {
  layout->addWidget(dateEditor, 1);
  layout->addWidget(priceEditor, 1);

  dateEditor->setCalendarPopup(true);
  priceEditor->setPlaceholderText(tr("Price"));

  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setFocusProxy(dateEditor);

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
  dateEditor->setDate(QDate::currentDate());
  priceEditor->setBlank();
}

bool pvui::controls::SecurityPriceInsertionWidget::submit() {
  using namespace pv;

  auto qDate = dateEditor->date();
  Date date(YearMonthDay(Year(qDate.year()), Month(qDate.month()), Day(qDate.day())));

  if (security_->prices().find(date) != security_->prices().cend())
    return false;

  if (priceEditor->cleanText().isEmpty())
    return false;

  if (!security_->setPrice(date, priceEditor->decimalValue()))
    return false;

  reset();
  dateEditor->setFocus();

  return true;
}

void pvui::controls::SecurityPriceInsertionWidget::setSecurity(pv::Security& security) {
  security_ = &security;
  reset();
}

#include "SecurityPriceInsertionWidget.h"
#include "pv/DataFile.h"
#include "pv/Security.h"
#include <QShortcut>
#include <optional>
#include "DateUtils.h"
#include "pvui/DataFileManager.h"
#include <cmath>

namespace pvui {
namespace controls {

SecurityPriceInsertionWidget::SecurityPriceInsertionWidget(DataFileManager& dataFileManager, QWidget* parent)
    : QWidget(parent), dataFileManager(dataFileManager) {
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

  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, &SecurityPriceInsertionWidget::handleDataFileChanged);
  handleDataFileChanged();
}

void SecurityPriceInsertionWidget::handleDataFileChanged() {
  setSecurity(std::nullopt);
}

void SecurityPriceInsertionWidget::reset() {
  dateEditor->setDate(QDate::currentDate());
  priceEditor->setBlank();
}

bool SecurityPriceInsertionWidget::submit() {
  if (!security_.has_value()) {
    return false;
  }

  auto qDate = dateEditor->date();
  auto date = toEpochDate(qDate);

  if (pv::security::price(*dataFileManager, *security_, date).has_value())
    return false;

  if (priceEditor->cleanText().isEmpty())
    return false;

  auto result = dataFileManager->setSecurityPrice(*security_, date, static_cast<pv::i64>(std::llround(priceEditor->value() * 100)));
  if (result != pv::ResultCode::OK) {
    return false;
  }

  reset();
  dateEditor->setFocus();
  emit submitted(qDate);

  return true;
}

void SecurityPriceInsertionWidget::setSecurity(std::optional<pv::i64> security) {
  security_ = std::move(security);
  reset();
  setEnabled(security.has_value());
}

} // namespace controls
} // namespace pvui


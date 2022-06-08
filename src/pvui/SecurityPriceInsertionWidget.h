#ifndef PVUI_SECURITY_PRICE_INSERTION_WIDGET_H
#define PVUI_SECURITY_PRICE_INSERTION_WIDGET_H

#include "ExtendedSpinBox.h"
#include "pv/Security.h"
#include <QDateEdit>
#include <QHBoxLayout>
#include <QWidget>

namespace pvui::controls {
class SecurityPriceInsertionWidget : public QWidget {
  Q_OBJECT
private:
  QHBoxLayout* layout = new QHBoxLayout(this);
  QDateEdit* dateEditor = new QDateEdit(QDate::currentDate());
  ExtendedDoubleSpinBox* priceEditor = new ExtendedDoubleSpinBox;

  pv::Security* security_ = nullptr;

  void reset();

public:
  SecurityPriceInsertionWidget(pv::Security& security, QWidget* parent = nullptr);
public slots:
  /// @brief Attempts to add a security price with the current user-provided
  /// values.
  /// @return `true` if the addition was successful, `false` otherwise
  bool submit();

  void setSecurity(pv::Security& security);

signals:
  void submitted(QDate date);
};
} // namespace pvui::controls
#endif // PVUI_SECURITY_PRICE_INSERTION_WIDGET_H

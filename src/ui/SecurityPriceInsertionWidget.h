#ifndef PVUI_SECURITY_PRICE_INSERTION_WIDGET_H
#define PVUI_SECURITY_PRICE_INSERTION_WIDGET_H

#include "DataFile.h"
#include "ExtendedSpinBox.h"
#include <QDateEdit>
#include <QHBoxLayout>
#include <QShowEvent>
#include <QWidget>

namespace pvui::controls {
class SecurityPriceInsertionWidget : public QWidget {
  Q_OBJECT
private:
  QHBoxLayout *layout = new QHBoxLayout(this);
  QDateEdit *dateEditor = new QDateEdit(QDate::currentDate());
  ExtendedDoubleSpinBox *priceEditor = new ExtendedDoubleSpinBox;

  pv::SecurityPtr security_;

  void reset();

protected:
  inline void showEvent(QShowEvent *evt) override {
    if (!evt->spontaneous()) {
      dateEditor->setFocus();
    }
  }

public:
  SecurityPriceInsertionWidget(pv::SecurityPtr security = nullptr,
                               QWidget *parent = nullptr);
public slots:
  /// @brief Attempts to add a security price with the current user-provided
  /// values.
  /// @return `true` if the addition was successful, `false` otherwise
  bool submit();

  void setSecurity(pv::SecurityPtr security);
};
} // namespace pvui::controls
#endif // PVUI_SECURITY_PRICE_INSERTION_WIDGET_H

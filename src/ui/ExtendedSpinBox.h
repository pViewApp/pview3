#ifndef PVUI_CONTROLS_EXTENDEDEDSPINBOX_H
#define PVUI_CONTROLS_EXTENDEDEDSPINBOX_H
#include "Types.h"
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QValidator>

namespace pvui::controls {
class ExtendedDoubleSpinBox : public QDoubleSpinBox {
  Q_OBJECT
public:
  inline ExtendedDoubleSpinBox(QWidget *parent = nullptr)
      : QDoubleSpinBox(parent) {
    setBlank();
    setMaximum(std::numeric_limits<double>().max());
    QObject::connect(this->lineEdit(), &QLineEdit::returnPressed, this,
                     &ExtendedDoubleSpinBox::returnPressed);
  }

  virtual double valueFromText(const QString &text) const override;

  virtual QString textFromValue(double value) const override;

  virtual QValidator::State validate(QString &text, int &pos) const override;

  inline void setPlaceholderText(QString placeholderText) {
    lineEdit()->setPlaceholderText(placeholderText);
  }

  inline void setBlank() { lineEdit()->setText(""); }

  inline pv::Decimal decimalValue() {
    return cleanText().isEmpty() ? 0 : pv::Decimal(cleanText().toStdString());
  }
signals:
  void returnPressed();
};
} // namespace pvui::controls

#endif // PVUI_BLANKABLE_SPIN_BOX

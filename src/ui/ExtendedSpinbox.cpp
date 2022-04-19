#include "ExtendedSpinBox.h"

double pvui::controls::ExtendedDoubleSpinBox::valueFromText(
    const QString &text) const {
  if (text.isEmpty()) {
    return 0.;
  }
  return QDoubleSpinBox::valueFromText(text);
}

QString
pvui::controls::ExtendedDoubleSpinBox::textFromValue(double value) const {
  if (value == 0 && cleanText().isEmpty())
    return "";
  return QDoubleSpinBox::textFromValue(value);
}

QValidator::State
pvui::controls::ExtendedDoubleSpinBox::validate(QString &text, int &pos) const {
  if (text.isEmpty())
    return QValidator::Acceptable;
  return QDoubleSpinBox::validate(text, pos);
}

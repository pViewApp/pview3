#ifndef PVUI_CONTROLS_EXTENDEDEDSPINBOX_H
#define PVUI_CONTROLS_EXTENDEDEDSPINBOX_H

#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QValidator>
#include <qspinbox.h>

namespace pvui::controls {

// These classes provide the QSpinBox's with a blank state and placeholders.

class ExtendedSpinBox : public QSpinBox {
  Q_OBJECT
public:
  ExtendedSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {
    setBlank();
    setMaximum(std::numeric_limits<int>().max());
  }

  int valueFromText(const QString& text) const override;

  QString textFromValue(int value) const override;

  QValidator::State validate(QString& text, int& pos) const override;

  void setPlaceholderText(QString placeholderText) { lineEdit()->setPlaceholderText(placeholderText); }

  void setBlank() { lineEdit()->setText(""); }
};

class ExtendedDoubleSpinBox : public QDoubleSpinBox {
  Q_OBJECT
public:
  ExtendedDoubleSpinBox(QWidget* parent = nullptr) : QDoubleSpinBox(parent) {
    setBlank();
    setMaximum(std::numeric_limits<double>().max());
  }

  double valueFromText(const QString& text) const override;

  QString textFromValue(double value) const override;

  QValidator::State validate(QString& text, int& pos) const override;

  void setPlaceholderText(QString placeholderText) { lineEdit()->setPlaceholderText(placeholderText); }

  void setBlank() { lineEdit()->setText(""); }
};
} // namespace pvui::controls

#endif // PVUI_BLANKABLE_SPIN_BOX

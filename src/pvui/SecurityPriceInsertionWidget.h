#ifndef PVUI_SECURITY_PRICE_INSERTION_WIDGET_H
#define PVUI_SECURITY_PRICE_INSERTION_WIDGET_H

#include "ExtendedSpinBox.h"
#include "pv/DataFile.h"
#include "pv/Security.h"
#include "pvui/DataFileManager.h"
#include <QDateEdit>
#include <QHBoxLayout>
#include <QWidget>

namespace pvui::controls {
class SecurityPriceInsertionWidget : public QWidget {
  Q_OBJECT
private:
  DataFileManager& dataFileManager;
  std::optional<pv::i64> security_ = std::nullopt;

  QHBoxLayout* layout = new QHBoxLayout(this);
  QDateEdit* dateEditor = new QDateEdit(QDate::currentDate());
  ExtendedDoubleSpinBox* priceEditor = new ExtendedDoubleSpinBox;

  void reset();

private slots:
  void handleDataFileChanged();
public:
  SecurityPriceInsertionWidget(DataFileManager& dataFileManager, QWidget* parent = nullptr);
public slots:
  /// @brief Attempts to add a security price with the current user-provided
  /// values.
  /// @return `true` if the addition was successful, `false` otherwise
  bool submit();

  void setSecurity(std::optional<pv::i64> security);

signals:
  void submitted(QDate date);
};
} // namespace pvui::controls
#endif // PVUI_SECURITY_PRICE_INSERTION_WIDGET_H

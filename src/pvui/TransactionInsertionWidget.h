#ifndef PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H
#define PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H

#include "ExtendedSpinBox.h"
#include "pv/Account.h"
#include "pv/Transaction.h"
#include <QComboBox>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QWidget>
#include <boost/signals2.hpp>
#include <optional>

namespace pvui {
namespace controls {
class TransactionInsertionWidget : public QWidget {
  Q_OBJECT
private:
  std::optional<pv::Account> account_;

  QHBoxLayout* layout = new QHBoxLayout(this);
  QDateEdit* dateEditor = new QDateEdit;
  QComboBox* actionEditor = new QComboBox;
  QComboBox* securityEditor = new QComboBox;
  controls::ExtendedDoubleSpinBox* numberOfSharesEditor = new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox* sharePriceEditor = new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox* commissionEditor = new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox* totalAmountEditor = new controls::ExtendedDoubleSpinBox;

  std::optional<boost::signals2::connection> dataFileSecurityConnection =
      std::nullopt; // Connection to the current DataFile's securityAdded()
                    // signal
  void reset();

public:
  TransactionInsertionWidget(std::optional<pv::Account> account = std::nullopt, QWidget* parent = nullptr);
protected slots:
  void setupActionList();
  void setupSecurityList();
public slots:
  bool submit();
  void setAccount(std::optional<pv::Account> account);
};
} // namespace controls
} // namespace pvui

#endif // PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H

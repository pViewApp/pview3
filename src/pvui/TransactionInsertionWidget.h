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
  pv::DataFile* dataFile_;
  pv::Account* account_;

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
  TransactionInsertionWidget(pv::DataFile* dataFile = nullptr, pv::Account* account = nullptr,
                             QWidget* parent = nullptr);
protected slots:
  void setupActionList();
  void setupSecurityList();
public slots:
  bool submit();

  /// \note Set \c dataFile and \c account to nullptr to disable.
  /// \note Undefined behaviour if (\c dataFile does not own \c account) AND \c account != \c nullptr
  void setAccount(pv::DataFile* dataFile, pv::Account* account);
};
} // namespace controls
} // namespace pvui

#endif // PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H

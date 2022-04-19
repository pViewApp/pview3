#ifndef PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H
#define PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H

#include "DataFile.h"
#include "ExtendedSpinBox.h"
#include <QCombobox>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QShowEvent>
#include <QWidget>
#include <boost/signals2.hpp>

namespace pvui {
namespace controls {
class TransactionInsertionWidget : public QWidget {
  Q_OBJECT
private:
  pv::AccountPtr account_;

  QHBoxLayout *layout = new QHBoxLayout(this);
  QDateEdit *dateEditor = new QDateEdit;
  QComboBox *actionEditor = new QComboBox;
  QComboBox *securityEditor = new QComboBox;
  controls::ExtendedDoubleSpinBox *numberOfSharesEditor =
      new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox *sharePriceEditor =
      new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox *commissionEditor =
      new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox *totalAmountEditor =
      new controls::ExtendedDoubleSpinBox;

  std::optional<boost::signals2::connection> dataFileSecurityConnection =
      std::nullopt; // Connection to the current DataFile's securityAdded()
                    // signal
  void reset();

protected:
  inline void showEvent(QShowEvent *showEvent) override {
    if (!showEvent->spontaneous()) {
      dateEditor->setFocus();
    }
  }

public:
  TransactionInsertionWidget(pv::AccountPtr account = nullptr,
                             QWidget *parent = nullptr);
protected slots:
  void setupSecurityList();
public slots:
  void submit();
  void setAccount(pv::AccountPtr account);
};
} // namespace controls
} // namespace pvui

#endif // PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H

#ifndef PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H
#define PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H

#include "ExtendedSpinBox.h"
#include "pv/Integer64.h"
#include "pv/Signals.h"
#include "pvui/DataFileManager.h"
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
  DataFileManager& dataFileManager;
  std::optional<pv::i64> account_;

  QHBoxLayout* layout = new QHBoxLayout(this);
  QDateEdit* dateEditor = new QDateEdit;
  QComboBox* actionEditor = new QComboBox;
  QComboBox* securityEditor = new QComboBox;
  controls::ExtendedSpinBox* numberOfSharesEditor = new controls::ExtendedSpinBox;
  controls::ExtendedDoubleSpinBox* sharePriceEditor = new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox* commissionEditor = new controls::ExtendedDoubleSpinBox;
  controls::ExtendedDoubleSpinBox* totalAmountEditor = new controls::ExtendedDoubleSpinBox;

  pv::ScopedConnection securityAddedConnection;
  pv::ScopedConnection securityUpdatedConnection;
  pv::ScopedConnection securityRemovedConnection;
  pv::ScopedConnection resetConnection;

  void reset();
  void repopulateSecurityList();
private slots:
  void handleDataFileChanged();
  void setupActionList();
  void setupSecurityList();
public:
  TransactionInsertionWidget(DataFileManager& dataFile,
                             QWidget* parent = nullptr);
public slots:
  bool submit();

  void setAccount(std::optional<pv::i64> account);
signals:
  void submitted(pv::i64 transaction);
};
} // namespace controls
} // namespace pvui

#endif // PVUI_CONTROLS_TRANSACTIONINSERTIONWIDGET_H

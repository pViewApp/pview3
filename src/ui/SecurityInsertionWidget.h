#ifndef PVUI_CONTROLS_SECURITYINSERTIONWIDGET_H
#define PVUI_CONTROLS_SECURITYINSERTIONWIDGET_H

#include "DataFileManager.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QShowEvent>
#include <QWidget>

namespace pvui {
namespace controls {
class SecurityInsertionWidget : public QWidget {
  Q_OBJECT
private:
  QLineEdit* symbolEditor = new QLineEdit;
  QLineEdit* nameEditor = new QLineEdit;
  QComboBox* assetClassEditor = new QComboBox;
  QComboBox* sectorEditor = new QComboBox;
  QHBoxLayout* layout = new QHBoxLayout(this);
  pvui::DataFileManager& dataFileManager_;
  void reset();

protected:
  inline void showEvent(QShowEvent* showEvent) override {
    if (!showEvent->spontaneous()) {
      symbolEditor->setFocus();
    }
  }

public:
  SecurityInsertionWidget(pvui::DataFileManager& manager, QWidget* parent = nullptr);
public slots:
  void submit();
};
} // namespace controls
} // namespace pvui

#endif // PVUI_CONTROLS_SECURITYINSERTIONWIDGET_H

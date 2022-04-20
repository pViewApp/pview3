#ifndef PVUI_SECURITY_DELEGATE_H
#define PVUI_SECURITY_DELEGATE_H

#include "DataFileManager.h"
#include <QApplication>
#include <QObject>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QWidget>

namespace pvui::delegates {
class SecurityDelegate : public QStyledItemDelegate {
private:
  pvui::DataFileManager& dataFileManager_;

public:
  inline SecurityDelegate(pvui::DataFileManager& dataFileManager, QWidget* parent = nullptr)
      : QStyledItemDelegate(parent), dataFileManager_(dataFileManager) {}

  inline void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QStyleOptionButton button;
    button.rect = QRect(0, 0, 30, 30);
    button.text = "X";
    button.state = QStyle::State_Enabled;

    QPushButton* pushButton = new QPushButton;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter, pushButton);
  }
};
} // namespace pvui::delegates

#endif // PVUI_SECURITY_DELEGATE_H

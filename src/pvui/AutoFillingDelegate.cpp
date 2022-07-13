#include "AutoFillingDelegate.h"
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

QWidget* pvui::AutoFillingDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                                 const QModelIndex& index) const {
  QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
  if (editor != nullptr) {
    editor->setAutoFillBackground(true);
    editor->setBackgroundRole(QPalette::Window);
  }
  return editor;
}

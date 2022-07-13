#ifndef PVUI_AUTOFILLING_DELEGATE_H
#define PVUI_AUTOFILLING_DELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QWidget>

namespace pvui {
/// A delegate that auto fills the backgrounds of editors to prevent content showing through while editing.
class AutoFillingDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const override;
};

} // namespace pvui

#endif // PVUI_AUTOFILLING_DELEGATE_H

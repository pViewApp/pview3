#ifndef PVUI_PAGE_H
#define PVUI_PAGE_H

#include <QLabel>
#include <QObject>
#include <QToolBar>
#include <QVBoxLayout>
#include <variant>

namespace pvui {
class PageWidget : public QWidget {
  Q_OBJECT
private:
  QVBoxLayout* layout_ = new QVBoxLayout(this);
  QLabel* title = new QLabel;
  QLabel* subtitle = new QLabel;
  QLayoutItem* content_;

protected:
  QVBoxLayout* layout() noexcept { return layout_; }
  QLabel* titleLabel() noexcept { return title; };
  QLabel* subtitleLabel() noexcept { return subtitle; };

public:
  PageWidget(QWidget* parent = nullptr);
protected slots:
  void setTitle(QString newTitle) { title->setText("<h1>" + newTitle.toHtmlEscaped() + "</h1>"); }
  void setSubtitle(QString newSubtitle) {
    subtitle->setText("<h3>" + newSubtitle.toHtmlEscaped() + "</h3>");
    subtitle->setVisible(true);
  }
};
} // namespace pvui

#endif // PVUI_PAGE_H

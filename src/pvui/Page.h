#ifndef PVUI_PAGE_H
#define PVUI_PAGE_H

#include <QLabel>
#include <QObject>
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

public:
  PageWidget(QWidget* parent = nullptr);
protected slots:
  void setTitle(QString newTitle) { title->setText("<h1>" + newTitle.toHtmlEscaped() + "</h1>"); }
  void setSubtitle(QString newSubtitle) {
    subtitle->setText("<h3>" + newSubtitle.toHtmlEscaped() + "</h3>");
    subtitle->setVisible(true);
  }

  void setContent(QLayoutItem* content) {
    if (content_ != nullptr) {
      layout_->removeItem(content_);
    }

    content_ = content;
    layout_->addItem(content_);
  }

  void setContent(QLayout* content) {
    if (content_ != nullptr) {
      layout_->removeItem(content_);
    }

    content_ = content;
    layout_->addLayout(content);
  }

  void setContent(QWidget* content) { setContent(new QWidgetItem(content)); }
};
} // namespace pvui

#endif // PVUI_PAGE_H

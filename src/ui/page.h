#ifndef PVUI_PAGE_H
#define PVUI_PAGE_H

#include <QGroupBox>
#include <QObject>
#include <QVBoxLayout>
#include <variant>

namespace pvui {
class PageWidget : public QWidget {
  Q_OBJECT
private:
  QString title_;
  QGroupBox *contentBox;
  QVBoxLayout *contentBoxLayout;
  QLayoutItem *content_;

public:
  PageWidget(QWidget *parent = nullptr);

  QString title() { return title_; }

protected slots:
  void setTitle(QString newTitle) {
    title_ = newTitle;
    emit titleChanged(newTitle);
  }

  void setContent(QLayoutItem *content) {
    if (content_ != nullptr) {
      contentBoxLayout->removeItem(content_);
    }

    content_ = content;
    contentBoxLayout->addItem(content_);
  }

  void setContent(QLayout *content) {
    if (content_ != nullptr) {
      contentBoxLayout->removeItem(content_);
    }

    content_ = content;
    contentBoxLayout->addLayout(content);
  }

  void setContent(QWidget *content) { setContent(new QWidgetItem(content)); }
signals:
  void titleChanged(const QString &newTitle);
};
} // namespace pvui

#endif // PVUI_PAGE_H

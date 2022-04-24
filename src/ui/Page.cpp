#include "Page.h"
#include <QStackedLayout>

pvui::PageWidget::PageWidget(QWidget* parent)
    : QWidget(parent), contentBox(new QGroupBox()), contentBoxLayout(new QVBoxLayout(contentBox)), content_(nullptr) {
  auto* layout = new QStackedLayout(this);
  layout->addWidget(contentBox);
  QObject::connect(this, &PageWidget::titleChanged, contentBox, &QGroupBox::setTitle);
}

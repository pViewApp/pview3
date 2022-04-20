#include "Page.h"
#include <QStackedLayout>

pvui::PageWidget::PageWidget(QWidget* parent)
    : QWidget(parent), content_(nullptr), contentBox(new QGroupBox()), contentBoxLayout(new QVBoxLayout(contentBox)) {
  auto* layout = new QStackedLayout(this);
  layout->addWidget(contentBox);
  QObject::connect(this, &PageWidget::titleChanged, contentBox, &QGroupBox::setTitle);
}

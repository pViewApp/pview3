#include "Page.h"
#include <QStackedLayout>

pvui::PageWidget::PageWidget(QWidget* parent) : QWidget(parent), content_(nullptr) {
  layout_->addWidget(title);
  layout_->addWidget(subtitle);
  subtitle->setVisible(false); // Not visible by default
}

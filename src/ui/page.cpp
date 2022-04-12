#include "page.h"

#include <QStackedLayout>

pvui::PageWidget::PageWidget(QWidget* parent) : QWidget(parent),
	contentBox(new QGroupBox()),
	content_(nullptr),
	contentBoxLayout(new QVBoxLayout(contentBox))
{
	auto* layout = new QStackedLayout(this);
	layout->addWidget(contentBox);
	QObject::connect(this, &PageWidget::titleChanged, contentBox, &QGroupBox::setTitle);
}

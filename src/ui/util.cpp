#include "util.h"

QSizePolicy pvui::horizontalStretchPolicy(int stretch)
{
	QSizePolicy policy{ QSizePolicy::Ignored, QSizePolicy::Ignored };
	policy.setHorizontalStretch(stretch);
	return policy;
}

pvui::PlaceholderDoubleSpinBox::PlaceholderDoubleSpinBox(QWidget* parent) : QDoubleSpinBox(parent) {}

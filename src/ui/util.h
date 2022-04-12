#ifndef UI_UTIL_H
#define UI_UTIL_H
#include <QSizepolicy>
#include <QDoubleSpinBox>
#include <QLineEdit>

namespace pvui {
	QSizePolicy horizontalStretchPolicy(int stretch);

	class PlaceholderDoubleSpinBox : public QDoubleSpinBox {
	public:
		PlaceholderDoubleSpinBox(QWidget* parent = nullptr);

		inline void setPlaceholderText(const QString& placeholderText) {
			lineEdit()->setPlaceholderText(placeholderText);
		}
	};
}
#endif // UI_UTIL_H

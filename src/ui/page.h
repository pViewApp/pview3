#ifndef UI_PAGE_H
#define UI_PAGE_H

#include <variant>
#include <QObject>
#include <QGroupBox>
#include <QVBoxLayout>

namespace pvui {
	class PageWidget : public QWidget {
		Q_OBJECT
	private:
		QString title_;
		QGroupBox* contentBox;
		QVBoxLayout* contentBoxLayout;
		QLayoutItem* content_;
	public:
		PageWidget(QWidget* parent = nullptr);

		inline QString title() {
			return title_;
		}

	protected slots:
		inline void setTitle(QString newTitle) {
			title_ = newTitle;
			emit titleChanged(newTitle);
		}
		
		inline void setContent(QLayoutItem* content) {
			if (content_ != nullptr) {
				contentBoxLayout->removeItem(content_);
			}

			content_ = content;
			contentBoxLayout->addItem(content_);
		}

		inline void setContent(QLayout* content) {
			if (content_ != nullptr) {
				contentBoxLayout->removeItem(content_);
			}

			content_ = content;
			contentBoxLayout->addLayout(content);
		}

		inline void setContent(QWidget* content) {
			setContent(new QWidgetItem(content));
		}
	signals:
		void titleChanged(const QString& newTitle);
	};
}

#endif // UI_PAGE_H

#ifndef PVUI_SECURITYPAGE_H
#define PVUI_SECURITYPAGE_H
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QValidator>
#include <QRegularExpression>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include "datafilemanager.h"
#include "page.h"

namespace pvui {
	namespace {
	}

	class SecuritySymbolValidator : public QValidator {
		Q_OBJECT
	public:
		QValidator::State validate(QString& input, int& pos) const override;
	};

	class SecurityInsertionWidget : public QWidget {
		Q_OBJECT
	private:
		QLineEdit* symbolEditor = new QLineEdit;
		QLineEdit* nameEditor = new QLineEdit;
		QComboBox* assetClassEditor = new QComboBox;
		QComboBox* sectorEditor = new QComboBox;
		QHBoxLayout* layout = new QHBoxLayout(this);
		pvui::DataFileManager& dataFileManager;
		void reset();
	protected:
		inline void showEvent(QShowEvent* showEvent) override {
			if (!showEvent->spontaneous()) {
				symbolEditor->setFocus();
			}
		}
	public:
		SecurityInsertionWidget(pvui::DataFileManager& manager, QWidget* parent = nullptr);
	public slots:
		void submit();
	};

	class SecurityPageWidget : public PageWidget {
		Q_OBJECT
	private:
		QTableView* table = new QTableView;
		QSortFilterProxyModel* tableModel = new QSortFilterProxyModel;
		pvui::DataFileManager& dataFileManager_;
		SecurityInsertionWidget* insertionWidget = new SecurityInsertionWidget(dataFileManager_);
	public:
		SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent = nullptr);
	};
}

#endif // PVUI_SECURITYPAGE_H

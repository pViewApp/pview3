#ifndef PVUI_SECURITYPAGE_H
#define PVUI_SECURITYPAGE_H
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QValidator>
#include <QRegularExpression>
#include "datafilemanager.h"
#include "page.h"

namespace pvui {
	namespace {
		constexpr int maximumSymbolLength = 10;
		inline static QRegularExpression const invalidSymbolRegularExpression = QRegularExpression("[^A-Z0-9.]");
	}

	class SecuritySymbolValidator : public QValidator {
		Q_OBJECT
	public:
		QValidator::State validate(QString& input, int& pos) const override {
			return input.contains(invalidSymbolRegularExpression) ? QValidator::State::Invalid : QValidator::State::Acceptable;
		}

		inline void fixup(QString& input) const override {
			input = input.trimmed().toUpper();
			input.truncate(maximumSymbolLength);
		}

	};
	class SecurityInsertionWidget : public QWidget {
		Q_OBJECT
	private:
		QLineEdit* symbolEditor;
		QLineEdit* nameEditor;
		QComboBox* assetClassEditor;
		QComboBox* sectorEditor;
		QHBoxLayout* layout;
	public:
		SecurityInsertionWidget(QWidget* parent = nullptr);
	};
	class SecurityPageWidget : public PageWidget {
		Q_OBJECT
	private:
		QTableView* table = new QTableView;
		SecurityInsertionWidget* insertionWidget = new SecurityInsertionWidget;
		const pvui::DataFileManager& dataFileManager_;
	public:
		SecurityPageWidget(const pvui::DataFileManager& dataFileManager, QWidget* parent = nullptr);
	};
}

#endif // PVUI_SECURITYPAGE_H

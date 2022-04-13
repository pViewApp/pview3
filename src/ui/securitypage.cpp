#include "securitypage.h"
#include "securitymodel.h"

pvui::SecurityInsertionWidget::SecurityInsertionWidget(QWidget* parent) : QWidget(parent),
	layout(new QHBoxLayout(this)),
	symbolEditor(new QLineEdit),
	nameEditor(new QLineEdit),
	assetClassEditor(new QComboBox),
	sectorEditor(new QComboBox)
{
	layout->addWidget(symbolEditor);
	layout->addWidget(nameEditor);
	layout->addWidget(assetClassEditor);
	layout->addWidget(sectorEditor);

	// Make sure all editors are the same size
	layout->setStretch(0, 1);
	layout->setStretch(1, 1);
	layout->setStretch(2, 1);
	layout->setStretch(3, 1);

	assetClassEditor->setEditable(true);
	sectorEditor->setEditable(true);

	symbolEditor->setPlaceholderText("Symbol");
	nameEditor->setPlaceholderText("Name");
	assetClassEditor->lineEdit()->setPlaceholderText("Asset Class");
	sectorEditor->lineEdit()->setPlaceholderText("Sector");

	static QStringList assetClasses = {
		tr("Equities"),
		tr("Fixed Income"),
		tr("Cash Equivalents"),
	};

	static QStringList sectors = {
		tr("Technology"),
		tr("Health Care"),
		tr("Financials"),
		tr("Real Estate"),
		tr("Energy"),
		tr("Materials"),
		tr("Consumer Discretionary"),
		tr("Industrials"),
		tr("Utilities"),
		tr("Consumer Staples"),
		tr("Telecommunication"),
		tr("Other"),
	};

	assetClassEditor->addItems(assetClasses);
	sectorEditor->addItems(sectors);

	assetClassEditor->setCurrentIndex(-1);
	sectorEditor->setCurrentIndex(-1);

	auto* validator = new SecuritySymbolValidator;
	symbolEditor->setValidator(validator);
	validator->setParent(symbolEditor); // Prevent memory leak
}

pvui::SecurityPageWidget::SecurityPageWidget(const pvui::DataFileManager& dataFileManager, QWidget* parent) : PageWidget(parent), dataFileManager_(dataFileManager)
{
	setTitle(tr("Securities"));

	auto* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(table);
	mainLayout->addWidget(insertionWidget);

	setContent(mainLayout);

	QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, [&](pv::DataFile& dataFile) {
		table->setModel(new models::SecurityModel(dataFile));
	});
}

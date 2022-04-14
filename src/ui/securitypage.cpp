#include <QHeaderView>
#include "securitypage.h"
#include "securitymodel.h"

constexpr int maximumSymbolLength = 10;
static const QRegularExpression invalidSymbolRegularExpression = QRegularExpression("[^A-Z0-9.]");

pvui::SecurityInsertionWidget::SecurityInsertionWidget(pvui::DataFileManager& dataFileManager, QWidget * parent) : QWidget(parent), dataFileManager(dataFileManager)
{
	layout->addWidget(symbolEditor, 1);
	layout->addWidget(nameEditor, 1);
	layout->addWidget(assetClassEditor, 1);
	layout->addWidget(sectorEditor, 1);

	assetClassEditor->setEditable(true);
	sectorEditor->setEditable(true);

	symbolEditor->setPlaceholderText("Symbol");
	nameEditor->setPlaceholderText("Name");
	assetClassEditor->lineEdit()->setPlaceholderText("Asset Class");
	sectorEditor->lineEdit()->setPlaceholderText("Sector");

	static const QSizePolicy sizePolicy = { QSizePolicy::Ignored, QSizePolicy::Preferred };

	symbolEditor->setSizePolicy(sizePolicy);
	nameEditor->setSizePolicy(sizePolicy);
	assetClassEditor->setSizePolicy(sizePolicy);
	sectorEditor->setSizePolicy(sizePolicy);

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

	auto* validator = new SecuritySymbolValidator;
	symbolEditor->setValidator(validator);
	validator->setParent(symbolEditor); // Prevent memory leak

	QObject::connect(symbolEditor, &QLineEdit::returnPressed, this, &SecurityInsertionWidget::submit);
	QObject::connect(nameEditor, &QLineEdit::returnPressed, this, &SecurityInsertionWidget::submit);
	QObject::connect(assetClassEditor->lineEdit(), &QLineEdit::returnPressed, this, &SecurityInsertionWidget::submit);
	QObject::connect(sectorEditor->lineEdit(), &QLineEdit::returnPressed, this, &SecurityInsertionWidget::submit);

	reset();
}

void pvui::SecurityInsertionWidget::reset()
{
	symbolEditor->setText("");
	nameEditor->setText("");
	assetClassEditor->setCurrentIndex(-1);
	sectorEditor->setCurrentIndex(-1);

	symbolEditor->setFocus();
}

pvui::SecurityPageWidget::SecurityPageWidget( pvui::DataFileManager& dataFileManager, QWidget* parent) : PageWidget(parent), dataFileManager_(dataFileManager)
{
	setTitle(tr("Securities"));

	auto* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(table);
	mainLayout->addWidget(insertionWidget);

	setContent(mainLayout);

	QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, [&](pv::DataFile& dataFile) {
		tableModel->setSourceModel(new models::SecurityModel(dataFile));
	});
	tableModel->setSourceModel(new models::SecurityModel(dataFileManager_.dataFile()));
	tableModel->sort(0, Qt::AscendingOrder);

	table->setSortingEnabled(true);
	table->setModel(tableModel);
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table->verticalHeader()->hide();
	table->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
}

void pvui::SecurityInsertionWidget::submit()
{
	QString symbol = symbolEditor->text();
	QString name = nameEditor->text().trimmed();
	QString assetClass = assetClassEditor->lineEdit()->text().trimmed();
	QString sector = sectorEditor->lineEdit()->text().trimmed();

	if (symbol.isEmpty() || name.isEmpty() || assetClass.isEmpty() || sector.isEmpty()) {
		return;
	}

	
	dataFileManager.dataFile().addSecurity(
		symbol.toStdString(),
		name.toStdString(),
		assetClass.toStdString(),
		sector.toStdString()
	);

	reset();
}

QValidator::State pvui::SecuritySymbolValidator::validate(QString& input, int& pos) const
{
	input = input.trimmed().toUpper();
	if (input.length() > maximumSymbolLength) return QValidator::State::Invalid;
	return input.contains(invalidSymbolRegularExpression) ? QValidator::State::Invalid : QValidator::State::Acceptable;
}

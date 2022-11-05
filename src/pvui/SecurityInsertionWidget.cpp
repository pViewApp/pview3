#include "SecurityInsertionWidget.h"
#include "SecurityUtils.h"
#include "pv/DataFile.h"
#include "pvui/DataFileManager.h"
#include <QShortcut>

namespace pvui {
namespace controls {
SecurityInsertionWidget::SecurityInsertionWidget(DataFileManager& dataFileManager, QWidget* parent)
    : QWidget(parent), dataFileManager_(dataFileManager) {
  layout->addWidget(symbolEditor, 1);
  layout->addWidget(nameEditor, 1);
  layout->addWidget(assetClassEditor, 1);
  layout->addWidget(sectorEditor, 1);
  layout->setMargin(0);

  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(symbolEditor);

  assetClassEditor->setEditable(true);
  sectorEditor->setEditable(true);

  symbolEditor->setPlaceholderText("Symbol");
  nameEditor->setPlaceholderText("Name");
  assetClassEditor->lineEdit()->setPlaceholderText("Asset Class");
  sectorEditor->lineEdit()->setPlaceholderText("Sector");

  static const QSizePolicy sizePolicy = {QSizePolicy::Ignored, QSizePolicy::Preferred};

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

  auto* validator = new util::SecuritySymbolValidator;
  symbolEditor->setValidator(validator);
  validator->setParent(symbolEditor); // Prevent memory leak

  QShortcut* returnShortcut = new QShortcut(Qt::Key_Return, this);
  QShortcut* enterShortcut = new QShortcut(Qt::Key_Enter, this);

  QObject::connect(returnShortcut, &QShortcut::activated, this, &SecurityInsertionWidget::submit);
  QObject::connect(enterShortcut, &QShortcut::activated, this, &SecurityInsertionWidget::submit);

  QObject::connect(&dataFileManager_, &DataFileManager::dataFileChanged, this, &SecurityInsertionWidget::handleDataFileChanged);

  handleDataFileChanged();
}

void SecurityInsertionWidget::handleDataFileChanged() {
  reset();
  setEnabled(dataFileManager_.has());
}

void SecurityInsertionWidget::reset() {
  symbolEditor->setText("");
  nameEditor->setText("");
  assetClassEditor->setCurrentIndex(-1);
  sectorEditor->setCurrentIndex(-1);
}

bool SecurityInsertionWidget::submit() {
  QString symbol = symbolEditor->text();
  QString name = nameEditor->text().trimmed();
  QString assetClass = assetClassEditor->lineEdit()->text().trimmed();
  QString sector = sectorEditor->lineEdit()->text().trimmed();

  if (symbol.isEmpty() || name.isEmpty() || assetClass.isEmpty() || sector.isEmpty()) {
    return false;
  }

  if (dataFileManager_->addSecurity(symbol.toStdString(), name.toStdString(), assetClass.toStdString(),
                                               sector.toStdString()) != pv::ResultCode::Ok)
    return false; // Failed
  reset();
  symbolEditor->setFocus();

  emit submitted(dataFileManager_->lastInsertedId());

  return true;
}

} // namespace controls
} // namespace pvui

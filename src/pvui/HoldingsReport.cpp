#include "HoldingsReport.h"
#include "FormatUtils.h"
#include "pv/Algorithms.h"
#include "pv/Security.h"
#include <QHeaderView>
#include <QShowEvent>
#include <QSpacerItem>
#include <limits>

namespace {
constexpr char headerStateKey[] = "pv/reports/holdings/tableHeaderState";
}

namespace pvui {
namespace reports {

HoldingsReport::HoldingsReport(const DataFileManager& manager, QWidget* parent)
    : Report(tr("Holdings"), manager, parent) {
  setContent(layout);
  QObject::connect(&manager, &DataFileManager::dataFileChanged, this, &HoldingsReport::setDataFile);
  setDataFile(*manager);
  table->setModel(&proxyModel);
  table->verticalHeader()->hide();
  table->setSortingEnabled(true);
  table->horizontalHeader()->setSectionsMovable(true);

  layout->addWidget(table);

  // Setup summary

  layout->addWidget(summaryGroupBox);
  summaryGroupBox->setLayout(summaryLayout);
  summaryGroupBox->setTitle(tr("Subtotal"));

  summaryLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Maximum));
  summaryLayout->addWidget(summaryCostBasisLabel);
  summaryLayout->addWidget(summaryMarketValueLabel);
  summaryLayout->addWidget(summaryIncomeLabel);

  summaryGroupBox->setAlignment(Qt::AlignTrailing);

  // Setup table header
  table->horizontalHeader()->setMinimumSectionSize(80);
  if (settings.contains(headerStateKey)) {
    table->horizontalHeader()->restoreState(settings.value(headerStateKey).toByteArray());
  }
  // Save state when the header is changed
  // clang-format off
  QObject::connect(table->horizontalHeader(), &QHeaderView::sectionResized, this,
                   [&] {
    settings.setValue(QString::fromUtf8(headerStateKey), table->horizontalHeader()->saveState());
  });
}

void HoldingsReport::reload() noexcept {
  populateSummary();

  if (!settings.contains(QString::fromUtf8(headerStateKey))) {
    table->resizeColumnsToContents();
  }
}

void HoldingsReport::setDataFile(const pv::DataFile& dataFile) {
  model = std::make_unique<models::HoldingsModel>(dataFile);
  proxyModel.setSourceModel(model.get());
}

void HoldingsReport::populateSummary() {
  static QString summaryCostBasisLabelText = QString::fromUtf8("<strong>%1</strong> %2").arg(tr("Cost Basis:"));
  static QString summaryMarketValueLabelText = QString::fromUtf8("<strong>%1</strong> %2").arg(tr("Market Value:"));
  static QString summaryIncomeLabelText = QString::fromUtf8("<strong>%1</strong> %2").arg(tr("Income (All Time):"));

  pv::Decimal costBasis = 0;
  pv::Decimal marketValue = 0;
  pv::Decimal income = 0;

  for (const pv::Security& security : dataFile().securities()) {
    costBasis += pv::algorithms::costBasis(security);
    marketValue += pv::algorithms::marketValue(security).value_or(0);
    income += pv::algorithms::totalIncome(security);
  }

  summaryCostBasisLabel->setText(summaryCostBasisLabelText.arg(util::formatMoney(costBasis)));
  summaryMarketValueLabel->setText(summaryMarketValueLabelText.arg(util::formatMoney(marketValue)));
  summaryIncomeLabel->setText(summaryIncomeLabelText.arg(util::formatMoney(income)));
}

} // namespace reports
} // namespace pvui

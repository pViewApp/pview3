#include "HoldingsReport.h"
#include "FormatUtils.h"
#include "pv/Algorithms.h"
#include "DateUtils.h"
#include <sqlite3.h>
#include "pv/Integer64.h"
#include "pv/Security.h"
#include "pvui/DataFileManager.h"
#include <QApplication>
#include <QHeaderView>
#include <QShowEvent>
#include <QSpacerItem>
#include <cassert>

namespace {
constexpr char headerStateKey[] = "pv/reports/holdings/tableHeaderState";
}

namespace pvui {
namespace reports {

HoldingsReport::HoldingsReport(DataFileManager& manager, QWidget* parent) : Report(tr("Holdings"), manager, parent) {
  QObject::connect(&manager, &DataFileManager::dataFileChanged, this, &HoldingsReport::handleDataFileChanged);
  table->setModel(&proxyModel);
  table->verticalHeader()->hide();
  table->setSortingEnabled(true);
  table->horizontalHeader()->setSectionsMovable(true);

  layout()->addWidget(table);

  // Setup summary

  layout()->addWidget(summaryGroupBox);
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

    handleDataFileChanged();
}

void HoldingsReport::reload() noexcept {
  populateSummary();

  if (!settings.contains(QString::fromUtf8(headerStateKey))) {
    table->resizeColumnsToContents();
  }
}

void HoldingsReport::handleDataFileChanged() {
  model = dataFileManager.has() ? std::make_unique<models::HoldingsModel>(*dataFileManager) : nullptr;
  proxyModel.setSourceModel(model.get());
}

void HoldingsReport::populateSummary() {
  assert(dataFileManager.has());
  static QString summaryCostBasisLabelText = QString::fromUtf8("<strong>%1</strong> %2").arg(tr("Cost Basis:"));
  static QString summaryMarketValueLabelText = QString::fromUtf8("<strong>%1</strong> %2").arg(tr("Market Value:"));
  static QString summaryIncomeLabelText = QString::fromUtf8("<strong>%1</strong> %2").arg(tr("Income (All Time):"));

  pv::i64 costBasis = 0;
  pv::i64 marketValue = 0;
  pv::i64 income = 0;

  auto query = dataFileManager->query("SELECT Id FROM Securities");
  while (sqlite3_step(&*query) == SQLITE_ROW) {
    pv::i64 security = sqlite3_column_int64(&*query, 0);
    costBasis += pv::algorithms::costBasis(*dataFileManager, security, currentEpochDate());
    marketValue += pv::algorithms::marketValue(*dataFileManager, security, currentEpochDate()).value_or(0);
    income += pv::algorithms::totalIncome(*dataFileManager, security, currentEpochDate());
  }

  summaryCostBasisLabel->setText(summaryCostBasisLabelText.arg(util::formatMoney(costBasis)));
  summaryMarketValueLabel->setText(summaryMarketValueLabelText.arg(util::formatMoney(marketValue)));
  summaryIncomeLabel->setText(summaryIncomeLabelText.arg(util::formatMoney(income)));
}

} // namespace reports
} // namespace pvui

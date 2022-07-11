#include "SecurityPage.h"
#include "SecurityModel.h"
#include <Qt>
#include "SecurityPriceDialog.h"
#include "DateUtils.h"
#include <cassert>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <sqlite3.h>
#include <QMetaObject>
#include "SecurityUtils.h"
#include "pv/Security.h"
#include "pvui/SecurityInsertionWidget.h"
#include <QCheckBox>
#include <QHeaderView>
#include <QMessageBox>
#include <optional>
#include <QStringLiteral>
#include <QDialog>
#include <QDateEdit>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>

namespace pvui {
namespace {

enum class OnConflictBehaviour : int { SKIP, REPLACE };

constexpr OnConflictBehaviour defaultOnConflictBehaviour = OnConflictBehaviour::SKIP;

class AdvancedSecurityPriceDownloadDialog : public QDialog {
private:
  QSettings settings;

  QVBoxLayout mainLayout;
  QFormLayout formLayout;

  QLabel durationLabel = QLabel(tr("Update Prices For:"));
  QSpinBox durationEditor = QSpinBox();

  QLabel onConflictLabel = QLabel(tr("On Conflict:"));
  QComboBox onConflictEditor = QComboBox();

  QDialogButtonBox buttonBox = QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

public:
  AdvancedSecurityPriceDownloadDialog(QWidget* parent = nullptr) : QDialog(parent) {
    setWindowTitle(tr("Update Security Prices"));
    settings.beginGroup(QStringLiteral("AdvancedSecurityPriceDownloadDialog"));

    setLayout(&mainLayout);

    mainLayout.addLayout(&formLayout);
    mainLayout.addWidget(&buttonBox);

    formLayout.addRow(&durationLabel, &durationEditor);
    formLayout.addRow(&onConflictLabel, &onConflictEditor);

    durationEditor.setPrefix(tr("Last "));
    durationEditor.setSuffix(tr(" Day(s)"));
    durationEditor.setMinimum(0);
    durationEditor.setMaximum(365 * 1000); // A millenium!
    durationEditor.setValue(settings.value(QStringLiteral("Duration"), 30).toInt());

    onConflictEditor.setEditable(false);
    onConflictEditor.addItem(tr("Replace"), QVariant(static_cast<int>(OnConflictBehaviour::REPLACE)));
    onConflictEditor.addItem(tr("Skip"), QVariant(static_cast<int>(OnConflictBehaviour::SKIP)));

    onConflictEditor.setCurrentIndex(
        settings.value(QStringLiteral("OnConflictBehaviour"), QVariant(static_cast<int>(defaultOnConflictBehaviour)))
            .toInt());

    // Disable Resizing
    setSizeGripEnabled(false);
    mainLayout.setSizeConstraint(QLayout::SetFixedSize);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, this, [this]() {
      accept();
      settings.setValue(QStringLiteral("Duration"), duration());
      settings.setValue(QStringLiteral("OnConflictBehaviour"), static_cast<int>(onConflictBehaviour()));
    });
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, this, &AdvancedSecurityPriceDownloadDialog::reject);
  }

  int duration() const noexcept { return durationEditor.value(); }

  OnConflictBehaviour onConflictBehaviour() const noexcept {
    return static_cast<OnConflictBehaviour>(onConflictEditor.currentData().toInt());
  }
};

} // namespace

SecurityPageWidget::SecurityPageWidget(DataFileManager& dataFileManager, QWidget* parent)
    : PageWidget(parent), dataFileManager_(dataFileManager) {
  settings.beginGroup(QStringLiteral("SecurityPage"));
  setTitle(tr("Securities"));

  // Setup layout
  layout()->addWidget(table);
  layout()->addWidget(insertionWidget);

  // Setup toolbar & table
  setupActions();

  // Setup table
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this,
                   &SecurityPageWidget::handleDataFileChanged);
  if (currentPriceDownload != nullptr) {
    currentPriceDownload->abort();
  }
  QObject::connect(insertionWidget, &controls::SecurityInsertionWidget::submitted, this,
                   &SecurityPageWidget::handleSecuritySubmitted);
  QObject::connect(
      &proxyModel, &QSortFilterProxyModel::sourceModelChanged, this,
      [this] {
        auto rc = table->model()->rowCount();
        (void)rc;
        table->update();
        table->updateGeometry();
        table->scrollToTop();
        table->scrollToBottom();
      },
      Qt::QueuedConnection);

  proxyModel.sort(0, Qt::AscendingOrder);
  table->setSortingEnabled(true);
  table->setModel(&proxyModel);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->hide();
  table->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
  table->setSelectionMode(QTableView::SingleSelection);
  QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&] {
    std::optional<pv::i64> security = currentSelectedSecurity();
    bool enabled = security.has_value();
    securityInfoAction.setEnabled(enabled);
    deleteSecurityAction.setEnabled(enabled);
    setToolBarLabel(security);
  });

  handleDataFileChanged();
}

void SecurityPageWidget::resetSecurityPriceUpdateDialog() {
  securityPriceUpdateDialog.setText("");
  securityPriceUpdateDialog.setCheckBox(nullptr);
}

void SecurityPageWidget::setupActions() {
  toolBar_->setObjectName(QString::fromUtf8("securityPageToolBar"));
  toolBar_->setWindowTitle(tr("Securities"));

  toolBar_->addWidget(toolBarTitleLabel);
  toolBarTitleLabel->setTextFormat(Qt::TextFormat::PlainText); // Disable HTML
  setToolBarLabel(std::nullopt);

  toolBar_->addAction(&securityInfoAction);
  toolBar_->addAction(&deleteSecurityAction);
  toolBar_->addAction(&updateSecurityPriceAction);
  toolBar_->addAction(&advancedUpdateSecurityPriceAction);

  securityInfoAction.setEnabled(false);
  deleteSecurityAction.setEnabled(false);
  deleteSecurityAction.setShortcut(QKeySequence::Delete);

  QObject::connect(&securityInfoAction, &QAction::triggered, this, [&]() {
    assert(dataFileManager_.has());
    std::optional<pv::i64> security = currentSelectedSecurity();
    if (!security.has_value())
      return;

    dialogs::SecurityPriceDialog* dialog = new dialogs::SecurityPriceDialog(dataFileManager_, *security);
    dialog->exec();
  });

  QObject::connect(&deleteSecurityAction, &QAction::triggered, this, [&]() {
    assert(dataFileManager_.has());
    std::optional<pv::i64> security = currentSelectedSecurity();

    if (!security.has_value() || !showSecurityDeleteWarning()) {
      return;
    }

    dataFileManager_->removeSecurity(*security);
  });

  QObject::connect(&updateSecurityPriceAction, &QAction::triggered, this,
                   &SecurityPageWidget::beginBasicUpdateSecurityPrices);
  QObject::connect(&advancedUpdateSecurityPriceAction, &QAction::triggered, this,
                   &SecurityPageWidget::beginAdvancedUpdateSecurityPrices);

  table->addAction(&securityInfoAction);
  table->addAction(&deleteSecurityAction);
  table->addAction(&updateSecurityPriceAction);
  table->addAction(&advancedUpdateSecurityPriceAction);
  table->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void SecurityPageWidget::handleDataFileChanged() {
  model = dataFileManager_.has() ? std::make_unique<models::SecurityModel>(*dataFileManager_) : nullptr;
  proxyModel.setSourceModel(model.get());
  bool enableActions = dataFileManager_.has();
  bool enableSecurityDependentActions = enableActions && currentSelectedSecurity().has_value();
  securityInfoAction.setEnabled(enableSecurityDependentActions);
  deleteSecurityAction.setEnabled(enableSecurityDependentActions);
  updateSecurityPriceAction.setEnabled(enableActions);
  advancedUpdateSecurityPriceAction.setEnabled(enableActions);

  setToolBarLabel(std::nullopt);
}

void SecurityPageWidget::handleSecuritySubmitted(pv::i64 security) {
  // Select the new security in the table
  QModelIndex index = proxyModel.mapFromSource(model->index(model->rowOfSecurity(security), 0));
  table->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  table->scrollTo(index);
}

bool SecurityPageWidget::showSecurityDeleteWarning() {
  if (!settings.value(QStringLiteral("WarnOnSecurityDeletion"), true).toBool()) {
    return true;
  }

  std::optional currentSecurity = currentSelectedSecurity();

  if (!dataFileManager_.has() || !currentSecurity) {
    return false;
  }

  QString text = tr("<html>Are you sure you want to delete <b>%1</b>? This cannot be undone.</html>")
                     .arg(QString::fromStdString(pv::security::symbol(*dataFileManager_, *currentSecurity)));
  QMessageBox* warning = new QMessageBox(QMessageBox::Icon::Question, tr("Delete Security?"), text,
                                         QMessageBox::Yes | QMessageBox::Cancel, this);
  QCheckBox* dontShowAgain = new QCheckBox(tr("&Don't show this again"));
  dontShowAgain->setChecked(false);
  warning->setCheckBox(dontShowAgain);
  warning->setAttribute(Qt::WA_DeleteOnClose);
  QObject::connect(warning, &QMessageBox::accepted, this,
                   [this, dontShowAgain]() { settings.setValue(QStringLiteral("WarnOnSecurityDeletion"), !dontShowAgain->isChecked()); });
  return warning->exec() == QMessageBox::Yes;
}

std::optional<pv::i64> SecurityPageWidget::currentSelectedSecurity() {
  QItemSelection selection = table->selectionModel()->selection();
  if (selection.isEmpty())
    return std::nullopt;

  QItemSelectionRange range = selection.first();

  if (range.isEmpty())
    return std::nullopt;

  QModelIndex index = range.topLeft();
  return model->securityOfRow(proxyModel.mapToSource(index).row());
}

void SecurityPageWidget::setToolBarLabel(std::optional<pv::i64> security) {
  static QString format = QString::fromUtf8("%1: ");
  if (security.has_value()) {
    toolBarTitleLabel->setText(format.arg(QString::fromStdString(pv::security::name(*dataFileManager_, *security))));
    toolBarTitleLabel->setEnabled(true);
    QFont bold = font();
    bold.setBold(true);
    toolBarTitleLabel->setFont(bold); // Use bold font
  } else {
    toolBarTitleLabel->setText(format.arg(QString::fromUtf8("(No Security Selected)")));
    toolBarTitleLabel->setDisabled(true);
    toolBarTitleLabel->setFont(font()); // Use normal font
  }
}

void SecurityPageWidget::beginUpdateSecurityPrices(QDate begin, int onConflictBehaviour) {
  if (currentPriceDownload != nullptr) {
    return; // Only 1 download at a time
  }

  std::optional<pv::i64> currentSecurity = currentSelectedSecurity();

  auto endDate = QDate::currentDate();

  QStringList symbols;

  if (currentSecurity.has_value()) {
    symbols.append(QString::fromStdString(pv::security::symbol(*dataFileManager_, *currentSecurity)));
  } else {
    auto listSecuritiesQuery = dataFileManager_->query("SELECT Id From Securities");
    while (sqlite3_step(&*listSecuritiesQuery) == SQLITE_ROW) {
      std::string symbol = pv::security::symbol(*dataFileManager_, sqlite3_column_int64(&*listSecuritiesQuery, 0));
      symbols += QString::fromStdString(symbol);
    }
  }

  currentPriceDownload = priceDownloader_.download(symbols, begin, endDate, this);
  currentPriceDownload->setParent(this);

  QObject::connect(currentPriceDownload, &SecurityPriceDownload::success, this,
                   [=](const std::map<QDate, pv::i64>& data, QString symbol) {
                     updateSecurityPrices(data, symbol, onConflictBehaviour);
                   });
  QObject::connect(currentPriceDownload, &SecurityPriceDownload::error, this,
                   &SecurityPageWidget::updateSecurityPricesError);
  QObject::connect(currentPriceDownload, &SecurityPriceDownload::complete, this,
                   &SecurityPageWidget::endUpdateSecurityPrices);
}

void SecurityPageWidget::beginBasicUpdateSecurityPrices() {
  beginUpdateSecurityPrices(QDate::currentDate().addMonths(-3), static_cast<int>(defaultOnConflictBehaviour));
}

void SecurityPageWidget::beginAdvancedUpdateSecurityPrices() {
  AdvancedSecurityPriceDownloadDialog* dialog = new AdvancedSecurityPriceDownloadDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  QObject::connect(dialog, &AdvancedSecurityPriceDownloadDialog::accepted, this, [this, dialog]() {
    if (dialog->duration() == 0) {
      return;
    }
    beginUpdateSecurityPrices(QDate::currentDate().addDays(-(dialog->duration())),
                              static_cast<int>(dialog->onConflictBehaviour()));
  });
  dialog->open();
}

void SecurityPageWidget::updateSecurityPrices(const std::map<QDate, pv::i64>& prices, QString symbol,
                                              int onConflictBehaviour) {
  if (!dataFileManager_.has()) {
    return;
  }

  pv::i64 security = *pv::security::securityForSymbol(*dataFileManager_, symbol.toStdString());

  bool transactionCreated = dataFileManager_->beginTransaction() == pv::ResultCode::OK;
  for (const auto& pair : prices) {
    pv::i64 date = toEpochDate(pair.first);
    if (static_cast<OnConflictBehaviour>(onConflictBehaviour) == OnConflictBehaviour::REPLACE ||
        !pv::security::price(*dataFileManager_, security, date).has_value()) {
      // Only do it if no existing price on pvDate
      dataFileManager_->setSecurityPrice(security, date, pair.second);
    }
  }
  if (transactionCreated) {
    dataFileManager_->commitTransaction();
  }
}

void SecurityPageWidget::updateSecurityPricesError(QNetworkReply::NetworkError err, QString symbol) {
  assert(err != QNetworkReply::NoError && "No error occured, but still called error slot?");

  if (err == QNetworkReply::ContentNotFoundError) {
    failedSecurityDownloadsSymbols += symbol;
    return;
  }

  currentPriceDownload->abort();
  resetSecurityPriceUpdateDialog();
  if (err == QNetworkReply::TimeoutError) {
    securityPriceUpdateDialog.setText(tr("The connection timed out."));
  } else if (err == QNetworkReply::HostNotFoundError || err == QNetworkReply::UnknownNetworkError) {
    securityPriceUpdateDialog.setText(tr("pView could not connect to the internet."));
  } else {
    securityPriceUpdateDialog.setText(tr("Sorry, a network error occured."));
    securityPriceUpdateDialog.setDetailedText(tr("Error Code: 0x%1").arg(err, 0, 16));
  }
  securityPriceUpdateDialog.show();
}

void SecurityPageWidget::endUpdateSecurityPrices() {
  delete currentPriceDownload;
  currentPriceDownload = nullptr;

  bool suppressFailedDownloadDialog =
      !settings.value(QStringLiteral("WarnOnSecurityPriceDownloadFailure"), true).toBool();

  if (!failedSecurityDownloadsSymbols.empty() && !suppressFailedDownloadDialog) {
    resetSecurityPriceUpdateDialog();
    QString base = tr("<html>pView couldn't download prices for the following security(s):<ul>", nullptr,
                      failedSecurityDownloadsSymbols.size());
    const auto& failedDownloads = failedSecurityDownloadsSymbols; // const reference to avoid clazy complaining
    for (const auto& symbol : failedDownloads) {
      base.append("<li>");
      base.append(symbol.toHtmlEscaped());
      base.append("</li>");
    }
    base.append("</ul></html>");

    QCheckBox* checkBox = new QCheckBox(tr("&Don't show this again"));
    checkBox->setChecked(false);
    QObject::connect(&securityPriceUpdateDialog, &QMessageBox::finished, this, [=](int result) {
      if (result == QDialog::Accepted) {
        settings.setValue(QStringLiteral("WarnOnSecurityPriceDownloadFailure"), !checkBox->isChecked());
      }
    });

    securityPriceUpdateDialog.setCheckBox(checkBox);
    securityPriceUpdateDialog.setText(base);
    securityPriceUpdateDialog.show();
  }

  failedSecurityDownloadsSymbols.clear();
}
} // namespace pvui

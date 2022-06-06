#include "MainWindow.h"
#include "DataFileManager.h"
#include <QApplication>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QString>
#include <functional>

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

namespace {
void hideToolBar(QToolBar* toolBar) {
  if (toolBar != nullptr) {
    toolBar->hide();
  }
}
} // namespace

void pvui::MainWindow::setupToolBars() {
  // Disable the hide toolbar context menu
  setContextMenuPolicy(Qt::NoContextMenu);

  // Setup the main toolbar
  mainToolBar->setObjectName(QString::fromUtf8("mainPageToolBar"));
  addToolBar(mainToolBar);
  mainToolBar->setWindowTitle(tr("Accounts"));
  mainToolBar->addAction(&newAccountAction);

  // Setup toolbars for each of the pages
  setupToolBar(accountPage->toolBar());
  for (auto* report : reports) {
    setupToolBar(report->toolBar());
  }
  setupToolBar(securityPage->toolBar());
}

// Hides all toolbars, needed because restoreState() restores some toolbars that should be hidden on startup
void pvui::MainWindow::hideToolBars() {
  for (auto* report : reports) {
    hideToolBar(report->toolBar());
  }

  hideToolBar(securityPage->toolBar());
}

void pvui::MainWindow::pageChanged() {
  // clear the actions
  QList actions(navigationWidget->actions());
  for (const auto& action : actions) {
    navigationWidget->removeAction(action);
  }

  QModelIndex indexOfNewPage = navigationWidget->selectionModel()->currentIndex();

  PageWidget* newPage = nullptr;
  // Create context menu
  if (navigationModel.isAccountsHeader(indexOfNewPage)) {
    navigationWidget->addAction(&newAccountAction);
  } else if (navigationModel.isAccountPage(indexOfNewPage)) {
    navigationWidget->addAction(&newAccountAction);
    navigationWidget->addAction(&deleteAccountAction);
  }

  if (navigationModel.isAccountPage(indexOfNewPage)) {
    newPage = accountPage;
    accountPage->setAccount(&*dataFileManager, navigationModel.accountFromIndex(indexOfNewPage));

  } else if (navigationModel.isSecuritiesPage(indexOfNewPage)) {
    newPage = securityPage;
  } else if (navigationModel.isReportPage(indexOfNewPage)) {
    auto* report = const_cast<Report*>(navigationModel.reportFromIndex(indexOfNewPage));
    newPage = report;
    report->reload();
  } else {
    contentLayout->setCurrentWidget(noPageOpen);
  }

  if (currentToolBar != nullptr) {
    currentToolBar->hide();
    currentToolBar = nullptr;
  }
  if (newPage != nullptr) {
    contentLayout->setCurrentWidget(newPage);
    currentToolBar = newPage->toolBar();
    if (currentToolBar != nullptr) {
      currentToolBar->show();
    }
  }
}

void pvui::MainWindow::closeEvent(QCloseEvent* event) {
  QSettings settings;
  settings.setValue("window/state", saveState());
  settings.setValue("window/geometry", saveGeometry());
  QMainWindow::closeEvent(event);
}

pvui::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  QWidget* centralWidget = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout;

  auto* splitter = new QSplitter(Qt::Orientation::Horizontal);

  centralWidget->setLayout(layout);

  layout->addWidget(splitter);
  splitter->addWidget(navigationWidget);
  splitter->addWidget(content);

  setCentralWidget(centralWidget);
  resize(windowWidth, windowHeight);
  setWindowTitle(QApplication::translate("windowTitle", "pView"));

  setupToolBars();
  setupNavigation();
  setupMenuBar();

  // Restore state
  QSettings settings;
  if (settings.contains("window/state")) {
    restoreState(settings.value("window/state").toByteArray());
  }
  if (settings.contains("window/geometry")) {
    restoreGeometry(settings.value("window/geometry").toByteArray());
  }

  hideToolBars();
}

void pvui::MainWindow::setupToolBar(QToolBar* toolBar) {
  if (toolBar == nullptr)
    return;
  addToolBar(toolBar);
  toolBar->hide();
}

void pvui::MainWindow::setupMenuBar() {
  auto* fileMenu = menuBar()->addMenu("&File");
  {
    auto* openItem = new QAction("&Open");
    openItem->setShortcut(QKeySequence::Open);

    auto* newItem = new QAction("&New");
    newItem->setShortcut(QKeySequence::New);

    fileMenu->addActions({openItem, newItem});
  }

  auto* accountMenu = menuBar()->addMenu("&Accounts");
  {
    auto* newItem = new QAction("&New");
    QObject::connect(newItem, &QAction::triggered, this, &MainWindow::showAddAccountDialog);

    accountMenu->addActions({newItem});
  }
}

void pvui::MainWindow::setupNavigation() {

  contentLayout->addWidget(accountPage);
  contentLayout->addWidget(securityPage);
  contentLayout->addWidget(noPageOpen);

  for (auto* report : reports) {
    contentLayout->addWidget(report);
  }
  navigationModel.addReports(reports);
  navigationWidget->setExpanded(navigationModel.reportsHeader(), true);

  contentLayout->setCurrentWidget(noPageOpen);

  noPageOpen->setAlignment(Qt::AlignCenter);

  navigationWidget->setModel(&navigationModel);
  navigationWidget->setHeaderHidden(true);
  navigationWidget->setMaximumWidth(750);

  navigationWidget->setSelectionMode(QTreeView::SelectionMode::SingleSelection);

  // Make the content stretch more than the navigation
  QSizePolicy policy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  policy.setHorizontalStretch(7);
  navigationWidget->setSizePolicy(policy);
  policy.setHorizontalStretch(30);
  content->setSizePolicy(policy);

  QObject::connect(navigationWidget->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                   &MainWindow::pageChanged);

  // Setup context menu

  deleteAccountAction.setShortcut(QKeySequence::StandardKey::Delete);
  deleteAccountAction.setShortcutContext(Qt::WidgetShortcut);

  QObject::connect(&newAccountAction, &QAction::triggered, this, &MainWindow::showAddAccountDialog);
  QObject::connect(&deleteAccountAction, &QAction::triggered, this, &MainWindow::showDeleteAccountDialog);

  navigationWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void pvui::MainWindow::showDeleteAccountDialog() {
  pv::Account* account = navigationModel.accountFromIndex(navigationWidget->selectionModel()->currentIndex());

  QMessageBox::Button userResponse = QMessageBox::warning(
      this, tr("Deleting Account %1").arg(QString::fromStdString(account->name())),
      tr("Are you sure you want to delete this account and it's transactions? This cannot be undone."),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  if (userResponse == QMessageBox::Yes) {
    dataFileManager->removeAccount(*account);
  }
}

void pvui::MainWindow::showAddAccountDialog() {
  bool ok = false;
  QString text = QInputDialog::getText(this, tr("Create an Account"), tr("Account Name:"), QLineEdit::Normal, "", &ok);

  if (ok) {
    auto trimmedText = text.trimmed();
    if (!trimmedText.isEmpty()) {
      pv::Account* account = dataFile().addAccount(trimmedText.toStdString());

      navigationWidget->expand(navigationModel.accountsHeader());
      navigationWidget->selectionModel()->setCurrentIndex(navigationModel.accountToIndex(*account),
                                                          QItemSelectionModel::ClearAndSelect);
    } else {
      QMessageBox::warning(this, tr("Invalid Account Name"), tr("The account name must not be empty"));
    }
  }
}

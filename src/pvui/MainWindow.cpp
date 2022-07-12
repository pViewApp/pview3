#include "MainWindow.h"
#include <QStatusBar>
#include "DataFileManager.h"
#include "pv/DataFile.h"
#include <QStandardPaths>
#include <QApplication>
#include <sqlite3.h>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QShortcut>
#include <QString>
#include <filesystem>
#include <functional>
#include <QAction>
#include <QStringLiteral>

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

namespace {
void hideToolBar(QToolBar* toolBar) {
  if (toolBar != nullptr) {
    toolBar->hide();
  }
}

#ifdef Q_OS_MACOS
constexpr char settingsActionText[] = "&Preferences";
#else
constexpr char settingsActionText[] = "&Settings";
#endif

} // namespace

pvui::MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), settingsDialog(this), fileMenu(tr("&File")), fileNewAction(tr("&New")),
      fileOpenAction(tr("&Open")), fileSettingsAction(tr(settingsActionText)), fileQuitAction(tr("&Quit")),
      accountsMenu(tr("&Accounts")), accountsNewAction(tr("&New Account")),
      accountsDeleteAction(tr("&Delete Account")) {
  settings.beginGroup(QStringLiteral("pvui/MainWindow"));

  QWidget* centralWidget = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout;

  auto* splitter = new QSplitter(Qt::Orientation::Horizontal);

  centralWidget->setLayout(layout);

  layout->addWidget(splitter);
  splitter->addWidget(navigationWidget);
  splitter->addWidget(content);

  setCentralWidget(centralWidget);
  resize(windowWidth, windowHeight);

  // Setup title
  updateWindowFileLocation();
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, &MainWindow::handleDataFileChanged);

  setupToolBars();
  setupNavigation();
  setupMenuBar();
  statusBar()->addWidget(statusBarLabel);
  statusBar()->setSizeGripEnabled(false);

  // Restore state
  if (settings.contains("state")) {
    restoreState(settings.value("state").toByteArray());
  }
  if (settings.contains("geometry")) {
    restoreGeometry(settings.value("geometry").toByteArray());
  }

  hideToolBars();
  handleDataFileChanged();

  if (settings.contains(QStringLiteral("lastOpenedFile"))) {
    try {
      fileOpen_(settings.value(QStringLiteral("lastOpenedFile")).toString().toStdString());
    } catch(...) {
      // Ignore, we just don't open the file if fail
      // It would be better to log this error though, maybe do that in future
    }
  }
}

void pvui::MainWindow::handleDataFileChanged() {
  updateWindowFileLocation();

  navigationWidget->selectionModel()->clearSelection();

  navigationWidget->setEnabled(dataFileManager.has());
  accountsNewAction.setEnabled(dataFileManager.has());
  accountsDeleteAction.setEnabled(dataFileManager.has());
  accountsMenu.setEnabled(dataFileManager.has());

  noPageOpen->setText(dataFileManager.has() ? tr("No Page Open") : tr("Create a new file with <b>File>New</b>."));
}

void pvui::MainWindow::setupToolBars() {
  // Disable the hide toolbar context menu
  setContextMenuPolicy(Qt::NoContextMenu);

  // Setup the main toolbar
  mainToolBar->setObjectName(QString::fromUtf8("mainPageToolBar"));
  addToolBar(mainToolBar);
  mainToolBar->setWindowTitle(tr("Accounts"));
  mainToolBar->addAction(&accountsNewAction);

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
    navigationWidget->addAction(&accountsNewAction);
  } else if (navigationModel.isAccountPage(indexOfNewPage)) {
    navigationWidget->addAction(&accountsNewAction);
    navigationWidget->addAction(&accountsDeleteAction);
  }

  if (navigationModel.isAccountPage(indexOfNewPage)) {
    newPage = accountPage;
    accountPage->setAccount(navigationModel.accountFromIndex(indexOfNewPage));

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
  settings.setValue("state", saveState());
  settings.setValue("geometry", saveGeometry());
  QMainWindow::closeEvent(event);
}

void pvui::MainWindow::setupToolBar(QToolBar* toolBar) {
  if (toolBar == nullptr)
    return;
  addToolBar(toolBar);
  toolBar->hide();
}

void pvui::MainWindow::setupMenuBar() {
  menuBar()->addMenu(&fileMenu);
  menuBar()->addMenu(&accountsMenu);

  // File

  fileNewAction.setShortcut(QKeySequence::New);
  fileOpenAction.setShortcut(QKeySequence::Open);
  fileSettingsAction.setShortcut(QKeySequence::Preferences);
  fileQuitAction.setShortcut(QKeySequence::Quit);

  QObject::connect(&fileNewAction, &QAction::triggered, this, &MainWindow::fileNew);
  QObject::connect(&fileOpenAction, &QAction::triggered, this, &MainWindow::fileOpen);
  QObject::connect(&fileQuitAction, &QAction::triggered, this, &MainWindow::fileQuit);
  QObject::connect(&fileSettingsAction, &QAction::triggered, this, &pvui::MainWindow::toolsSettings);

  fileSettingsAction.setMenuRole(QAction::MenuRole::PreferencesRole);
  fileQuitAction.setMenuRole(QAction::MenuRole::QuitRole);

  fileMenu.addActions({&fileNewAction, &fileOpenAction});
  fileMenu.addSeparator();
  fileMenu.addAction(&fileSettingsAction);
  fileMenu.addSeparator();
  fileMenu.addAction(&fileQuitAction);

  // Accounts

  accountsMenu.addAction(&accountsNewAction);

  QObject::connect(&accountsNewAction, &QAction::triggered, this, &MainWindow::accountsNew);
  QObject::connect(&accountsDeleteAction, &QAction::triggered, this, &MainWindow::accountsDelete);

  // Tools
}

void pvui::MainWindow::fileNew() {
  QString dir = settings
                    .value(QStringLiteral("pvui/mainWindow/newDirectory"),
                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                    .toString();
  QString fileQStr =
      QFileDialog::getSaveFileName(this, tr("Open File"), dir, tr("pView Files (*.pvf);;All Files (*.*)"));
  if (fileQStr.isNull()) {
    return;
  };
  std::string file = fileQStr.toStdString();
  try {
    // delete the file if it exists already, since we want to replace it
    std::filesystem::remove(std::filesystem::path(file));
  } catch (...) {
    // failure is ok, we can still open the file
  }

  try {
    dataFileManager.setDataFile(pv::DataFile(file));
    settings.setValue(QStringLiteral("lastOpenedFile"), fileQStr);
  } catch (...) {
    QMessageBox::critical(this, tr("Failed to Create File"),
                          tr("pView couldn't create the file. Please try again later."));
  }

  updateWindowFileLocation();
}

void pvui::MainWindow::fileOpen() {
  QString dir = settings
                    .value(QStringLiteral("pvui/mainWindow/openDirectory"),
                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                    .toString();
  QString file_ = QFileDialog::getOpenFileName(this, tr("Open File"), dir, tr("pView Files (*.pvf);;All Files (*.*)"));
  if (file_.isNull()) {
    return;
  }
  std::string file = file_.toStdString();

  try {
    fileOpen_(std::move(file));
  } catch(...) {
    QString fileName = QString::fromStdString(std::filesystem::path(file).filename().string());
    QMessageBox::critical(
        this, tr("Failed to Open File"),
        tr("pView couldn't open %1. Please check that the file exists and is a valid pView file.").arg(fileName));
  }
}

void pvui::MainWindow::fileOpen_(std::string location) {
  dataFileManager.setDataFile(
      pv::DataFile(location, SQLITE_OPEN_READWRITE)); // unset SQLITE_OPEN_CREATE, because we don't want to create a
                                                      // new file if it doesn't already exist
  settings.setValue(QStringLiteral("lastOpenedFile"), QString::fromStdString(location));
  updateWindowFileLocation();
}

void pvui::MainWindow::fileQuit() { close(); }

void pvui::MainWindow::accountsDelete() {
  pv::i64 account = navigationModel.accountFromIndex(navigationWidget->selectionModel()->currentIndex());
  QString accountName = QString::fromStdString(pv::account::name(*dataFileManager, account)).toHtmlEscaped();
  QMessageBox::Button userResponse = QMessageBox::question(
      this, tr("Delete Account?"),
      tr("<html>Are you sure you want to delete the account <b>%1</b> and it's transactions? This cannot be undone.</html>").arg(accountName),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
  if (userResponse == QMessageBox::Yes) {
    dataFileManager->removeAccount(account);
  }
}

void pvui::MainWindow::accountsNew() {
  bool ok = false;
  QString text = QInputDialog::getText(this, tr("Create an Account"), tr("Account Name:"), QLineEdit::Normal, "", &ok);
  if (ok) {
    auto trimmedText = text.trimmed();
    if (!trimmedText.isEmpty()) {
      if (dataFileManager->addAccount(trimmedText.toStdString()) != pv::ResultCode::OK) {
        return;
      }
      pv::i64 account = dataFileManager->lastInsertedId();
      navigationWidget->expand(navigationModel.accountsHeader());
      navigationWidget->selectionModel()->setCurrentIndex(navigationModel.accountToIndex(account),
                                                          QItemSelectionModel::ClearAndSelect);
    } else {
      QMessageBox::warning(this, tr("Invalid Account Name"), tr("The account name must not be empty"));
    }
  }
}

void pvui::MainWindow::toolsSettings() {
  settingsDialog.refresh();
  settingsDialog.open();
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

  accountsDeleteAction.setShortcut(QKeySequence::Delete);
  accountsDeleteAction.setShortcutContext(Qt::WidgetShortcut);
  navigationWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void pvui::MainWindow::updateWindowFileLocation() {
  if (dataFileManager.has()) {
    std::optional dataFilePath = dataFileManager->filePath();
    std::filesystem::path filePath =
        dataFilePath.value_or(tr("Temporary File (Changes Will Not Be Saved)").toStdString());
    setWindowTitle(tr("%1 - pView").arg(QString::fromStdString(filePath.filename().string())));
    setWindowFilePath(QString::fromStdString(filePath.string()));
    statusBarLabel->setText(dataFilePath ? tr("Autosaving all changes.") : tr("Temporary File - Changes will not be saved."));
  } else {
    setWindowTitle(tr("No File Open - pView"));
    setWindowFilePath(QStringLiteral(""));
    statusBarLabel->setText(tr("No file open."));
  }
}


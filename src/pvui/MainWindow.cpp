#include "MainWindow.h"
#include "DataFileManager.h"
#include "ThemeManager.h"
#include "Version.h"
#include "pv/DataFile.h"
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QOperatingSystemVersion>
#include <QShortcut>
#include <QStandardPaths>
#include <QStatusBar>
#include <QString>
#include <QStringLiteral>
#include <QTimer>
#include <QToolBar>
#include <Qt>
#include <filesystem>
#include <functional>
#include <sqlite3.h>

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

namespace {
#ifdef Q_OS_MACOS
constexpr char settingsActionText[] = "&Preferences...";
#else
constexpr char settingsActionText[] = "&Settings...";
#endif

} // namespace

pvui::MainWindow::MainWindow(mac::WindowList& windowList, QWidget* parent)
    : QMainWindow(parent), windowList(windowList), settingsDialog(this), fileMenu(tr("&File")),
      fileNewAction(tr("&New...")), fileOpenAction(tr("&Open...")), fileSettingsAction(tr(settingsActionText)),
      fileNewWindowAction(tr("New &Window")),
#ifdef Q_OS_MACOS
      fileCloseWindowAction(tr("Close Window")),
#endif
      fileQuitAction(tr("&Quit")), accountsMenu(tr("&Accounts")), accountsNewAction(tr("&New Account...")),
      accountsDeleteAction(tr("&Delete Account")), helpMenu(tr("&Help")), helpAboutAction(tr("&About pView")), helpWebsiteAction(tr("Visit pView Website")) {
  windowList.addWindow(this);
  settings.beginGroup(QStringLiteral("MainWindow"));

  QWidget* centralWidget = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout;
  centralWidget->setLayout(layout);

  splitter.setOrientation(Qt::Horizontal);
  layout->addWidget(&splitter);
  splitter.addWidget(navigationWidget);
  splitter.addWidget(content);

  setCentralWidget(centralWidget);
  resize(windowWidth, windowHeight);

  // Setup title
  updateWindowFileLocation();
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, &MainWindow::handleDataFileChanged);

  setupNavigation();
  setupActions();
  statusBar()->addWidget(statusBarLabel);
  statusBar()->addWidget(&securityPriceDownloadProgressBar);
  statusBar()->setSizeGripEnabled(false);

  securityPriceDownloadProgressBar.setVisible(false);
  QObject::connect(securityPage, &SecurityPageWidget::securityPriceDownloadStarted, [&](int numberOfSecurities) {
    securityPriceDownloadProgressBar.setFormat(tr("Downloading Security Prices (%v/%m)"));
    securityPriceDownloadProgressBar.reset();
    securityPriceDownloadProgressBar.setRange(0, numberOfSecurities);
    securityPriceDownloadProgressBar.setVisible(true);
  });
  QObject::connect(securityPage, &SecurityPageWidget::securityPriceDownloadCompleted, [&] {
    securityPriceDownloadProgressBar.setFormat(tr("Done!"));
    QTimer::singleShot(1000, [this]() { securityPriceDownloadProgressBar.setVisible(false); });
  });
  QObject::connect(securityPage, &SecurityPageWidget::securityPriceDownloadProgressUpdated,
                   [&](int completed) { securityPriceDownloadProgressBar.setValue(completed); });

  setAcceptDrops(true);

  // Restore state
  if (settings.contains("State")) {
    restoreState(settings.value("State").toByteArray());
  }
  if (settings.contains("Geometry")) {
    restoreGeometry(settings.value("Geometry").toByteArray());
  }
  if (settings.contains("SplitterState")) {
    splitter.restoreState(settings.value("SplitterState").toByteArray());
  }

  if (settings.contains(QStringLiteral("LastOpenedFile"))) {
    try {
      fileOpen_(settings.value(QStringLiteral("LastOpenedFile")).toString().toStdString());
    } catch (...) {
      // Ignore, we just don't open the file if fail
      // It would be better to log this error though, maybe do that in future
    }
  }
  handleDataFileChanged();
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

void pvui::MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  event->setDropAction(Qt::CopyAction);
  if (event->mimeData()->hasUrls()) {
    auto urls = event->mimeData()->urls();
    if (urls.size() == 0) {
      return; // don't accept
    }
    if (urls.last().isLocalFile()) {
      event->acceptProposedAction();
    }
  }
}
void pvui::MainWindow::dropEvent(QDropEvent* event) {
  const QMimeData* mime = event->mimeData();
  if (mime->hasUrls()) {
    auto urls = mime->urls();
    if (urls.size() == 0) {
      return;
    }
    if (urls.last().isLocalFile()) {
      fileOpenWithWarning_(urls.last().toLocalFile().toStdString());
    }
  }
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

  if (newPage != nullptr) {
    contentLayout->setCurrentWidget(newPage);
  }
}

void pvui::MainWindow::closeEvent(QCloseEvent* event) {
  settings.setValue("State", saveState());
  settings.setValue("Geometry", saveGeometry());
  settings.setValue("SplitterState", splitter.saveState());
  QMainWindow::closeEvent(event);
  windowList.removeWindow(this);
}

void pvui::MainWindow::setupActions() {
  menuBar()->addMenu(&fileMenu);
  menuBar()->addMenu(&accountsMenu);
#ifdef Q_OS_MACOS
  menuBar()->addMenu(windowList.windowMenu());
#endif
  menuBar()->addMenu(&helpMenu);

  // File
  fileNewAction.setShortcut(QKeySequence::New);
  fileOpenAction.setShortcut(QKeySequence::Open);
  fileSettingsAction.setShortcut(QKeySequence::Preferences);
  fileNewWindowAction.setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_N)));
#ifdef Q_OS_MACOS
  fileCloseWindowAction.setShortcut(QKeySequence::Close);
#endif
  fileQuitAction.setShortcut(QKeySequence::Quit);

  fileNewAction.setIcon(QIcon::fromTheme("document-new"));
  fileOpenAction.setIcon(QIcon::fromTheme("document-open"));
  fileSettingsAction.setIcon(QIcon::fromTheme("preferences-desktop"));
  fileQuitAction.setIcon(QIcon::fromTheme("application-exit"));

  QObject::connect(&fileNewAction, &QAction::triggered, this, &MainWindow::fileNew);
  QObject::connect(&fileOpenAction, &QAction::triggered, this, &MainWindow::fileOpen);
  QObject::connect(&fileQuitAction, &QAction::triggered, this, &MainWindow::fileQuit);
  QObject::connect(&fileSettingsAction, &QAction::triggered, this, &pvui::MainWindow::fileSettings);
  QObject::connect(&fileNewWindowAction, &QAction::triggered, this, &pvui::MainWindow::fileNewWindow);
#ifdef Q_OS_MACOS
  QObject::connect(&fileCloseWindowAction, &QAction::triggered, this, &pvui::MainWindow::close);
#endif
  fileSettingsAction.setMenuRole(QAction::MenuRole::PreferencesRole);
  fileQuitAction.setMenuRole(QAction::MenuRole::QuitRole);

  fileMenu.addActions({&fileNewAction, &fileOpenAction});
  fileMenu.addSeparator();
  fileMenu.addAction(&fileNewWindowAction);
#ifdef Q_OS_MACOS
  fileMenu.addAction(&fileCloseWindowAction);
#endif
  fileMenu.addSeparator();
  fileMenu.addAction(&fileSettingsAction);
  fileMenu.addSeparator();
  fileMenu.addAction(&fileQuitAction);

  // Accounts
  accountsMenu.addAction(&accountsNewAction);

  QObject::connect(&accountsNewAction, &QAction::triggered, this, &MainWindow::accountsNew);
  QObject::connect(&accountsDeleteAction, &QAction::triggered, this, &MainWindow::accountsDelete);

  accountsNewAction.setIcon(QIcon::fromTheme("list-add-user"));

  // Help
  helpMenu.addActions({&helpWebsiteAction, &helpAboutAction});
  QObject::connect(&helpWebsiteAction, &QAction::triggered, this, &MainWindow::helpWebsite);
  QObject::connect(&helpAboutAction, &QAction::triggered, this, &MainWindow::helpAbout);
  helpAboutAction.setIcon(QIcon::fromTheme("help-about"));
  helpAboutAction.setMenuRole(QAction::MenuRole::AboutRole);


  // Toolbar
  QToolBar* toolbar = new QToolBar();
  toolbar->addAction(&fileNewAction);
  toolbar->addAction(&fileOpenAction);
  toolbar->addAction(&fileSettingsAction);
  toolbar->addSeparator();
  toolbar->addAction(&accountsNewAction);
  toolbar->setObjectName(QStringLiteral("pview-toolbar"));
  toolbar->setWindowTitle(tr("Show Toolbar"));
  toolbar->setMovable(false);
  menuBar()->setNativeMenuBar(true);
#ifdef Q_OS_MACOS
  toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#elif defined(Q_OS_UNIX)
  toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
#else
  toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif
  addToolBar(toolbar);
  setUnifiedTitleAndToolBarOnMac(true);
}

void pvui::MainWindow::fileNew() {
  QString dir = settings
                    .value(QStringLiteral("pvui/mainWindow/newDirectory"),
                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                    .toString();
  QString fileQStr =
      QFileDialog::getSaveFileName(this, tr("New File"), dir, tr("pView Files (*.pvf);;All Files (*.*)"));
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
    settings.setValue(QStringLiteral("LastOpenedFile"), fileQStr);
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

  fileOpenWithWarning_(file);
}

void pvui::MainWindow::fileOpen_(const std::string& location) {
  dataFileManager.setDataFile(
      pv::DataFile(location, SQLITE_OPEN_READWRITE)); // unset SQLITE_OPEN_CREATE, because we don't want to create a
                                                      // new file if it doesn't already exist
  settings.setValue(QStringLiteral("LastOpenedFile"), QString::fromStdString(location));
  contentLayout->setCurrentWidget(noPageOpen);
  updateWindowFileLocation();
}

void pvui::MainWindow::fileOpenWithWarning_(const std::string& location) noexcept {
  try {
    fileOpen_(std::move(location));
  } catch (...) {
    QString fileName = QString::fromStdString(std::filesystem::path(location).filename().string());
    QMessageBox::critical(
        this, tr("Failed to Open File"),
        tr("pView couldn't open %1. Please check that the file exists and is a valid pView file.").arg(fileName));
  }
}

void pvui::MainWindow::fileSettings() {
  settingsDialog.refresh();
  settingsDialog.open();
}

void pvui::MainWindow::fileQuit() { QCoreApplication::exit(); }

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
      if (dataFileManager->addAccount(trimmedText.toStdString()) != pv::ResultCode::Ok) {
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

void pvui::MainWindow::helpAbout() {
  QString helpContent = QStringLiteral(R"(
<html>
<h1>%1</h1>
<ul>
<li>%2</li>
<li>%3</li>
</ul>
</html>
)")
                        .arg(tr("About pView"), tr("Version: %1").arg(pvui::versionString()),
                             tr("Operating System: %1").arg(QOperatingSystemVersion::current().name()));
  QMessageBox::about(this, tr("About pView"), helpContent);
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

  QObject::connect(navigationWidget->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                   &MainWindow::pageChanged);

  // Setup context menu
  accountsDeleteAction.setShortcut(QKeySequence::Delete);
  accountsDeleteAction.setShortcutContext(Qt::WidgetShortcut);
  navigationWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

bool pvui::MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef Q_OS_WIN
  long longVal = static_cast<long>(*result);
  return pvui::ThemeManager::handleNativeEvent(eventType, message, &longVal);
#else
  (void) eventType;
  (void) message;
  (void) result;
  return false;
#endif
}

void pvui::MainWindow::updateWindowFileLocation() {
  QString titleFormat =
#ifdef Q_OS_MACOS
      QStringLiteral("%1");
#else
      tr("%1 - pView");
#endif
  if (dataFileManager.has()) {
    std::optional dataFilePath = dataFileManager->filePath();
    std::filesystem::path filePath =
        dataFilePath.value_or(tr("Temporary File (Changes Will Not Be Saved)").toStdString());
    setWindowTitle(titleFormat.arg(QString::fromStdString(filePath.filename().string())));
    setWindowFilePath(QString::fromStdString(filePath.string()));
    statusBarLabel->setText(dataFilePath ? tr("Autosaving all changes.") : tr("Temporary File - Changes will not be saved."));
  } else {
    setWindowTitle(titleFormat.arg(tr("No File Open")));
    setWindowFilePath(QString());
    statusBarLabel->setText(tr("No file open."));
  }
}
void pvui::MainWindow::fileNewWindow() {
  auto* newWindow = new MainWindow(windowList);
  newWindow->setAttribute(Qt::WA_DeleteOnClose);
  newWindow->show();
  newWindow->activateWindow();
  newWindow->raise();
}
bool pvui::MainWindow::event(QEvent* e) {
  if (e->type() == QEvent::WindowActivate) {
    windowList.setActiveWindow(this);
    return true;
  }
  return QMainWindow::event(e);
}
void pvui::MainWindow::helpWebsite() {
  QDesktopServices::openUrl(QUrl("https://pviewapp.github.io"));
}

#include "MainWindow.h"
#include "DataFileManager.h"
#include <QApplication>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QShortcut>
#include <QString>
#include <functional>

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

void pvui::MainWindow::pageChanged() {
  // clear the actions
  QList actions(navigationWidget->actions());
  for (const auto& action : actions) {
    navigationWidget->removeAction(action);
  }

  QModelIndex indexOfNewPage = navigationWidget->selectionModel()->currentIndex();

  // Create context menu
  if (navigationModel.isAccountsHeader(indexOfNewPage)) {
    navigationWidget->addAction(&newAccountAction);
  } else if (navigationModel.isAccountPage(indexOfNewPage)) {
    navigationWidget->addAction(&newAccountAction);
    navigationWidget->addAction(&deleteAccountAction);
  }

  if (navigationModel.isAccountPage(indexOfNewPage)) {
    contentLayout->setCurrentWidget(accountPage);
    accountPage->setAccount(navigationModel.mapToAccount(indexOfNewPage));

  } else if (navigationModel.isSecuritiesPage(indexOfNewPage)) {
    contentLayout->setCurrentWidget(securityPage);
  } else {
    contentLayout->setCurrentWidget(noPageOpen);
  }
}

pvui::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  QWidget* centralWidget = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout;

  auto* splitter = new QSplitter(Qt::Orientation::Horizontal);

  centralWidget->setLayout(layout);

  layout->addWidget(splitter);
  splitter->addWidget(navigationWidget);
  splitter->addWidget(content);

  content->setLayout(new QStackedLayout);

  setCentralWidget(centralWidget);
  resize(windowWidth, windowHeight);
  setWindowTitle(QApplication::translate("windowTitle", "pView"));

  setupNavigation();
  setupMenuBar();
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
  contentLayout->setCurrentWidget(noPageOpen);

  noPageOpen->setAlignment(Qt::AlignCenter);

  navigationWidget->setModel(&navigationModel);
  navigationWidget->setHeaderHidden(true);
  navigationWidget->setMaximumWidth(500);

  navigationWidget->setSelectionMode(QTreeView::SelectionMode::SingleSelection);

  QObject::connect(navigationWidget->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                   &MainWindow::pageChanged);

  // Setup context menu

  deleteAccountAction.setShortcut(QKeySequence::StandardKey::Delete);

  QObject::connect(&newAccountAction, &QAction::triggered, this, &MainWindow::showAddAccountDialog);
  QObject::connect(&deleteAccountAction, &QAction::triggered, this, &MainWindow::showDeleteAccountDialog);

  navigationWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void pvui::MainWindow::showDeleteAccountDialog() {
  pv::AccountPtr account = navigationModel.mapToAccount(navigationWidget->selectionModel()->currentIndex());

  QMessageBox::Button userResponse = QMessageBox::warning(
      this, tr("Deleting Account %1").arg(QString::fromStdString(account->name())),
      tr("Are you sure you want to delete this account and it's transactions? This cannot be undone."),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  if (userResponse == QMessageBox::Yes) {
    account->dataFile().removeAccount(account);
  }
}

void pvui::MainWindow::showAddAccountDialog() {
  bool ok = false;
  QString text = QInputDialog::getText(this, tr("Create an Account"), tr("Account Name:"), QLineEdit::Normal, "", &ok);

  if (ok) {
    auto trimmedText = text.trimmed();
    if (!trimmedText.isEmpty()) {
      pv::AccountPtr account = dataFile().addAccount(trimmedText.toStdString());

      navigationWidget->expand(navigationModel.accountsHeader());
      navigationWidget->selectionModel()->setCurrentIndex(navigationModel.mapFromAccount(account),
                                                          QItemSelectionModel::ClearAndSelect);
    } else {
      QMessageBox::warning(this, tr("Invalid Account Name"), tr("The account name must not be empty"));
    }
  }
}

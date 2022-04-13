#include <functional>

#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QLayout>
#include <QMenuBar>
#include <QLabel>

#include "mainwindow.h"
#include "datafile.h"

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

pvui::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
	navigationWidget(new QTreeView),
	content(new QWidget),
	noPageOpen(new QLabel("No Page Open")),
	navigationModel(new QStandardItemModel),
	contentLayout(new QStackedLayout(content)),
	accountPage(new AccountPageWidget),
	m_navigationAccountItem(new QStandardItem("Accounts"))
{
	QWidget* centralWidget = new QWidget;
	QVBoxLayout* layout = new QVBoxLayout;

	auto* splitter = new QSplitter(Qt::Orientation::Horizontal);

	centralWidget->setLayout(layout);

	layout->addWidget(splitter);
	splitter->addWidget(navigationWidget);
	splitter->addWidget(content);

	content->setLayout(new QStackedLayout);

	setCentralWidget(centralWidget);
	resize(WINDOW_WIDTH, WINDOW_HEIGHT);
	setWindowTitle(
		QApplication::translate("windowTitle", "pView")
	);

	setupNavigation();
	setupDataFile();
	setupMenuBar();
}

void pvui::MainWindow::setupMenuBar()
{
	auto* fileMenu = menuBar()->addMenu("&File");
	{
		auto* openItem = new QAction("&Open");
		openItem->setShortcut(QKeySequence::Open);

		auto* newItem = new QAction("&New");
		newItem->setShortcut(QKeySequence::New);

		fileMenu->addActions({ openItem, newItem });
	}

	auto* accountMenu = menuBar()->addMenu("&Accounts");
	{
		auto* newItem = new QAction("&New");
		QObject::connect(newItem, &QAction::triggered, this, &MainWindow::addAccount);

		accountMenu->addActions({ newItem });
	}
}

void pvui::MainWindow::setupNavigation()
{

	contentLayout->addWidget(accountPage);
	contentLayout->addWidget(securityPage);
	contentLayout->addWidget(noPageOpen);
	contentLayout->setCurrentWidget(noPageOpen);

	noPageOpen->setAlignment(Qt::AlignCenter);

	navigationWidget->setModel(navigationModel);
	navigationWidget->setHeaderHidden(true);
	navigationWidget->setMaximumWidth(500);


	QItemSelectionModel* selectionModel = new QItemSelectionModel(navigationModel);
	navigationWidget->setSelectionMode(QTreeView::SelectionMode::ExtendedSelection);
	navigationWidget->setSelectionModel(selectionModel);

	QStandardItem* reports = new QStandardItem("Reports");
	reports->setEditable(false);
	securitiesNavigationItem->setEditable(false);

	navigationModel->setHorizontalHeaderLabels({ "Navigation" });

	navigationModel->appendRow(m_navigationAccountItem);
	navigationModel->appendRow(reports);
	navigationModel->appendRow(securitiesNavigationItem);

	QObject::connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::pageChanged);
}

void pvui::MainWindow::setupDataFile()
{
	dataFile().accountAdded().connect([&](pv::Account* account) {
		navigationWidget->setExpanded(m_navigationAccountItem->index(), true);

		auto* navigationItem = new QStandardItem(QString::fromStdString(account->name()));
		m_navigationAccountItem->appendRow(navigationItem);

		accountNavigationItems.insert({ navigationItem, account });
	});
}

void pvui::MainWindow::addAccount()
{
	bool ok = false;
	QString text = QInputDialog::getText(this, tr("Create an Account"), tr("Account Name:"), QLineEdit::Normal, "", &ok);

	if (ok) {
		auto trimmedText = text.trimmed();
		if (!trimmedText.isEmpty()) {
			dataFile().addAccount(trimmedText.toStdString());
		}
		else {
			QMessageBox::warning(this, tr("Invalid Account Name"), tr("The account name must not be empty"));
		}
	}
}

void pvui::MainWindow::pageChanged(const QItemSelection& selection)
{
	contentLayout->setCurrentWidget(noPageOpen);
	if (selection.indexes().isEmpty()) return;
	auto index = selection.indexes().first();

	auto* selectedItem = navigationModel->itemFromIndex(index);

	if (accountNavigationItems.find(selectedItem) != accountNavigationItems.cend()) {
		// This is an account page
		accountPage->setAccount(accountNavigationItems.at(selectedItem));
		contentLayout->setCurrentWidget(accountPage);
	}
	else if (selectedItem == securitiesNavigationItem) {
		contentLayout->setCurrentWidget(securityPage);
	}
		
}

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <unordered_map>
#include <QLabel>
#include <QMainWindow>
#include <QWidget>
#include <QSplitter>
#include <QTreeView>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QEvent>
#include <QStackedLayout>

#include "AccountPage.h"
#include "DataFileManager.h"
#include "DataFile.h"
#include "SecurityPage.h"

namespace pvui {
	class MainWindow : public QMainWindow
	{
		Q_OBJECT
	private:
		QTreeView* navigationWidget;
		QStandardItemModel* navigationModel;
		QWidget* content;
		QLabel* noPageOpen;
		QStackedLayout* contentLayout;
		AccountPageWidget* accountPage;
		DataFileManager dataFileManager;
		SecurityPageWidget* securityPage = new SecurityPageWidget(dataFileManager);
		QStandardItem* m_navigationAccountItem;

		QStandardItem* securitiesNavigationItem = new QStandardItem(tr("Securities"));
		std::unordered_map<QStandardItem*, pv::Account*> accountNavigationItems;

		void setupDataFile();
	public:
		MainWindow(QWidget* parent = nullptr);

		void setupMenuBar();
		void setupNavigation();
	protected:
		inline pv::DataFile& dataFile() noexcept {
			return dataFileManager.dataFile();
		}
	protected slots:
		void pageChanged(const QItemSelection& selection);
		void addAccount();
	};
}
#endif // UI_MAINWINDOW_H

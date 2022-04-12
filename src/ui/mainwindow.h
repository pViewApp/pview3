#pragma once

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

#include "accountpage.h"
#include "datafilemanager.h"
#include "datafile.h"
#include "accountdialog.h"

namespace pvui {
	class NavigationDelegate : public QItemDelegate
	{
		Q_OBJECT

	public:
		NavigationDelegate(QObject *parent = nullptr) : QItemDelegate(parent) {}
		inline bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex &index) override {
			if (QItemDelegate::editorEvent(event, model, option, index)) return true;

			if (event->type() == QEvent::FocusOut) {
				emit editingFinished(index);
			}

			return false;
		}
	signals:
		void editingFinished(const QModelIndex&) const;
	};

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
		QStandardItem* m_navigationAccountItem;
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

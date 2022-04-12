#pragma once

#ifndef UI_ACCOUNTDIALOG_H
#define UI_ACCOUNTDIALOG_H

#include <QObject>
#include <QWidget>

#include "datafile.h"
#include "datafilemanager.h"

namespace pvui {
	class AccountDialog : public QWidget {
		Q_OBJECT
	private:
		const DataFileManager& manager;
	public:
	};
}

#endif // UI_ACCOUNTDIALOG_H
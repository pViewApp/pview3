#ifndef PVUI_MODELS_NAVIGATIONMODEL_H
#define PVUI_MODELS_NAVIGATIONMODEL_H

#include "DataFileManager.h"
#include "Report.h"
#include "pv/DataFile.h"
#include "pv/Signals.h"
#include <QAbstractItemModel>
#include <boost/signals2/connection.hpp>
#include <vector>

namespace pvui {
namespace models {

class NavigationModel : public QAbstractItemModel {
  Q_OBJECT
private:
  DataFileManager& dataFileManager_;

  QModelIndex accountsHeaderIndex;
  QModelIndex reportsHeaderIndex;
  QModelIndex securitiesPageIndex;

  pv::ScopedConnection accountAddedConnection;
  pv::ScopedConnection accountRemovedConnection;
  std::vector<boost::signals2::scoped_connection> accountNameChangedConnections;

  std::vector<pv::Account*> accounts;
  std::vector<const Report*> reports;

  void setupAccount(pv::Account* account) noexcept;
private slots:
  void setDataFile(pv::DataFile& dataFile) noexcept;

protected:
public:
  explicit NavigationModel(DataFileManager& dataFileManager, QObject* parent = nullptr);

  int columnCount(const QModelIndex&) const override {
    return 1; // Only the page name
  }

  int rowCount(const QModelIndex& parent) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  QModelIndex index(int row, int column, const QModelIndex& parent) const override;

  QModelIndex parent(const QModelIndex& index) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  /// @brief Checks if the provided index is the account section header.
  /// @arg index the index
  /// @return `true` if the index is the account section header, false otherwise
  bool isAccountsHeader(const QModelIndex& index) const;

  /// @brief Checks if the provided index an account page.
  /// @arg index the index
  /// @return `true` if the index is an account page
  bool isAccountPage(const QModelIndex& index) const;

  /// @brief Checks if the provided index is the report section header.
  /// @arg index the index
  /// @return `true` if the index is the account report header, false otherwise
  bool isReportsHeader(const QModelIndex& index) const;

  /// @brief Checks if the provided index is a report page.
  /// @arg index the index
  /// @return `true` if the index is a report page, false otherwise
  bool isReportPage(const QModelIndex& index) const;

  /// @brief Checks if the provided index is the securities page.
  /// @arg index the index
  /// @return `true` if the index is the securities page, false otherwise
  bool isSecuritiesPage(const QModelIndex& index) const;

  QModelIndex accountsHeader() const noexcept { return accountsHeaderIndex; }

  QModelIndex reportsHeader() const noexcept { return reportsHeaderIndex; }

  QModelIndex securitiesPage() const noexcept { return securitiesPageIndex; }

  pv::Account* accountFromIndex(const QModelIndex& index) const;

  QModelIndex accountToIndex(const pv::Account& account) const;

  const Report* reportFromIndex(const QModelIndex& index) const;
public slots:
  void addReport(const pvui::Report* report);
  void addReports(const std::vector<pvui::Report*>& reports);
  void removeReport(const pvui::Report* report);

signals:
  void accountAdded(pv::Account* account);
  void accountRemoved(const pv::Account* account);
};

} // namespace models
} // namespace pvui

#endif // PVUI_MODELS_NAVIGATIONMODEL_H

#ifndef PVUI_REPORT_H
#define PVUI_REPORT_H

#include "DataFileManager.h"
#include "Page.h"
#include <QList>
#include <QColor>
#include <QwtPlot>
#include <cstddef>

namespace pvui {

/// \note Reports can assume that the dataFileManager is not null during reload(). This means that you should never reload() a report when
/// you have a null \c DataFileManager.
class Report : public PageWidget {
  Q_OBJECT
protected:
  DataFileManager& dataFileManager;
  QString name_;
public:
  Report(QString name, DataFileManager& dataFileManager, QWidget* parent = nullptr)
      : PageWidget(parent), dataFileManager(dataFileManager), name_(name) {
    setTitle(name);
    QObject::connect(this, &Report::nameChanged, this, &Report::setTitle);
  }

  QString name() const noexcept { return name_; }

  void setName(QString name) noexcept {
    name_ = std::move(name);
    emit nameChanged(name_);
  }

  virtual void reload() {}

  /// \brief Creates a QwtPlot with proper styling.
  ///
  /// The returned plot will have unspecified cosmetic changes.
  static QwtPlot* createPlot(QWidget* parent = nullptr) noexcept;

  static const QColor plotColor(std::size_t index) noexcept;
signals:
  void nameChanged(QString name);
};

} // namespace pvui

#endif // PVUI_REPORT_H

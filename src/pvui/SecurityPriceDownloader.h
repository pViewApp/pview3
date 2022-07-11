#ifndef PVUI_SECURITYPRICEDOWNLOADER_H
#define PVUI_SECURITYPRICEDOWNLOADER_H

#include "pv/Security.h"
#include <QDate>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <unordered_set>
#include <vector>

namespace pvui {

class SecurityPriceDownload : public QObject {
  Q_OBJECT
public:
  struct Download {
    QString symbol;
    QNetworkReply* reply;
  };

private:
  std::unordered_set<QNetworkReply*> replies;

public:
  SecurityPriceDownload(Download download, QObject* parent = nullptr);
  SecurityPriceDownload(std::vector<Download> downloads, QObject* parent = nullptr);

public slots:
  void abort();
signals:
  void success(const std::map<QDate, pv::i64>& data, QString symbol);
  void error(QNetworkReply::NetworkError error, QString symbol);
  void complete();
};

class SecurityPriceDownloader : public QObject {
  Q_OBJECT
private:
  QNetworkAccessManager manager;

public:
  /// \brief creates a SecurityPriceDownload object which downloads security prices for \c symbol,
  /// beginning with \c begin (inclusive) and ending with \c end (exclusive).
  ///
  /// The returned SecurityPriceDownload will have this SecurityPriceDownloader as a parent if no parent is supplied.
  SecurityPriceDownload* download(QString symbol, QDate begin, QDate end, QObject* parent = nullptr);
  SecurityPriceDownload* download(QStringList symbol, QDate begin, QDate end, QObject* parent = nullptr);
};

} // namespace pvui

#endif // PVUI_SECURITYPRICEDOWNLOADER_H

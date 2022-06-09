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
  SecurityPriceDownload(Download download);
  SecurityPriceDownload(std::vector<Download> downloads);

public slots:
  void abort();
signals:
  void success(std::map<QDate, pv::Decimal> data, QString symbol);
  void error(QNetworkReply::NetworkError error, QString symbol);
  void complete();
};

class SecurityPriceDownloader : public QObject {
  Q_OBJECT
private:
  QNetworkAccessManager manager;

public:
  SecurityPriceDownload* download(QString symbol, QDate begin, QDate end);
  SecurityPriceDownload* download(QStringList symbol, QDate begin, QDate end);
};

} // namespace pvui

#endif // PVUI_SECURITYPRICEDOWNLOADER_H

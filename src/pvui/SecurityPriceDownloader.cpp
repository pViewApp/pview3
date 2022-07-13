#include "SecurityPriceDownloader.h"
#include <QNetworkRequest>
#include <cassert>
#include <cmath>
#include <utility>

namespace pvui {

namespace {

QUrl generateDownloadUrl(const QString& symbol, const QDate& begin, const QDate& end) {
  static QString base = "https://query1.finance.yahoo.com/v7/finance/download/"
                        "%1?period1=%2&period2=%3&interval=1d&events=history&includeAdjustedClose=true";
  auto beginSecs =
      QDateTime(begin, QTime(23, 59, 59, 999), Qt::LocalTime).toSecsSinceEpoch(); // Inclusive on begin date
  auto endSecs = QDateTime(end.addDays(-1), QTime(23, 59, 59, 999), Qt::LocalTime)
                     .toSecsSinceEpoch(); // subtract 1 day because exclusive on end date
  return base.arg(symbol, QString::number(beginSecs), QString::number(endSecs));
}

std::map<QDate, pv::i64> parse(QIODevice& data) {
  constexpr int dateColumn = 0;
  constexpr int priceColumn = 4; // Closing price
  constexpr unsigned int skipHeaderRows = 1;

  std::map<QDate, pv::i64> output;

  QStringList currentLine;

  for (unsigned int i = 0; i < skipHeaderRows; ++i) {
    data.readLine();
  }

  while (data.bytesAvailable()) {
    // use bytesAvailable() instead of canReadLine(), because canReadLine() needs complete lines, and the last line
    // might not be complete (end with \n)
    currentLine = QString::fromUtf8(data.readLine()).split(',');
    if (priceColumn >= currentLine.size()) {
      assert(false && "Could not parse security prices, not enough commas!");
      continue;
    }

    QDate date = QDate::fromString(currentLine.at(dateColumn), Qt::ISODate); // Make sure to use ISO format (yyyy-MM-dd)

    if (!date.isValid()) {
      continue;
    }

    bool priceOk;
    double priceDouble = currentLine.at(priceColumn).toDouble(&priceOk);
    if (!priceOk) {
      continue;
    }

    pv::i64 price = static_cast<pv::i64>(std::llround(priceDouble * 100));

    output.insert({std::move(date), price});
  }

  return output;
}

} // namespace

SecurityPriceDownload::SecurityPriceDownload(Download download, QObject* parent)
    : SecurityPriceDownload(std::vector<Download>({download}), parent) {}

SecurityPriceDownload::SecurityPriceDownload(std::vector<Download> downloads, QObject* parent) : QObject(parent) {
  replies.reserve(downloads.size());
  for (auto& download : downloads) {
    const auto& symbol = download.symbol;
    auto* reply = download.reply;

    QDate endDate = download.endDate;
    replies.insert(reply);

    assert(reply->isOpen() && "SecurityPriceDownload replies must be open");
    reply->setParent(this);

    QObject::connect(reply, &QNetworkReply::errorOccurred, this,
                     [this, symbol, reply](QNetworkReply::NetworkError err) {
                       emit error(err, symbol);
                       reply->deleteLater();

                       replies.erase(reply);
                       if (replies.empty()) {
                         emit complete();
                       }
                     });

    
    QObject::connect(reply, &QNetworkReply::finished, this, [this, symbol, reply, endDate]() {
      auto data = parse(*reply);
      // Sometimes, Yahoo returns a data after the ending date
      // We remove that data here
      auto firstDateAfterEnd = data.lower_bound(endDate);
      if (firstDateAfterEnd != data.cend()) {
        data.erase(firstDateAfterEnd, data.cend());
      }
      emit success(data, symbol);
      reply->deleteLater();

      replies.erase(reply);
      if (replies.empty()) {
        emit complete();
      }
    });
  }
}

void SecurityPriceDownload::abort() {
  for (auto* reply : replies) {
    reply->blockSignals(true);
    reply->abort();
    reply->deleteLater(); // Don't delete now, the reply might still be being read
  }

  replies.clear();
}

SecurityPriceDownload* SecurityPriceDownloader::download(QString symbol, QDate begin, QDate end, QObject* parent) {
  auto* reply = manager.get(QNetworkRequest(generateDownloadUrl(symbol, begin, end)));

  return new SecurityPriceDownload({symbol, reply, end}, parent == nullptr ? this : parent);
}

SecurityPriceDownload* SecurityPriceDownloader::download(QStringList symbols, QDate begin, QDate end, QObject* parent) {
  std::vector<SecurityPriceDownload::Download> downloads;
  for (const auto& symbol : symbols) {
    auto* reply = manager.get(QNetworkRequest(generateDownloadUrl(symbol, begin, end)));
    downloads.push_back(SecurityPriceDownload::Download{symbol, reply, end});
  }
  return new SecurityPriceDownload(std::move(downloads), parent == nullptr ? this : parent);
}

} // namespace pvui

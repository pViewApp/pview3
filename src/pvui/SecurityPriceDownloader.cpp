#include "SecurityPriceDownloader.h"
#include <QNetworkRequest>
#include <cassert>
#include <utility>

namespace pvui {

namespace {

QUrl generateDownloadUrl(const QString& symbol, const QDate& begin, const QDate& end) {
  static QString base = "https://query1.finance.yahoo.com/v7/finance/download/"
                        "%1?period1=%2&period2=%3&interval=1d&events=history&includeAdjustedClose=true";
  auto beginSecs = QDateTime(begin, QTime(0, 0), Qt::LocalTime).toSecsSinceEpoch();
  auto endSecs = QDateTime(end, QTime(0, 0), Qt::LocalTime).toSecsSinceEpoch();
  return base.arg(symbol, QString::number(beginSecs), QString::number(endSecs));
}

std::map<QDate, pv::Decimal> parse(QIODevice& data) {
  constexpr int dateColumn = 0;
  constexpr int priceColumn = 4; // Closing price
  constexpr unsigned int skipHeaderRows = 1;

  std::map<QDate, pv::Decimal> output;

  QStringList currentLine;

  for (unsigned int i = 0; i < skipHeaderRows; ++i) {
    data.readLine();
  }

  while (data.canReadLine()) {
    currentLine = QString::fromUtf8(data.readLine()).split(',');
    if (priceColumn >= currentLine.size()) {
      assert(false && "Could not parse security prices, not enough commas!");
      continue;
    }

    QDate date = QDate::fromString(currentLine.at(dateColumn), Qt::ISODate); // Make sure to use ISO format (yyyy-MM-dd)
    pv::Decimal price;

    if (!date.isValid()) {
      continue;
    }
    try {
      price = pv::Decimal(currentLine.at(priceColumn).toStdString());
    } catch (...) {
      continue;
    }

    output.insert({std::move(date), std::move(price)});
  }

  return output;
}

} // namespace

SecurityPriceDownload::SecurityPriceDownload(Download download)
    : SecurityPriceDownload(std::vector<Download>({download})) {}

SecurityPriceDownload::SecurityPriceDownload(std::vector<Download> downloads) {
  replies.reserve(downloads.size());
  for (auto& download : downloads) {
    const auto& symbol = download.symbol;
    auto* reply = download.reply;

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

    QObject::connect(reply, &QNetworkReply::finished, this, [this, symbol, reply]() {
      emit success(parse(*reply), symbol);
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

SecurityPriceDownload* SecurityPriceDownloader::download(QString symbol, QDate begin, QDate end) {
  auto* reply = manager.get(QNetworkRequest(generateDownloadUrl(symbol, begin, end)));

  return new SecurityPriceDownload({symbol, reply});
}

SecurityPriceDownload* SecurityPriceDownloader::download(QStringList symbols, QDate begin, QDate end) {
  std::vector<SecurityPriceDownload::Download> downloads;
  for (const auto& symbol : symbols) {
    auto* reply = manager.get(QNetworkRequest(generateDownloadUrl(symbol, begin, end)));
    downloads.push_back(SecurityPriceDownload::Download{symbol, reply});
  }
  return new SecurityPriceDownload(std::move(downloads));
}

} // namespace pvui

#include "SolarTermDataSource.h"

#include <cstdlib>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace {
QString defaultSolarTermDataFilePath()
{
#ifdef FPOFD_SOURCE_DIR
    return QDir(QStringLiteral(FPOFD_SOURCE_DIR)).filePath(
        QStringLiteral("data/solar_terms/sample_solar_terms.json")
    );
#else
    return QStringLiteral("data/solar_terms/sample_solar_terms.json");
#endif
}

QString offsetTextForDateTime(const QDateTime &dateTime)
{
    if (!dateTime.isValid()) {
        return QStringLiteral("未対応");
    }

    const int totalSeconds = dateTime.offsetFromUtc();
    const int absoluteSeconds = std::abs(totalSeconds);
    const int hours = absoluteSeconds / 3600;
    const int minutes = (absoluteSeconds % 3600) / 60;
    const QChar sign = totalSeconds >= 0 ? QLatin1Char('+') : QLatin1Char('-');

    return QStringLiteral("%1%2:%3")
        .arg(sign)
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'));
}
}

SolarTermDataSource::SolarTermDataSource(QString dataFilePath)
    : m_dataFilePath(dataFilePath)
{
}

SolarTermYearData SolarTermDataSource::loadYearData(int year) const
{
    const QString resolvedPath = dataFilePath();
    QFile file(resolvedPath);

    if (!file.exists()) {
        return {
            false,
            false,
            year,
            {},
            QStringLiteral("節入りデータファイルが見つかりません。")
        };
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return {
            false,
            false,
            year,
            {},
            QStringLiteral("節入りデータファイルを開けませんでした。")
        };
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!document.isObject()) {
        return {
            false,
            false,
            year,
            {},
            QStringLiteral("節入りデータファイルの JSON 形式が不正です。")
        };
    }

    const QJsonObject rootObject = document.object();
    const QJsonObject yearsObject = rootObject.value(QStringLiteral("years")).toObject();
    const QString yearKey = QString::number(year);

    if (!yearsObject.contains(yearKey) || !yearsObject.value(yearKey).isArray()) {
        return {
            true,
            false,
            year,
            {},
            QStringLiteral("指定年の節入りデータは未整備です。")
        };
    }

    const QJsonArray entryArray = yearsObject.value(yearKey).toArray();
    QList<SolarTermEntry> entries;
    for (const QJsonValue &entryValue : entryArray) {
        if (!entryValue.isObject()) {
            continue;
        }

        const QJsonObject entryObject = entryValue.toObject();
        const QString termName = entryObject.value(QStringLiteral("term")).toString();
        const QString atIsoString = entryObject.value(QStringLiteral("at")).toString();
        const QDateTime atDateTime = QDateTime::fromString(atIsoString, Qt::ISODate);

        if (termName.isEmpty() || !atDateTime.isValid()) {
            continue;
        }

        entries.append({
            year,
            termName,
            atIsoString,
            atDateTime,
            offsetTextForDateTime(atDateTime)
        });
    }

    if (entries.isEmpty()) {
        return {
            true,
            false,
            year,
            {},
            QStringLiteral("指定年データはありますが、有効な節入り日時を読み取れませんでした。")
        };
    }

    return {
        true,
        true,
        year,
        entries,
        QStringLiteral("指定年の節入りデータを日時付きで読み込みました。")
    };
}

QString SolarTermDataSource::dataFilePath() const
{
    return m_dataFilePath.isEmpty() ? defaultSolarTermDataFilePath() : m_dataFilePath;
}

void SolarTermDataSource::setDataFilePath(const QString &dataFilePath)
{
    m_dataFilePath = dataFilePath;
}

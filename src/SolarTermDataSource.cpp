#include "SolarTermDataSource.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
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

    const QJsonArray entries = yearsObject.value(yearKey).toArray();
    return {
        true,
        true,
        year,
        entries,
        QStringLiteral("指定年の節入りデータを読み込みました。")
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

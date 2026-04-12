#include "SolarTermDataSource.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace {
QString sourceRootPath()
{
#ifdef FPOFD_SOURCE_DIR
    return QStringLiteral(FPOFD_SOURCE_DIR);
#else
    return QDir::currentPath();
#endif
}

QString verifiedSolarTermDataFilePath()
{
    return QDir(sourceRootPath()).filePath(
        QStringLiteral("data/solar_terms/verified_solar_terms.json")
    );
}

QString provisionalSolarTermDataFilePath()
{
    return QDir(sourceRootPath()).filePath(
        QStringLiteral("data/solar_terms/provisional_solar_terms.json")
    );
}

QString legacySolarTermDataFilePath()
{
    return QDir(sourceRootPath()).filePath(
        QStringLiteral("data/solar_terms/sample_solar_terms.json")
    );
}

QString defaultSolarTermDataFilePath()
{
    return legacySolarTermDataFilePath();
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

QString valueOrFallback(const QJsonObject &object, const QString &key, const QString &fallback)
{
    const QString value = object.value(key).toString();
    return value.isEmpty() ? fallback : value;
}

QList<SolarTermEntry> readFlatEntriesForYear(
    const QJsonObject &rootObject,
    int year,
    const QString &fallbackSourceType,
    const QString &fallbackSourceTitle,
    const QString &fallbackSourceAuthor,
    const QString &fallbackSourceUrl,
    const QString &fallbackConfidence,
    const QString &fallbackAdoptable,
    const QString &fallbackNotes
)
{
    QList<SolarTermEntry> entries;
    const QJsonArray entryArray = rootObject.value(QStringLiteral("entries")).toArray();
    for (const QJsonValue &entryValue : entryArray) {
        if (!entryValue.isObject()) {
            continue;
        }

        const QJsonObject entryObject = entryValue.toObject();
        if (entryObject.value(QStringLiteral("year")).toInt(std::numeric_limits<int>::min()) != year) {
            continue;
        }

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
            valueOrFallback(entryObject, QStringLiteral("timezone"), offsetTextForDateTime(atDateTime)),
            valueOrFallback(entryObject, QStringLiteral("source_type"), fallbackSourceType),
            valueOrFallback(entryObject, QStringLiteral("source_title"), fallbackSourceTitle),
            valueOrFallback(entryObject, QStringLiteral("source_author"), fallbackSourceAuthor),
            valueOrFallback(entryObject, QStringLiteral("source_url"), fallbackSourceUrl),
            valueOrFallback(entryObject, QStringLiteral("confidence"), fallbackConfidence),
            valueOrFallback(entryObject, QStringLiteral("adoptable"), fallbackAdoptable),
            valueOrFallback(entryObject, QStringLiteral("notes"), fallbackNotes)
        });
    }

    return entries;
}

QList<SolarTermEntry> readLegacyEntriesForYear(const QJsonObject &rootObject, int year)
{
    QList<SolarTermEntry> entries;
    const QJsonObject yearsObject = rootObject.value(QStringLiteral("years")).toObject();
    const QString yearKey = QString::number(year);
    if (!yearsObject.contains(yearKey) || !yearsObject.value(yearKey).isArray()) {
        return entries;
    }

    const QJsonArray entryArray = yearsObject.value(yearKey).toArray();
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
            offsetTextForDateTime(atDateTime),
            QStringLiteral("reference"),
            QStringLiteral("FourPillarsOfDestiny サンプル節入りデータ"),
            QStringLiteral("FourPillarsOfDestiny project"),
            QString(),
            QStringLiteral("C"),
            QStringLiteral("provisional"),
            QStringLiteral("既存の固定サンプル節入りデータです。正式出典確認前の legacy fallback として扱います。")
        });
    }

    return entries;
}

QList<SolarTermEntry> readEntriesFromFile(
    const QString &filePath,
    int year,
    const QString &fallbackSourceType,
    const QString &fallbackSourceTitle,
    const QString &fallbackSourceAuthor,
    const QString &fallbackSourceUrl,
    const QString &fallbackConfidence,
    const QString &fallbackAdoptable,
    const QString &fallbackNotes,
    bool *fileAvailable,
    QString *statusMessage
)
{
    if (fileAvailable) {
        *fileAvailable = false;
    }

    QFile file(filePath);
    if (!file.exists()) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("節入りデータファイルが見つかりません。");
        }
        return {};
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("節入りデータファイルを開けませんでした。");
        }
        return {};
    }

    if (fileAvailable) {
        *fileAvailable = true;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("節入りデータファイルの JSON 形式が不正です。");
        }
        return {};
    }

    const QJsonObject rootObject = document.object();
    QList<SolarTermEntry> entries;
    if (rootObject.contains(QStringLiteral("entries")) && rootObject.value(QStringLiteral("entries")).isArray()) {
        entries = readFlatEntriesForYear(
            rootObject,
            year,
            fallbackSourceType,
            fallbackSourceTitle,
            fallbackSourceAuthor,
            fallbackSourceUrl,
            fallbackConfidence,
            fallbackAdoptable,
            fallbackNotes
        );
    } else if (rootObject.contains(QStringLiteral("years")) && rootObject.value(QStringLiteral("years")).isObject()) {
        entries = readLegacyEntriesForYear(rootObject, year);
    }

    if (statusMessage) {
        *statusMessage = entries.isEmpty()
            ? QStringLiteral("指定年の節入りデータは未整備です。")
            : QStringLiteral("指定年の節入りデータを日時付きで読み込みました。");
    }
    return entries;
}

QString strongestConfidenceForEntries(const QList<SolarTermEntry> &entries)
{
    if (entries.isEmpty()) {
        return QStringLiteral("未対応");
    }

    const QString ranking = QStringLiteral("SABC");
    int bestIndex = ranking.size();
    QString bestConfidence = QStringLiteral("C");
    for (const SolarTermEntry &entry : entries) {
        const QString confidence = entry.confidence.isEmpty() ? QStringLiteral("C") : entry.confidence;
        const int confidenceIndex = ranking.indexOf(confidence.left(1));
        if (confidenceIndex >= 0 && confidenceIndex < bestIndex) {
            bestIndex = confidenceIndex;
            bestConfidence = confidence.left(1);
        }
    }

    return bestConfidence;
}

QString combinedNotesForEntries(const QList<SolarTermEntry> &entries)
{
    QSet<QString> uniqueNotes;
    QStringList noteList;
    for (const SolarTermEntry &entry : entries) {
        if (entry.notes.isEmpty() || uniqueNotes.contains(entry.notes)) {
            continue;
        }
        uniqueNotes.insert(entry.notes);
        noteList << entry.notes;
    }

    return noteList.join(QStringLiteral(" / "));
}

SolarTermYearData buildYearData(
    int year,
    const QList<SolarTermEntry> &entries,
    const QString &statusMessage,
    const QString &sourceQuality,
    const QString &adoptable,
    const QString &yearStatus,
    const QString &notes
)
{
    QList<SolarTermEntry> sortedEntries = entries;
    std::sort(sortedEntries.begin(), sortedEntries.end(), [](const SolarTermEntry &left, const SolarTermEntry &right) {
        return left.atDateTime < right.atDateTime;
    });

    return {
        true,
        !sortedEntries.isEmpty(),
        year,
        sortedEntries,
        statusMessage,
        sourceQuality,
        adoptable,
        yearStatus,
        notes
    };
}
}

SolarTermDataSource::SolarTermDataSource(QString dataFilePath)
    : m_dataFilePath(dataFilePath)
{
}

SolarTermYearData SolarTermDataSource::loadYearData(int year) const
{
    if (!m_dataFilePath.isEmpty()) {
        bool fileAvailable = false;
        QString statusMessage;
        const QList<SolarTermEntry> entries = readEntriesFromFile(
            m_dataFilePath,
            year,
            QStringLiteral("reference"),
            QStringLiteral("カスタム節入りデータ"),
            QStringLiteral("FourPillarsOfDestiny project"),
            QString(),
            QStringLiteral("C"),
            QStringLiteral("provisional"),
            QStringLiteral("カスタム指定された節入りデータです。"),
            &fileAvailable,
            &statusMessage
        );

        if (!fileAvailable) {
            return {
                false,
                false,
                year,
                {},
                statusMessage,
                QStringLiteral("未対応"),
                QStringLiteral("未対応"),
                QStringLiteral("missing"),
                QString()
            };
        }

        if (entries.isEmpty()) {
            return {
                true,
                false,
                year,
                {},
                statusMessage,
                QStringLiteral("未対応"),
                QStringLiteral("未対応"),
                QStringLiteral("missing"),
                QString()
            };
        }

        return buildYearData(
            year,
            entries,
            statusMessage,
            strongestConfidenceForEntries(entries),
            entries.first().adoptable,
            QStringLiteral("custom"),
            combinedNotesForEntries(entries)
        );
    }

    bool verifiedFileAvailable = false;
    QString verifiedStatusMessage;
    const QList<SolarTermEntry> verifiedEntries = readEntriesFromFile(
        verifiedSolarTermDataFilePath(),
        year,
        QStringLiteral("official"),
        QStringLiteral("公的確認済み節入りデータ"),
        QStringLiteral("FourPillarsOfDestiny project"),
        QString(),
        QStringLiteral("S"),
        QStringLiteral("verified"),
        QStringLiteral("公的ソース確認済みの採用節入りデータです。"),
        &verifiedFileAvailable,
        &verifiedStatusMessage
    );
    if (!verifiedEntries.isEmpty()) {
        return buildYearData(
            year,
            verifiedEntries,
            QStringLiteral("指定年の verified 節入りデータを日時付きで読み込みました。"),
            strongestConfidenceForEntries(verifiedEntries),
            QStringLiteral("verified"),
            QStringLiteral("verified"),
            combinedNotesForEntries(verifiedEntries)
        );
    }

    bool provisionalFileAvailable = false;
    QString provisionalStatusMessage;
    const QList<SolarTermEntry> provisionalEntries = readEntriesFromFile(
        provisionalSolarTermDataFilePath(),
        year,
        QStringLiteral("reference"),
        QStringLiteral("暫定節入りデータ"),
        QStringLiteral("FourPillarsOfDestiny project"),
        QString(),
        QStringLiteral("C"),
        QStringLiteral("provisional"),
        QStringLiteral("旧年代または比較年の暫定節入りデータです。"),
        &provisionalFileAvailable,
        &provisionalStatusMessage
    );
    if (!provisionalEntries.isEmpty()) {
        return buildYearData(
            year,
            provisionalEntries,
            QStringLiteral("指定年の provisional 節入りデータを日時付きで読み込みました。"),
            strongestConfidenceForEntries(provisionalEntries),
            QStringLiteral("provisional"),
            QStringLiteral("provisional"),
            combinedNotesForEntries(provisionalEntries)
        );
    }

    bool legacyFileAvailable = false;
    QString legacyStatusMessage;
    const QList<SolarTermEntry> legacyEntries = readEntriesFromFile(
        legacySolarTermDataFilePath(),
        year,
        QStringLiteral("reference"),
        QStringLiteral("FourPillarsOfDestiny サンプル節入りデータ"),
        QStringLiteral("FourPillarsOfDestiny project"),
        QString(),
        QStringLiteral("C"),
        QStringLiteral("provisional"),
        QStringLiteral("既存の固定サンプル節入りデータです。"),
        &legacyFileAvailable,
        &legacyStatusMessage
    );
    if (!legacyEntries.isEmpty()) {
        return buildYearData(
            year,
            legacyEntries,
            QStringLiteral("指定年の節入りデータを日時付きで読み込みました。"),
            strongestConfidenceForEntries(legacyEntries),
            QStringLiteral("provisional"),
            QStringLiteral("legacy_sample"),
            combinedNotesForEntries(legacyEntries)
        );
    }

    if (!verifiedFileAvailable && !provisionalFileAvailable && !legacyFileAvailable) {
        return {
            false,
            false,
            year,
            {},
            QStringLiteral("節入りデータファイルが見つかりません。"),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QStringLiteral("missing"),
            QString()
        };
    }

    return {
        true,
        false,
        year,
        {},
        QStringLiteral("指定年の節入りデータは未整備です。"),
        QStringLiteral("未対応"),
        QStringLiteral("未対応"),
        QStringLiteral("missing"),
        QString()
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

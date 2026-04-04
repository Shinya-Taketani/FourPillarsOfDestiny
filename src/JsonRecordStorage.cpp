#include "JsonRecordStorage.h"

#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

JsonRecordStorage::JsonRecordStorage(QString baseDirectoryPath)
    : m_baseDirectoryPath(baseDirectoryPath)
{
}

bool JsonRecordStorage::save(const SavedChartRecord &record, QString *savedFilePath, QString *errorMessage) const
{
    const QString directoryPath = storageDirectoryPath();
    QDir directory;

    if (!directory.mkpath(directoryPath)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("保存先ディレクトリを作成できませんでした。");
        }
        return false;
    }

    const QString fileName = QStringLiteral("record_%1.json")
                                 .arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz")));
    const QString filePath = QDir(directoryPath).filePath(fileName);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("保存ファイルを開けませんでした。");
        }
        return false;
    }

    const QJsonDocument document(record.toJsonObject());
    const qint64 written = file.write(document.toJson(QJsonDocument::Indented));
    file.close();

    if (written < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("JSON ファイルの書き込みに失敗しました。");
        }
        return false;
    }

    if (savedFilePath) {
        *savedFilePath = filePath;
    }

    return true;
}

QVariantList JsonRecordStorage::listRecords(QString *errorMessage) const
{
    QVariantList records;
    QDir directory(storageDirectoryPath());

    if (!directory.exists()) {
        return records;
    }

    const QFileInfoList files = directory.entryInfoList(
        {QStringLiteral("*.json")},
        QDir::Files,
        QDir::Time | QDir::Reversed
    );

    for (const QFileInfo &fileInfo : files) {
        SavedChartRecord record;
        QString loadError;
        if (!load(fileInfo.absoluteFilePath(), &record, &loadError)) {
            continue;
        }

        records.append(QVariantMap{
            {QStringLiteral("filePath"), fileInfo.absoluteFilePath()},
            {QStringLiteral("fileName"), fileInfo.fileName()},
            {QStringLiteral("savedAt"), record.savedAt},
            {QStringLiteral("birthDate"), record.birthInfo.birthDate},
            {QStringLiteral("gender"), record.birthInfo.gender}
        });
    }

    if (records.isEmpty() && errorMessage && directory.entryList({QStringLiteral("*.json")}, QDir::Files).isEmpty()) {
        *errorMessage = QString();
    }

    return records;
}

bool JsonRecordStorage::load(const QString &filePath, SavedChartRecord *outRecord, QString *errorMessage) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("保存ファイルを開けませんでした。");
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("JSON 形式が不正です。");
        }
        return false;
    }

    if (!SavedChartRecord::fromJsonObject(document.object(), outRecord)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("保存データの復元に失敗しました。");
        }
        return false;
    }

    return true;
}

QString JsonRecordStorage::storageDirectoryPath() const
{
    if (!m_baseDirectoryPath.isEmpty()) {
        return QDir(m_baseDirectoryPath).filePath(QStringLiteral("records"));
    }

    const QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataLocation).filePath(QStringLiteral("records"));
}

void JsonRecordStorage::setBaseDirectoryPath(const QString &baseDirectoryPath)
{
    m_baseDirectoryPath = baseDirectoryPath;
}

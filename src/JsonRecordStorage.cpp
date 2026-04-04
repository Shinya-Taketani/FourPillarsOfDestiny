#include "JsonRecordStorage.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
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

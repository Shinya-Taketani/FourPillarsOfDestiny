#pragma once

#include <QString>

#include "SavedChartRecord.h"

class JsonRecordStorage
{
public:
    explicit JsonRecordStorage(QString baseDirectoryPath = QString());

    bool save(const SavedChartRecord &record, QString *savedFilePath = nullptr, QString *errorMessage = nullptr) const;
    QString storageDirectoryPath() const;
    void setBaseDirectoryPath(const QString &baseDirectoryPath);

private:
    QString m_baseDirectoryPath;
};

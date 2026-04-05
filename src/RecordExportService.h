#pragma once

#include <QString>

#include "SavedChartRecord.h"

class RecordExportService
{
public:
    explicit RecordExportService(QString baseDirectoryPath = QString());

    bool exportAsText(const SavedChartRecord &record, QString *exportedFilePath = nullptr, QString *errorMessage = nullptr) const;
    bool exportAsJson(const SavedChartRecord &record, QString *exportedFilePath = nullptr, QString *errorMessage = nullptr) const;
    QString exportDirectoryPath() const;
    void setBaseDirectoryPath(const QString &baseDirectoryPath);

private:
    QString m_baseDirectoryPath;
};

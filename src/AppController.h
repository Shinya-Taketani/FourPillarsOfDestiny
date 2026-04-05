#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "BirthInfo.h"
#include "ChartCalculator.h"
#include "ChartResult.h"
#include "InterpretationEngine.h"
#include "InterpretationResult.h"
#include "JsonRecordStorage.h"
#include "RecordExportService.h"
#include "SavedChartRecord.h"

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appTitle READ appTitle CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);

    QString appTitle() const;

    Q_INVOKABLE QString helloMessage() const;
    Q_INVOKABLE void setBirthInfo(const QString &birthDate, const QString &birthTime, const QString &gender);
    Q_INVOKABLE QVariantMap currentBirthInfo() const;
    Q_INVOKABLE bool isBirthInfoValid() const;
    Q_INVOKABLE QStringList birthInfoValidationErrors() const;
    Q_INVOKABLE QVariantMap calculateChartResult() const;
    Q_INVOKABLE QVariantMap calculateInterpretationResult() const;
    Q_INVOKABLE bool saveCurrentRecord();
    Q_INVOKABLE QVariantList savedRecords() const;
    Q_INVOKABLE bool loadSavedRecord(const QString &filePath);
    Q_INVOKABLE QVariantMap currentChartResult() const;
    Q_INVOKABLE QVariantMap currentInterpretationResult() const;
    Q_INVOKABLE QString lastSaveMessage() const;
    Q_INVOKABLE bool exportCurrentRecordAsText();
    Q_INVOKABLE bool exportCurrentRecordAsJson();
    Q_INVOKABLE QString lastExportMessage() const;

    void setRecordStorageDirectory(const QString &baseDirectoryPath);
    void setExportDirectory(const QString &baseDirectoryPath);

private:
    BirthInfo m_birthInfo;
    ChartCalculator m_chartCalculator;
    InterpretationEngine m_interpretationEngine;
    JsonRecordStorage m_recordStorage;
    RecordExportService m_recordExportService;
    mutable ChartResult m_chartResult;
    mutable InterpretationResult m_interpretationResult;
    QString m_lastSaveMessage;
    QString m_lastExportMessage;
};

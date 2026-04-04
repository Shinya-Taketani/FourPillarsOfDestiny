#include <QDateTime>

#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

QString AppController::appTitle() const
{
    return QStringLiteral("FourPillarsOfDestiny");
}

QString AppController::helloMessage() const
{
    return QStringLiteral("泰山流四柱推命ソフトの初期画面です。");
}

void AppController::setBirthInfo(const QString &birthDate, const QString &birthTime, const QString &gender)
{
    m_birthInfo.birthDate = birthDate;
    m_birthInfo.birthTime = birthTime;
    m_birthInfo.gender = gender;
}

QVariantMap AppController::currentBirthInfo() const
{
    return m_birthInfo.toVariantMap();
}

bool AppController::isBirthInfoValid() const
{
    return m_birthInfo.isValid();
}

QStringList AppController::birthInfoValidationErrors() const
{
    return m_birthInfo.validationErrors();
}

QVariantMap AppController::calculateChartResult() const
{
    m_chartResult = m_chartCalculator.calculate(m_birthInfo);
    return m_chartResult.toVariantMap();
}

QVariantMap AppController::calculateInterpretationResult() const
{
    m_interpretationResult = m_interpretationEngine.interpret(m_chartResult);
    return m_interpretationResult.toVariantMap();
}

bool AppController::saveCurrentRecord()
{
    if (!m_birthInfo.isValid()) {
        m_lastSaveMessage = QStringLiteral("入力内容に不備があるため保存できません。");
        return false;
    }

    if (m_chartResult.yearPillar.isEmpty() || m_interpretationResult.summaryText.isEmpty()) {
        m_lastSaveMessage = QStringLiteral("命式結果または解釈結果が未生成のため保存できません。");
        return false;
    }

    SavedChartRecord record{
        m_birthInfo,
        m_chartResult,
        m_interpretationResult,
        QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
    };

    QString savedFilePath;
    QString errorMessage;
    const bool success = m_recordStorage.save(record, &savedFilePath, &errorMessage);

    if (!success) {
        m_lastSaveMessage = errorMessage.isEmpty()
            ? QStringLiteral("保存に失敗しました。")
            : errorMessage;
        return false;
    }

    m_lastSaveMessage = QStringLiteral("保存しました: %1").arg(savedFilePath);
    return true;
}

QVariantList AppController::savedRecords() const
{
    return m_recordStorage.listRecords();
}

bool AppController::loadSavedRecord(const QString &filePath)
{
    SavedChartRecord record;
    QString errorMessage;

    if (!m_recordStorage.load(filePath, &record, &errorMessage)) {
        m_lastSaveMessage = errorMessage.isEmpty()
            ? QStringLiteral("保存データの読み込みに失敗しました。")
            : errorMessage;
        return false;
    }

    m_birthInfo = record.birthInfo;
    m_chartResult = record.chartResult;
    m_interpretationResult = record.interpretationResult;
    m_lastSaveMessage = QStringLiteral("保存データを読み込みました。");
    return true;
}

QVariantMap AppController::currentChartResult() const
{
    return m_chartResult.toVariantMap();
}

QVariantMap AppController::currentInterpretationResult() const
{
    return m_interpretationResult.toVariantMap();
}

QString AppController::lastSaveMessage() const
{
    return m_lastSaveMessage;
}

void AppController::setRecordStorageDirectory(const QString &baseDirectoryPath)
{
    m_recordStorage.setBaseDirectoryPath(baseDirectoryPath);
}

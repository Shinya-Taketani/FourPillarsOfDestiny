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

QVariantMap AppController::mockChartResult() const
{
    return {
        {QStringLiteral("yearPillar"), QStringLiteral("甲子")},
        {QStringLiteral("monthPillar"), QStringLiteral("乙丑")},
        {QStringLiteral("dayPillar"), QStringLiteral("丙寅")},
        {QStringLiteral("hourPillar"), QStringLiteral("丁卯")},
        {QStringLiteral("description"), QStringLiteral("これは仮の命式結果です。計算ロジックは未実装です。")}
    };
}

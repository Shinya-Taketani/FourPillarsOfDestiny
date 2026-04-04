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

QVariantMap AppController::calculateChartResult() const
{
    m_chartResult = m_chartCalculator.calculate(m_birthInfo);
    return m_chartResult.toVariantMap();
}

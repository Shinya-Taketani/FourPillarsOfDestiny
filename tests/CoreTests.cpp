#include <QtTest>

#include "AppController.h"
#include "BirthInfo.h"
#include "ChartCalculator.h"
#include "ChartResult.h"

class CoreTests : public QObject
{
    Q_OBJECT

private slots:
    void chartCalculatorReturnsNonEmptyResult();
    void chartResultToVariantMapContainsRequiredKeys();
    void appControllerStoresBirthInfo();
    void appControllerReturnsChartResultMap();
};

void CoreTests::chartCalculatorReturnsNonEmptyResult()
{
    ChartCalculator calculator;
    BirthInfo birthInfo;

    birthInfo.birthDate = QStringLiteral("1990-01-01");
    birthInfo.birthTime = QStringLiteral("13:30");
    birthInfo.gender = QStringLiteral("男性");

    const ChartResult result = calculator.calculate(birthInfo);

    QVERIFY(!result.yearPillar.isEmpty());
    QVERIFY(!result.monthPillar.isEmpty());
    QVERIFY(!result.dayPillar.isEmpty());
    QVERIFY(!result.hourPillar.isEmpty());
    QVERIFY(!result.description.isEmpty());
}

void CoreTests::chartResultToVariantMapContainsRequiredKeys()
{
    ChartResult result{
        QStringLiteral("甲子"),
        QStringLiteral("乙丑"),
        QStringLiteral("丙寅"),
        QStringLiteral("丁卯"),
        QStringLiteral("これは仮の命式結果です。")
    };

    const QVariantMap resultMap = result.toVariantMap();

    QVERIFY(resultMap.contains(QStringLiteral("yearPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("monthPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("dayPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("hourPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("description")));
}

void CoreTests::appControllerStoresBirthInfo()
{
    AppController controller;

    controller.setBirthInfo(
        QStringLiteral("2000-02-03"),
        QStringLiteral("08:45"),
        QStringLiteral("女性")
    );

    const QVariantMap birthInfo = controller.currentBirthInfo();

    QCOMPARE(birthInfo.value(QStringLiteral("birthDate")).toString(), QStringLiteral("2000-02-03"));
    QCOMPARE(birthInfo.value(QStringLiteral("birthTime")).toString(), QStringLiteral("08:45"));
    QCOMPARE(birthInfo.value(QStringLiteral("gender")).toString(), QStringLiteral("女性"));
}

void CoreTests::appControllerReturnsChartResultMap()
{
    AppController controller;

    controller.setBirthInfo(
        QStringLiteral("1988-12-24"),
        QStringLiteral("06:15"),
        QStringLiteral("未指定")
    );

    const QVariantMap resultMap = controller.calculateChartResult();

    QVERIFY(resultMap.contains(QStringLiteral("yearPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("monthPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("dayPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("hourPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("description")));
    QVERIFY(!resultMap.value(QStringLiteral("description")).toString().isEmpty());
}

QTEST_MAIN(CoreTests)

#include "CoreTests.moc"

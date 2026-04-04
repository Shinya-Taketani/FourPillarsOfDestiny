#include <QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "AppController.h"
#include "BirthInfo.h"
#include "ChartCalculator.h"
#include "ChartResult.h"
#include "InterpretationEngine.h"
#include "InterpretationResult.h"
#include "JsonRecordStorage.h"
#include "SavedChartRecord.h"

class CoreTests : public QObject
{
    Q_OBJECT

private slots:
    void chartCalculatorReturnsNonEmptyResult();
    void chartResultToVariantMapContainsRequiredKeys();
    void birthInfoValidationAcceptsValidInput();
    void birthInfoValidationRejectsEmptyBirthDate();
    void birthInfoValidationRejectsEmptyBirthTime();
    void birthInfoValidationRejectsEmptyGender();
    void appControllerStoresBirthInfo();
    void appControllerExposesBirthInfoValidation();
    void appControllerReturnsChartResultMap();
    void interpretationEngineReturnsNonEmptyResult();
    void appControllerReturnsInterpretationResultMap();
    void savedChartRecordConvertsToJsonObject();
    void jsonRecordStorageWritesJsonFile();
    void appControllerCanSaveCurrentRecord();
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

void CoreTests::birthInfoValidationAcceptsValidInput()
{
    BirthInfo birthInfo;

    birthInfo.birthDate = QStringLiteral("1990-01-01");
    birthInfo.birthTime = QStringLiteral("13:30");
    birthInfo.gender = QStringLiteral("男性");

    QVERIFY(birthInfo.isValid());
    QVERIFY(birthInfo.validationErrors().isEmpty());
}

void CoreTests::birthInfoValidationRejectsEmptyBirthDate()
{
    BirthInfo birthInfo;

    birthInfo.birthDate = QStringLiteral("");
    birthInfo.birthTime = QStringLiteral("13:30");
    birthInfo.gender = QStringLiteral("男性");

    QVERIFY(!birthInfo.isValid());
    QVERIFY(!birthInfo.validationErrors().isEmpty());
}

void CoreTests::birthInfoValidationRejectsEmptyBirthTime()
{
    BirthInfo birthInfo;

    birthInfo.birthDate = QStringLiteral("1990-01-01");
    birthInfo.birthTime = QStringLiteral("");
    birthInfo.gender = QStringLiteral("男性");

    QVERIFY(!birthInfo.isValid());
    QVERIFY(!birthInfo.validationErrors().isEmpty());
}

void CoreTests::birthInfoValidationRejectsEmptyGender()
{
    BirthInfo birthInfo;

    birthInfo.birthDate = QStringLiteral("1990-01-01");
    birthInfo.birthTime = QStringLiteral("13:30");
    birthInfo.gender = QStringLiteral("");

    QVERIFY(!birthInfo.isValid());
    QVERIFY(!birthInfo.validationErrors().isEmpty());
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

void CoreTests::appControllerExposesBirthInfoValidation()
{
    AppController controller;

    controller.setBirthInfo(
        QStringLiteral("2000/02/03"),
        QStringLiteral("08:45"),
        QStringLiteral("女性")
    );

    QVERIFY(!controller.isBirthInfoValid());
    QVERIFY(!controller.birthInfoValidationErrors().isEmpty());
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

void CoreTests::interpretationEngineReturnsNonEmptyResult()
{
    InterpretationEngine engine;
    ChartResult chartResult{
        QStringLiteral("甲子"),
        QStringLiteral("乙丑"),
        QStringLiteral("丙寅"),
        QStringLiteral("丁卯"),
        QStringLiteral("これは仮の命式結果です。")
    };

    const InterpretationResult result = engine.interpret(chartResult);

    QVERIFY(!result.summaryText.isEmpty());
    QVERIFY(!result.detailText.isEmpty());
    QVERIFY(!result.cautionText.isEmpty());
}

void CoreTests::appControllerReturnsInterpretationResultMap()
{
    AppController controller;

    controller.setBirthInfo(
        QStringLiteral("1988-12-24"),
        QStringLiteral("06:15"),
        QStringLiteral("未指定")
    );

    controller.calculateChartResult();
    const QVariantMap interpretationMap = controller.calculateInterpretationResult();

    QVERIFY(interpretationMap.contains(QStringLiteral("summaryText")));
    QVERIFY(interpretationMap.contains(QStringLiteral("detailText")));
    QVERIFY(interpretationMap.contains(QStringLiteral("cautionText")));
    QVERIFY(!interpretationMap.value(QStringLiteral("summaryText")).toString().isEmpty());
    QVERIFY(!interpretationMap.value(QStringLiteral("detailText")).toString().isEmpty());
}

void CoreTests::savedChartRecordConvertsToJsonObject()
{
    SavedChartRecord record{
        BirthInfo{QStringLiteral("1990-01-01"), QStringLiteral("13:30"), QStringLiteral("男性")},
        ChartResult{
            QStringLiteral("甲子"),
            QStringLiteral("乙丑"),
            QStringLiteral("丙寅"),
            QStringLiteral("丁卯"),
            QStringLiteral("これは仮の命式結果です。")
        },
        InterpretationResult{
            QStringLiteral("これは仮の解釈結果です。"),
            QStringLiteral("本実装の解釈ロジックは未対応です。"),
            QStringLiteral("要確認事項があります。")
        },
        QStringLiteral("2026-04-05T00:00:00Z")
    };

    const QJsonObject jsonObject = record.toJsonObject();

    QVERIFY(jsonObject.contains(QStringLiteral("schemaVersion")));
    QVERIFY(jsonObject.contains(QStringLiteral("savedAt")));
    QVERIFY(jsonObject.contains(QStringLiteral("birthInfo")));
    QVERIFY(jsonObject.contains(QStringLiteral("chartResult")));
    QVERIFY(jsonObject.contains(QStringLiteral("interpretationResult")));
}

void CoreTests::jsonRecordStorageWritesJsonFile()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    JsonRecordStorage storage(temporaryDir.path());
    const SavedChartRecord record{
        BirthInfo{QStringLiteral("1990-01-01"), QStringLiteral("13:30"), QStringLiteral("男性")},
        ChartResult{
            QStringLiteral("甲子"),
            QStringLiteral("乙丑"),
            QStringLiteral("丙寅"),
            QStringLiteral("丁卯"),
            QStringLiteral("これは仮の命式結果です。")
        },
        InterpretationResult{
            QStringLiteral("これは仮の解釈結果です。"),
            QStringLiteral("本実装の解釈ロジックは未対応です。"),
            QStringLiteral("要確認事項があります。")
        },
        QStringLiteral("2026-04-05T00:00:00Z")
    };

    QString savedFilePath;
    QString errorMessage;

    QVERIFY(storage.save(record, &savedFilePath, &errorMessage));
    QVERIFY(errorMessage.isEmpty());
    QVERIFY(QFile::exists(savedFilePath));

    QFile savedFile(savedFilePath);
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonDocument document = QJsonDocument::fromJson(savedFile.readAll());
    QVERIFY(document.isObject());
    QVERIFY(document.object().contains(QStringLiteral("birthInfo")));
    QVERIFY(document.object().contains(QStringLiteral("chartResult")));
    QVERIFY(document.object().contains(QStringLiteral("interpretationResult")));
}

void CoreTests::appControllerCanSaveCurrentRecord()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    AppController controller;
    controller.setRecordStorageDirectory(temporaryDir.path());
    controller.setBirthInfo(
        QStringLiteral("1988-12-24"),
        QStringLiteral("06:15"),
        QStringLiteral("未指定")
    );

    controller.calculateChartResult();
    controller.calculateInterpretationResult();

    QVERIFY(controller.saveCurrentRecord());
    QVERIFY(!controller.lastSaveMessage().isEmpty());
}

QTEST_MAIN(CoreTests)

#include "CoreTests.moc"

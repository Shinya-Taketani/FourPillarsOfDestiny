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
#include "RecordExportService.h"
#include "SavedChartRecord.h"
#include "SolarTermDataSource.h"
#include "SolarTermResolver.h"

class CoreTests : public QObject
{
    Q_OBJECT

private slots:
    void chartCalculatorReturnsNonEmptyResult();
    void chartCalculatorYearPillarChangesWithBirthYear();
    void chartCalculatorReturnsStableResultForSameInput();
    void chartCalculatorHourPillarChangesWithBirthTime();
    void chartCalculatorHandlesMissingBirthTime();
    void chartCalculatorReturnsMonthPillarFor1955SampleYear();
    void chartCalculatorReturnsMonthPillarFor1971YearStartCase();
    void chartCalculatorReturnsMonthPillarFor1985SampleYear();
    void chartCalculatorReturnsMonthPillarFor1995SampleYear();
    void chartCalculatorReturnsMonthPillarForSupportedSampleYear();
    void chartCalculatorCalculatesTenGodsForSupportedSampleYear();
    void chartCalculatorCalculatesHiddenStemsForSupportedSampleYear();
    void chartCalculatorCalculatesFiveElementsForSupportedSampleYear();
    void chartCalculatorCalculatesSeasonalEvaluationForSupportedSampleYear();
    void chartCalculatorCalculatesStrengthEvaluationForSupportedSampleYear();
    void chartCalculatorCalculatesClimateEvaluationForSupportedSampleYear();
    void chartCalculatorCalculatesUsefulGodCandidatesForSupportedSampleYear();
    void chartCalculatorCalculatesPatternCandidatesForSupportedSampleYear();
    void chartCalculatorCalculatesMajorFortunesForSupportedSampleYear();
    void chartCalculatorCalculatesAnnualFortunesForSupportedSampleYear();
    void chartCalculatorChangesMonthPillarAcross1955SolarTermBoundary();
    void chartCalculatorChangesMonthPillarAcross1971Boundary();
    void chartCalculatorChangesMonthPillarAcross1985SolarTermBoundary();
    void chartCalculatorChangesMonthPillarAcrossSolarTermBoundary();
    void chartCalculatorChangesClimateEvaluationAcross1971Boundary();
    void chartCalculatorChangesUsefulGodCandidatesAcross1971Boundary();
    void chartCalculatorChangesPatternCandidatesAcross1971Boundary();
    void chartCalculatorChangesMajorFortunesAcross1971Boundary();
    void chartCalculatorChangesAnnualFortunesAcross1971Boundary();
    void chartCalculatorReturnsUnsupportedMonthPillarForUnsupportedYear();
    void chartCalculatorProvidesMonthPillarStatusMessage();
    void solarTermDataSourceLoads1955YearData();
    void solarTermDataSourceLoads1971YearData();
    void solarTermDataSourceLoads1985YearData();
    void solarTermDataSourceLoads1995YearData();
    void solarTermDataSourceLoadsSampleYearData();
    void solarTermDataSourceHandlesMissingYearData();
    void solarTermResolverReturnsSupportedSampleContract();
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
    void jsonRecordStorageListsSavedFiles();
    void jsonRecordStorageLoadsSavedRecord();
    void appControllerCanListAndLoadSavedRecords();
    void recordExportServiceWritesTextFile();
    void recordExportServiceWritesJsonFile();
    void appControllerCanExportCurrentRecord();
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

void CoreTests::chartCalculatorYearPillarChangesWithBirthYear()
{
    ChartCalculator calculator;

    const BirthInfo birthInfo1984{
        QStringLiteral("1984-06-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo birthInfo1985{
        QStringLiteral("1985-06-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result1984 = calculator.calculate(birthInfo1984);
    const ChartResult result1985 = calculator.calculate(birthInfo1985);

    QCOMPARE(result1984.yearPillar, QStringLiteral("甲子"));
    QCOMPARE(result1985.yearPillar, QStringLiteral("乙丑"));
    QVERIFY(result1984.yearPillar != result1985.yearPillar);
    QVERIFY(result1984.tenGods.value(QStringLiteral("yearPillar")).toString()
            != result1985.tenGods.value(QStringLiteral("yearPillar")).toString());
}

void CoreTests::chartCalculatorReturnsStableResultForSameInput()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-01-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult firstResult = calculator.calculate(birthInfo);
    const ChartResult secondResult = calculator.calculate(birthInfo);

    QCOMPARE(firstResult.yearPillar, secondResult.yearPillar);
    QCOMPARE(firstResult.monthPillar, secondResult.monthPillar);
    QCOMPARE(firstResult.dayPillar, secondResult.dayPillar);
    QCOMPARE(firstResult.hourPillar, secondResult.hourPillar);
    QCOMPARE(firstResult.tenGods, secondResult.tenGods);
    QCOMPARE(firstResult.hiddenStems, secondResult.hiddenStems);
    QCOMPARE(firstResult.fiveElements, secondResult.fiveElements);
    QCOMPARE(
        firstResult.fiveElementDistributionStatusMessage,
        secondResult.fiveElementDistributionStatusMessage
    );
    QCOMPARE(firstResult.seasonalEvaluation, secondResult.seasonalEvaluation);
    QCOMPARE(
        firstResult.seasonalEvaluationStatusMessage,
        secondResult.seasonalEvaluationStatusMessage
    );
    QCOMPARE(firstResult.strengthEvaluation, secondResult.strengthEvaluation);
    QCOMPARE(
        firstResult.strengthEvaluationStatusMessage,
        secondResult.strengthEvaluationStatusMessage
    );
    QCOMPARE(firstResult.climateEvaluation, secondResult.climateEvaluation);
    QCOMPARE(
        firstResult.climateEvaluationStatusMessage,
        secondResult.climateEvaluationStatusMessage
    );
    QCOMPARE(firstResult.usefulGodCandidates, secondResult.usefulGodCandidates);
    QCOMPARE(
        firstResult.usefulGodCandidatesStatusMessage,
        secondResult.usefulGodCandidatesStatusMessage
    );
    QCOMPARE(firstResult.patternCandidates, secondResult.patternCandidates);
    QCOMPARE(
        firstResult.patternCandidatesStatusMessage,
        secondResult.patternCandidatesStatusMessage
    );
    QCOMPARE(firstResult.majorFortunes, secondResult.majorFortunes);
    QCOMPARE(
        firstResult.majorFortunesStatusMessage,
        secondResult.majorFortunesStatusMessage
    );
    QCOMPARE(firstResult.annualFortunes, secondResult.annualFortunes);
    QCOMPARE(
        firstResult.annualFortunesStatusMessage,
        secondResult.annualFortunesStatusMessage
    );
}

void CoreTests::chartCalculatorHourPillarChangesWithBirthTime()
{
    ChartCalculator calculator;

    const BirthInfo earlyBirthInfo{
        QStringLiteral("1990-01-01"),
        QStringLiteral("00:30"),
        QStringLiteral("男性")
    };
    const BirthInfo lateBirthInfo{
        QStringLiteral("1990-01-01"),
        QStringLiteral("03:30"),
        QStringLiteral("男性")
    };

    const ChartResult earlyResult = calculator.calculate(earlyBirthInfo);
    const ChartResult lateResult = calculator.calculate(lateBirthInfo);

    QVERIFY(earlyResult.hourPillar != lateResult.hourPillar);
}

void CoreTests::chartCalculatorHandlesMissingBirthTime()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-01-01"),
        QString(),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.hourPillar, QStringLiteral("時柱未計算"));
}

void CoreTests::chartCalculatorReturnsMonthPillarFor1955SampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1955-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("戊寅"));
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("節入りサンプルデータによる限定実装")));
}

void CoreTests::chartCalculatorReturnsMonthPillarFor1971YearStartCase()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("己丑"));
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("前年の最終月区間")));
}

void CoreTests::chartCalculatorReturnsMonthPillarFor1985SampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1985-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("戊寅"));
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("節入りサンプルデータによる限定実装")));
}

void CoreTests::chartCalculatorReturnsMonthPillarFor1995SampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1995-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("戊寅"));
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("節入りサンプルデータによる限定実装")));
}

void CoreTests::chartCalculatorReturnsMonthPillarForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("戊寅"));
    QVERIFY(!result.monthPillarStatusMessage.isEmpty());
    QVERIFY(result.description.contains(QStringLiteral("節入りサンプルデータによる限定実装")));
}

void CoreTests::chartCalculatorCalculatesTenGodsForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.dayPillar, QStringLiteral("辛丑"));
    QCOMPARE(result.hourPillar, QStringLiteral("乙未"));
    QCOMPARE(result.tenGods.value(QStringLiteral("yearPillar")).toString(), QStringLiteral("劫財"));
    QCOMPARE(result.tenGods.value(QStringLiteral("monthPillar")).toString(), QStringLiteral("印綬"));
    QCOMPARE(result.tenGods.value(QStringLiteral("dayPillar")).toString(), QStringLiteral("日主"));
    QCOMPARE(result.tenGods.value(QStringLiteral("hourPillar")).toString(), QStringLiteral("偏財"));
}

void CoreTests::chartCalculatorCalculatesHiddenStemsForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QStringList expectedYearHiddenStems{QStringLiteral("丁"), QStringLiteral("己")};
    const QStringList expectedMonthHiddenStems{QStringLiteral("甲"), QStringLiteral("丙"), QStringLiteral("戊")};
    const QStringList expectedDayHiddenStems{QStringLiteral("己"), QStringLiteral("癸"), QStringLiteral("辛")};
    const QStringList expectedHourHiddenStems{QStringLiteral("己"), QStringLiteral("丁"), QStringLiteral("乙")};

    QCOMPARE(result.hiddenStems.value(QStringLiteral("yearPillar")).toStringList(), expectedYearHiddenStems);
    QCOMPARE(result.hiddenStems.value(QStringLiteral("monthPillar")).toStringList(), expectedMonthHiddenStems);
    QCOMPARE(result.hiddenStems.value(QStringLiteral("dayPillar")).toStringList(), expectedDayHiddenStems);
    QCOMPARE(result.hiddenStems.value(QStringLiteral("hourPillar")).toStringList(), expectedHourHiddenStems);
}

void CoreTests::chartCalculatorCalculatesFiveElementsForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.fiveElements.value(QStringLiteral("wood")).toInt(), 4);
    QCOMPARE(result.fiveElements.value(QStringLiteral("fire")).toInt(), 4);
    QCOMPARE(result.fiveElements.value(QStringLiteral("earth")).toInt(), 7);
    QCOMPARE(result.fiveElements.value(QStringLiteral("metal")).toInt(), 3);
    QCOMPARE(result.fiveElements.value(QStringLiteral("water")).toInt(), 1);
    QVERIFY(result.fiveElementDistributionStatusMessage.contains(QStringLiteral("単純件数")));
}

void CoreTests::chartCalculatorCalculatesSeasonalEvaluationForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.seasonalEvaluation.value(QStringLiteral("monthBranch")).toString(), QStringLiteral("寅"));
    QCOMPARE(result.seasonalEvaluation.value(QStringLiteral("season")).toString(), QStringLiteral("春"));
    QCOMPARE(result.seasonalEvaluation.value(QStringLiteral("suitability")).toString(), QStringLiteral("不利"));
    QVERIFY(result.seasonalEvaluationStatusMessage.contains(QStringLiteral("月支ベース")));
}

void CoreTests::chartCalculatorCalculatesStrengthEvaluationForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.strengthEvaluation.value(QStringLiteral("label")).toString(), QStringLiteral("やや強め"));
    QVERIFY(result.strengthEvaluation.value(QStringLiteral("reason")).toString().contains(QStringLiteral("暫定判定")));
    QVERIFY(result.strengthEvaluationStatusMessage.contains(QStringLiteral("暫定的")));
}

void CoreTests::chartCalculatorCalculatesClimateEvaluationForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.climateEvaluation.value(QStringLiteral("monthBranch")).toString(), QStringLiteral("寅"));
    QCOMPARE(result.climateEvaluation.value(QStringLiteral("temperature")).toString(), QStringLiteral("やや暖"));
    QCOMPARE(result.climateEvaluation.value(QStringLiteral("moisture")).toString(), QStringLiteral("やや湿"));
    QVERIFY(result.climateEvaluation.value(QStringLiteral("note")).toString().contains(QStringLiteral("簡易評価")));
    QVERIFY(result.climateEvaluationStatusMessage.contains(QStringLiteral("月支ベース")));
}

void CoreTests::chartCalculatorCalculatesUsefulGodCandidatesForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QStringList candidates = result.usefulGodCandidates.value(QStringLiteral("candidates")).toStringList();

    QCOMPARE(candidates.size(), 3);
    QCOMPARE(candidates.first(), QStringLiteral("水"));
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("reason")).toString().contains(QStringLiteral("五行分布")));
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("note")).toString().contains(QStringLiteral("断定")));
    QVERIFY(result.usefulGodCandidatesStatusMessage.contains(QStringLiteral("暫定候補")));
}

void CoreTests::chartCalculatorCalculatesPatternCandidatesForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QStringList candidates = result.patternCandidates.value(QStringLiteral("candidates")).toStringList();

    QCOMPARE(candidates.size(), 3);
    QCOMPARE(candidates.at(0), QStringLiteral("印綬格"));
    QCOMPARE(candidates.at(1), QStringLiteral("正財格"));
    QCOMPARE(candidates.at(2), QStringLiteral("正官格"));
    QVERIFY(result.patternCandidates.value(QStringLiteral("reason")).toString().contains(QStringLiteral("月干通変星")));
    QVERIFY(result.patternCandidates.value(QStringLiteral("note")).toString().contains(QStringLiteral("断定")));
    QVERIFY(result.patternCandidatesStatusMessage.contains(QStringLiteral("暫定候補")));
}

void CoreTests::chartCalculatorCalculatesMajorFortunesForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QVariantList majorFortunes = result.majorFortunes;

    QCOMPARE(majorFortunes.size(), 8);
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("startAge")).toInt(), 1);
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("endAge")).toInt(), 10);
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("戊寅"));
    QCOMPARE(majorFortunes.at(1).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("己卯"));
    QVERIFY(majorFortunes.at(0).toMap().value(QStringLiteral("note")).toString().contains(QStringLiteral("仮表示")));
    QVERIFY(result.majorFortunesStatusMessage.contains(QStringLiteral("仮骨格")));
}

void CoreTests::chartCalculatorCalculatesAnnualFortunesForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QVariantList annualFortunes = result.annualFortunes;

    QCOMPARE(annualFortunes.size(), 12);
    QCOMPARE(annualFortunes.at(0).toMap().value(QStringLiteral("year")).toInt(), 1990);
    QCOMPARE(annualFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("庚午"));
    QCOMPARE(annualFortunes.at(1).toMap().value(QStringLiteral("year")).toInt(), 1991);
    QCOMPARE(annualFortunes.at(1).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("辛未"));
    QVERIFY(annualFortunes.at(0).toMap().value(QStringLiteral("note")).toString().contains(QStringLiteral("参考表示")));
    QVERIFY(result.annualFortunesStatusMessage.contains(QStringLiteral("仮骨格")));
}

void CoreTests::chartCalculatorChangesMonthPillarAcross1955SolarTermBoundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1955-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1955-03-06"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.monthPillar, QStringLiteral("戊寅"));
    QCOMPARE(afterResult.monthPillar, QStringLiteral("己卯"));
    QVERIFY(beforeResult.monthPillar != afterResult.monthPillar);
}

void CoreTests::chartCalculatorChangesMonthPillarAcross1971Boundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1971-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.monthPillar, QStringLiteral("己丑"));
    QCOMPARE(afterResult.monthPillar, QStringLiteral("庚寅"));
    QVERIFY(beforeResult.monthPillar != afterResult.monthPillar);
}

void CoreTests::chartCalculatorChangesMonthPillarAcross1985SolarTermBoundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1985-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1985-03-06"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.monthPillar, QStringLiteral("戊寅"));
    QCOMPARE(afterResult.monthPillar, QStringLiteral("己卯"));
    QVERIFY(beforeResult.monthPillar != afterResult.monthPillar);
}

void CoreTests::solarTermDataSourceLoads1955YearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1955);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1955);
    QVERIFY(!yearData.entries.isEmpty());
}

void CoreTests::solarTermDataSourceLoads1971YearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1971);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1971);
    QVERIFY(!yearData.entries.isEmpty());
}

void CoreTests::solarTermDataSourceLoads1985YearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1985);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1985);
    QVERIFY(!yearData.entries.isEmpty());
}

void CoreTests::solarTermDataSourceLoads1995YearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1995);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1995);
    QVERIFY(!yearData.entries.isEmpty());
}

void CoreTests::solarTermDataSourceLoadsSampleYearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1990);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1990);
    QVERIFY(!yearData.entries.isEmpty());
}

void CoreTests::solarTermDataSourceHandlesMissingYearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1949);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(!yearData.hasYearData);
    QCOMPARE(yearData.year, 1949);
    QVERIFY(yearData.entries.isEmpty());
}

void CoreTests::chartCalculatorChangesMonthPillarAcrossSolarTermBoundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1990-03-06"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.monthPillar, QStringLiteral("戊寅"));
    QCOMPARE(afterResult.monthPillar, QStringLiteral("己卯"));
    QVERIFY(beforeResult.monthPillar != afterResult.monthPillar);
}

void CoreTests::chartCalculatorChangesClimateEvaluationAcross1971Boundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1971-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.climateEvaluation.value(QStringLiteral("temperature")).toString(), QStringLiteral("寒"));
    QCOMPARE(afterResult.climateEvaluation.value(QStringLiteral("temperature")).toString(), QStringLiteral("やや暖"));
    QVERIFY(beforeResult.climateEvaluation != afterResult.climateEvaluation);
}

void CoreTests::chartCalculatorChangesUsefulGodCandidatesAcross1971Boundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1971-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QVERIFY(beforeResult.usefulGodCandidates != afterResult.usefulGodCandidates);
}

void CoreTests::chartCalculatorChangesPatternCandidatesAcross1971Boundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1971-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QVERIFY(beforeResult.patternCandidates != afterResult.patternCandidates);
}

void CoreTests::chartCalculatorChangesMajorFortunesAcross1971Boundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1971-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QVERIFY(beforeResult.majorFortunes != afterResult.majorFortunes);
}

void CoreTests::chartCalculatorChangesAnnualFortunesAcross1971Boundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1972-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QVERIFY(beforeResult.annualFortunes != afterResult.annualFortunes);
}

void CoreTests::chartCalculatorReturnsUnsupportedMonthPillarForUnsupportedYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1949-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("月柱未対応"));
    QCOMPARE(result.monthPillarStatusMessage, QStringLiteral("節入りデータは外部 JSON 方式を採用していますが、指定年データが未整備です。"));
    QCOMPARE(result.tenGods.value(QStringLiteral("monthPillar")).toString(), QStringLiteral("未対応"));
    QCOMPARE(
        result.hiddenStems.value(QStringLiteral("monthPillar")).toStringList(),
        QStringList{QStringLiteral("未対応")}
    );
    QVERIFY(result.fiveElementDistributionStatusMessage.contains(QStringLiteral("月柱")));
    QCOMPARE(result.seasonalEvaluation.value(QStringLiteral("season")).toString(), QStringLiteral("未対応"));
    QCOMPARE(result.seasonalEvaluation.value(QStringLiteral("suitability")).toString(), QStringLiteral("未対応"));
    QVERIFY(result.seasonalEvaluationStatusMessage.contains(QStringLiteral("月柱未対応")));
    QCOMPARE(result.strengthEvaluation.value(QStringLiteral("label")).toString(), QStringLiteral("未対応"));
    QVERIFY(result.strengthEvaluationStatusMessage.contains(QStringLiteral("月柱未対応")));
    QCOMPARE(result.climateEvaluation.value(QStringLiteral("temperature")).toString(), QStringLiteral("未対応"));
    QCOMPARE(result.climateEvaluation.value(QStringLiteral("moisture")).toString(), QStringLiteral("未対応"));
    QVERIFY(result.climateEvaluationStatusMessage.contains(QStringLiteral("月柱未対応")));
    QCOMPARE(
        result.usefulGodCandidates.value(QStringLiteral("candidates")).toStringList(),
        QStringList{QStringLiteral("未対応")}
    );
    QVERIFY(result.usefulGodCandidatesStatusMessage.contains(QStringLiteral("未対応")));
    QCOMPARE(
        result.patternCandidates.value(QStringLiteral("candidates")).toStringList(),
        QStringList{QStringLiteral("未対応")}
    );
    QVERIFY(result.patternCandidatesStatusMessage.contains(QStringLiteral("未対応")));
    QCOMPARE(result.majorFortunes.size(), 1);
    QCOMPARE(result.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("未対応"));
    QVERIFY(result.majorFortunesStatusMessage.contains(QStringLiteral("未対応")));
    QCOMPARE(result.annualFortunes.size(), 12);
    QCOMPARE(result.annualFortunes.at(0).toMap().value(QStringLiteral("year")).toInt(), 1949);
    QCOMPARE(result.annualFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("己丑"));
    QVERIFY(result.annualFortunesStatusMessage.contains(QStringLiteral("仮骨格")));
    QVERIFY(result.description.contains(QStringLiteral("指定年データが未整備")));
}

void CoreTests::chartCalculatorProvidesMonthPillarStatusMessage()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1971-02-01"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("己丑"));
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("前年の最終月区間")));
}

void CoreTests::solarTermResolverReturnsSupportedSampleContract()
{
    SolarTermResolver resolver;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const QString yearPillar = QStringLiteral("庚午");

    const SolarTermResolution resolution = resolver.resolveMonthPillar(birthInfo, yearPillar);

    QVERIFY(resolution.isImplemented);
    QVERIFY(resolution.canDetermineMonthPillar);
    QCOMPARE(resolution.monthPillar, QStringLiteral("戊寅"));
    QVERIFY(resolution.statusMessage.contains(QStringLiteral("節入りサンプルデータによる限定実装")));
}

void CoreTests::chartResultToVariantMapContainsRequiredKeys()
{
    ChartResult result{
        QStringLiteral("甲子"),
        QStringLiteral("乙丑"),
        QStringLiteral("丙寅"),
        QStringLiteral("丁卯"),
        QStringLiteral("これは仮の命式結果です。"),
        QStringLiteral("月柱は限定実装です。"),
        {
            {QStringLiteral("yearPillar"), QStringLiteral("未実装")},
            {QStringLiteral("monthPillar"), QStringLiteral("未実装")},
            {QStringLiteral("dayPillar"), QStringLiteral("未実装")},
            {QStringLiteral("hourPillar"), QStringLiteral("未実装")}
        },
        {
            {QStringLiteral("yearPillar"), QStringList{QStringLiteral("甲")}},
            {QStringLiteral("monthPillar"), QStringList{QStringLiteral("乙"), QStringLiteral("癸")}},
            {QStringLiteral("dayPillar"), QStringList{QStringLiteral("丙")}},
            {QStringLiteral("hourPillar"), QStringList{QStringLiteral("丁"), QStringLiteral("辛")}}
        },
        {
            {QStringLiteral("wood"), 1},
            {QStringLiteral("fire"), 2},
            {QStringLiteral("earth"), 3},
            {QStringLiteral("metal"), 4},
            {QStringLiteral("water"), 5}
        },
        QStringLiteral("五行分布の最小集計です。"),
        {
            {QStringLiteral("monthBranch"), QStringLiteral("寅")},
            {QStringLiteral("season"), QStringLiteral("春")},
            {QStringLiteral("suitability"), QStringLiteral("有利")}
        },
        QStringLiteral("季節評価の最小判定です。"),
        {
            {QStringLiteral("label"), QStringLiteral("やや強め")},
            {QStringLiteral("reason"), QStringLiteral("暫定理由です。")},
            {QStringLiteral("score"), 2}
        },
        QStringLiteral("暫定強弱評価です。"),
        {
            {QStringLiteral("monthBranch"), QStringLiteral("寅")},
            {QStringLiteral("temperature"), QStringLiteral("やや暖")},
            {QStringLiteral("moisture"), QStringLiteral("やや湿")},
            {QStringLiteral("note"), QStringLiteral("調候前提の簡易評価です。")}
        },
        QStringLiteral("寒暖・乾湿評価です。"),
        {
            {QStringLiteral("candidates"), QStringList{QStringLiteral("水"), QStringLiteral("金"), QStringLiteral("木")}},
            {QStringLiteral("reason"), QStringLiteral("暫定候補理由です。")},
            {QStringLiteral("note"), QStringLiteral("断定しない参考候補です。")}
        },
        QStringLiteral("用神候補の暫定表示です。"),
        {
            {QStringLiteral("candidates"), QStringList{QStringLiteral("印綬格"), QStringLiteral("正財格"), QStringLiteral("正官格")}},
            {QStringLiteral("reason"), QStringLiteral("格局候補の暫定理由です。")},
            {QStringLiteral("note"), QStringLiteral("断定しない格局候補です。")}
        },
        QStringLiteral("格局候補の暫定表示です。"),
        QVariantList{
            QVariantMap{
                {QStringLiteral("index"), 1},
                {QStringLiteral("startAge"), 1},
                {QStringLiteral("endAge"), 10},
                {QStringLiteral("label"), QStringLiteral("1〜10歳")},
                {QStringLiteral("pillar"), QStringLiteral("戊寅")},
                {QStringLiteral("note"), QStringLiteral("大運表示骨格の仮データです。")}
            }
        },
        QStringLiteral("大運表示の仮骨格です。"),
        QVariantList{
            QVariantMap{
                {QStringLiteral("year"), 1990},
                {QStringLiteral("pillar"), QStringLiteral("庚午")},
                {QStringLiteral("note"), QStringLiteral("流年表示骨格の仮データです。")}
            }
        },
        QStringLiteral("流年表示の仮骨格です。")
    };

    const QVariantMap resultMap = result.toVariantMap();
    const QStringList expectedMonthHiddenStems{QStringLiteral("乙"), QStringLiteral("癸")};
    const QStringList expectedUsefulGodCandidates{
        QStringLiteral("水"),
        QStringLiteral("金"),
        QStringLiteral("木")
    };
    const QStringList expectedPatternCandidates{
        QStringLiteral("印綬格"),
        QStringLiteral("正財格"),
        QStringLiteral("正官格")
    };

    QVERIFY(resultMap.contains(QStringLiteral("yearPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("monthPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("dayPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("hourPillar")));
    QVERIFY(resultMap.contains(QStringLiteral("description")));
    QVERIFY(resultMap.contains(QStringLiteral("monthPillarStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("tenGods")));
    QVERIFY(resultMap.contains(QStringLiteral("hiddenStems")));
    QVERIFY(resultMap.contains(QStringLiteral("fiveElements")));
    QVERIFY(resultMap.contains(QStringLiteral("fiveElementDistributionStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("seasonalEvaluation")));
    QVERIFY(resultMap.contains(QStringLiteral("seasonalEvaluationStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("strengthEvaluation")));
    QVERIFY(resultMap.contains(QStringLiteral("strengthEvaluationStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("climateEvaluation")));
    QVERIFY(resultMap.contains(QStringLiteral("climateEvaluationStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("usefulGodCandidates")));
    QVERIFY(resultMap.contains(QStringLiteral("usefulGodCandidatesStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("patternCandidates")));
    QVERIFY(resultMap.contains(QStringLiteral("patternCandidatesStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("majorFortunes")));
    QVERIFY(resultMap.contains(QStringLiteral("majorFortunesStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("annualFortunes")));
    QVERIFY(resultMap.contains(QStringLiteral("annualFortunesStatusMessage")));
    QCOMPARE(resultMap.value(QStringLiteral("monthPillarStatusMessage")).toString(), QStringLiteral("月柱は限定実装です。"));
    QCOMPARE(
        resultMap.value(QStringLiteral("tenGods")).toMap().value(QStringLiteral("yearPillar")).toString(),
        QStringLiteral("未実装")
    );
    QCOMPARE(resultMap.value(QStringLiteral("hiddenStems")).toMap().value(QStringLiteral("monthPillar")).toStringList(), expectedMonthHiddenStems);
    QCOMPARE(resultMap.value(QStringLiteral("fiveElements")).toMap().value(QStringLiteral("earth")).toInt(), 3);
    QCOMPARE(
        resultMap.value(QStringLiteral("fiveElementDistributionStatusMessage")).toString(),
        QStringLiteral("五行分布の最小集計です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("seasonalEvaluation")).toMap().value(QStringLiteral("season")).toString(),
        QStringLiteral("春")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("seasonalEvaluationStatusMessage")).toString(),
        QStringLiteral("季節評価の最小判定です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("strengthEvaluation")).toMap().value(QStringLiteral("label")).toString(),
        QStringLiteral("やや強め")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("strengthEvaluationStatusMessage")).toString(),
        QStringLiteral("暫定強弱評価です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("climateEvaluation")).toMap().value(QStringLiteral("temperature")).toString(),
        QStringLiteral("やや暖")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("climateEvaluationStatusMessage")).toString(),
        QStringLiteral("寒暖・乾湿評価です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("usefulGodCandidates")).toMap().value(QStringLiteral("candidates")).toStringList(),
        expectedUsefulGodCandidates
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("usefulGodCandidatesStatusMessage")).toString(),
        QStringLiteral("用神候補の暫定表示です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("patternCandidates")).toMap().value(QStringLiteral("candidates")).toStringList(),
        expectedPatternCandidates
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("patternCandidatesStatusMessage")).toString(),
        QStringLiteral("格局候補の暫定表示です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunes")).toList().at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("戊寅")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunesStatusMessage")).toString(),
        QStringLiteral("大運表示の仮骨格です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("annualFortunes")).toList().at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("庚午")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("annualFortunesStatusMessage")).toString(),
        QStringLiteral("流年表示の仮骨格です。")
    );
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

void CoreTests::jsonRecordStorageListsSavedFiles()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    JsonRecordStorage storage(temporaryDir.path());
    const SavedChartRecord record{
        BirthInfo{QStringLiteral("1991-02-03"), QStringLiteral("09:10"), QStringLiteral("女性")},
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

    QVERIFY(storage.save(record));

    const QVariantList records = storage.listRecords();
    QVERIFY(!records.isEmpty());

    const QVariantMap firstRecord = records.first().toMap();
    QVERIFY(firstRecord.contains(QStringLiteral("filePath")));
    QVERIFY(firstRecord.contains(QStringLiteral("savedAt")));
    QCOMPARE(firstRecord.value(QStringLiteral("birthDate")).toString(), QStringLiteral("1991-02-03"));
}

void CoreTests::jsonRecordStorageLoadsSavedRecord()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    JsonRecordStorage storage(temporaryDir.path());
    const SavedChartRecord savedRecord{
        BirthInfo{QStringLiteral("1992-03-04"), QStringLiteral("10:20"), QStringLiteral("男性")},
        ChartResult{
            QStringLiteral("甲子"),
            QStringLiteral("乙丑"),
            QStringLiteral("丙寅"),
            QStringLiteral("丁卯"),
            QStringLiteral("これは仮の命式結果です。"),
            QStringLiteral("月柱は限定実装です。"),
            {
                {QStringLiteral("yearPillar"), QStringLiteral("未実装")},
                {QStringLiteral("monthPillar"), QStringLiteral("未実装")},
                {QStringLiteral("dayPillar"), QStringLiteral("未実装")},
                {QStringLiteral("hourPillar"), QStringLiteral("未実装")}
            },
            {
                {QStringLiteral("yearPillar"), QStringList{QStringLiteral("癸")}},
                {QStringLiteral("monthPillar"), QStringList{QStringLiteral("己"), QStringLiteral("癸"), QStringLiteral("辛")}},
                {QStringLiteral("dayPillar"), QStringList{QStringLiteral("甲"), QStringLiteral("丙"), QStringLiteral("戊")}},
                {QStringLiteral("hourPillar"), QStringList{QStringLiteral("乙")}}
            },
            {
                {QStringLiteral("wood"), 2},
                {QStringLiteral("fire"), 1},
                {QStringLiteral("earth"), 4},
                {QStringLiteral("metal"), 1},
                {QStringLiteral("water"), 3}
            },
            QStringLiteral("月柱まで含めた最小集計です。"),
            {
                {QStringLiteral("monthBranch"), QStringLiteral("丑")},
                {QStringLiteral("season"), QStringLiteral("冬")},
                {QStringLiteral("suitability"), QStringLiteral("中立")}
            },
            QStringLiteral("季節評価の保存確認用データです。"),
            {
                {QStringLiteral("label"), QStringLiteral("中立寄り")},
                {QStringLiteral("reason"), QStringLiteral("保存確認用の暫定理由です。")},
                {QStringLiteral("score"), 0}
            },
            QStringLiteral("暫定強弱評価の保存確認用データです。"),
            {
                {QStringLiteral("monthBranch"), QStringLiteral("丑")},
                {QStringLiteral("temperature"), QStringLiteral("寒")},
                {QStringLiteral("moisture"), QStringLiteral("やや湿")},
                {QStringLiteral("note"), QStringLiteral("保存確認用の寒暖・乾湿メモです。")}
            },
            QStringLiteral("寒暖・乾湿評価の保存確認用データです。"),
            {
                {QStringLiteral("candidates"), QStringList{QStringLiteral("火"), QStringLiteral("木"), QStringLiteral("土")}},
                {QStringLiteral("reason"), QStringLiteral("保存確認用の用神候補理由です。")},
                {QStringLiteral("note"), QStringLiteral("保存確認用の暫定候補注記です。")}
            },
            QStringLiteral("用神候補の保存確認用データです。"),
            {
                {QStringLiteral("candidates"), QStringList{QStringLiteral("偏財格"), QStringLiteral("食神格"), QStringLiteral("偏官格")}},
                {QStringLiteral("reason"), QStringLiteral("保存確認用の格局候補理由です。")},
                {QStringLiteral("note"), QStringLiteral("保存確認用の格局候補注記です。")}
            },
            QStringLiteral("格局候補の保存確認用データです。"),
            QVariantList{
                QVariantMap{
                    {QStringLiteral("index"), 1},
                    {QStringLiteral("startAge"), 1},
                    {QStringLiteral("endAge"), 10},
                    {QStringLiteral("label"), QStringLiteral("1〜10歳")},
                    {QStringLiteral("pillar"), QStringLiteral("己丑")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の大運仮表示です。")}
                },
                QVariantMap{
                    {QStringLiteral("index"), 2},
                    {QStringLiteral("startAge"), 11},
                    {QStringLiteral("endAge"), 20},
                    {QStringLiteral("label"), QStringLiteral("11〜20歳")},
                    {QStringLiteral("pillar"), QStringLiteral("庚寅")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の大運仮表示です。")}
                }
            },
            QStringLiteral("大運表示の保存確認用データです。"),
            QVariantList{
                QVariantMap{
                    {QStringLiteral("year"), 1992},
                    {QStringLiteral("pillar"), QStringLiteral("壬申")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の流年仮表示です。")}
                },
                QVariantMap{
                    {QStringLiteral("year"), 1993},
                    {QStringLiteral("pillar"), QStringLiteral("癸酉")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の流年仮表示です。")}
                }
            },
            QStringLiteral("流年表示の保存確認用データです。")
        },
        InterpretationResult{
            QStringLiteral("これは仮の解釈結果です。"),
            QStringLiteral("本実装の解釈ロジックは未対応です。"),
            QStringLiteral("要確認事項があります。")
        },
        QStringLiteral("2026-04-05T00:00:00Z")
    };

    QString filePath;
    QVERIFY(storage.save(savedRecord, &filePath));

    SavedChartRecord loadedRecord;
    QString errorMessage;
    QVERIFY(storage.load(filePath, &loadedRecord, &errorMessage));
    QVERIFY(errorMessage.isEmpty());
    const QStringList expectedMonthHiddenStems{QStringLiteral("己"), QStringLiteral("癸"), QStringLiteral("辛")};
    const QStringList expectedUsefulGodCandidates{
        QStringLiteral("火"),
        QStringLiteral("木"),
        QStringLiteral("土")
    };
    const QStringList expectedPatternCandidates{
        QStringLiteral("偏財格"),
        QStringLiteral("食神格"),
        QStringLiteral("偏官格")
    };
    QCOMPARE(loadedRecord.birthInfo.birthDate, QStringLiteral("1992-03-04"));
    QCOMPARE(loadedRecord.birthInfo.gender, QStringLiteral("男性"));
    QCOMPARE(loadedRecord.chartResult.yearPillar, QStringLiteral("甲子"));
    QCOMPARE(loadedRecord.chartResult.monthPillarStatusMessage, QStringLiteral("月柱は限定実装です。"));
    QCOMPARE(
        loadedRecord.chartResult.tenGods.value(QStringLiteral("monthPillar")).toString(),
        QStringLiteral("未実装")
    );
    QCOMPARE(loadedRecord.chartResult.hiddenStems.value(QStringLiteral("monthPillar")).toStringList(), expectedMonthHiddenStems);
    QCOMPARE(loadedRecord.chartResult.fiveElements.value(QStringLiteral("earth")).toInt(), 4);
    QCOMPARE(
        loadedRecord.chartResult.fiveElementDistributionStatusMessage,
        QStringLiteral("月柱まで含めた最小集計です。")
    );
    QCOMPARE(
        loadedRecord.chartResult.seasonalEvaluation.value(QStringLiteral("season")).toString(),
        QStringLiteral("冬")
    );
    QCOMPARE(
        loadedRecord.chartResult.seasonalEvaluationStatusMessage,
        QStringLiteral("季節評価の保存確認用データです。")
    );
    QCOMPARE(
        loadedRecord.chartResult.strengthEvaluation.value(QStringLiteral("label")).toString(),
        QStringLiteral("中立寄り")
    );
    QCOMPARE(
        loadedRecord.chartResult.strengthEvaluationStatusMessage,
        QStringLiteral("暫定強弱評価の保存確認用データです。")
    );
    QCOMPARE(
        loadedRecord.chartResult.climateEvaluation.value(QStringLiteral("temperature")).toString(),
        QStringLiteral("寒")
    );
    QCOMPARE(
        loadedRecord.chartResult.climateEvaluationStatusMessage,
        QStringLiteral("寒暖・乾湿評価の保存確認用データです。")
    );
    QCOMPARE(
        loadedRecord.chartResult.usefulGodCandidates.value(QStringLiteral("candidates")).toStringList(),
        expectedUsefulGodCandidates
    );
    QCOMPARE(
        loadedRecord.chartResult.usefulGodCandidatesStatusMessage,
        QStringLiteral("用神候補の保存確認用データです。")
    );
    QCOMPARE(
        loadedRecord.chartResult.patternCandidates.value(QStringLiteral("candidates")).toStringList(),
        expectedPatternCandidates
    );
    QCOMPARE(
        loadedRecord.chartResult.patternCandidatesStatusMessage,
        QStringLiteral("格局候補の保存確認用データです。")
    );
    QCOMPARE(loadedRecord.chartResult.majorFortunes.size(), 2);
    QCOMPARE(
        loadedRecord.chartResult.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("己丑")
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortunesStatusMessage,
        QStringLiteral("大運表示の保存確認用データです。")
    );
    QCOMPARE(loadedRecord.chartResult.annualFortunes.size(), 2);
    QCOMPARE(
        loadedRecord.chartResult.annualFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("壬申")
    );
    QCOMPARE(
        loadedRecord.chartResult.annualFortunesStatusMessage,
        QStringLiteral("流年表示の保存確認用データです。")
    );
    QCOMPARE(loadedRecord.interpretationResult.summaryText, QStringLiteral("これは仮の解釈結果です。"));
}

void CoreTests::appControllerCanListAndLoadSavedRecords()
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

    const QVariantList records = controller.savedRecords();
    QVERIFY(!records.isEmpty());

    const QString filePath = records.first().toMap().value(QStringLiteral("filePath")).toString();
    QVERIFY(!filePath.isEmpty());
    QVERIFY(controller.loadSavedRecord(filePath));
    QCOMPARE(controller.currentBirthInfo().value(QStringLiteral("birthDate")).toString(), QStringLiteral("1988-12-24"));
    QVERIFY(!controller.currentChartResult().value(QStringLiteral("yearPillar")).toString().isEmpty());
    QVERIFY(!controller.currentInterpretationResult().value(QStringLiteral("summaryText")).toString().isEmpty());
}

void CoreTests::recordExportServiceWritesTextFile()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    RecordExportService exportService(temporaryDir.path());
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

    QString filePath;
    QString errorMessage;
    QVERIFY(exportService.exportAsText(record, &filePath, &errorMessage));
    QVERIFY(errorMessage.isEmpty());
    QVERIFY(QFile::exists(filePath));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());
    QVERIFY(content.contains(QStringLiteral("生年月日: 1990-01-01")));
    QVERIFY(content.contains(QStringLiteral("年柱: 甲子")));
    QVERIFY(content.contains(QStringLiteral("蔵干(年支):")));
    QVERIFY(content.contains(QStringLiteral("五行(木):")));
    QVERIFY(content.contains(QStringLiteral("季節適性:")));
    QVERIFY(content.contains(QStringLiteral("暫定強弱評価:")));
    QVERIFY(content.contains(QStringLiteral("寒暖傾向:")));
    QVERIFY(content.contains(QStringLiteral("乾湿傾向:")));
    QVERIFY(content.contains(QStringLiteral("用神候補:")));
    QVERIFY(content.contains(QStringLiteral("格局候補:")));
    QVERIFY(content.contains(QStringLiteral("大運一覧:")));
    QVERIFY(content.contains(QStringLiteral("流年一覧:")));
    QVERIFY(content.contains(QStringLiteral("summaryText: これは仮の解釈結果です。")));
}

void CoreTests::recordExportServiceWritesJsonFile()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    RecordExportService exportService(temporaryDir.path());
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

    QString filePath;
    QString errorMessage;
    QVERIFY(exportService.exportAsJson(record, &filePath, &errorMessage));
    QVERIFY(errorMessage.isEmpty());
    QVERIFY(QFile::exists(filePath));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    QVERIFY(document.isObject());
    QVERIFY(document.object().contains(QStringLiteral("exportedAt")));
    QVERIFY(document.object().contains(QStringLiteral("birthInfo")));
    QVERIFY(document.object().contains(QStringLiteral("chartResult")));
    QVERIFY(document.object().contains(QStringLiteral("interpretationResult")));
}

void CoreTests::appControllerCanExportCurrentRecord()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    AppController controller;
    controller.setExportDirectory(temporaryDir.path());
    controller.setBirthInfo(
        QStringLiteral("1988-12-24"),
        QStringLiteral("06:15"),
        QStringLiteral("未指定")
    );

    controller.calculateChartResult();
    controller.calculateInterpretationResult();

    QVERIFY(controller.exportCurrentRecordAsText());
    QVERIFY(!controller.lastExportMessage().isEmpty());
    QVERIFY(controller.exportCurrentRecordAsJson());
    QVERIFY(!controller.lastExportMessage().isEmpty());
}

QTEST_MAIN(CoreTests)

#include "CoreTests.moc"

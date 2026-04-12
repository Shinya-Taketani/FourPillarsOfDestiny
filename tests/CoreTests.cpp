#include <QtTest>

#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>
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

namespace {

QString verificationCasesFilePath()
{
    return QString::fromUtf8(FPOFD_SOURCE_DIR)
        + QStringLiteral("/tests/data/verification_cases.json");
}

QString specGapRegistryFilePath()
{
    return QString::fromUtf8(FPOFD_SOURCE_DIR)
        + QStringLiteral("/tests/data/spec_gap_registry.json");
}

QString testSamplesMasterCsvFilePath()
{
    return QString::fromUtf8(FPOFD_SOURCE_DIR)
        + QStringLiteral("/tests/data/test_samples_master.csv");
}

QString expectedPillarsCsvFilePath()
{
    return QString::fromUtf8(FPOFD_SOURCE_DIR)
        + QStringLiteral("/tests/data/expected_pillars.csv");
}

QString expectedJudgementCsvFilePath()
{
    return QString::fromUtf8(FPOFD_SOURCE_DIR)
        + QStringLiteral("/tests/data/expected_judgement.csv");
}

QString expectedRelationsCsvFilePath()
{
    return QString::fromUtf8(FPOFD_SOURCE_DIR)
        + QStringLiteral("/tests/data/expected_relations.csv");
}

QString expectedLuckCycleCsvFilePath()
{
    return QString::fromUtf8(FPOFD_SOURCE_DIR)
        + QStringLiteral("/tests/data/expected_luck_cycle.csv");
}

QStringList parseCsvLine(const QString &line)
{
    QStringList columns;
    QString current;
    bool inQuotes = false;

    for (QChar character : line) {
        if (character == QLatin1Char('"')) {
            inQuotes = !inQuotes;
            continue;
        }

        if (character == QLatin1Char(',') && !inQuotes) {
            columns << current;
            current.clear();
            continue;
        }

        current.append(character);
    }

    columns << current;
    return columns;
}

QList<QHash<QString, QString>> loadCsvRows(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString content = QString::fromUtf8(file.readAll());
    const QStringList lines = content.split(QRegularExpression(QStringLiteral("\\r?\\n")), Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return {};
    }

    const QStringList header = parseCsvLine(lines.first());
    QList<QHash<QString, QString>> rows;

    for (int index = 1; index < lines.size(); ++index) {
        const QStringList columns = parseCsvLine(lines.at(index));
        if (columns.size() != header.size()) {
            continue;
        }

        QHash<QString, QString> row;
        for (int headerIndex = 0; headerIndex < header.size(); ++headerIndex) {
            row.insert(header.at(headerIndex), columns.at(headerIndex));
        }
        rows.append(row);
    }

    return rows;
}

QJsonArray loadVerificationCases()
{
    QFile file(verificationCasesFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    return document.object().value(QStringLiteral("cases")).toArray();
}

QJsonArray loadSpecGapRegistry()
{
    QFile file(specGapRegistryFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    return document.object().value(QStringLiteral("specGaps")).toArray();
}

QJsonObject findSpecGapByRuleHint(const QJsonArray &specGaps, const QString &ruleHint)
{
    for (const QJsonValue &specGapValue : specGaps) {
        const QJsonObject specGapObject = specGapValue.toObject();
        if (specGapObject.value(QStringLiteral("ruleHint")).toString() == ruleHint) {
            return specGapObject;
        }
    }

    return {};
}

bool isExternalConfidence(const QString &confidence)
{
    return confidence == QStringLiteral("external")
        || confidence.startsWith(QStringLiteral("external_"));
}

bool isSupportedReviewStatus(const QString &reviewStatus)
{
    static const QSet<QString> supportedStatuses{
        QStringLiteral("pending"),
        QStringLiteral("confirmed_spec_gap_candidate"),
        QStringLiteral("investigate_implementation_candidate"),
        QStringLiteral("ready_for_regression_review")
    };

    return supportedStatuses.contains(reviewStatus);
}

bool isSupportedRelationScope(const QString &targetScope)
{
    return targetScope == QStringLiteral("natal")
        || targetScope == QStringLiteral("natal_vs_luck")
        || targetScope == QStringLiteral("natal_vs_year");
}

bool tryParseBooleanText(const QString &text, bool *value)
{
    if (text == QStringLiteral("true")) {
        *value = true;
        return true;
    }
    if (text == QStringLiteral("false")) {
        *value = false;
        return true;
    }

    return false;
}

QString pillarKeyToLabel(const QString &pillarKey)
{
    if (pillarKey == QStringLiteral("yearPillar")) {
        return QStringLiteral("年柱");
    }
    if (pillarKey == QStringLiteral("monthPillar")) {
        return QStringLiteral("月柱");
    }
    if (pillarKey == QStringLiteral("dayPillar")) {
        return QStringLiteral("日柱");
    }
    if (pillarKey == QStringLiteral("hourPillar")) {
        return QStringLiteral("時柱");
    }

    return pillarKey;
}

QStringList mismatchFields(
    const QString &expectedYearPillar,
    const QString &expectedMonthPillar,
    const QString &expectedDayPillar,
    const QString &expectedHourPillar,
    const ChartResult &result
)
{
    QStringList fields;

    if (result.yearPillar != expectedYearPillar) {
        fields << QStringLiteral("yearPillar");
    }
    if (result.monthPillar != expectedMonthPillar) {
        fields << QStringLiteral("monthPillar");
    }
    if (result.dayPillar != expectedDayPillar) {
        fields << QStringLiteral("dayPillar");
    }
    if (result.hourPillar != expectedHourPillar) {
        fields << QStringLiteral("hourPillar");
    }

    return fields;
}

QString classifyVerificationDifference(const QStringList &fields)
{
    if (fields.isEmpty()) {
        return QStringLiteral("match");
    }

    if (fields.size() == 2
        && fields.contains(QStringLiteral("dayPillar"))
        && fields.contains(QStringLiteral("hourPillar"))) {
        return QStringLiteral("possible_day_boundary_rule_gap");
    }

    if (fields.size() > 1) {
        return QStringLiteral("multiple_pillar_mismatch");
    }

    if (fields.contains(QStringLiteral("dayPillar"))) {
        return QStringLiteral("possible_day_boundary_rule_gap");
    }
    if (fields.contains(QStringLiteral("hourPillar"))) {
        return QStringLiteral("possible_hour_rule_gap");
    }
    if (fields.contains(QStringLiteral("monthPillar")) || fields.contains(QStringLiteral("yearPillar"))) {
        return QStringLiteral("possible_calendar_rule_gap");
    }

    return QStringLiteral("requires_manual_review");
}

QString suggestVerificationAction(const QString &classification)
{
    if (classification == QStringLiteral("match")) {
        return QStringLiteral("no_action_needed");
    }
    if (classification == QStringLiteral("possible_day_boundary_rule_gap")) {
        return QStringLiteral("keep_as_spec_gap");
    }
    if (classification == QStringLiteral("possible_hour_rule_gap")) {
        return QStringLiteral("manual_review_required");
    }
    if (classification == QStringLiteral("possible_calendar_rule_gap")) {
        return QStringLiteral("manual_review_required");
    }
    if (classification == QStringLiteral("multiple_pillar_mismatch")) {
        return QStringLiteral("investigate_implementation");
    }
    if (classification == QStringLiteral("requires_manual_review")) {
        return QStringLiteral("manual_review_required");
    }

    return QStringLiteral("manual_review_required");
}

QString determineReviewOutcome(
    const QString &actualClassification,
    const QString &suggestedAction,
    const QString &expectedDifferenceCategory,
    const QString &expectedAction
)
{
    if (expectedDifferenceCategory.isEmpty() || expectedAction.isEmpty()) {
        return QStringLiteral("manual_review_required");
    }

    if (actualClassification != expectedDifferenceCategory) {
        return QStringLiteral("classification_mismatch");
    }

    if (suggestedAction != expectedAction) {
        return QStringLiteral("action_mismatch");
    }

    if (expectedAction == QStringLiteral("keep_as_spec_gap")) {
        return QStringLiteral("confirmed_spec_gap");
    }
    if (expectedAction == QStringLiteral("investigate_implementation")) {
        return QStringLiteral("confirmed_bug_candidate");
    }
    if (expectedAction == QStringLiteral("manual_review_required")) {
        return QStringLiteral("manual_review_required");
    }
    if (expectedAction == QStringLiteral("no_action_needed")) {
        return QStringLiteral("ready_for_regression_review");
    }

    return QStringLiteral("manual_review_required");
}

QString formatVerificationDifferenceReport(
    const QJsonObject &caseObject,
    const ChartResult &result
)
{
    const QJsonObject expected = caseObject.value(QStringLiteral("expected")).toObject();
    const QStringList fields = mismatchFields(
        expected.value(QStringLiteral("yearPillar")).toString(),
        expected.value(QStringLiteral("monthPillar")).toString(),
        expected.value(QStringLiteral("dayPillar")).toString(),
        expected.value(QStringLiteral("hourPillar")).toString(),
        result
    );

    const QString classification = classifyVerificationDifference(fields);
    const QString suggestedAction = suggestVerificationAction(classification);
    const QString caseId = caseObject.value(QStringLiteral("caseId")).toString();
    const QString description = caseObject.value(QStringLiteral("description")).toString();
    const QString notes = caseObject.value(QStringLiteral("notes")).toString();
    const QString expectedDifferenceCategory =
        caseObject.value(QStringLiteral("expectedDifferenceCategory")).toString();
    const QString expectedAction =
        caseObject.value(QStringLiteral("expectedAction")).toString();
    const QString ruleHint = caseObject.value(QStringLiteral("ruleHint")).toString();
    const QString reviewStatus = caseObject.value(QStringLiteral("reviewStatus")).toString();
    const QString reviewNotes = caseObject.value(QStringLiteral("reviewNotes")).toString();
    const QString reviewOutcome = determineReviewOutcome(
        classification,
        suggestedAction,
        expectedDifferenceCategory,
        expectedAction
    );

    return QStringLiteral(
        "[%1] %2\n"
        "expected: %3 / %4 / %5 / %6\n"
        "actual: %7 / %8 / %9 / %10\n"
        "mismatch fields: %11\n"
        "actual classification: %12\n"
        "expectedDifferenceCategory: %13\n"
        "suggested action: %14\n"
        "expectedAction: %15\n"
        "ruleHint: %16\n"
        "reviewStatus: %17\n"
        "review outcome: %18\n"
        "reviewNotes: %19\n"
        "notes: %20"
    ).arg(
        caseId,
        description,
        expected.value(QStringLiteral("yearPillar")).toString(),
        expected.value(QStringLiteral("monthPillar")).toString(),
        expected.value(QStringLiteral("dayPillar")).toString(),
        expected.value(QStringLiteral("hourPillar")).toString(),
        result.yearPillar,
        result.monthPillar,
        result.dayPillar,
        result.hourPillar,
        fields.isEmpty() ? QStringLiteral("none") : fields.join(QStringLiteral(", ")),
        classification,
        expectedDifferenceCategory.isEmpty() ? QStringLiteral("未設定") : expectedDifferenceCategory,
        suggestedAction,
        expectedAction.isEmpty() ? QStringLiteral("未設定") : expectedAction,
        ruleHint.isEmpty() ? QStringLiteral("未設定") : ruleHint,
        reviewStatus.isEmpty() ? QStringLiteral("pending") : reviewStatus,
        reviewOutcome,
        reviewNotes.isEmpty() ? QStringLiteral("未設定") : reviewNotes,
        notes.isEmpty() ? QStringLiteral("外部由来ケースのため要手動確認") : notes
    );
}

}

class CoreTests : public QObject
{
    Q_OBJECT

private slots:
    void chartCalculatorReturnsNonEmptyResult();
    void chartCalculatorMatchesRegressionVerificationCases_data();
    void chartCalculatorMatchesRegressionVerificationCases();
    void chartCalculatorReportsNonRegressionVerificationDifferences();
    void testSamplesMasterCsvLoadsRows();
    void expectedPillarsCsvLoadsRows();
    void expectedPillarsCsvMatchesRegressionCaseIds();
    void expectedJudgementCsvLoadsRows();
    void expectedJudgementCsvMatchesKnownSampleIds();
    void expectedJudgementCsvContainsRegressionCases();
    void expectedRelationsCsvLoadsRows();
    void expectedRelationsCsvMatchesKnownSampleIds();
    void expectedRelationsCsvContainsSupportedScopes();
    void expectedLuckCycleCsvLoadsRows();
    void expectedLuckCycleCsvMatchesKnownSampleIds();
    void expectedLuckCycleCsvContainsRegressionCases();
    void verificationNonRegressionCasesContainReviewMetadata();
    void verificationSpecGapRegistryCoversKeepAsSpecGapCases();
    void chartCalculatorYearPillarChangesWithBirthYear();
    void chartCalculatorReturnsStableResultForSameInput();
    void chartCalculatorHourPillarChangesWithBirthTime();
    void chartCalculatorHandlesMissingBirthTime();
    void chartCalculatorChangesDayPillarAcross23HourBoundary();
    void chartCalculatorChangesYearPillarAcrossLichunBoundary();
    void chartCalculatorReturnsMonthPillarFor1955SampleYear();
    void chartCalculatorReturnsMonthPillarFor1971YearStartCase();
    void chartCalculatorReturnsMonthPillarFor1985SampleYear();
    void chartCalculatorReturnsMonthPillarFor1995SampleYear();
    void chartCalculatorReturnsMonthPillarForSupportedSampleYear();
    void chartCalculatorChangesMonthPillarAcrossPrincipalTermTimeBoundary();
    void chartCalculatorCalculatesTenGodsForSupportedSampleYear();
    void chartCalculatorCalculatesHiddenStemsForSupportedSampleYear();
    void chartCalculatorCalculatesFiveElementsForSupportedSampleYear();
    void chartCalculatorCalculatesSeasonalEvaluationForSupportedSampleYear();
    void chartCalculatorCalculatesStrengthEvaluationForSupportedSampleYear();
    void chartCalculatorCalculatesClimateEvaluationForSupportedSampleYear();
    void chartCalculatorCalculatesUsefulGodCandidatesForSupportedSampleYear();
    void chartCalculatorCalculatesPatternCandidatesForSupportedSampleYear();
    void chartCalculatorMatchesRegressionJudgementCsvForSupportedSampleYear();
    void chartCalculatorMatchesRegressionRelationsCsvForSupportedSampleYear();
    void chartCalculatorMatchesRegressionLuckCycleCsvForSupportedSampleYear();
    void chartCalculatorCalculatesMajorFortuneDirectionForSupportedSampleYear();
    void chartCalculatorChangesMajorFortuneDirectionWithYearStemPolarity();
    void chartCalculatorReturnsUndeterminedMajorFortuneDirectionForUnspecifiedGender();
    void chartCalculatorCalculatesSolarTermDifferencePreparationForSupportedSampleYear();
    void chartCalculatorChangesReferencedSolarTermByDirection();
    void chartCalculatorCalculatesMajorFortunesForSupportedSampleYear();
    void chartCalculatorReflectsMajorFortuneDirectionInPillars();
    void chartCalculatorChangesFortuneStartAgeByDirection();
    void chartCalculatorCalculatesAnnualFortunesForSupportedSampleYear();
    void chartCalculatorAddsAnnualFortuneRelationsForSupportedSampleYear();
    void chartCalculatorAddsExtendedMajorFortuneRelationsForSupportedSampleYear();
    void chartCalculatorAddsMajorFortuneRelationsForSupportedSampleYear();
    void chartCalculatorPreservesMajorFortuneCoreFieldsWhenAddingRelations();
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
    void solarTermDataSourceLoads1883YearData();
    void solarTermDataSourceLoads1923YearData();
    void solarTermDataSourceLoads2026YearData();
    void solarTermDataSourceLoadsSampleYearData();
    void solarTermDataSourceHandlesMissingYearData();
    void chartCalculatorMatches2026LichunBoundaryCaseYearAndMonthPillars();
    void chartCalculatorChangesYearAndMonthPillarsAcross2026LichunBoundary();
    void chartCalculatorMatchesTz001MonthPillarIf1923DataIsSupported();
    void chartCalculatorMatchesTz002MonthPillarIf1883DataIsSupported();
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

void CoreTests::chartCalculatorMatchesRegressionVerificationCases_data()
{
    QTest::addColumn<QString>("birthDate");
    QTest::addColumn<QString>("birthTime");
    QTest::addColumn<QString>("gender");
    QTest::addColumn<QString>("expectedYearPillar");
    QTest::addColumn<QString>("expectedMonthPillar");
    QTest::addColumn<QString>("expectedDayPillar");
    QTest::addColumn<QString>("expectedHourPillar");

    const QJsonArray cases = loadVerificationCases();
    QVERIFY2(!cases.isEmpty(), "検証用ケースが 1 件もありません。");

    int enabledCaseCount = 0;
    for (const QJsonValue &caseValue : cases) {
        const QJsonObject caseObject = caseValue.toObject();
        if (!caseObject.value(QStringLiteral("enabledForRegression")).toBool(true)) {
            continue;
        }

        const QJsonObject expected = caseObject.value(QStringLiteral("expected")).toObject();
        const QString caseId = caseObject.value(QStringLiteral("caseId")).toString();
        QVERIFY2(!caseId.isEmpty(), "caseId が空の検証ケースがあります。");

        const QByteArray rowName = caseId.toUtf8();
        QTest::addRow("%s", rowName.constData())
            << caseObject.value(QStringLiteral("birthDate")).toString()
            << caseObject.value(QStringLiteral("birthTime")).toString()
            << caseObject.value(QStringLiteral("gender")).toString()
            << expected.value(QStringLiteral("yearPillar")).toString()
            << expected.value(QStringLiteral("monthPillar")).toString()
            << expected.value(QStringLiteral("dayPillar")).toString()
            << expected.value(QStringLiteral("hourPillar")).toString();
        ++enabledCaseCount;
    }

    QVERIFY2(enabledCaseCount > 0, "回帰テスト対象の検証ケースが 1 件もありません。");
}

void CoreTests::chartCalculatorMatchesRegressionVerificationCases()
{
    QFETCH(QString, birthDate);
    QFETCH(QString, birthTime);
    QFETCH(QString, gender);
    QFETCH(QString, expectedYearPillar);
    QFETCH(QString, expectedMonthPillar);
    QFETCH(QString, expectedDayPillar);
    QFETCH(QString, expectedHourPillar);

    ChartCalculator calculator;
    const BirthInfo birthInfo{birthDate, birthTime, gender};

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.yearPillar, expectedYearPillar);
    QCOMPARE(result.monthPillar, expectedMonthPillar);
    QCOMPARE(result.dayPillar, expectedDayPillar);
    QCOMPARE(result.hourPillar, expectedHourPillar);
}

void CoreTests::chartCalculatorReportsNonRegressionVerificationDifferences()
{
    const QJsonArray cases = loadVerificationCases();
    QVERIFY2(!cases.isEmpty(), "検証用ケースが 1 件もありません。");

    ChartCalculator calculator;
    int reportedCaseCount = 0;
    QMap<QString, int> reviewOutcomeCounts;

    for (const QJsonValue &caseValue : cases) {
        const QJsonObject caseObject = caseValue.toObject();
        const bool enabledForRegression = caseObject.value(QStringLiteral("enabledForRegression")).toBool(true);
        const QString confidence = caseObject.value(QStringLiteral("confidence")).toString();
        if (enabledForRegression && !isExternalConfidence(confidence)) {
            continue;
        }

        const BirthInfo birthInfo{
            caseObject.value(QStringLiteral("birthDate")).toString(),
            caseObject.value(QStringLiteral("birthTime")).toString(),
            caseObject.value(QStringLiteral("gender")).toString()
        };
        const ChartResult result = calculator.calculate(birthInfo);
        const QJsonObject expected = caseObject.value(QStringLiteral("expected")).toObject();
        const QStringList fields = mismatchFields(
            expected.value(QStringLiteral("yearPillar")).toString(),
            expected.value(QStringLiteral("monthPillar")).toString(),
            expected.value(QStringLiteral("dayPillar")).toString(),
            expected.value(QStringLiteral("hourPillar")).toString(),
            result
        );
        const QString actualClassification = classifyVerificationDifference(fields);
        const QString suggestedAction = suggestVerificationAction(actualClassification);
        const QString reviewOutcome = determineReviewOutcome(
            actualClassification,
            suggestedAction,
            caseObject.value(QStringLiteral("expectedDifferenceCategory")).toString(),
            caseObject.value(QStringLiteral("expectedAction")).toString()
        );
        const QString report = formatVerificationDifferenceReport(caseObject, result);
        QWARN(qPrintable(report));
        reviewOutcomeCounts[reviewOutcome] += 1;
        ++reportedCaseCount;
    }

    QVERIFY2(reportedCaseCount > 0, "比較レポート対象の non-regression ケースが 1 件もありません。");
    const QString summary = QStringLiteral(
        "summary: confirmed_spec_gap=%1, confirmed_bug_candidate=%2, classification_mismatch=%3, action_mismatch=%4, manual_review_required=%5, ready_for_regression_review=%6"
    )
        .arg(reviewOutcomeCounts.value(QStringLiteral("confirmed_spec_gap")))
        .arg(reviewOutcomeCounts.value(QStringLiteral("confirmed_bug_candidate")))
        .arg(reviewOutcomeCounts.value(QStringLiteral("classification_mismatch")))
        .arg(reviewOutcomeCounts.value(QStringLiteral("action_mismatch")))
        .arg(reviewOutcomeCounts.value(QStringLiteral("manual_review_required")))
        .arg(reviewOutcomeCounts.value(QStringLiteral("ready_for_regression_review")));
    QWARN(qPrintable(summary));
}

void CoreTests::testSamplesMasterCsvLoadsRows()
{
    const QList<QHash<QString, QString>> rows = loadCsvRows(testSamplesMasterCsvFilePath());

    QVERIFY2(!rows.isEmpty(), "test_samples_master.csv を読み込めません。");
    QVERIFY(rows.size() >= 4);

    for (const QHash<QString, QString> &row : rows) {
        QVERIFY2(!row.value(QStringLiteral("sample_id")).isEmpty(), "sample_id が空の台帳行があります。");
        QVERIFY2(!row.value(QStringLiteral("sample_group")).isEmpty(), "sample_group が空の台帳行があります。");
        QVERIFY2(!row.value(QStringLiteral("reliability_rank")).isEmpty(), "reliability_rank が空の台帳行があります。");
        QVERIFY2(!row.value(QStringLiteral("sample_status")).isEmpty(), "sample_status が空の台帳行があります。");
    }
}

void CoreTests::expectedPillarsCsvLoadsRows()
{
    const QList<QHash<QString, QString>> rows = loadCsvRows(expectedPillarsCsvFilePath());

    QVERIFY2(!rows.isEmpty(), "expected_pillars.csv を読み込めません。");
    QVERIFY(rows.size() >= 4);

    for (const QHash<QString, QString> &row : rows) {
        QVERIFY2(!row.value(QStringLiteral("sample_id")).isEmpty(), "sample_id が空の正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("year_pillar")).isEmpty(), "year_pillar が空の正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("month_pillar")).isEmpty(), "month_pillar が空の正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("day_pillar")).isEmpty(), "day_pillar が空の正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("hour_pillar")).isEmpty(), "hour_pillar が空の正答表行があります。");
    }
}

void CoreTests::expectedPillarsCsvMatchesRegressionCaseIds()
{
    const QList<QHash<QString, QString>> expectedRows = loadCsvRows(expectedPillarsCsvFilePath());
    QVERIFY2(!expectedRows.isEmpty(), "expected_pillars.csv を読み込めません。");

    QSet<QString> expectedSampleIds;
    for (const QHash<QString, QString> &row : expectedRows) {
        expectedSampleIds.insert(row.value(QStringLiteral("sample_id")));
    }

    const QJsonArray verificationCases = loadVerificationCases();
    QVERIFY2(!verificationCases.isEmpty(), "verification_cases.json を読み込めません。");

    int regressionCaseCount = 0;
    int overlappedCaseCount = 0;

    for (const QJsonValue &caseValue : verificationCases) {
        const QJsonObject caseObject = caseValue.toObject();
        const QString caseId = caseObject.value(QStringLiteral("caseId")).toString();
        if (expectedSampleIds.contains(caseId)) {
            ++overlappedCaseCount;
        }

        if (!caseObject.value(QStringLiteral("enabledForRegression")).toBool(true)) {
            continue;
        }

        ++regressionCaseCount;
        QVERIFY2(
            expectedSampleIds.contains(caseId),
            qPrintable(QStringLiteral("regression ケース %1 が expected_pillars.csv に存在しません。").arg(caseId))
        );
    }

    QVERIFY(regressionCaseCount > 0);
    QVERIFY2(overlappedCaseCount >= 3, "expected_pillars.csv と verification_cases.json の整合ケース数が不足しています。");
}

void CoreTests::expectedJudgementCsvLoadsRows()
{
    const QList<QHash<QString, QString>> rows = loadCsvRows(expectedJudgementCsvFilePath());

    QVERIFY2(!rows.isEmpty(), "expected_judgement.csv を読み込めません。");
    QVERIFY(rows.size() >= 4);

    for (const QHash<QString, QString> &row : rows) {
        QVERIFY2(!row.value(QStringLiteral("sample_id")).isEmpty(), "sample_id が空の判定正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("strength")).isEmpty(), "strength が空の判定正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("pattern_status")).isEmpty(), "pattern_status が空の判定正答表行があります。");
    }
}

void CoreTests::expectedJudgementCsvMatchesKnownSampleIds()
{
    const QList<QHash<QString, QString>> judgementRows = loadCsvRows(expectedJudgementCsvFilePath());
    QVERIFY2(!judgementRows.isEmpty(), "expected_judgement.csv を読み込めません。");

    const QList<QHash<QString, QString>> sampleRows = loadCsvRows(testSamplesMasterCsvFilePath());
    QVERIFY2(!sampleRows.isEmpty(), "test_samples_master.csv を読み込めません。");

    QSet<QString> sampleIds;
    for (const QHash<QString, QString> &row : sampleRows) {
        sampleIds.insert(row.value(QStringLiteral("sample_id")));
    }

    bool containsReviewNeeded = false;
    for (const QHash<QString, QString> &row : judgementRows) {
        const QString sampleId = row.value(QStringLiteral("sample_id"));
        QVERIFY2(
            sampleIds.contains(sampleId),
            qPrintable(QStringLiteral("判定正答表の sample_id %1 がサンプル台帳に存在しません。").arg(sampleId))
        );

        const QString patternStatus = row.value(QStringLiteral("pattern_status"));
        if (patternStatus == QStringLiteral("review_needed")) {
            containsReviewNeeded = true;
        }
    }

    QVERIFY2(containsReviewNeeded, "external ケース向けの review_needed 行が expected_judgement.csv にありません。");
}

void CoreTests::expectedJudgementCsvContainsRegressionCases()
{
    const QList<QHash<QString, QString>> judgementRows = loadCsvRows(expectedJudgementCsvFilePath());
    QVERIFY2(!judgementRows.isEmpty(), "expected_judgement.csv を読み込めません。");

    QSet<QString> judgementSampleIds;
    for (const QHash<QString, QString> &row : judgementRows) {
        judgementSampleIds.insert(row.value(QStringLiteral("sample_id")));
    }

    const QJsonArray verificationCases = loadVerificationCases();
    QVERIFY2(!verificationCases.isEmpty(), "verification_cases.json を読み込めません。");

    int regressionCaseCount = 0;
    for (const QJsonValue &caseValue : verificationCases) {
        const QJsonObject caseObject = caseValue.toObject();
        if (!caseObject.value(QStringLiteral("enabledForRegression")).toBool(true)) {
            continue;
        }

        const QString caseId = caseObject.value(QStringLiteral("caseId")).toString();
        ++regressionCaseCount;
        QVERIFY2(
            judgementSampleIds.contains(caseId),
            qPrintable(QStringLiteral("regression ケース %1 が expected_judgement.csv に存在しません。").arg(caseId))
        );
    }

    QVERIFY(regressionCaseCount > 0);
}

void CoreTests::expectedRelationsCsvLoadsRows()
{
    const QList<QHash<QString, QString>> rows = loadCsvRows(expectedRelationsCsvFilePath());

    QVERIFY2(!rows.isEmpty(), "expected_relations.csv を読み込めません。");
    QVERIFY(rows.size() >= 6);

    for (const QHash<QString, QString> &row : rows) {
        QVERIFY2(!row.value(QStringLiteral("sample_id")).isEmpty(), "sample_id が空の関係正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("target_scope")).isEmpty(), "target_scope が空の関係正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("relation_type")).isEmpty(), "relation_type が空の関係正答表行があります。");
    }
}

void CoreTests::expectedRelationsCsvMatchesKnownSampleIds()
{
    const QList<QHash<QString, QString>> relationRows = loadCsvRows(expectedRelationsCsvFilePath());
    QVERIFY2(!relationRows.isEmpty(), "expected_relations.csv を読み込めません。");

    const QList<QHash<QString, QString>> sampleRows = loadCsvRows(testSamplesMasterCsvFilePath());
    QVERIFY2(!sampleRows.isEmpty(), "test_samples_master.csv を読み込めません。");

    QSet<QString> sampleIds;
    for (const QHash<QString, QString> &row : sampleRows) {
        sampleIds.insert(row.value(QStringLiteral("sample_id")));
    }

    for (const QHash<QString, QString> &row : relationRows) {
        const QString sampleId = row.value(QStringLiteral("sample_id"));
        QVERIFY2(
            sampleIds.contains(sampleId),
            qPrintable(QStringLiteral("関係正答表の sample_id %1 がサンプル台帳に存在しません。").arg(sampleId))
        );
    }
}

void CoreTests::expectedRelationsCsvContainsSupportedScopes()
{
    const QList<QHash<QString, QString>> relationRows = loadCsvRows(expectedRelationsCsvFilePath());
    QVERIFY2(!relationRows.isEmpty(), "expected_relations.csv を読み込めません。");

    bool containsNatalScope = false;
    bool containsLuckScope = false;
    bool containsYearScope = false;

    for (const QHash<QString, QString> &row : relationRows) {
        const QString targetScope = row.value(QStringLiteral("target_scope"));
        QVERIFY2(
            isSupportedRelationScope(targetScope),
            qPrintable(QStringLiteral("未対応の target_scope %1 があります。").arg(targetScope))
        );

        bool isEstablished = false;
        QVERIFY2(
            tryParseBooleanText(row.value(QStringLiteral("is_established")), &isEstablished),
            qPrintable(QStringLiteral("is_established=%1 を true/false として解釈できません。")
                .arg(row.value(QStringLiteral("is_established"))))
        );

        if (targetScope == QStringLiteral("natal")) {
            containsNatalScope = true;
        } else if (targetScope == QStringLiteral("natal_vs_luck")) {
            containsLuckScope = true;
        } else if (targetScope == QStringLiteral("natal_vs_year")) {
            containsYearScope = true;
        }
    }

    QVERIFY(containsNatalScope);
    QVERIFY(containsLuckScope);
    QVERIFY(containsYearScope);
}

void CoreTests::expectedLuckCycleCsvLoadsRows()
{
    const QList<QHash<QString, QString>> rows = loadCsvRows(expectedLuckCycleCsvFilePath());

    QVERIFY2(!rows.isEmpty(), "expected_luck_cycle.csv を読み込めません。");
    QVERIFY(rows.size() >= 4);

    for (const QHash<QString, QString> &row : rows) {
        QVERIFY2(!row.value(QStringLiteral("sample_id")).isEmpty(), "sample_id が空の大運正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("luck_direction")).isEmpty(), "luck_direction が空の大運正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("qiyun_age")).isEmpty(), "qiyun_age が空の大運正答表行があります。");
        QVERIFY2(!row.value(QStringLiteral("luck_1")).isEmpty(), "luck_1 が空の大運正答表行があります。");
    }
}

void CoreTests::expectedLuckCycleCsvMatchesKnownSampleIds()
{
    const QList<QHash<QString, QString>> luckRows = loadCsvRows(expectedLuckCycleCsvFilePath());
    QVERIFY2(!luckRows.isEmpty(), "expected_luck_cycle.csv を読み込めません。");

    const QList<QHash<QString, QString>> sampleRows = loadCsvRows(testSamplesMasterCsvFilePath());
    QVERIFY2(!sampleRows.isEmpty(), "test_samples_master.csv を読み込めません。");

    QSet<QString> sampleIds;
    for (const QHash<QString, QString> &row : sampleRows) {
        sampleIds.insert(row.value(QStringLiteral("sample_id")));
    }

    for (const QHash<QString, QString> &row : luckRows) {
        const QString sampleId = row.value(QStringLiteral("sample_id"));
        QVERIFY2(
            sampleIds.contains(sampleId),
            qPrintable(QStringLiteral("大運正答表の sample_id %1 がサンプル台帳に存在しません。").arg(sampleId))
        );
    }
}

void CoreTests::expectedLuckCycleCsvContainsRegressionCases()
{
    const QList<QHash<QString, QString>> luckRows = loadCsvRows(expectedLuckCycleCsvFilePath());
    QVERIFY2(!luckRows.isEmpty(), "expected_luck_cycle.csv を読み込めません。");

    QSet<QString> luckSampleIds;
    for (const QHash<QString, QString> &row : luckRows) {
        luckSampleIds.insert(row.value(QStringLiteral("sample_id")));
    }

    const QJsonArray verificationCases = loadVerificationCases();
    QVERIFY2(!verificationCases.isEmpty(), "verification_cases.json を読み込めません。");

    int regressionCaseCount = 0;
    for (const QJsonValue &caseValue : verificationCases) {
        const QJsonObject caseObject = caseValue.toObject();
        if (!caseObject.value(QStringLiteral("enabledForRegression")).toBool(true)) {
            continue;
        }

        const QString caseId = caseObject.value(QStringLiteral("caseId")).toString();
        ++regressionCaseCount;
        QVERIFY2(
            luckSampleIds.contains(caseId),
            qPrintable(QStringLiteral("regression ケース %1 が expected_luck_cycle.csv に存在しません。").arg(caseId))
        );
    }

    QVERIFY(regressionCaseCount > 0);
}

void CoreTests::verificationSpecGapRegistryCoversKeepAsSpecGapCases()
{
    const QJsonArray verificationCases = loadVerificationCases();
    QVERIFY2(!verificationCases.isEmpty(), "検証用ケースが 1 件もありません。");

    const QJsonArray specGaps = loadSpecGapRegistry();
    QVERIFY2(!specGaps.isEmpty(), "spec gap registry を読み込めません。");

    int matchedCaseCount = 0;

    for (const QJsonValue &caseValue : verificationCases) {
        const QJsonObject caseObject = caseValue.toObject();
        if (caseObject.value(QStringLiteral("enabledForRegression")).toBool(true)) {
            continue;
        }
        if (caseObject.value(QStringLiteral("expectedAction")).toString()
            != QStringLiteral("keep_as_spec_gap")) {
            continue;
        }

        const QString caseId = caseObject.value(QStringLiteral("caseId")).toString();
        const QString ruleHint = caseObject.value(QStringLiteral("ruleHint")).toString();
        QVERIFY2(!caseId.isEmpty(), "keep_as_spec_gap ケースの caseId が空です。");
        QVERIFY2(!ruleHint.isEmpty(), qPrintable(QStringLiteral("%1 の ruleHint が空です。").arg(caseId)));

        const QJsonObject specGapObject = findSpecGapByRuleHint(specGaps, ruleHint);
        QVERIFY2(
            !specGapObject.isEmpty(),
            qPrintable(QStringLiteral("%1 に対応する spec gap が registry にありません。").arg(ruleHint))
        );

        const QJsonArray relatedCaseIds = specGapObject.value(QStringLiteral("relatedCaseIds")).toArray();
        bool containsCaseId = false;
        for (const QJsonValue &relatedCaseIdValue : relatedCaseIds) {
            if (relatedCaseIdValue.toString() == caseId) {
                containsCaseId = true;
                break;
            }
        }

        QVERIFY2(
            containsCaseId,
            qPrintable(QStringLiteral("%1 が %2 の relatedCaseIds に含まれていません。").arg(caseId, ruleHint))
        );
        QVERIFY2(
            !specGapObject.value(QStringLiteral("status")).toString().isEmpty(),
            qPrintable(QStringLiteral("%1 の status が空です。").arg(ruleHint))
        );
        QVERIFY2(
            !specGapObject.value(QStringLiteral("adoptedSpec")).toString().isEmpty(),
            qPrintable(QStringLiteral("%1 の adoptedSpec が空です。").arg(ruleHint))
        );

        ++matchedCaseCount;
    }

    QVERIFY2(matchedCaseCount > 0, "keep_as_spec_gap の non-regression ケースが 1 件もありません。");
}

void CoreTests::verificationNonRegressionCasesContainReviewMetadata()
{
    const QJsonArray verificationCases = loadVerificationCases();
    QVERIFY2(!verificationCases.isEmpty(), "検証用ケースが 1 件もありません。");

    int externalNonRegressionCount = 0;

    for (const QJsonValue &caseValue : verificationCases) {
        const QJsonObject caseObject = caseValue.toObject();
        if (caseObject.value(QStringLiteral("enabledForRegression")).toBool(true)) {
            continue;
        }
        if (!isExternalConfidence(caseObject.value(QStringLiteral("confidence")).toString())) {
            continue;
        }

        const QString caseId = caseObject.value(QStringLiteral("caseId")).toString();
        QVERIFY2(!caseId.isEmpty(), "external non-regression ケースの caseId が空です。");
        QVERIFY2(
            !caseObject.value(QStringLiteral("expectedDifferenceCategory")).toString().isEmpty(),
            qPrintable(QStringLiteral("%1 の expectedDifferenceCategory が空です。").arg(caseId))
        );
        QVERIFY2(
            !caseObject.value(QStringLiteral("expectedAction")).toString().isEmpty(),
            qPrintable(QStringLiteral("%1 の expectedAction が空です。").arg(caseId))
        );
        QVERIFY2(
            !caseObject.value(QStringLiteral("ruleHint")).toString().isEmpty(),
            qPrintable(QStringLiteral("%1 の ruleHint が空です。").arg(caseId))
        );
        QVERIFY2(
            !caseObject.value(QStringLiteral("reviewStatus")).toString().isEmpty(),
            qPrintable(QStringLiteral("%1 の reviewStatus が空です。").arg(caseId))
        );
        QVERIFY2(
            isSupportedReviewStatus(caseObject.value(QStringLiteral("reviewStatus")).toString()),
            qPrintable(QStringLiteral("%1 の reviewStatus が未対応値です。").arg(caseId))
        );
        QVERIFY2(
            !caseObject.value(QStringLiteral("reviewNotes")).toString().isEmpty(),
            qPrintable(QStringLiteral("%1 の reviewNotes が空です。").arg(caseId))
        );

        ++externalNonRegressionCount;
    }

    QVERIFY2(
        externalNonRegressionCount >= 5,
        qPrintable(QStringLiteral("external non-regression ケース数が不足しています: %1").arg(externalNonRegressionCount))
    );
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
    QCOMPARE(firstResult.majorFortuneDirection, secondResult.majorFortuneDirection);
    QCOMPARE(
        firstResult.majorFortuneDirectionStatusMessage,
        secondResult.majorFortuneDirectionStatusMessage
    );
    QCOMPARE(
        firstResult.solarTermDifferencePreparation,
        secondResult.solarTermDifferencePreparation
    );
    QCOMPARE(
        firstResult.solarTermDifferencePreparationStatusMessage,
        secondResult.solarTermDifferencePreparationStatusMessage
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

void CoreTests::chartCalculatorChangesDayPillarAcross23HourBoundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1990-02-05"),
        QStringLiteral("22:59"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1990-02-05"),
        QStringLiteral("23:00"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.dayPillar, QStringLiteral("辛丑"));
    QCOMPARE(afterResult.dayPillar, QStringLiteral("壬寅"));
    QVERIFY(beforeResult.dayPillar != afterResult.dayPillar);
}

void CoreTests::chartCalculatorChangesYearPillarAcrossLichunBoundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1990-02-04"),
        QStringLiteral("11:59"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1990-02-04"),
        QStringLiteral("12:00"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.yearPillar, QStringLiteral("己巳"));
    QCOMPARE(afterResult.yearPillar, QStringLiteral("庚午"));
    QVERIFY(beforeResult.yearPillar != afterResult.yearPillar);
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
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("本アプリ採用仕様")));
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
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("小寒")));
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
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("本アプリ採用仕様")));
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
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("本アプリ採用仕様")));
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
    QVERIFY(result.description.contains(QStringLiteral("正節の節入り日時基準")));
}

void CoreTests::chartCalculatorChangesMonthPillarAcrossPrincipalTermTimeBoundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("1990-04-05"),
        QStringLiteral("11:59"),
        QStringLiteral("男性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("1990-04-05"),
        QStringLiteral("12:00"),
        QStringLiteral("男性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.monthPillar, QStringLiteral("己卯"));
    QCOMPARE(afterResult.monthPillar, QStringLiteral("庚辰"));
    QVERIFY(beforeResult.monthPillar != afterResult.monthPillar);
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
    const QVariantList rankedElements = result.usefulGodCandidates.value(QStringLiteral("rankedElements")).toList();

    QCOMPARE(candidates.size(), 3);
    QCOMPARE(candidates.first(), QStringLiteral("水"));
    QCOMPARE(result.usefulGodCandidates.value(QStringLiteral("balanceState")).toString(), QStringLiteral("strong"));
    QCOMPARE(result.usefulGodCandidates.value(QStringLiteral("referenceScore")).toInt(), 4);
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("strengthPriority")).canConvert<QStringList>());
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("climatePriority")).canConvert<QStringList>());
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("shortagePriority")).canConvert<QStringList>());
    QVERIFY(!result.usefulGodCandidates.value(QStringLiteral("strengthPriority")).toStringList().isEmpty());
    QVERIFY(!result.usefulGodCandidates.value(QStringLiteral("climatePriority")).toStringList().isEmpty());
    QVERIFY(!result.usefulGodCandidates.value(QStringLiteral("shortagePriority")).toStringList().isEmpty());
    QCOMPARE(rankedElements.size(), 5);
    QCOMPARE(rankedElements.at(0).toMap().value(QStringLiteral("element")).toString(), QStringLiteral("水"));
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("reason")).toString().contains(QStringLiteral("五行分布")));
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("reason")).toString().contains(QStringLiteral("balanceState")));
    QVERIFY(result.usefulGodCandidates.value(QStringLiteral("note")).toString().contains(QStringLiteral("断定")));
    QVERIFY(result.usefulGodCandidatesStatusMessage.contains(QStringLiteral("暫定候補")));
    QVERIFY(result.usefulGodCandidatesStatusMessage.contains(QStringLiteral("構造化情報")));
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
    const QVariantList rankedPatterns = result.patternCandidates.value(QStringLiteral("rankedPatterns")).toList();

    QCOMPARE(candidates.size(), 3);
    QCOMPARE(candidates.at(0), QStringLiteral("印綬格"));
    QVERIFY(candidates.contains(QStringLiteral("正財格")));
    QVERIFY(candidates.contains(QStringLiteral("正官格")));
    QCOMPARE(result.patternCandidates.value(QStringLiteral("primaryBasis")).toString(), QStringLiteral("month_tengod_centered"));
    QCOMPARE(result.patternCandidates.value(QStringLiteral("monthTenGod")).toString(), QStringLiteral("印綬"));
    QVERIFY(result.patternCandidates.value(QStringLiteral("monthHiddenStemTenGods")).canConvert<QStringList>());
    QVERIFY(result.patternCandidates.value(QStringLiteral("strengthSupport")).toString().contains(QStringLiteral("balanceState")));
    QVERIFY(result.patternCandidates.value(QStringLiteral("usefulGodSupport")).toString().contains(QStringLiteral("用神候補")));
    QCOMPARE(rankedPatterns.size(), 3);
    QCOMPARE(rankedPatterns.at(0).toMap().value(QStringLiteral("pattern")).toString(), QStringLiteral("印綬格"));
    QVERIFY(result.patternCandidates.value(QStringLiteral("reason")).toString().contains(QStringLiteral("月干通変星")));
    QVERIFY(result.patternCandidates.value(QStringLiteral("reason")).toString().contains(QStringLiteral("用神候補")));
    QVERIFY(result.patternCandidates.value(QStringLiteral("reason")).toString().contains(QStringLiteral("五行分布")));
    QVERIFY(result.patternCandidates.value(QStringLiteral("note")).toString().contains(QStringLiteral("断定")));
    QVERIFY(result.patternCandidatesStatusMessage.contains(QStringLiteral("暫定候補")));
}

void CoreTests::chartCalculatorMatchesRegressionJudgementCsvForSupportedSampleYear()
{
    const QList<QHash<QString, QString>> judgementRows = loadCsvRows(expectedJudgementCsvFilePath());
    QVERIFY2(!judgementRows.isEmpty(), "expected_judgement.csv を読み込めません。");

    QHash<QString, QString> targetRow;
    for (const QHash<QString, QString> &row : judgementRows) {
        if (row.value(QStringLiteral("sample_id")) == QStringLiteral("REG-1990-0130")) {
            targetRow = row;
            break;
        }
    }

    QVERIFY2(!targetRow.isEmpty(), "REG-1990-0130 が expected_judgement.csv にありません。");

    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.strengthEvaluation.value(QStringLiteral("balanceState")).toString(), targetRow.value(QStringLiteral("strength")));
    QCOMPARE(result.patternCandidates.value(QStringLiteral("candidates")).toStringList().value(0), targetRow.value(QStringLiteral("pattern_name")));
    QCOMPARE(result.usefulGodCandidates.value(QStringLiteral("candidates")).toStringList().value(0), targetRow.value(QStringLiteral("useful_god")));
}

void CoreTests::chartCalculatorMatchesRegressionRelationsCsvForSupportedSampleYear()
{
    const QList<QHash<QString, QString>> relationRows = loadCsvRows(expectedRelationsCsvFilePath());
    QVERIFY2(!relationRows.isEmpty(), "expected_relations.csv を読み込めません。");

    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    for (const QHash<QString, QString> &row : relationRows) {
        if (row.value(QStringLiteral("sample_id")) != QStringLiteral("REG-1990-0130")) {
            continue;
        }
        if (row.value(QStringLiteral("target_scope")) != QStringLiteral("natal_vs_year")) {
            continue;
        }

        bool isEstablished = false;
        QVERIFY(tryParseBooleanText(row.value(QStringLiteral("is_established")), &isEstablished));

        const QString subjectA = row.value(QStringLiteral("subject_a"));
        const QString subjectB = row.value(QStringLiteral("subject_b"));
        const QString targetLabel = pillarKeyToLabel(subjectB);

        QVariantMap relationSource;
        if (subjectA == QStringLiteral("annualFortune[0]")) {
            relationSource = result.annualFortunes.at(0).toMap();
        } else if (subjectA == QStringLiteral("annualFortune[1]")) {
            relationSource = result.annualFortunes.at(1).toMap();
        } else {
            continue;
        }

        QStringList actualMatches;
        const QString relationType = row.value(QStringLiteral("relation_type"));
        if (relationType == QStringLiteral("same_stem")) {
            actualMatches = relationSource.value(QStringLiteral("sameStemMatches")).toStringList();
        } else if (relationType == QStringLiteral("same_branch")) {
            actualMatches = relationSource.value(QStringLiteral("sameBranchMatches")).toStringList();
        } else if (relationType == QStringLiteral("clash")) {
            actualMatches = relationSource.value(QStringLiteral("clashBranches")).toStringList();
        } else if (relationType == QStringLiteral("stem_combination")) {
            actualMatches = relationSource.value(QStringLiteral("stemCombinationCandidates")).toStringList();
        } else {
            continue;
        }

        if (isEstablished) {
            QVERIFY2(
                actualMatches.contains(targetLabel),
                qPrintable(QStringLiteral("%1 / %2 が %3 に含まれていません。")
                    .arg(subjectA, relationType, targetLabel))
            );
        } else {
            QVERIFY2(
                !actualMatches.contains(targetLabel),
                qPrintable(QStringLiteral("%1 / %2 に %3 が含まれています。")
                    .arg(subjectA, relationType, targetLabel))
            );
        }
    }
}

void CoreTests::chartCalculatorMatchesRegressionLuckCycleCsvForSupportedSampleYear()
{
    const QList<QHash<QString, QString>> luckRows = loadCsvRows(expectedLuckCycleCsvFilePath());
    QVERIFY2(!luckRows.isEmpty(), "expected_luck_cycle.csv を読み込めません。");

    QHash<QString, QString> targetRow;
    for (const QHash<QString, QString> &row : luckRows) {
        if (row.value(QStringLiteral("sample_id")) == QStringLiteral("REG-1990-0130")) {
            targetRow = row;
            break;
        }
    }

    QVERIFY2(!targetRow.isEmpty(), "REG-1990-0130 が expected_luck_cycle.csv にありません。");

    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QVariantList majorFortunes = result.majorFortunes;

    QCOMPARE(result.majorFortuneDirection.value(QStringLiteral("direction")).toString(), targetRow.value(QStringLiteral("luck_direction")));
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("startAge")).toString(), targetRow.value(QStringLiteral("qiyun_age")));
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), targetRow.value(QStringLiteral("luck_1")));
    QCOMPARE(majorFortunes.at(1).toMap().value(QStringLiteral("pillar")).toString(), targetRow.value(QStringLiteral("luck_2")));
}

void CoreTests::chartCalculatorCalculatesMajorFortuneDirectionForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(
        result.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("順行")
    );
    QVERIFY(
        result.majorFortuneDirection.value(QStringLiteral("reason")).toString().contains(QStringLiteral("年干"))
    );
    QVERIFY(
        result.majorFortuneDirection.value(QStringLiteral("note")).toString().contains(QStringLiteral("一般四柱推命"))
    );
    QVERIFY(result.majorFortuneDirectionStatusMessage.contains(QStringLiteral("一般四柱推命")));
}

void CoreTests::chartCalculatorChangesMajorFortuneDirectionWithYearStemPolarity()
{
    ChartCalculator calculator;
    const BirthInfo maleYangYearBirthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo maleYinYearBirthInfo{
        QStringLiteral("1991-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo femaleYinYearBirthInfo{
        QStringLiteral("1991-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("女性")
    };

    const ChartResult maleYangYearResult = calculator.calculate(maleYangYearBirthInfo);
    const ChartResult maleYinYearResult = calculator.calculate(maleYinYearBirthInfo);
    const ChartResult femaleYinYearResult = calculator.calculate(femaleYinYearBirthInfo);

    QCOMPARE(
        maleYangYearResult.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("順行")
    );
    QCOMPARE(
        maleYinYearResult.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("逆行")
    );
    QCOMPARE(
        femaleYinYearResult.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("順行")
    );
}

void CoreTests::chartCalculatorReturnsUndeterminedMajorFortuneDirectionForUnspecifiedGender()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("未指定")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(
        result.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("未対応")
    );
    QVERIFY(result.majorFortuneDirectionStatusMessage.contains(QStringLiteral("未対応")));
}

void CoreTests::chartCalculatorCalculatesSolarTermDifferencePreparationForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(
        result.solarTermDifferencePreparation.value(QStringLiteral("referenceTerm")).toString(),
        QStringLiteral("啓蟄")
    );
    QCOMPARE(
        result.solarTermDifferencePreparation.value(QStringLiteral("referenceDirection")).toString(),
        QStringLiteral("直後節入り")
    );
    QCOMPARE(
        result.solarTermDifferencePreparation.value(QStringLiteral("absoluteDifferenceMinutes")).toLongLong(),
        41670
    );
    QCOMPARE(
        result.solarTermDifferencePreparation.value(QStringLiteral("differenceDays")).toString(),
        QStringLiteral("28.94")
    );
    QCOMPARE(
        result.solarTermDifferencePreparation.value(QStringLiteral("calculatedStartAge")).toInt(),
        10
    );
    QVERIFY(
        result.solarTermDifferencePreparation.value(QStringLiteral("note")).toString().contains(QStringLiteral("本アプリ採用仕様"))
    );
    QVERIFY(result.solarTermDifferencePreparationStatusMessage.contains(QStringLiteral("採用仕様")));
}

void CoreTests::chartCalculatorChangesReferencedSolarTermByDirection()
{
    ChartCalculator calculator;
    const BirthInfo maleBirthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo femaleBirthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("女性")
    };

    const ChartResult maleResult = calculator.calculate(maleBirthInfo);
    const ChartResult femaleResult = calculator.calculate(femaleBirthInfo);

    QCOMPARE(
        maleResult.solarTermDifferencePreparation.value(QStringLiteral("referenceTerm")).toString(),
        QStringLiteral("啓蟄")
    );
    QCOMPARE(
        maleResult.solarTermDifferencePreparation.value(QStringLiteral("referenceDirection")).toString(),
        QStringLiteral("直後節入り")
    );
    QCOMPARE(
        femaleResult.solarTermDifferencePreparation.value(QStringLiteral("referenceTerm")).toString(),
        QStringLiteral("立春")
    );
    QCOMPARE(
        femaleResult.solarTermDifferencePreparation.value(QStringLiteral("referenceDirection")).toString(),
        QStringLiteral("直前節入り")
    );
    QVERIFY(maleResult.solarTermDifferencePreparation != femaleResult.solarTermDifferencePreparation);
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
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("startAge")).toInt(), 10);
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("endAge")).toInt(), 19);
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("己卯"));
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("tenGod")).toString(), QStringLiteral("偏印"));
    QCOMPARE(majorFortunes.at(0).toMap().value(QStringLiteral("twelvePhase")).toString(), QStringLiteral("絶"));
    QCOMPARE(
        majorFortunes.at(0).toMap().value(QStringLiteral("sameStemMatches")).toStringList(),
        QStringList{}
    );
    QCOMPARE(
        majorFortunes.at(0).toMap().value(QStringLiteral("sameBranchMatches")).toStringList(),
        QStringList{}
    );
    QCOMPARE(
        majorFortunes.at(0).toMap().value(QStringLiteral("clashBranches")).toStringList(),
        QStringList{}
    );
    QCOMPARE(
        majorFortunes.at(0).toMap().value(QStringLiteral("stemCombinationCandidates")).toStringList(),
        QStringList{}
    );
    QCOMPARE(
        majorFortunes.at(0).toMap().value(QStringLiteral("relationSummary")).toString(),
        QStringLiteral("該当なし")
    );
    QCOMPARE(majorFortunes.at(1).toMap().value(QStringLiteral("startAge")).toInt(), 20);
    QCOMPARE(majorFortunes.at(1).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("庚辰"));
    QVERIFY(majorFortunes.at(0).toMap().value(QStringLiteral("note")).toString().contains(QStringLiteral("確定計算")));
    QVERIFY(majorFortunes.at(0).toMap().value(QStringLiteral("note")).toString().contains(QStringLiteral("順逆反映済み")));
    QVERIFY(majorFortunes.at(0).toMap().value(QStringLiteral("note")).toString().contains(QStringLiteral("通変星と十二運")));
    QVERIFY(result.majorFortunesStatusMessage.contains(QStringLiteral("採用仕様")));
    QVERIFY(result.majorFortunesStatusMessage.contains(QStringLiteral("順逆反映済み")));
    QVERIFY(result.majorFortunesStatusMessage.contains(QStringLiteral("通変星と十二運")));
    QVERIFY(result.majorFortunesStatusMessage.contains(QStringLiteral("同干・同支・冲候補・干合候補")));
}

void CoreTests::chartCalculatorReflectsMajorFortuneDirectionInPillars()
{
    ChartCalculator calculator;
    const BirthInfo maleBirthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo femaleBirthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("女性")
    };

    const ChartResult maleResult = calculator.calculate(maleBirthInfo);
    const ChartResult femaleResult = calculator.calculate(femaleBirthInfo);

    QCOMPARE(
        maleResult.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("順行")
    );
    QCOMPARE(
        femaleResult.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("逆行")
    );
    QCOMPARE(
        maleResult.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("己卯")
    );
    QCOMPARE(
        femaleResult.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("丁丑")
    );
    QVERIFY(maleResult.majorFortunes.at(0).toMap().value(QStringLiteral("tenGod")).toString()
            != femaleResult.majorFortunes.at(0).toMap().value(QStringLiteral("tenGod")).toString());
    QVERIFY(maleResult.majorFortunes.at(0).toMap().value(QStringLiteral("twelvePhase")).toString()
            != femaleResult.majorFortunes.at(0).toMap().value(QStringLiteral("twelvePhase")).toString());
    QVERIFY(maleResult.majorFortunesStatusMessage.contains(QStringLiteral("順逆反映済み")));
    QVERIFY(femaleResult.majorFortunesStatusMessage.contains(QStringLiteral("順逆反映済み")));
}

void CoreTests::chartCalculatorChangesFortuneStartAgeByDirection()
{
    ChartCalculator calculator;
    const BirthInfo maleBirthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };
    const BirthInfo femaleBirthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("女性")
    };

    const ChartResult maleResult = calculator.calculate(maleBirthInfo);
    const ChartResult femaleResult = calculator.calculate(femaleBirthInfo);

    QCOMPARE(
        maleResult.majorFortunes.at(0).toMap().value(QStringLiteral("startAge")).toInt(),
        10
    );
    QCOMPARE(
        femaleResult.majorFortunes.at(0).toMap().value(QStringLiteral("startAge")).toInt(),
        1
    );
    QCOMPARE(
        maleResult.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("己卯")
    );
    QCOMPARE(
        femaleResult.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("丁丑")
    );
    QVERIFY(maleResult.majorFortunes != femaleResult.majorFortunes);
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
    QCOMPARE(annualFortunes.at(0).toMap().value(QStringLiteral("tenGod")).toString(), QStringLiteral("劫財"));
    QCOMPARE(annualFortunes.at(0).toMap().value(QStringLiteral("twelvePhase")).toString(), QStringLiteral("病"));
    QCOMPARE(
        annualFortunes.at(0).toMap().value(QStringLiteral("sameStemMatches")).toStringList(),
        QStringList{QStringLiteral("年柱")}
    );
    QCOMPARE(
        annualFortunes.at(0).toMap().value(QStringLiteral("sameBranchMatches")).toStringList(),
        QStringList{QStringLiteral("年柱")}
    );
    QCOMPARE(
        annualFortunes.at(0).toMap().value(QStringLiteral("clashBranches")).toStringList(),
        QStringList{}
    );
    QCOMPARE(
        annualFortunes.at(0).toMap().value(QStringLiteral("stemCombinationCandidates")).toStringList(),
        QStringList{QStringLiteral("時柱")}
    );
    QVERIFY(annualFortunes.at(0).toMap().value(QStringLiteral("relationSummary")).toString().contains(QStringLiteral("同干")));
    QVERIFY(annualFortunes.at(0).toMap().value(QStringLiteral("relationNote")).toString().contains(QStringLiteral("吉凶断定ではなく")));
    QCOMPARE(annualFortunes.at(1).toMap().value(QStringLiteral("year")).toInt(), 1991);
    QCOMPARE(annualFortunes.at(1).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("辛未"));
    QCOMPARE(annualFortunes.at(1).toMap().value(QStringLiteral("tenGod")).toString(), QStringLiteral("日主"));
    QCOMPARE(annualFortunes.at(1).toMap().value(QStringLiteral("twelvePhase")).toString(), QStringLiteral("衰"));
    QCOMPARE(
        annualFortunes.at(1).toMap().value(QStringLiteral("sameStemMatches")).toStringList(),
        QStringList{QStringLiteral("日柱")}
    );
    QCOMPARE(
        annualFortunes.at(1).toMap().value(QStringLiteral("sameBranchMatches")).toStringList(),
        QStringList{QStringLiteral("時柱")}
    );
    QCOMPARE(
        annualFortunes.at(1).toMap().value(QStringLiteral("clashBranches")).toStringList(),
        QStringList{QStringLiteral("日柱")}
    );
    QCOMPARE(
        annualFortunes.at(1).toMap().value(QStringLiteral("stemCombinationCandidates")).toStringList(),
        QStringList{}
    );
    QVERIFY(annualFortunes.at(0).toMap().value(QStringLiteral("note")).toString().contains(QStringLiteral("参考表示")));
    QVERIFY(annualFortunes.at(0).toMap().value(QStringLiteral("note")).toString().contains(QStringLiteral("通変星と十二運")));
    QVERIFY(result.annualFortunesStatusMessage.contains(QStringLiteral("流年表示です")));
    QVERIFY(result.annualFortunesStatusMessage.contains(QStringLiteral("同干・同支・冲候補・干合候補")));
}

void CoreTests::chartCalculatorAddsAnnualFortuneRelationsForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QVariantMap firstAnnualFortune = result.annualFortunes.at(0).toMap();
    const QVariantMap secondAnnualFortune = result.annualFortunes.at(1).toMap();

    QVERIFY(firstAnnualFortune.contains(QStringLiteral("relationSummary")));
    QVERIFY(firstAnnualFortune.contains(QStringLiteral("sameStemMatches")));
    QVERIFY(firstAnnualFortune.contains(QStringLiteral("sameBranchMatches")));
    QVERIFY(firstAnnualFortune.contains(QStringLiteral("clashBranches")));
    QVERIFY(firstAnnualFortune.contains(QStringLiteral("stemCombinationCandidates")));
    QVERIFY(firstAnnualFortune.contains(QStringLiteral("relationNote")));
    QVERIFY(firstAnnualFortune.value(QStringLiteral("relationSummary")).toString().contains(QStringLiteral("干合候補")));
    QVERIFY(secondAnnualFortune.value(QStringLiteral("relationSummary")).toString().contains(QStringLiteral("冲候補")));
}

void CoreTests::chartCalculatorAddsExtendedMajorFortuneRelationsForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QVariantMap firstMajorFortune = result.majorFortunes.at(0).toMap();

    QVERIFY(firstMajorFortune.contains(QStringLiteral("sameStemMatches")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("sameBranchMatches")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("clashBranches")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("stemCombinationCandidates")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("relationSummary")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("relationNote")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("pillar")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("tenGod")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("twelvePhase")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("note")));
    QVERIFY(!firstMajorFortune.value(QStringLiteral("relationSummary")).toString().isEmpty());
    QCOMPARE(firstMajorFortune.value(QStringLiteral("relationSummary")).toString(), QStringLiteral("該当なし"));
}

void CoreTests::chartCalculatorAddsMajorFortuneRelationsForSupportedSampleYear()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QVariantMap firstMajorFortune = result.majorFortunes.at(0).toMap();

    QVERIFY(firstMajorFortune.contains(QStringLiteral("relationSummary")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("sameStemMatches")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("sameBranchMatches")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("clashBranches")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("stemCombinationCandidates")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("relationNote")));
    QVERIFY(!firstMajorFortune.value(QStringLiteral("relationSummary")).toString().isEmpty());

    const QVariant sameStemMatches = firstMajorFortune.value(QStringLiteral("sameStemMatches"));
    const QVariant sameBranchMatches = firstMajorFortune.value(QStringLiteral("sameBranchMatches"));
    const QVariant clashBranches = firstMajorFortune.value(QStringLiteral("clashBranches"));
    const QVariant stemCombinationCandidates = firstMajorFortune.value(QStringLiteral("stemCombinationCandidates"));

    QVERIFY(sameStemMatches.canConvert<QStringList>());
    QVERIFY(sameBranchMatches.canConvert<QStringList>());
    QVERIFY(clashBranches.canConvert<QStringList>());
    QVERIFY(stemCombinationCandidates.canConvert<QStringList>());
}

void CoreTests::chartCalculatorPreservesMajorFortuneCoreFieldsWhenAddingRelations()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1990-02-05"),
        QStringLiteral("13:30"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);
    const QVariantMap firstMajorFortune = result.majorFortunes.at(0).toMap();

    QVERIFY(firstMajorFortune.contains(QStringLiteral("pillar")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("tenGod")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("twelvePhase")));
    QVERIFY(firstMajorFortune.contains(QStringLiteral("note")));

    QVERIFY(!firstMajorFortune.value(QStringLiteral("pillar")).toString().isEmpty());
    QVERIFY(!firstMajorFortune.value(QStringLiteral("tenGod")).toString().isEmpty());
    QVERIFY(!firstMajorFortune.value(QStringLiteral("twelvePhase")).toString().isEmpty());
    QVERIFY(!firstMajorFortune.value(QStringLiteral("note")).toString().isEmpty());
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

void CoreTests::solarTermDataSourceLoads1883YearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1883);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1883);
    QCOMPARE(yearData.entries.size(), 12);
    QCOMPARE(yearData.entries.first().termName, QStringLiteral("小寒"));
    QCOMPARE(yearData.entries.last().termName, QStringLiteral("大雪"));
    QCOMPARE(
        yearData.entries.last().atDateTime.toString(Qt::ISODate),
        QStringLiteral("1883-12-07T12:00:00+09:00")
    );
}

void CoreTests::solarTermDataSourceLoads1923YearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1923);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1923);
    QCOMPARE(yearData.entries.size(), 12);
    QCOMPARE(yearData.entries.first().termName, QStringLiteral("小寒"));
    QCOMPARE(yearData.entries.at(1).termName, QStringLiteral("立春"));
    QCOMPARE(
        yearData.entries.first().atDateTime.toString(Qt::ISODate),
        QStringLiteral("1923-01-06T12:00:00+09:00")
    );
}

void CoreTests::solarTermDataSourceLoads2026YearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(2026);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 2026);
    QVERIFY(!yearData.entries.isEmpty());
    QCOMPARE(yearData.entries.at(1).termName, QStringLiteral("立春"));
    QCOMPARE(
        yearData.entries.at(1).atDateTime.toString(Qt::ISODate),
        QStringLiteral("2026-02-04T05:02:00+09:00")
    );
}

void CoreTests::solarTermDataSourceLoadsSampleYearData()
{
    SolarTermDataSource dataSource;

    const SolarTermYearData yearData = dataSource.loadYearData(1990);

    QVERIFY(yearData.dataSourceAvailable);
    QVERIFY(yearData.hasYearData);
    QCOMPARE(yearData.year, 1990);
    QVERIFY(!yearData.entries.isEmpty());
    QCOMPARE(yearData.entries.size(), 12);
    QCOMPARE(yearData.entries.first().termName, QStringLiteral("小寒"));
    QCOMPARE(yearData.entries.at(1).termName, QStringLiteral("立春"));
    QVERIFY(yearData.entries.first().atDateTime.isValid());
    QCOMPARE(yearData.entries.first().timeZoneOffsetText, QStringLiteral("+09:00"));
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

void CoreTests::chartCalculatorMatches2026LichunBoundaryCaseYearAndMonthPillars()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("2026-02-04"),
        QStringLiteral("06:00"),
        QStringLiteral("女性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.yearPillar, QStringLiteral("丙午"));
    QCOMPARE(result.monthPillar, QStringLiteral("庚寅"));
}

void CoreTests::chartCalculatorChangesYearAndMonthPillarsAcross2026LichunBoundary()
{
    ChartCalculator calculator;
    const BirthInfo beforeBoundary{
        QStringLiteral("2026-02-04"),
        QStringLiteral("04:30"),
        QStringLiteral("女性")
    };
    const BirthInfo afterBoundary{
        QStringLiteral("2026-02-04"),
        QStringLiteral("06:00"),
        QStringLiteral("女性")
    };

    const ChartResult beforeResult = calculator.calculate(beforeBoundary);
    const ChartResult afterResult = calculator.calculate(afterBoundary);

    QCOMPARE(beforeResult.yearPillar, QStringLiteral("乙巳"));
    QCOMPARE(beforeResult.monthPillar, QStringLiteral("己丑"));
    QCOMPARE(afterResult.yearPillar, QStringLiteral("丙午"));
    QCOMPARE(afterResult.monthPillar, QStringLiteral("庚寅"));
    QVERIFY(beforeResult.yearPillar != afterResult.yearPillar);
    QVERIFY(beforeResult.monthPillar != afterResult.monthPillar);
}

void CoreTests::chartCalculatorMatchesTz001MonthPillarIf1923DataIsSupported()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1923-01-15"),
        QStringLiteral("14:00"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("癸丑"));
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("本アプリ採用仕様")));
    QVERIFY(!result.description.contains(QStringLiteral("指定年の正節データが未整備")));
}

void CoreTests::chartCalculatorMatchesTz002MonthPillarIf1883DataIsSupported()
{
    ChartCalculator calculator;
    const BirthInfo birthInfo{
        QStringLiteral("1883-12-14"),
        QStringLiteral("04:00"),
        QStringLiteral("男性")
    };

    const ChartResult result = calculator.calculate(birthInfo);

    QCOMPARE(result.monthPillar, QStringLiteral("甲子"));
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("本アプリ採用仕様")));
    QVERIFY(!result.description.contains(QStringLiteral("指定年の正節データが未整備")));
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
    QCOMPARE(result.monthPillarStatusMessage, QStringLiteral("指定年の正節データが未整備のため、月柱を確定できません。"));
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
    QCOMPARE(
        result.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("逆行")
    );
    QVERIFY(result.majorFortuneDirectionStatusMessage.contains(QStringLiteral("一般四柱推命")));
    QCOMPARE(
        result.solarTermDifferencePreparation.value(QStringLiteral("referenceTerm")).toString(),
        QStringLiteral("未対応")
    );
    QVERIFY(result.solarTermDifferencePreparationStatusMessage.contains(QStringLiteral("出生日時より前の節入り日時を特定できませんでした")));
    QCOMPARE(result.majorFortunes.size(), 1);
    QCOMPARE(result.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("未対応"));
    QVERIFY(result.majorFortunesStatusMessage.contains(QStringLiteral("未対応")));
    QCOMPARE(result.annualFortunes.size(), 12);
    QCOMPARE(result.annualFortunes.at(0).toMap().value(QStringLiteral("year")).toInt(), 1949);
    QCOMPARE(result.annualFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(), QStringLiteral("己丑"));
    QVERIFY(result.annualFortunesStatusMessage.contains(QStringLiteral("流年表示")));
    QVERIFY(result.description.contains(QStringLiteral("指定年の正節データが未整備")));
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
    QVERIFY(result.monthPillarStatusMessage.contains(QStringLiteral("小寒")));
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
    QVERIFY(resolution.statusMessage.contains(QStringLiteral("本アプリ採用仕様")));
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
            {QStringLiteral("note"), QStringLiteral("断定しない参考候補です。")},
            {QStringLiteral("balanceState"), QStringLiteral("strong")},
            {QStringLiteral("referenceScore"), 4},
            {QStringLiteral("strengthPriority"), QStringList{QStringLiteral("水"), QStringLiteral("火")}},
            {QStringLiteral("climatePriority"), QStringList{QStringLiteral("水"), QStringLiteral("火"), QStringLiteral("土")}},
            {QStringLiteral("shortagePriority"), QStringList{QStringLiteral("水"), QStringLiteral("金"), QStringLiteral("木"), QStringLiteral("火")}},
            {QStringLiteral("rankedElements"), QVariantList{
                QVariantMap{{QStringLiteral("element"), QStringLiteral("水")}, {QStringLiteral("score"), 10}},
                QVariantMap{{QStringLiteral("element"), QStringLiteral("金")}, {QStringLiteral("score"), 4}},
                QVariantMap{{QStringLiteral("element"), QStringLiteral("木")}, {QStringLiteral("score"), 3}}
            }}
        },
        QStringLiteral("用神候補の暫定表示です。"),
        {
            {QStringLiteral("candidates"), QStringList{QStringLiteral("印綬格"), QStringLiteral("正財格"), QStringLiteral("正官格")}},
            {QStringLiteral("reason"), QStringLiteral("格局候補の暫定理由です。")},
            {QStringLiteral("note"), QStringLiteral("断定しない格局候補です。")},
            {QStringLiteral("primaryBasis"), QStringLiteral("month_tengod_centered")},
            {QStringLiteral("monthTenGod"), QStringLiteral("印綬")},
            {QStringLiteral("monthHiddenStemTenGods"), QStringList{QStringLiteral("正財"), QStringLiteral("正官")}},
            {QStringLiteral("strengthSupport"), QStringLiteral("保存確認用の強弱補助です。")},
            {QStringLiteral("usefulGodSupport"), QStringLiteral("保存確認用の用神候補補助です。")},
            {QStringLiteral("rankedPatterns"), QVariantList{
                QVariantMap{{QStringLiteral("pattern"), QStringLiteral("印綬格")}, {QStringLiteral("score"), 10}},
                QVariantMap{{QStringLiteral("pattern"), QStringLiteral("正財格")}, {QStringLiteral("score"), 7}},
                QVariantMap{{QStringLiteral("pattern"), QStringLiteral("正官格")}, {QStringLiteral("score"), 6}}
            }}
        },
        QStringLiteral("格局候補の暫定表示です。"),
        {
            QVariantMap{
                {QStringLiteral("index"), 1},
                {QStringLiteral("startAge"), 5},
                {QStringLiteral("endAge"), 14},
                {QStringLiteral("label"), QStringLiteral("5〜14歳")},
                {QStringLiteral("pillar"), QStringLiteral("戊寅")},
                {QStringLiteral("tenGod"), QStringLiteral("印綬")},
                {QStringLiteral("twelvePhase"), QStringLiteral("胎")},
                {QStringLiteral("sameStemMatches"), QStringList{QStringLiteral("日柱")}},
                {QStringLiteral("sameBranchMatches"), QStringList{}},
                {QStringLiteral("clashBranches"), QStringList{}},
                {QStringLiteral("stemCombinationCandidates"), QStringList{}},
                {QStringLiteral("relationSummary"), QStringLiteral("同干: 日柱")},
                {QStringLiteral("relationNote"), QStringLiteral("吉凶断定ではなく、原命式との関係候補の参考表示です。")},
                {QStringLiteral("note"), QStringLiteral("起運年齢参考値つきの大運仮データです。")}
            }
        },
        QStringLiteral("大運表示の採用仕様データです。"),
        QVariantList{
                QVariantMap{
                    {QStringLiteral("year"), 1990},
                    {QStringLiteral("pillar"), QStringLiteral("庚午")},
                    {QStringLiteral("tenGod"), QStringLiteral("劫財")},
                    {QStringLiteral("twelvePhase"), QStringLiteral("病")},
                    {QStringLiteral("sameStemMatches"), QStringList{QStringLiteral("年柱")}},
                    {QStringLiteral("sameBranchMatches"), QStringList{QStringLiteral("年柱")}},
                    {QStringLiteral("clashBranches"), QStringList{}},
                    {QStringLiteral("stemCombinationCandidates"), QStringList{QStringLiteral("時柱")}},
                    {QStringLiteral("relationSummary"), QStringLiteral("同干: 年柱 / 同支: 年柱 / 干合候補: 時柱")},
                    {QStringLiteral("relationNote"), QStringLiteral("吉凶断定ではなく、原命式との関係候補の参考表示です。")},
                    {QStringLiteral("note"), QStringLiteral("流年表示骨格の仮データです。")}
                }
            },
        QStringLiteral("流年表示の仮骨格です。"),
        {
            {QStringLiteral("direction"), QStringLiteral("順行")},
            {QStringLiteral("reason"), QStringLiteral("順逆の一般ルール判定理由です。")},
            {QStringLiteral("note"), QStringLiteral("順逆の一般ルール判定です。")}
        },
        QStringLiteral("順逆の一般ルール判定です。"),
        {
            {QStringLiteral("birthDateTime"), QStringLiteral("1990-02-05T13:30:00+09:00")},
            {QStringLiteral("referenceTerm"), QStringLiteral("立春")},
            {QStringLiteral("referenceDirection"), QStringLiteral("直前節入り")},
            {QStringLiteral("referenceDateTime"), QStringLiteral("1990-02-04T00:00:00+09:00")},
            {QStringLiteral("differenceMinutes"), 2250},
            {QStringLiteral("absoluteDifferenceMinutes"), 2250},
            {QStringLiteral("differenceDays"), QStringLiteral("1.56")},
            {QStringLiteral("calculatedStartAge"), 1},
            {QStringLiteral("note"), QStringLiteral("節入り差の参考情報です。")}
        },
        QStringLiteral("節入り差準備の採用仕様表示です。")
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
    QVERIFY(resultMap.contains(QStringLiteral("majorFortuneDirection")));
    QVERIFY(resultMap.contains(QStringLiteral("majorFortuneDirectionStatusMessage")));
    QVERIFY(resultMap.contains(QStringLiteral("solarTermDifferencePreparation")));
    QVERIFY(resultMap.contains(QStringLiteral("solarTermDifferencePreparationStatusMessage")));
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
        resultMap.value(QStringLiteral("usefulGodCandidates")).toMap().value(QStringLiteral("balanceState")).toString(),
        QStringLiteral("strong")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("usefulGodCandidates")).toMap().value(QStringLiteral("referenceScore")).toInt(),
        4
    );
    QVERIFY(
        resultMap.value(QStringLiteral("usefulGodCandidates")).toMap().contains(QStringLiteral("strengthPriority"))
    );
    QVERIFY(
        resultMap.value(QStringLiteral("usefulGodCandidates")).toMap().contains(QStringLiteral("climatePriority"))
    );
    QVERIFY(
        resultMap.value(QStringLiteral("usefulGodCandidates")).toMap().contains(QStringLiteral("shortagePriority"))
    );
    QVERIFY(
        resultMap.value(QStringLiteral("usefulGodCandidates")).toMap().contains(QStringLiteral("rankedElements"))
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
        resultMap.value(QStringLiteral("patternCandidates")).toMap().value(QStringLiteral("primaryBasis")).toString(),
        QStringLiteral("month_tengod_centered")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("patternCandidates")).toMap().value(QStringLiteral("monthTenGod")).toString(),
        QStringLiteral("印綬")
    );
    QVERIFY(
        resultMap.value(QStringLiteral("patternCandidates")).toMap().contains(QStringLiteral("monthHiddenStemTenGods"))
    );
    QVERIFY(
        resultMap.value(QStringLiteral("patternCandidates")).toMap().contains(QStringLiteral("strengthSupport"))
    );
    QVERIFY(
        resultMap.value(QStringLiteral("patternCandidates")).toMap().contains(QStringLiteral("usefulGodSupport"))
    );
    QVERIFY(
        resultMap.value(QStringLiteral("patternCandidates")).toMap().contains(QStringLiteral("rankedPatterns"))
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("patternCandidatesStatusMessage")).toString(),
        QStringLiteral("格局候補の暫定表示です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortuneDirection")).toMap().value(QStringLiteral("direction")).toString(),
        QStringLiteral("順行")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortuneDirectionStatusMessage")).toString(),
        QStringLiteral("順逆の一般ルール判定です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("solarTermDifferencePreparation")).toMap().value(QStringLiteral("referenceTerm")).toString(),
        QStringLiteral("立春")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("solarTermDifferencePreparation")).toMap().value(QStringLiteral("calculatedStartAge")).toInt(),
        1
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("solarTermDifferencePreparationStatusMessage")).toString(),
        QStringLiteral("節入り差準備の採用仕様表示です。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunes")).toList().at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("戊寅")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunes")).toList().at(0).toMap().value(QStringLiteral("tenGod")).toString(),
        QStringLiteral("印綬")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunes")).toList().at(0).toMap().value(QStringLiteral("twelvePhase")).toString(),
        QStringLiteral("胎")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunes")).toList().at(0).toMap().value(QStringLiteral("startAge")).toInt(),
        5
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunes")).toList().at(0).toMap().value(QStringLiteral("relationSummary")).toString(),
        QStringLiteral("同干: 日柱")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("majorFortunesStatusMessage")).toString(),
        QStringLiteral("大運表示の採用仕様データです。")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("annualFortunes")).toList().at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("庚午")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("annualFortunes")).toList().at(0).toMap().value(QStringLiteral("tenGod")).toString(),
        QStringLiteral("劫財")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("annualFortunes")).toList().at(0).toMap().value(QStringLiteral("twelvePhase")).toString(),
        QStringLiteral("病")
    );
    QCOMPARE(
        resultMap.value(QStringLiteral("annualFortunes")).toList().at(0).toMap().value(QStringLiteral("relationSummary")).toString(),
        QStringLiteral("同干: 年柱 / 同支: 年柱 / 干合候補: 時柱")
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
                {QStringLiteral("note"), QStringLiteral("保存確認用の暫定候補注記です。")},
                {QStringLiteral("balanceState"), QStringLiteral("neutral")},
                {QStringLiteral("referenceScore"), 0},
                {QStringLiteral("strengthPriority"), QStringList{}},
                {QStringLiteral("climatePriority"), QStringList{QStringLiteral("火"), QStringLiteral("水")}},
                {QStringLiteral("shortagePriority"), QStringList{QStringLiteral("火"), QStringLiteral("木"), QStringLiteral("土")}},
                {QStringLiteral("rankedElements"), QVariantList{
                    QVariantMap{{QStringLiteral("element"), QStringLiteral("火")}, {QStringLiteral("score"), 7}},
                    QVariantMap{{QStringLiteral("element"), QStringLiteral("木")}, {QStringLiteral("score"), 6}},
                    QVariantMap{{QStringLiteral("element"), QStringLiteral("土")}, {QStringLiteral("score"), 5}}
                }}
            },
            QStringLiteral("用神候補の保存確認用データです。"),
            {
                {QStringLiteral("candidates"), QStringList{QStringLiteral("偏財格"), QStringLiteral("食神格"), QStringLiteral("偏官格")}},
                {QStringLiteral("reason"), QStringLiteral("保存確認用の格局候補理由です。")},
                {QStringLiteral("note"), QStringLiteral("保存確認用の格局候補注記です。")},
                {QStringLiteral("primaryBasis"), QStringLiteral("month_tengod_centered")},
                {QStringLiteral("monthTenGod"), QStringLiteral("偏財")},
                {QStringLiteral("monthHiddenStemTenGods"), QStringList{QStringLiteral("食神"), QStringLiteral("偏官")}},
                {QStringLiteral("strengthSupport"), QStringLiteral("保存確認用の強弱補助です。")},
                {QStringLiteral("usefulGodSupport"), QStringLiteral("保存確認用の用神候補補助です。")},
                {QStringLiteral("rankedPatterns"), QVariantList{
                    QVariantMap{{QStringLiteral("pattern"), QStringLiteral("偏財格")}, {QStringLiteral("score"), 9}},
                    QVariantMap{{QStringLiteral("pattern"), QStringLiteral("食神格")}, {QStringLiteral("score"), 7}},
                    QVariantMap{{QStringLiteral("pattern"), QStringLiteral("偏官格")}, {QStringLiteral("score"), 5}}
                }}
            },
            QStringLiteral("格局候補の保存確認用データです。"),
            {
                QVariantMap{
                    {QStringLiteral("index"), 1},
                    {QStringLiteral("startAge"), 6},
                    {QStringLiteral("endAge"), 15},
                    {QStringLiteral("label"), QStringLiteral("6〜15歳")},
                    {QStringLiteral("pillar"), QStringLiteral("己丑")},
                    {QStringLiteral("tenGod"), QStringLiteral("偏印")},
                    {QStringLiteral("twelvePhase"), QStringLiteral("養")},
                    {QStringLiteral("sameStemMatches"), QStringList{}},
                    {QStringLiteral("sameBranchMatches"), QStringList{QStringLiteral("月柱")}},
                    {QStringLiteral("clashBranches"), QStringList{}},
                    {QStringLiteral("stemCombinationCandidates"), QStringList{QStringLiteral("日柱")}},
                    {QStringLiteral("relationSummary"), QStringLiteral("同支: 月柱 / 干合候補: 日柱")},
                    {QStringLiteral("relationNote"), QStringLiteral("保存確認用の大運関係候補です。")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の起運年齢参考値つき大運です。")}
                },
                QVariantMap{
                    {QStringLiteral("index"), 2},
                    {QStringLiteral("startAge"), 16},
                    {QStringLiteral("endAge"), 25},
                    {QStringLiteral("label"), QStringLiteral("16〜25歳")},
                    {QStringLiteral("pillar"), QStringLiteral("庚寅")},
                    {QStringLiteral("tenGod"), QStringLiteral("劫財")},
                    {QStringLiteral("twelvePhase"), QStringLiteral("胎")},
                    {QStringLiteral("sameStemMatches"), QStringList{QStringLiteral("時柱")}},
                    {QStringLiteral("sameBranchMatches"), QStringList{}},
                    {QStringLiteral("clashBranches"), QStringList{QStringLiteral("年柱")}},
                    {QStringLiteral("stemCombinationCandidates"), QStringList{QStringLiteral("月柱")}},
                    {QStringLiteral("relationSummary"), QStringLiteral("同干: 時柱 / 冲候補: 年柱 / 干合候補: 月柱")},
                    {QStringLiteral("relationNote"), QStringLiteral("保存確認用の大運関係候補です。")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の起運年齢参考値つき大運です。")}
                }
            },
            QStringLiteral("大運表示の保存確認用データです。起運年齢は参考値です。"),
            QVariantList{
                QVariantMap{
                    {QStringLiteral("year"), 1992},
                    {QStringLiteral("pillar"), QStringLiteral("壬申")},
                    {QStringLiteral("tenGod"), QStringLiteral("傷官")},
                    {QStringLiteral("twelvePhase"), QStringLiteral("帝旺")},
                    {QStringLiteral("sameStemMatches"), QStringList{QStringLiteral("時柱")}},
                    {QStringLiteral("sameBranchMatches"), QStringList{}},
                    {QStringLiteral("clashBranches"), QStringList{QStringLiteral("月柱")}},
                    {QStringLiteral("stemCombinationCandidates"), QStringList{QStringLiteral("年柱")}},
                    {QStringLiteral("relationSummary"), QStringLiteral("同干: 時柱 / 冲候補: 月柱 / 干合候補: 年柱")},
                    {QStringLiteral("relationNote"), QStringLiteral("保存確認用の流年関係候補です。")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の流年仮表示です。")}
                },
                QVariantMap{
                    {QStringLiteral("year"), 1993},
                    {QStringLiteral("pillar"), QStringLiteral("癸酉")},
                    {QStringLiteral("tenGod"), QStringLiteral("食神")},
                    {QStringLiteral("twelvePhase"), QStringLiteral("建禄")},
                    {QStringLiteral("sameStemMatches"), QStringList{QStringLiteral("年柱")}},
                    {QStringLiteral("sameBranchMatches"), QStringList{}},
                    {QStringLiteral("clashBranches"), QStringList{QStringLiteral("時柱")}},
                    {QStringLiteral("stemCombinationCandidates"), QStringList{QStringLiteral("月柱")}},
                    {QStringLiteral("relationSummary"), QStringLiteral("同干: 年柱 / 冲候補: 時柱 / 干合候補: 月柱")},
                    {QStringLiteral("relationNote"), QStringLiteral("保存確認用の流年関係候補です。")},
                    {QStringLiteral("note"), QStringLiteral("保存確認用の流年仮表示です。")}
                }
            },
            QStringLiteral("流年表示の保存確認用データです。"),
            {
                {QStringLiteral("direction"), QStringLiteral("逆行")},
                {QStringLiteral("reason"), QStringLiteral("保存確認用の順逆一般ルール理由です。")},
                {QStringLiteral("note"), QStringLiteral("保存確認用の順逆一般ルール判定です。")}
            },
            QStringLiteral("順逆の保存確認用一般ルールデータです。"),
            {
                {QStringLiteral("birthDateTime"), QStringLiteral("1992-03-04T10:20:00+09:00")},
                {QStringLiteral("referenceTerm"), QStringLiteral("啓蟄")},
                {QStringLiteral("referenceDirection"), QStringLiteral("直後節入り")},
                {QStringLiteral("referenceDateTime"), QStringLiteral("1992-03-06T00:00:00+09:00")},
                {QStringLiteral("differenceMinutes"), -2260},
                {QStringLiteral("absoluteDifferenceMinutes"), 2260},
                {QStringLiteral("differenceDays"), QStringLiteral("1.57")},
                {QStringLiteral("calculatedStartAge"), 1},
                {QStringLiteral("note"), QStringLiteral("保存確認用の節入り差参考情報です。")}
            },
            QStringLiteral("節入り差準備の保存確認用データです。")
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
        loadedRecord.chartResult.usefulGodCandidates.value(QStringLiteral("balanceState")).toString(),
        QStringLiteral("neutral")
    );
    QCOMPARE(
        loadedRecord.chartResult.usefulGodCandidates.value(QStringLiteral("referenceScore")).toInt(),
        0
    );
    QVERIFY(
        loadedRecord.chartResult.usefulGodCandidates.contains(QStringLiteral("strengthPriority"))
    );
    QVERIFY(
        loadedRecord.chartResult.usefulGodCandidates.contains(QStringLiteral("climatePriority"))
    );
    QVERIFY(
        loadedRecord.chartResult.usefulGodCandidates.contains(QStringLiteral("shortagePriority"))
    );
    QVERIFY(
        loadedRecord.chartResult.usefulGodCandidates.contains(QStringLiteral("rankedElements"))
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
        loadedRecord.chartResult.patternCandidates.value(QStringLiteral("primaryBasis")).toString(),
        QStringLiteral("month_tengod_centered")
    );
    QCOMPARE(
        loadedRecord.chartResult.patternCandidates.value(QStringLiteral("monthTenGod")).toString(),
        QStringLiteral("偏財")
    );
    QVERIFY(
        loadedRecord.chartResult.patternCandidates.contains(QStringLiteral("monthHiddenStemTenGods"))
    );
    QVERIFY(
        loadedRecord.chartResult.patternCandidates.contains(QStringLiteral("strengthSupport"))
    );
    QVERIFY(
        loadedRecord.chartResult.patternCandidates.contains(QStringLiteral("usefulGodSupport"))
    );
    QVERIFY(
        loadedRecord.chartResult.patternCandidates.contains(QStringLiteral("rankedPatterns"))
    );
    QCOMPARE(
        loadedRecord.chartResult.patternCandidatesStatusMessage,
        QStringLiteral("格局候補の保存確認用データです。")
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortuneDirection.value(QStringLiteral("direction")).toString(),
        QStringLiteral("逆行")
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortuneDirectionStatusMessage,
        QStringLiteral("順逆の保存確認用一般ルールデータです。")
    );
    QCOMPARE(
        loadedRecord.chartResult.solarTermDifferencePreparation.value(QStringLiteral("referenceTerm")).toString(),
        QStringLiteral("啓蟄")
    );
    QCOMPARE(
        loadedRecord.chartResult.solarTermDifferencePreparation.value(QStringLiteral("calculatedStartAge")).toInt(),
        1
    );
    QCOMPARE(
        loadedRecord.chartResult.solarTermDifferencePreparationStatusMessage,
        QStringLiteral("節入り差準備の保存確認用データです。")
    );
    QCOMPARE(loadedRecord.chartResult.majorFortunes.size(), 2);
    QCOMPARE(
        loadedRecord.chartResult.majorFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("己丑")
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortunes.at(0).toMap().value(QStringLiteral("startAge")).toInt(),
        6
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortunes.at(0).toMap().value(QStringLiteral("tenGod")).toString(),
        QStringLiteral("偏印")
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortunes.at(0).toMap().value(QStringLiteral("twelvePhase")).toString(),
        QStringLiteral("養")
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortunes.at(0).toMap().value(QStringLiteral("relationSummary")).toString(),
        QStringLiteral("同支: 月柱 / 干合候補: 日柱")
    );
    QCOMPARE(
        loadedRecord.chartResult.majorFortunesStatusMessage,
        QStringLiteral("大運表示の保存確認用データです。起運年齢は参考値です。")
    );
    QCOMPARE(loadedRecord.chartResult.annualFortunes.size(), 2);
    QCOMPARE(
        loadedRecord.chartResult.annualFortunes.at(0).toMap().value(QStringLiteral("pillar")).toString(),
        QStringLiteral("壬申")
    );
    QCOMPARE(
        loadedRecord.chartResult.annualFortunes.at(0).toMap().value(QStringLiteral("tenGod")).toString(),
        QStringLiteral("傷官")
    );
    QCOMPARE(
        loadedRecord.chartResult.annualFortunes.at(0).toMap().value(QStringLiteral("twelvePhase")).toString(),
        QStringLiteral("帝旺")
    );
    QCOMPARE(
        loadedRecord.chartResult.annualFortunes.at(0).toMap().value(QStringLiteral("relationSummary")).toString(),
        QStringLiteral("同干: 時柱 / 冲候補: 月柱 / 干合候補: 年柱")
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
    ChartResult chartResult{
        QStringLiteral("甲子"),
        QStringLiteral("乙丑"),
        QStringLiteral("丙寅"),
        QStringLiteral("丁卯"),
        QStringLiteral("これは仮の命式結果です。")
    };
    chartResult.majorFortunes = QVariantList{
        QVariantMap{
            {QStringLiteral("index"), 1},
            {QStringLiteral("startAge"), 5},
            {QStringLiteral("endAge"), 14},
            {QStringLiteral("label"), QStringLiteral("5〜14歳")},
            {QStringLiteral("pillar"), QStringLiteral("戊寅")},
            {QStringLiteral("tenGod"), QStringLiteral("印綬")},
            {QStringLiteral("twelvePhase"), QStringLiteral("胎")},
            {QStringLiteral("note"), QStringLiteral("出力確認用の大運です。")}
        }
    };
    chartResult.annualFortunes = QVariantList{
        QVariantMap{
            {QStringLiteral("year"), 1990},
            {QStringLiteral("pillar"), QStringLiteral("庚午")},
            {QStringLiteral("tenGod"), QStringLiteral("劫財")},
            {QStringLiteral("twelvePhase"), QStringLiteral("病")},
            {QStringLiteral("relationSummary"), QStringLiteral("同干: 年柱 / 同支: 年柱 / 干合候補: 時柱")},
            {QStringLiteral("note"), QStringLiteral("出力確認用の流年です。")}
        }
    };
    const SavedChartRecord record{
        BirthInfo{QStringLiteral("1990-01-01"), QStringLiteral("13:30"), QStringLiteral("男性")},
        chartResult,
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
    QVERIFY(content.contains(QStringLiteral("順逆:")));
    QVERIFY(content.contains(QStringLiteral("出生日時(節入り差準備):")));
    QVERIFY(content.contains(QStringLiteral("起運年齢:")));
    QVERIFY(content.contains(QStringLiteral("大運一覧:")));
    QVERIFY(content.contains(QStringLiteral("流年一覧:")));
    QVERIFY(content.contains(QStringLiteral("関係:")));
    QVERIFY(content.contains(QStringLiteral("十二運:")));
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

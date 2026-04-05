#include "RecordExportService.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStringConverter>
#include <QStandardPaths>
#include <QTextStream>

namespace {
QString formatStemList(const QVariant &value)
{
    const QStringList values = value.toStringList();
    if (!values.isEmpty()) {
        return values.join(QStringLiteral(" "));
    }

    const QString text = value.toString();
    return text.isEmpty() ? QStringLiteral("未対応") : text;
}

QString formatElementCount(const QVariant &value)
{
    return value.isValid() ? value.toString() : QStringLiteral("0");
}

QString buildTextContent(const SavedChartRecord &record)
{
    QString content;
    QTextStream stream(&content);
    stream.setEncoding(QStringConverter::Utf8);

    stream << "FourPillarsOfDestiny 出力\n";
    stream << "savedAt: " << record.savedAt << "\n";
    stream << "\n";
    stream << "[BirthInfo]\n";
    stream << "生年月日: " << record.birthInfo.birthDate << "\n";
    stream << "出生時刻: " << record.birthInfo.birthTime << "\n";
    stream << "性別: " << record.birthInfo.gender << "\n";
    stream << "\n";
    stream << "[ChartResult]\n";
    stream << "年柱: " << record.chartResult.yearPillar << "\n";
    stream << "月柱: " << record.chartResult.monthPillar << "\n";
    stream << "月柱状態: " << record.chartResult.monthPillarStatusMessage << "\n";
    stream << "日柱: " << record.chartResult.dayPillar << "\n";
    stream << "時柱: " << record.chartResult.hourPillar << "\n";
    stream << "命式説明: " << record.chartResult.description << "\n";
    stream << "通変星(年柱): " << record.chartResult.tenGods.value(QStringLiteral("yearPillar")).toString() << "\n";
    stream << "通変星(月柱): " << record.chartResult.tenGods.value(QStringLiteral("monthPillar")).toString() << "\n";
    stream << "通変星(日柱): " << record.chartResult.tenGods.value(QStringLiteral("dayPillar")).toString() << "\n";
    stream << "通変星(時柱): " << record.chartResult.tenGods.value(QStringLiteral("hourPillar")).toString() << "\n";
    stream << "蔵干(年支): " << formatStemList(record.chartResult.hiddenStems.value(QStringLiteral("yearPillar"))) << "\n";
    stream << "蔵干(月支): " << formatStemList(record.chartResult.hiddenStems.value(QStringLiteral("monthPillar"))) << "\n";
    stream << "蔵干(日支): " << formatStemList(record.chartResult.hiddenStems.value(QStringLiteral("dayPillar"))) << "\n";
    stream << "蔵干(時支): " << formatStemList(record.chartResult.hiddenStems.value(QStringLiteral("hourPillar"))) << "\n";
    stream << "五行(木): " << formatElementCount(record.chartResult.fiveElements.value(QStringLiteral("wood"))) << "\n";
    stream << "五行(火): " << formatElementCount(record.chartResult.fiveElements.value(QStringLiteral("fire"))) << "\n";
    stream << "五行(土): " << formatElementCount(record.chartResult.fiveElements.value(QStringLiteral("earth"))) << "\n";
    stream << "五行(金): " << formatElementCount(record.chartResult.fiveElements.value(QStringLiteral("metal"))) << "\n";
    stream << "五行(水): " << formatElementCount(record.chartResult.fiveElements.value(QStringLiteral("water"))) << "\n";
    stream << "五行集計状態: " << record.chartResult.fiveElementDistributionStatusMessage << "\n";
    stream << "月支季節: " << record.chartResult.seasonalEvaluation.value(QStringLiteral("season")).toString() << "\n";
    stream << "季節適性: " << record.chartResult.seasonalEvaluation.value(QStringLiteral("suitability")).toString() << "\n";
    stream << "季節評価状態: " << record.chartResult.seasonalEvaluationStatusMessage << "\n";
    stream << "暫定強弱評価: " << record.chartResult.strengthEvaluation.value(QStringLiteral("label")).toString() << "\n";
    stream << "暫定強弱理由: " << record.chartResult.strengthEvaluation.value(QStringLiteral("reason")).toString() << "\n";
    stream << "暫定強弱状態: " << record.chartResult.strengthEvaluationStatusMessage << "\n";
    stream << "寒暖傾向: " << record.chartResult.climateEvaluation.value(QStringLiteral("temperature")).toString() << "\n";
    stream << "乾湿傾向: " << record.chartResult.climateEvaluation.value(QStringLiteral("moisture")).toString() << "\n";
    stream << "寒暖・乾湿メモ: " << record.chartResult.climateEvaluation.value(QStringLiteral("note")).toString() << "\n";
    stream << "寒暖・乾湿状態: " << record.chartResult.climateEvaluationStatusMessage << "\n";
    stream << "\n";
    stream << "[InterpretationResult]\n";
    stream << "summaryText: " << record.interpretationResult.summaryText << "\n";
    stream << "detailText: " << record.interpretationResult.detailText << "\n";
    stream << "cautionText: " << record.interpretationResult.cautionText << "\n";
    stream << "\n";
    stream << "注記: この内容は仮実装の出力です。\n";

    return content;
}
}

RecordExportService::RecordExportService(QString baseDirectoryPath)
    : m_baseDirectoryPath(baseDirectoryPath)
{
}

bool RecordExportService::exportAsText(
    const SavedChartRecord &record,
    QString *exportedFilePath,
    QString *errorMessage
) const
{
    const QString directoryPath = exportDirectoryPath();
    QDir directory;

    if (!directory.mkpath(directoryPath)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("出力先ディレクトリを作成できませんでした。");
        }
        return false;
    }

    const QString fileName = QStringLiteral("export_%1.txt")
                                 .arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz")));
    const QString filePath = QDir(directoryPath).filePath(fileName);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("テキスト出力ファイルを開けませんでした。");
        }
        return false;
    }

    const QByteArray bytes = buildTextContent(record).toUtf8();
    const qint64 written = file.write(bytes);
    file.close();

    if (written < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("テキスト出力に失敗しました。");
        }
        return false;
    }

    if (exportedFilePath) {
        *exportedFilePath = filePath;
    }

    return true;
}

bool RecordExportService::exportAsJson(
    const SavedChartRecord &record,
    QString *exportedFilePath,
    QString *errorMessage
) const
{
    const QString directoryPath = exportDirectoryPath();
    QDir directory;

    if (!directory.mkpath(directoryPath)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("出力先ディレクトリを作成できませんでした。");
        }
        return false;
    }

    const QString fileName = QStringLiteral("export_%1.json")
                                 .arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz")));
    const QString filePath = QDir(directoryPath).filePath(fileName);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("JSON 出力ファイルを開けませんでした。");
        }
        return false;
    }

    QJsonObject jsonObject = record.toJsonObject();
    jsonObject.remove(QStringLiteral("savedAt"));
    jsonObject.insert(
        QStringLiteral("exportedAt"),
        QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
    );

    const QJsonDocument document(jsonObject);
    const qint64 written = file.write(document.toJson(QJsonDocument::Indented));
    file.close();

    if (written < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("JSON 出力に失敗しました。");
        }
        return false;
    }

    if (exportedFilePath) {
        *exportedFilePath = filePath;
    }

    return true;
}

QString RecordExportService::exportDirectoryPath() const
{
    if (!m_baseDirectoryPath.isEmpty()) {
        return QDir(m_baseDirectoryPath).filePath(QStringLiteral("exports"));
    }

    const QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataLocation).filePath(QStringLiteral("exports"));
}

void RecordExportService::setBaseDirectoryPath(const QString &baseDirectoryPath)
{
    m_baseDirectoryPath = baseDirectoryPath;
}

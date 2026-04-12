#!/usr/bin/env python3
"""検証データセット全体の簡易サマリーレポートを生成する。"""

from __future__ import annotations

import csv
import json
from collections import Counter
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
TEST_DATA_DIR = REPO_ROOT / "tests" / "data"
OUTPUT_PATH = REPO_ROOT / "docs" / "verification_dataset_summary.md"


def read_csv_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle))


def read_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def duplicate_ids(values: list[str]) -> list[str]:
    counter = Counter(value for value in values if value)
    return sorted([value for value, count in counter.items() if count > 1])


def yes_no(value: bool) -> str:
    return "yes" if value else "no"


def joined(values: list[str]) -> str:
    return ", ".join(values) if values else "なし"


def main() -> None:
    master_rows = read_csv_rows(TEST_DATA_DIR / "test_samples_master.csv")
    pillars_rows = read_csv_rows(TEST_DATA_DIR / "expected_pillars.csv")
    judgement_rows = read_csv_rows(TEST_DATA_DIR / "expected_judgement.csv")
    relations_rows = read_csv_rows(TEST_DATA_DIR / "expected_relations.csv")
    luck_rows = read_csv_rows(TEST_DATA_DIR / "expected_luck_cycle.csv")
    annual_rows = read_csv_rows(TEST_DATA_DIR / "expected_annual_fortunes.csv")
    verification_cases = read_json(TEST_DATA_DIR / "verification_cases.json").get("cases", [])
    spec_gaps = read_json(TEST_DATA_DIR / "spec_gap_registry.json").get("specGaps", [])

    master_ids = {row["sample_id"] for row in master_rows if row.get("sample_id")}
    pillars_ids = {row["sample_id"] for row in pillars_rows if row.get("sample_id")}
    judgement_ids = {row["sample_id"] for row in judgement_rows if row.get("sample_id")}
    relations_ids = {row["sample_id"] for row in relations_rows if row.get("sample_id")}
    luck_ids = {row["sample_id"] for row in luck_rows if row.get("sample_id")}
    annual_ids = {row["sample_id"] for row in annual_rows if row.get("sample_id")}
    case_ids = {case.get("caseId", "") for case in verification_cases if case.get("caseId")}

    all_ids = sorted(
        master_ids
        | pillars_ids
        | judgement_ids
        | relations_ids
        | luck_ids
        | annual_ids
        | case_ids
    )

    regression_cases = [case for case in verification_cases if case.get("enabledForRegression", True)]
    non_regression_cases = [case for case in verification_cases if not case.get("enabledForRegression", True)]
    external_cases = [case for case in verification_cases if str(case.get("confidence", "")).startswith("external")]
    keep_as_spec_gap_cases = [
        case for case in verification_cases if case.get("expectedAction") == "keep_as_spec_gap"
    ]

    expected_action_counts = Counter(case.get("expectedAction", "unknown") for case in verification_cases)
    review_status_counts = Counter(case.get("reviewStatus", "unknown") for case in verification_cases)

    spec_gap_rule_hints = {gap.get("ruleHint", "") for gap in spec_gaps if gap.get("ruleHint")}
    keep_as_spec_gap_missing_registry = sorted(
        [
            case.get("caseId", "")
            for case in keep_as_spec_gap_cases
            if case.get("ruleHint") not in spec_gap_rule_hints
        ]
    )

    master_missing_from_expected = sorted(
        master_ids - (pillars_ids | judgement_ids | relations_ids | luck_ids | annual_ids)
    )
    expected_missing_from_master = sorted(
        (pillars_ids | judgement_ids | relations_ids | luck_ids | annual_ids) - master_ids
    )
    verification_missing_from_master = sorted(case_ids - master_ids)

    duplicates = {
        "master.sample_id": duplicate_ids([row.get("sample_id", "") for row in master_rows]),
        "pillars.sample_id": duplicate_ids([row.get("sample_id", "") for row in pillars_rows]),
        "judgement.sample_id": duplicate_ids([row.get("sample_id", "") for row in judgement_rows]),
        "luck_cycle.sample_id": duplicate_ids([row.get("sample_id", "") for row in luck_rows]),
        "annual_fortunes.sample_id": duplicate_ids([row.get("sample_id", "") for row in annual_rows]),
        "verification.caseId": duplicate_ids([case.get("caseId", "") for case in verification_cases]),
    }

    regression_candidates = sorted(
        case.get("caseId", "")
        for case in verification_cases
        if case.get("reviewStatus") == "ready_for_regression_review" and case.get("caseId")
    )
    investigate_candidates = sorted(
        case.get("caseId", "")
        for case in verification_cases
        if case.get("expectedAction") == "investigate_implementation" and case.get("caseId")
    )
    manual_review_candidates = sorted(
        case.get("caseId", "")
        for case in verification_cases
        if case.get("expectedAction") == "manual_review_required" and case.get("caseId")
    )

    lines: list[str] = []
    lines.append("# verification dataset summary")
    lines.append("")
    lines.append("生成方法: `python3 tools/generate_verification_dataset_report.py`")
    lines.append("")
    lines.append("## 1. 全体件数サマリー")
    lines.append("")
    lines.append("| 項目 | 件数 |")
    lines.append("| --- | ---: |")
    lines.append(f"| test_samples_master | {len(master_rows)} |")
    lines.append(f"| expected_pillars | {len(pillars_rows)} |")
    lines.append(f"| expected_judgement | {len(judgement_rows)} |")
    lines.append(f"| expected_relations | {len(relations_rows)} |")
    lines.append(f"| expected_luck_cycle | {len(luck_rows)} |")
    lines.append(f"| expected_annual_fortunes | {len(annual_rows)} |")
    lines.append(f"| verification_cases total | {len(verification_cases)} |")
    lines.append(f"| regression cases | {len(regression_cases)} |")
    lines.append(f"| non-regression cases | {len(non_regression_cases)} |")
    lines.append(f"| external cases | {len(external_cases)} |")
    lines.append(f"| spec gaps | {len(spec_gaps)} |")
    lines.append("")
    lines.append("## 2. sample_id / caseId カバレッジ表")
    lines.append("")
    lines.append("| id | master | pillars | judgement | relations | luck_cycle | annual_fortunes | verification_case |")
    lines.append("| --- | --- | --- | --- | --- | --- | --- | --- |")
    for dataset_id in all_ids:
        lines.append(
            f"| {dataset_id} | {yes_no(dataset_id in master_ids)} | {yes_no(dataset_id in pillars_ids)} "
            f"| {yes_no(dataset_id in judgement_ids)} | {yes_no(dataset_id in relations_ids)} "
            f"| {yes_no(dataset_id in luck_ids)} | {yes_no(dataset_id in annual_ids)} "
            f"| {yes_no(dataset_id in case_ids)} |"
        )
    lines.append("")
    lines.append("## 3. 欠落・不整合一覧")
    lines.append("")
    lines.append(f"- master にあるが expected 系に無い sample_id: {joined(master_missing_from_expected)}")
    lines.append(f"- expected 系にあるが master に無い sample_id: {joined(expected_missing_from_master)}")
    lines.append(f"- verification_cases にあるが master に無い caseId: {joined(verification_missing_from_master)}")
    lines.append(
        f"- keep_as_spec_gap なのに spec_gap_registry に ruleHint がないケース: {joined(keep_as_spec_gap_missing_registry)}"
    )
    lines.append("- duplicate sample_id / caseId:")
    for label, values in duplicates.items():
        lines.append(f"  - {label}: {joined(values)}")
    lines.append("  - relations.sample_id: 複数 relation 行を許容するため重複チェック対象外")
    lines.append("")
    lines.append("## 4. verification_cases の review サマリー")
    lines.append("")
    lines.append(f"- enabledForRegression = true 件数: {len(regression_cases)}")
    lines.append(f"- enabledForRegression = false 件数: {len(non_regression_cases)}")
    lines.append("- expectedAction ごとの件数:")
    for key in ["keep_as_spec_gap", "investigate_implementation", "manual_review_required", "no_action_needed"]:
        lines.append(f"  - {key}: {expected_action_counts.get(key, 0)}")
    lines.append("- reviewStatus ごとの件数:")
    for key in sorted(review_status_counts):
        lines.append(f"  - {key}: {review_status_counts[key]}")
    lines.append("")
    lines.append("## 5. spec gap サマリー")
    lines.append("")
    lines.append("| ruleHint | title | status | relatedCaseIds | adoptedSpec |")
    lines.append("| --- | --- | --- | --- | --- |")
    for gap in spec_gaps:
        related = ",".join(gap.get("relatedCaseIds", []))
        lines.append(
            f"| {gap.get('ruleHint', '')} | {gap.get('title', '')} | {gap.get('status', '')} "
            f"| {related} | {gap.get('adoptedSpec', '')} |"
        )
    lines.append("")
    lines.append("## 6. 次の候補")
    lines.append("")
    lines.append(f"- 次に regression 昇格候補になりそうな caseId: {joined(regression_candidates)}")
    lines.append(f"- 次に investigate_implementation 対象として掘る caseId: {joined(investigate_candidates)}")
    lines.append(f"- 次に manual_review のまま追加確認が必要な caseId: {joined(manual_review_candidates)}")
    lines.append("")

    OUTPUT_PATH.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Ekspor telemetry dan pump_events Supabase ke satu file Excel."""

# PENTING: service role key HANYA dipakai di laptop sendiri.
# Jangan simpan key tersebut di kode, dashboard, atau repository.

from __future__ import annotations

import argparse
import os
import sys
from datetime import date, datetime, time, timedelta, timezone
from pathlib import Path
from typing import Any
from zoneinfo import ZoneInfo

import pandas as pd
import requests
from openpyxl.utils import get_column_letter


PAGE_SIZE = 1_000
JAKARTA = ZoneInfo("Asia/Jakarta")
TABLE_COLUMNS = {
    "telemetry": [
        "id",
        "created_at",
        "device_id",
        "adc",
        "persen",
        "suhu",
        "hum",
        "pompa",
        "mode",
        "state",
    ],
    "pump_events": ["id", "created_at", "device_id", "durasi_ms", "sumber"],
}


def parse_date(value: str) -> date:
    """Terima tanggal ISO agar rentang ekspor tidak ambigu."""
    try:
        return date.fromisoformat(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError("Gunakan format tanggal YYYY-MM-DD.") from error


def local_midnight_to_utc(value: date) -> str:
    """Ubah awal hari Asia/Jakarta menjadi timestamp UTC untuk filter Supabase."""
    local = datetime.combine(value, time.min, tzinfo=JAKARTA)
    return local.astimezone(timezone.utc).isoformat()


def date_filters(dari: date | None, sampai: date | None) -> list[tuple[str, str]]:
    """Buat filter created_at; --sampai bersifat inklusif untuk satu hari penuh."""
    filters: list[tuple[str, str]] = []
    if dari:
        filters.append(("created_at", f"gte.{local_midnight_to_utc(dari)}"))
    if sampai:
        filters.append(
            ("created_at", f"lt.{local_midnight_to_utc(sampai + timedelta(days=1))}")
        )
    return filters


def fetch_all(
    session: requests.Session,
    base_url: str,
    table: str,
    filters: list[tuple[str, str]],
) -> list[dict[str, Any]]:
    """Ambil semua halaman Data API dengan maksimum 1.000 baris per request."""
    rows: list[dict[str, Any]] = []
    offset = 0
    params = [("select", "*"), ("order", "id.asc"), *filters]

    while True:
        headers = {
            "Range-Unit": "items",
            "Range": f"{offset}-{offset + PAGE_SIZE - 1}",
        }
        try:
            response = session.get(
                f"{base_url}/rest/v1/{table}",
                params=params,
                headers=headers,
                timeout=30,
            )
            response.raise_for_status()
        except requests.RequestException as error:
            status = getattr(error.response, "status_code", None)
            detail = f" (HTTP {status})" if status else ""
            raise RuntimeError(f"Gagal mengambil tabel {table}{detail}.") from error

        try:
            page = response.json()
        except ValueError as error:
            raise RuntimeError(f"Respons tabel {table} bukan JSON yang valid.") from error

        if not isinstance(page, list):
            raise RuntimeError(f"Respons tabel {table} bukan daftar baris.")

        rows.extend(page)
        if len(page) < PAGE_SIZE:
            return rows
        offset += len(page)


def prepare_dataframe(rows: list[dict[str, Any]], columns: list[str]) -> pd.DataFrame:
    """Susun kolom tetap dan ubah timestamp UTC menjadi waktu Jakarta tanpa timezone."""
    frame = pd.DataFrame(rows, columns=columns)
    if not frame.empty:
        timestamps = pd.to_datetime(frame["created_at"], utc=True, errors="coerce")
        frame["created_at"] = timestamps.dt.tz_convert(JAKARTA).dt.tz_localize(None)
    return frame


def write_excel(dataframes: dict[str, pd.DataFrame], output_path: Path) -> None:
    """Tulis satu sheet per tabel dengan timestamp lokal yang dapat dibaca Excel."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with pd.ExcelWriter(
        output_path,
        engine="openpyxl",
        datetime_format="yyyy-mm-dd hh:mm:ss",
    ) as writer:
        for sheet_name, frame in dataframes.items():
            frame.to_excel(writer, sheet_name=sheet_name, index=False)
            sheet = writer.sheets[sheet_name]
            sheet.freeze_panes = "A2"
            sheet.auto_filter.ref = sheet.dimensions

            for index, column in enumerate(frame.columns, start=1):
                if column == "created_at":
                    for cell in sheet[get_column_letter(index)][1:]:
                        cell.number_format = "yyyy-mm-dd hh:mm:ss"
                sheet.column_dimensions[get_column_letter(index)].width = min(
                    max(len(column) + 2, 14), 24
                )


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Ekspor telemetry dan pump_events ke XLSX. Rentang tanggal memakai "
            "hari Asia/Jakarta."
        )
    )
    parser.add_argument("--dari", type=parse_date, metavar="YYYY-MM-DD")
    parser.add_argument("--sampai", type=parse_date, metavar="YYYY-MM-DD")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("tomat_export.xlsx"),
        metavar="FILE.xlsx",
        help="Nama atau lokasi file Excel hasil ekspor.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    if args.dari and args.sampai and args.dari > args.sampai:
        print("--dari tidak boleh lebih akhir dari --sampai.", file=sys.stderr)
        return 2

    supabase_url = os.environ.get("SUPABASE_URL", "").strip().rstrip("/")
    service_key = os.environ.get("SUPABASE_SERVICE_KEY", "").strip()
    missing = [
        name
        for name, value in {
            "SUPABASE_URL": supabase_url,
            "SUPABASE_SERVICE_KEY": service_key,
        }.items()
        if not value
    ]
    if missing:
        print(
            "Variabel lingkungan belum diisi: " + ", ".join(missing) + ".",
            file=sys.stderr,
        )
        return 2

    session = requests.Session()
    session.headers.update(
        {
            "apikey": service_key,
            "Authorization": f"Bearer {service_key}",
            "Accept": "application/json",
        }
    )

    filters = date_filters(args.dari, args.sampai)
    try:
        dataframes = {
            table: prepare_dataframe(
                fetch_all(session, supabase_url, table, filters), columns
            )
            for table, columns in TABLE_COLUMNS.items()
        }
        write_excel(dataframes, args.output)
    except RuntimeError as error:
        print(str(error), file=sys.stderr)
        return 1

    print(f"Ekspor selesai: {args.output.resolve()}")
    for table, frame in dataframes.items():
        print(f"- {table}: {len(frame)} baris")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

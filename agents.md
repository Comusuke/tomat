# AGENTS.md: Tomat Smart Farming (cakupan Codex: website dan database)

Sistem penyiraman otomatis tomat: ESP32 + Supabase + website.
Baca `docs/PRD.md` sebelum mengerjakan tugas apa pun.

## Cakupan kerja
Yang boleh dikerjakan:
- `dashboard/index.html`
- `supabase/schema.sql`
- `tools/` (skrip ekspor Excel)
- `docs/` dan `README.md`

DILARANG diubah, dibuat ulang, atau "dirapikan":
- Semua file di `firmware/` (sketch ESP32 dikelola pemilik secara terpisah).
Jika sebuah tugas tampaknya butuh perubahan firmware, JANGAN mengubahnya.
Jelaskan perubahan apa yang dibutuhkan, lalu tunggu pemilik yang mengerjakannya.

## Kontrak data dengan firmware (TIDAK BOLEH berubah tanpa persetujuan pemilik)
Firmware mengirim dan membaca data dengan nama persis seperti di bawah.
Mengubah nama tabel, nama kolom, atau nilai di bawah akan memutus alat.

Tabel `telemetry` (ESP32 menulis): `id`, `created_at`, `device_id` (default 'kebun-1'),
`adc` int, `persen` int, `suhu` real (boleh null), `hum` real (boleh null),
`pompa` boolean, `mode` text ('auto' | 'manual'),
`state` text ('IDLE' | 'MENYIRAM' | 'MERESAP' | 'PENGAMAN').

Tabel `pump_events` (ESP32 menulis): `id`, `created_at`, `device_id`,
`durasi_ms` int, `sumber` text ('auto' | 'manual' | 'watchdog').

Tabel `commands` (website menulis, ESP32 membaca):
`id`, `created_at`, `device_id`, `cmd` text ('auto' | 'manual' | 'pump_on' | 'pump_off'),
`done` boolean default false, `hasil` text.

Cara ESP32 memakai `commands`:
- GET `commands?device_id=eq.kebun-1&done=eq.false&order=id.asc&limit=1&select=id,cmd`
- PATCH `commands?id=eq.<id>` dengan body `{"done":true,"hasil":"..."}`
- Perintah yang lebih tua dari 2 menit atau sudah `done` TIDAK boleh bisa dibaca role anon.

Hak akses:
| Role | telemetry | pump_events | commands |
|---|---|---|---|
| anon (ESP32) | insert | insert | select (baru ≤ 2 menit, done = false), update hanya kolom done dan hasil |
| authenticated (pengguna) | select | select | select, insert (hanya dengan done = false) |

## Aturan keselamatan sisi website dan database
1. Website hanya meminta aksi lewat tabel `commands`. Jangan menambah cara lain untuk
   mengendalikan pompa, dan jangan menambah perintah baru di luar empat nilai `cmd`.
2. Jangan mengubah atau melonggarkan batas 2 menit pada perintah.
3. RLS wajib aktif di semua tabel. Jangan memberi hak lebih luas dari tabel di atas.
4. Jangan menyediakan cara apa pun bagi website untuk melewati pengaman di firmware
   (misalnya mengirim perintah massal atau berulang otomatis).
5. Tombol "Nyalakan pompa" di website tidak boleh mengirim lebih dari satu perintah per klik,
   dan harus menonaktifkan tombol sementara sambil menunggu respons alat.

## Rahasia
- Dashboard hanya boleh memakai key publik (anon/publishable) lewat konstanta di bagian atas.
- Service role key tidak boleh muncul di dashboard, repo, atau contoh kode. Hanya boleh dibaca
  dari variabel lingkungan di skrip `tools/` yang dijalankan di laptop pemilik.
- Jangan menulis password, token, atau key asli di file mana pun; pakai placeholder.
- Jangan membuka pendaftaran publik di Supabase Auth.

## Konvensi dashboard
- Satu file `index.html`, tanpa langkah build; supabase-js dan Chart.js dari CDN.
- Data dari database ditulis ke halaman dengan `textContent`, bukan `innerHTML`.
- Antarmuka berbahasa Indonesia, nyaman di layar HP.
- Status alat dianggap offline bila data terakhir lebih tua dari 5 menit.

## Cara bekerja
- Satu tugas kecil per permintaan. Jangan menambah fitur di luar permintaan.
- Jika ada hal yang tidak ditentukan, tanyakan; jangan menebak.
- Ubah seperlunya; jangan menulis ulang file yang sudah berfungsi.
- Jika permintaan bertentangan dengan aturan di atas, jelaskan konfliknya dan minta konfirmasi.
- Di akhir tugas tulis: apa yang diubah, file apa saja, dan apa yang harus kuuji manual.

## Pengujian
- Kamu tidak bisa menguji alat fisik. Jangan mengklaim sistem "sudah teruji" secara keseluruhan.
- Untuk SQL: sertakan cara memverifikasi RLS (akses tanpa login harus ditolak).
- Untuk dashboard: jelaskan cara membukanya dan apa yang harus terlihat.

## Git
- Jangan commit `secrets.h` atau `.env`. Periksa `git status` sebelum commit.
- Pesan commit bermakna: `feat:`, `fix:`, `docs:`.
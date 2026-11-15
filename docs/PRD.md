# PRD: Tomat Smart Farming

| | |
|---|---|
| Versi | 1.0 (MVP) |
| Status | Draft |
| Pemilik | (isi namamu) |
| Terakhir diperbarui | (isi tanggal) |

## 1. Ringkasan

Sistem penyiraman otomatis berbasis ESP32 untuk tanaman tomat. Alat membaca
kelembapan tanah, suhu, dan kelembapan udara, menyiram otomatis dengan pompa
sesuai ambang yang ditetapkan, lalu mencatat semua data ke database Supabase.
Pengguna memantau dan mengontrol alat dari website, dan dapat mengekspor
riwayat data ke Excel.

## 2. Latar Belakang dan Masalah

- Penyiraman manual tidak konsisten dan menyita waktu.
- Tanaman tomat sensitif terhadap kekurangan maupun kelebihan air.
- Pengguna tidak bisa memantau kondisi kebun saat berada di tempat lain.
- Tanpa catatan data, pola penyiraman tidak bisa dievaluasi.

## 3. Tujuan dan Non-Tujuan

**Tujuan (MVP)**
1. Menjaga kelembapan tanah di rentang yang ditetapkan, tanpa intervensi manual.
2. Pompa tidak boleh menyala berlebihan walau terjadi kegagalan (sensor, jaringan, tangki kosong).
3. Semua data tercatat dan dapat dilihat serta diekspor dari mana saja.
4. Pengguna dapat mengontrol pompa jarak jauh secara aman.

**Non-tujuan (di luar MVP)**
- Banyak perangkat/kebun dengan manajemen multi-pengguna.
- Aplikasi mobile native.
- Prediksi cuaca atau rekomendasi pemupukan.
- Kontrol pompa secara real-time (latensi beberapa detik dapat diterima).

## 4. Pengguna

| Peran | Kebutuhan |
|---|---|
| Pemilik kebun (pengguna utama) | Memantau kondisi, mengontrol pompa, melihat riwayat, mengekspor data |
| Pengembang/pemelihara | Mengkalibrasi sensor, mengubah ambang, membaca log untuk debugging |

## 5. Arsitektur

```
ESP32 --HTTPS--> Supabase (PostgreSQL + REST + Auth) <--HTTPS-- Website
  |
  +-- sensor tanah, DHT22, relay + pompa
```

Prinsip utama:
- **Logika penyiraman dan semua pengaman berada di ESP32.** Database dan website hanya mencatat, menampilkan, dan meminta aksi.
- Penyiraman otomatis tetap berjalan walau Wi-Fi atau Supabase tidak tersedia.
- ESP32 yang memulai semua komunikasi (kirim data, ambil perintah). ESP32 tidak membuka port ke internet.
- Website tidak berbicara langsung dengan ESP32; keduanya hanya berhubungan dengan database.

## 6. Struktur Repositori

```
tomat-smart-farming/
├── AGENTS.md
├── README.md
├── .gitignore
├── docs/
│   └── PRD.md
├── firmware/
│   └── tomat_smart_farming/            (nama folder harus sama dengan nama .ino)
│       ├── tomat_smart_farming.ino
│       ├── secrets.h                   (rahasia, TIDAK di-commit)
│       └── secrets.example.h           (contoh placeholder, di-commit)
├── dashboard/
│   └── index.html
└── supabase/
    └── schema.sql
```

## 7. Kebutuhan Fungsional

### 7.1 Firmware (ESP32)

| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-01 | Membaca sensor tanah tiap 1 detik dengan filter median 11 sampel | Must |
| FR-02 | Membaca DHT22 tiap 2,5 detik | Must |
| FR-03 | Sensor dianggap valid bila ADC dalam rentang valid; di luar itu dianggap sensor bermasalah | Must |
| FR-04 | Menyiram otomatis dengan state machine: IDLE → MENYIRAM → MERESAP → IDLE/PENGAMAN | Must |
| FR-05 | Mulai menyiram bila tanah kering; lanjut pulsa berikutnya hingga mencapai target lembap (histeresis) | Must |
| FR-06 | Menunggu air meresap (`WAKTU_RESAP`) sebelum membaca ulang dan memutuskan pulsa berikutnya | Must |
| FR-07 | Masuk mode PENGAMAN setelah `MAKS_PULSA` pulsa berturut-turut tanpa mencapai target, lalu mencoba lagi setelah `JEDA_PENGAMAN` | Must |
| FR-08 | Mode manual: pompa dapat dinyalakan dari website, otomatis mati setelah maksimal 15 detik | Must |
| FR-09 | Mengirim telemetri ke Supabase secara periodik | Must |
| FR-10 | Mengambil perintah baru dari tabel `commands` tiap ±5 detik, menjalankannya, dan menandainya selesai beserta hasilnya | Must |
| FR-11 | Mencatat setiap penyiraman (durasi, sumber: auto/manual/watchdog) ke `pump_events` | Should |
| FR-12 | Menyambung ulang Wi-Fi otomatis bila terputus | Must |
| FR-13 | Mencetak alasan reset (power-on, brownout, watchdog, panic) saat boot | Should |

### 7.2 Website

| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-20 | Login email + password; pendaftaran publik dinonaktifkan | Must |
| FR-21 | Menampilkan nilai terbaru: kelembapan tanah (%), ADC, suhu, kelembapan udara, status pompa, mode, state | Must |
| FR-22 | Menampilkan status alat online/offline (offline bila data terakhir lebih tua dari 5 menit) | Must |
| FR-23 | Grafik riwayat terbaru (tanah dan suhu) | Should |
| FR-24 | Tombol kontrol: mode otomatis, mode manual, pompa ON, pompa OFF, dengan umpan balik hasil | Must |
| FR-25 | Tabel riwayat penyiraman terbaru | Should |
| FR-26 | Unduh riwayat sebagai CSV yang dapat dibuka di Excel | Must |
| FR-27 | Pembaruan data otomatis tiap 15 detik | Should |

## 8. Kebutuhan Keselamatan (Safety)

Bagian ini tidak boleh dilemahkan oleh perubahan fitur apa pun.

| ID | Kebutuhan |
|---|---|
| SF-01 | Relay harus OFF saat boot, saat sensor tidak valid, dan saat reset |
| SF-02 | Watchdog pompa: pompa dipaksa OFF bila menyala lebih dari 20 detik, apa pun state-nya |
| SF-03 | Pompa dilarang menyala (otomatis maupun manual) bila tanah sudah terlalu basah |
| SF-04 | Pompa manual otomatis mati maksimal 15 detik |
| SF-05 | Jeda minimal 1 detik antara OFF dan ON berikutnya |
| SF-06 | Perintah dari server hanya berlaku bila dibuat dalam 2 menit terakhir; perintah lama tidak dijalankan setelah alat kembali online |
| SF-07 | Perintah server tetap melewati semua pengaman firmware (server hanya meminta, firmware memutuskan) |
| SF-08 | Pembacaan sensor diabaikan 400 ms setelah relay berpindah, untuk menghindari noise |
| SF-09 | Aksi jaringan ditunda saat pompa menyala (kecuali mode manual) agar timing pompa tidak tertahan |
| SF-10 | Sensor level air di tangki mencegah pompa menyala saat air habis (perangkat keras, **wajib sebelum dipasang permanen**; belum ada di kode MVP) |

## 9. Kebutuhan Non-Fungsional

| ID | Kebutuhan |
|---|---|
| NF-01 | RLS aktif di semua tabel; ESP32 memakai key publik (anon) dengan hak terbatas; service role key tidak boleh ada di firmware, website, atau repo |
| NF-02 | Kredensial Wi-Fi dan key disimpan di `secrets.h` yang tidak di-commit |
| NF-03 | Sistem berjalan otomatis tanpa internet; kehilangan koneksi tidak boleh menghentikan penyiraman |
| NF-04 | Perintah dari website dieksekusi alat dalam ≤ 10 detik (kondisi normal) |
| NF-05 | Jeda tampilan data maksimal sama dengan interval kirim (15 detik benchmark, 60 detik produksi) |
| NF-06 | Website berfungsi di browser HP dan laptop modern |
| NF-07 | Semua parameter kalibrasi terkumpul di satu blok konfigurasi di firmware |

## 10. Model Data

**telemetry**: `id`, `created_at`, `device_id`, `adc`, `persen`, `suhu`, `hum`, `pompa`, `mode`, `state`

**pump_events**: `id`, `created_at`, `device_id`, `durasi_ms`, `sumber`

**commands**: `id`, `created_at`, `device_id`, `cmd` (`auto` | `manual` | `pump_on` | `pump_off`), `done`, `hasil`

| Role | telemetry | pump_events | commands |
|---|---|---|---|
| anon (ESP32) | insert | insert | select (baru ≤ 2 menit, belum selesai), update `done`/`hasil` |
| authenticated (pengguna) | select | select | select, insert |

## 11. Parameter Kalibrasi

Sensor tanah: **semakin kecil ADC, semakin basah.**

| Parameter | Nilai awal | Status |
|---|---|---|
| `BATAS_KERING` (mulai siram, ADC ≥) | 850 | Berdasarkan pengukuran kering 900-1000 |
| `TARGET_LEMBAP` (cukup, ADC ≤) | 600 | **Estimasi, harus diukur** |
| `BATAS_BASAH` (terlalu basah, ADC ≤) | 400 | **Estimasi, harus diukur** |
| `ADC_KERING` / `ADC_BASAH` (skala 0-100%) | 950 / 300 | **ADC_BASAH estimasi** |
| `DURASI_SIRAM` | 3 dtk (benchmark) / 6 dtk (produksi) | Sesuaikan dengan debit pompa |
| `WAKTU_RESAP` | 10 dtk / 90 dtk | Sesuaikan dengan jenis media tanam |
| `MAKS_PULSA` | 3 | Tinjau setelah mengukur total air per siklus |
| `JEDA_PENGAMAN` | 30 dtk / 30 menit | |
| Interval kirim telemetri | 15 dtk / 60 dtk | |

## 12. Kebutuhan Perangkat Keras

| Komponen | Catatan |
|---|---|
| ESP32 Dev Module | Pin: DHT22 → GPIO18, sensor tanah → GPIO34, relay → GPIO27 |
| Sensor kelembapan tanah | Output analog, ADC kecil = basah (verifikasi arah sebelum dipakai) |
| DHT22 | Butuh pull-up pada jalur data bila modul belum menyertakan |
| Modul relay (active-LOW) | Periksa jumper VCC/JD-VCC |
| Pompa air DC | Catu daya **terpisah** dari ESP32; GND disatukan |
| Dioda flyback + kapasitor | Meredam lonjakan dari pompa agar ESP32 tidak restart |
| Sensor level air tangki | Wajib untuk pemasangan permanen (SF-10) |
| Casing tahan air | Sebelum dipasang di luar ruangan |

## 13. Kriteria Penerimaan (MVP)

| # | Skenario | Hasil yang diharapkan |
|---|---|---|
| AC-01 | Tanah kering, mode otomatis | Pompa menyala sesuai durasi, lalu menunggu resap, lalu membaca ulang |
| AC-02 | Tanah mencapai target setelah pulsa ke-N | Siklus berhenti, hitungan pulsa direset |
| AC-03 | Tanah tidak mencapai target setelah `MAKS_PULSA` | Masuk PENGAMAN, pompa mati, status tampil di website |
| AC-04 | Sensor dicabut | Pompa mati, status "Sensor bermasalah" |
| AC-05 | Pompa dipaksa menyala lebih dari 20 dtk (simulasi) | Watchdog mematikan pompa |
| AC-06 | Tanah terlalu basah, perintah pompa ON | Ditolak dengan pesan jelas |
| AC-07 | Wi-Fi/internet dimatikan | Penyiraman otomatis tetap berjalan; data kembali tercatat saat online |
| AC-08 | Perintah dikirim saat alat offline > 2 menit | Pompa tidak menyala saat alat online kembali |
| AC-09 | Tekan "Nyalakan pompa" di mode manual | Pompa menyala, mati otomatis maks 15 dtk, event tercatat |
| AC-10 | Pengguna tanpa login mengakses data | Ditolak |
| AC-11 | Unduh CSV | File terbuka di Excel dengan kolom dan waktu yang benar |
| AC-12 | Uji lama (soak test) 7 hari di lokasi | Tanpa restart tak terduga, tanpa pompa menyala di luar pola |

## 14. Metrik Keberhasilan

- Persentase waktu tanah berada dalam rentang lembap yang ditetapkan.
- Jumlah kejadian PENGAMAN dan watchdog per minggu (target mendekati nol setelah kalibrasi).
- Keberhasilan pengiriman telemetri (target ≥ 95% selama uji lama).
- Jumlah restart tak terduga selama uji lama (target 0).

## 15. Risiko dan Mitigasi

| Risiko | Dampak | Mitigasi |
|---|---|---|
| Pompa menyala terus (bug/hang) | Tanaman terendam, pompa rusak | SF-02, watchdog perangkat keras (rencana), pengaman fisik |
| Tangki kosong | Pompa kering rusak | SF-10 sensor level air, deteksi PENGAMAN |
| Restart ESP32 saat pompa menyala | Pengukuran dan kontrol terganggu | Catu daya pompa terpisah, dioda flyback, kapasitor |
| Key atau akun bocor | Orang lain mengontrol pompa | RLS, matikan pendaftaran publik, ganti kredensial, SF-06/SF-07 |
| Nilai kalibrasi basah salah | Penyiraman berlebihan/kurang | Kalibrasi nyata sebelum pemasangan, uji lama |
| Internet atau Supabase gangguan | Data hilang sementara | Logika lokal tetap jalan (NF-03) |
| Batas paket gratis Supabase | Pencatatan terhenti | Pantau penggunaan, interval kirim 60 dtk, kebijakan retensi |
| Sertifikat TLS tidak diverifikasi (`setInsecure`) | Rentan man-in-the-middle | Ganti ke `setCACert()` untuk produksi |

## 16. Milestone

| Fase | Isi | Selesai bila |
|---|---|---|
| 1 | Rangkaian, catu daya pompa terpisah, kalibrasi sensor dengan data nyata | AC-01 sampai AC-06 lolos di meja uji |
| 2 | Supabase: tabel, RLS, akun; firmware kirim telemetri | Data masuk tiap interval (kode 201) |
| 3 | Website: login, tampilan, grafik, kontrol, ekspor | AC-09 sampai AC-11 lolos |
| 4 | Uji lama dan perbaikan | AC-12 lolos |
| 5 | Pemasangan di kebun, dokumentasi | Alat berjalan sesuai metrik |

## 17. Rencana Pengembangan Lanjutan (di luar MVP)

- Watchdog perangkat keras (`esp_task_wdt`) dan OTA update.
- Ambang kalibrasi disimpan di NVS dan dapat diubah dari website.
- NTP agar waktu di alat akurat, serta antrean data saat offline.
- Notifikasi Telegram: alat offline, PENGAMAN aktif, sensor bermasalah, tangki kosong.
- Edge Function untuk autentikasi per perangkat; verifikasi sertifikat TLS.
- Kebijakan retensi dan peringkasan data lama.
- Dukungan beberapa perangkat/kebun.

## 18. Pertanyaan Terbuka

1. Berapa nilai ADC tanah saat lembap pas dan saat terlalu basah? (butuh ukuran nyata)
2. Berapa debit pompa (ml/detik) dan kebutuhan air per siram untuk ukuran pot/polybag yang dipakai?
3. Berapa jumlah tanaman per alat, dan apakah satu sensor mewakili semuanya?
4. Apakah alat dipasang dengan Wi-Fi tetap atau hotspot HP?
5. Siapa saja yang boleh mengakses dashboard?
# Brief Firmware ESP32 untuk Claude

Dokumen ini adalah konteks kerja untuk mengembangkan firmware ESP32 pada proyek
Tomat Smart Farming. Baca sebelum mengubah kode firmware.

## Tujuan

Bangun firmware penyiraman otomatis yang aman, tetap berjalan tanpa internet,
dan berkomunikasi dengan Supabase melalui HTTPS REST. Semua logika keputusan dan
pengaman harus berada di ESP32. Website hanya menampilkan data dan membuat
permintaan melalui tabel `commands`.

## Batasan penting

- Jangan mengubah nama tabel, kolom, nilai enum, endpoint, atau format JSON.
- Jangan menambahkan perintah selain `auto`, `manual`, `pump_on`, `pump_off`.
- Jangan memakai service-role key. ESP32 hanya memakai publishable/anon key.
- Jangan membuat pompa dapat dikendalikan melalui jalur lain selain `commands`.
- Perintah dari server selalu harus melewati pengaman firmware.
- Jangan menganggap jaringan tersedia; penyiraman otomatis harus tetap berjalan
  ketika Wi-Fi atau Supabase gagal.
- Jika ada kebutuhan yang bertentangan dengan kontrak ini, berhenti dan jelaskan
  konflik tersebut sebelum mengubah kode.

## Perangkat dan pin

- ESP32 Dev Module
- DHT22: GPIO18
- Sensor tanah analog: GPIO34
- Relay pompa active-LOW: GPIO27
- Catu daya pompa terpisah, GND disatukan
- Sensor level tangki wajib dipasang sebelum penggunaan permanen

Verifikasi arah sensor tanah sebelum kalibrasi: ADC lebih kecil berarti tanah
lebih basah.

## Kontrak Supabase

Base URL: `https://<project-ref>.supabase.co`

Semua request memakai HTTPS dan header:

```http
apikey: <publishable-or-anon-key>
Content-Type: application/json
```

### 1. Telemetri

Kirim `POST /rest/v1/telemetry` setiap 15 detik saat benchmark atau 60 detik
untuk produksi.

```json
{
  "device_id": "kebun-1",
  "adc": 700,
  "persen": 55,
  "suhu": 28.0,
  "hum": 72.0,
  "pompa": false,
  "mode": "auto",
  "state": "IDLE"
}
```

Kolom wajib: `device_id`, `adc`, `persen`, `suhu`, `hum`, `pompa`, `mode`,
`state`. `suhu` dan `hum` boleh `null`. Respons sukses yang diharapkan: `201`.

Nilai `mode`: `auto` atau `manual`.

Nilai `state`: `IDLE`, `MENYIRAM`, `MERESAP`, atau `PENGAMAN`.

### 2. Event pompa

Kirim satu `POST /rest/v1/pump_events` setelah setiap penyiraman selesai.

```json
{
  "device_id": "kebun-1",
  "durasi_ms": 3000,
  "sumber": "auto"
}
```

Nilai `sumber`: `auto`, `manual`, atau `watchdog`. Respons sukses: `201`.

Jangan membuat event berulang untuk satu sesi penyiraman.

### 3. Membaca command

Ambil satu command setiap sekitar 5 detik:

```http
GET /rest/v1/commands?device_id=eq.kebun-1&done=eq.false&order=id.asc&limit=1&select=id,cmd
```

Respons sukses: `200`, contoh:

```json
[{"id":12,"cmd":"manual"}]
```

Hanya command yang dibuat dalam 2 menit terakhir yang boleh diproses. Command
lama tidak boleh dijalankan ketika alat kembali online.

Command yang valid hanya:

- `auto`: pindah ke mode otomatis.
- `manual`: pindah ke mode manual.
- `pump_on`: nyalakan pompa hanya jika mode manual dan tanah tidak terlalu basah.
- `pump_off`: matikan pompa.

Satu command dijalankan paling banyak sekali berdasarkan `id`.

### 4. Menandai command selesai

Setelah command diproses, kirim:

```http
PATCH /rest/v1/commands?id=eq.<id>
Prefer: return=minimal
```

Body:

```json
{"done":true,"hasil":"mode manual aktif"}
```

Respons sukses: `204`.

Selalu tandai `done=true`, termasuk ketika command ditolak. Isi `hasil` harus
menjelaskan hasil sebenarnya, misalnya `ditolak: tanah terlalu basah`.

## Logika keselamatan wajib

1. Relay harus OFF saat boot, reset, sensor tidak valid, dan kondisi error.
2. Watchdog mematikan pompa jika menyala lebih dari 20 detik, apa pun state-nya.
3. Pompa dilarang menyala jika tanah terlalu basah, termasuk command manual.
4. Pompa manual otomatis OFF maksimal setelah 15 detik.
5. Beri jeda minimal 1 detik antara OFF dan ON berikutnya.
6. Abaikan pembacaan sensor selama 400 ms setelah relay berpindah.
7. Saat pompa menyala, tunda pekerjaan jaringan yang dapat mengganggu timing.
8. Sensor level tangki harus mencegah pompa menyala saat air habis.
9. Kegagalan Wi-Fi/HTTPS tidak boleh menghentikan state machine otomatis.

## State machine penyiraman

Gunakan alur:

```text
IDLE -> MENYIRAM -> MERESAP -> IDLE
                         \-> MENYIRAM (jika masih kering)
                         \-> PENGAMAN (jika pulsa maksimum tercapai)
PENGAMAN -> IDLE setelah jeda pengaman
```

Parameter awal yang harus diletakkan dalam satu blok konfigurasi:

```cpp
BATAS_KERING  = 850;  // mulai siram, ADC >= nilai ini
TARGET_LEMBAP = 600;  // berhenti setelah ADC <= nilai ini
BATAS_BASAH   = 400;  // dilarang menyiram jika ADC <= nilai ini
ADC_KERING    = 950;
ADC_BASAH     = 300;
DURASI_SIRAM  = 3000; // benchmark; produksi dapat 6000 ms
WAKTU_RESAP   = 10000; // benchmark; produksi dapat 90000 ms
MAKS_PULSA    = 3;
JEDA_PENGAMAN = 30000; // benchmark; produksi dapat 1800000 ms
```

Nilai di atas adalah titik awal, bukan pengganti kalibrasi fisik. Jangan membuat
website mengubah nilai ini karena kontrak firmware saat ini belum menyediakan
command konfigurasi.

## Pembacaan sensor

- Sensor tanah dibaca tiap 1 detik dengan filter median 11 sampel.
- DHT22 dibaca tiap 2,5 detik.
- ADC di luar rentang valid dianggap sensor bermasalah.
- Saat sensor bermasalah: pompa OFF, state aman, dan alasan dicatat di log.
- Konversi `persen` harus konsisten dengan `ADC_KERING` dan `ADC_BASAH`, lalu
  dibatasi pada rentang 0–100.

## Perilaku jaringan

- Sambungkan ulang Wi-Fi secara otomatis tanpa memblokir state machine terlalu
  lama.
- Gunakan timeout HTTPS yang wajar dan log kode HTTP/error.
- Validasi status response: telemetry/event `201`, command GET `200`, PATCH
  `204`.
- Jangan mengeksekusi ulang command jika request timeout lalu hasilnya tidak
  diketahui; gunakan `id` dan status database sebagai sumber rekonsiliasi.
- Pastikan waktu perangkat valid sebelum menerapkan batas umur 2 menit. Jika
  waktu belum valid, jangan menjalankan command server.

## Kriteria penerimaan firmware

- Tanah kering pada mode auto: pompa menyala sesuai durasi, menunggu resap,
  lalu membaca ulang.
- Target tercapai: pompa berhenti dan hitungan pulsa kembali nol.
- Pulsa maksimum tercapai: state `PENGAMAN`, pompa OFF.
- Sensor dicabut/tidak valid: pompa OFF dan alasan terlihat di telemetry/log.
- Pompa dipaksa lebih dari 20 detik: watchdog mematikannya dan event bersumber
  `watchdog` tercatat.
- `pump_on` pada mode selain manual ditolak dan tetap ditandai selesai.
- `pump_on` ketika tanah terlalu basah ditolak dengan pesan jelas.
- Pompa manual mati otomatis maksimal 15 detik.
- Command lebih tua dari 2 menit tidak pernah dijalankan.
- Wi-Fi/Supabase mati: otomatisasi lokal tetap berjalan.
- Setelah jaringan pulih: telemetri kembali terkirim tanpa mengubah state
  keselamatan.

## Format jawaban Claude saat mengubah firmware

Sebelum memberi patch, Claude harus melaporkan:

1. File firmware yang akan diubah.
2. Risiko keselamatan dan cara mitigasinya.
3. Kontrak endpoint/kolom yang tetap dipertahankan.
4. Cara menguji perubahan di meja uji tanpa pompa terpasang.

Setelah patch, laporkan pengujian yang benar-benar dilakukan. Jangan mengklaim
alat fisik sudah teruji jika hanya melakukan pemeriksaan kompilasi atau review
kode.


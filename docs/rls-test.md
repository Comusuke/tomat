# Panduan uji RLS Supabase

Panduan ini menguji skema pada `supabase/schema.sql` melalui Data API.
Gunakan **publishable/anon key**, bukan service role key. Semua contoh memakai
`device_id=rls-test`; firmware hanya mencari `kebun-1`, sehingga data uji ini
tidak akan diproses perangkat.

Ganti placeholder berikut di terminal lokal, jangan simpan nilainya di repo:

- `<SUPABASE_URL>`: contoh `https://xxxx.supabase.co`
- `<ANON_KEY>`: publishable atau anon key
- `<USER_EMAIL>` dan `<USER_PASSWORD>`: akun yang sudah dibuat pemilik
- `<USER_JWT>`: access token dari langkah login

Gunakan `curl.exe` pada PowerShell agar tidak memakai alias `curl`.

## Persiapan data commands

Di Supabase SQL Editor, jalankan dua perintah ini. Catat kedua `id` hasilnya.
Baris pertama harus diuji dalam dua menit; jalankan ulang bila sudah kedaluwarsa.

```sql
insert into public.commands (device_id, cmd, done)
values ('rls-test', 'auto', false)
returning id;

insert into public.commands (device_id, created_at, cmd, done)
values ('rls-test', now() - interval '3 minutes', 'manual', false)
returning id;
```

Gunakan hasilnya sebagai `<COMMAND_BARU_ID>` dan `<COMMAND_LAMA_ID>`.

## 1. Anon dapat memasukkan telemetry

```powershell
curl.exe -i -X POST "<SUPABASE_URL>/rest/v1/telemetry" -H "apikey: <ANON_KEY>" -H "Content-Type: application/json" -H "Prefer: return=minimal" -d '{"device_id":"rls-test","adc":700,"persen":50,"suhu":25.5,"hum":70.0,"pompa":false,"mode":"auto","state":"IDLE"}'
```

Hasil diharapkan: `HTTP/2 201 Created`.

## 2. Anon tidak dapat membaca telemetry

```powershell
curl.exe -i "<SUPABASE_URL>/rest/v1/telemetry?select=*" -H "apikey: <ANON_KEY>"
```

Hasil diharapkan pada skema ini: `HTTP/2 403 Forbidden` atau pesan
`permission denied`. Respons `[]` juga hanya dapat diterima bila konfigurasi
proyek memang tidak memberi hak baca anon.

## 3. Anon tidak dapat memasukkan commands

```powershell
curl.exe -i -X POST "<SUPABASE_URL>/rest/v1/commands" -H "apikey: <ANON_KEY>" -H "Content-Type: application/json" -d '{"device_id":"rls-test","cmd":"auto","done":false}'
```

Hasil diharapkan: `HTTP/2 403 Forbidden` atau pesan `permission denied`.

## 4. Anon membaca kolom minimum command baru untuk firmware

```powershell
curl.exe -i "<SUPABASE_URL>/rest/v1/commands?device_id=eq.rls-test&done=eq.false&order=id.asc&limit=1&select=id,cmd,created_at" -H "apikey: <ANON_KEY>"
```

Hasil diharapkan: `HTTP/2 200 OK` berisi `<COMMAND_BARU_ID>` dengan kolom
`id`, `cmd`, dan `created_at`. `<COMMAND_LAMA_ID>` tidak boleh muncul. Setelah
dua menit, respons boleh `[]`.

Policy SELECT anon tidak membatasi `done=false`, karena PostgREST memerlukan
policy SELECT pada baris hasil PATCH. Filter `done=false` tetap ada pada kueri
firmware di atas.

## 5. Anon tidak dapat mengubah kolom selain done dan hasil

```powershell
curl.exe -i -X PATCH "<SUPABASE_URL>/rest/v1/commands?id=eq.<COMMAND_BARU_ID>" -H "apikey: <ANON_KEY>" -H "Content-Type: application/json" -d '{"cmd":"pump_on"}'
```

Hasil diharapkan: `HTTP/2 403 Forbidden` atau pesan `permission denied`.
Nilai `cmd` harus tetap `auto`.

## 6. Anon dapat menandai command baru selesai

```powershell
curl.exe -i -X PATCH "<SUPABASE_URL>/rest/v1/commands?id=eq.<COMMAND_BARU_ID>" -H "apikey: <ANON_KEY>" -H "Content-Type: application/json" -H "Prefer: return=minimal" -d '{"done":true,"hasil":"uji"}'
```

Hasil diharapkan: `HTTP/2 204 No Content`. Dari SQL Editor, verifikasi hasilnya:

```sql
select id, done, hasil
from public.commands
where id = <COMMAND_BARU_ID>;
```

Nilai yang diharapkan: `done=true` dan `hasil='uji'`.

## 7. Anon tidak dapat menyelesaikan commands yang lebih tua dari dua menit

```powershell
curl.exe -i -X PATCH "<SUPABASE_URL>/rest/v1/commands?id=eq.<COMMAND_LAMA_ID>" -H "apikey: <ANON_KEY>" -H "Content-Type: application/json" -H "Prefer: return=representation" -d '{"done":true,"hasil":"uji"}'
```

Hasil diharapkan: tidak ada baris yang berubah. PostgREST dapat mengembalikan
`204 No Content` atau `200 OK` dengan `[]`, karena RLS menyembunyikan baris
yang tidak memenuhi policy. Periksa dari SQL Editor bahwa `done` tetap `false`:

```sql
select id, done, hasil
from public.commands
where id = <COMMAND_LAMA_ID>;
```

## 8. Signup akun baru ditolak

Sebelum menguji, buka **Authentication → Settings** dan pastikan **Allow new
users to sign up** nonaktif. Lalu jalankan:

```powershell
curl.exe -i -X POST "<SUPABASE_URL>/auth/v1/signup" -H "apikey: <ANON_KEY>" -H "Content-Type: application/json" -d '{"email":"<TEST_EMAIL_BARU>","password":"<PASSWORD_UJI>"}'
```

Hasil diharapkan: respons gagal, biasanya `400 Bad Request` dengan pesan
signup tidak diizinkan. Jika respons berhasil atau akun baru muncul, signup
publik masih aktif dan harus dinonaktifkan di pengaturan Auth.

## 9. Pengguna login dapat membaca telemetry dan membuat commands baru

Login dengan akun yang sudah ada:

```powershell
curl.exe -i -X POST "<SUPABASE_URL>/auth/v1/token?grant_type=password" -H "apikey: <ANON_KEY>" -H "Content-Type: application/json" -d '{"email":"<USER_EMAIL>","password":"<USER_PASSWORD>"}'
```

Hasil diharapkan: `HTTP/2 200 OK`. Salin `access_token` dari respons menjadi
`<USER_JWT>` untuk tiga perintah berikut.

Baca telemetry:

```powershell
curl.exe -i "<SUPABASE_URL>/rest/v1/telemetry?select=*&limit=1" -H "apikey: <ANON_KEY>" -H "Authorization: Bearer <USER_JWT>"
```

Hasil diharapkan: `HTTP/2 200 OK`, berisi array data atau `[]` bila belum ada data.

Buat command yang belum selesai:

```powershell
curl.exe -i -X POST "<SUPABASE_URL>/rest/v1/commands" -H "apikey: <ANON_KEY>" -H "Authorization: Bearer <USER_JWT>" -H "Content-Type: application/json" -H "Prefer: return=representation" -d '{"device_id":"rls-test","cmd":"auto","done":false}'
```

Hasil diharapkan: `HTTP/2 201 Created`.

Upaya membuat command dengan `done=true` harus ditolak:

```powershell
curl.exe -i -X POST "<SUPABASE_URL>/rest/v1/commands" -H "apikey: <ANON_KEY>" -H "Authorization: Bearer <USER_JWT>" -H "Content-Type: application/json" -d '{"device_id":"rls-test","cmd":"auto","done":true}'
```

Hasil diharapkan: `HTTP/2 403 Forbidden` atau pesan pelanggaran RLS.

## Catatan hasil

Simpan hanya kode HTTP dan pesan galat bila pengujian gagal. Jangan menyimpan
`<ANON_KEY>`, password, atau `<USER_JWT>` pada dokumen, terminal history yang
dibagikan, atau commit.

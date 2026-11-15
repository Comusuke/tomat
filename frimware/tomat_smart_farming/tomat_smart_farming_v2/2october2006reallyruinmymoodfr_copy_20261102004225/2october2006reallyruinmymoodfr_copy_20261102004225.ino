// =====================================================
// TOMAT SMART FARMING - ESP32 + Firebase Realtime Database
// Relay Active HIGH
// v2 : diagnosis Wi-Fi, sambung ulang otomatis, sensor median
// =====================================================

#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <time.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "secrets.h"

#define DHTPIN 18
#define DHTTYPE DHT22
#define MOISTURE_PIN 34
#define RELAY_PIN 27

#define POMPA_ON HIGH
#define POMPA_OFF LOW

const int ADC_BATAS_KERING = 1800;
const int DURASI_SIRAM_DETIK = 5;
const int JEDA_RESAP_DETIK = 10;
const int JEDA_CEK_RUTIN_DETIK = 15;

const int ADC_KERING = 2000;
const int ADC_BASAH = 900;

const unsigned long JEDA_COBA_WIFI_MS = 20000;
const unsigned long TUNGGU_WIFI_AWAL_MS = 20000;

DHT dht(DHTPIN, DHTTYPE);

String firebaseIdToken = "";

// ========== WAKTU ==========
unsigned long long waktuUnixMs() {
  time_t sekarang = time(nullptr);
  if (sekarang > 1700000000) {
    return static_cast<unsigned long long>(sekarang) * 1000ULL;
  }

  return static_cast<unsigned long long>(millis());
}

// ========== SENSOR ==========
// Median 11 sampel: meredam noise ADC ESP32 (terutama saat radio Wi-Fi aktif)
int bacaTanahMedian() {
  int s[11];
  for (int i = 0; i < 11; i++) {
    s[i] = analogRead(MOISTURE_PIN);
    delayMicroseconds(500);
  }
  for (int i = 1; i < 11; i++) {
    int k = s[i], j = i - 1;
    while (j >= 0 && s[j] > k) {
      s[j + 1] = s[j];
      j--;
    }
    s[j + 1] = k;
  }
  return s[5];
}

int hitungPersenTanah(int adc) {
  int persen = map(adc, ADC_KERING, ADC_BASAH, 0, 100);
  return constrain(persen, 0, 100);
}

// ========== WI-FI ==========
void cetakDaftarJaringan() {
  Serial.println("[WIFI] memindai jaringan 2.4GHz...");
  int n = WiFi.scanNetworks();
  Serial.printf("[WIFI] jaringan terlihat: %d\n", n);

  bool ditemukan = false;
  for (int i = 0; i < n; i++) {
    bool cocok = (WiFi.SSID(i) == String(WIFI_SSID));
    if (cocok) ditemukan = true;
    Serial.printf("   %s%s (%d dBm)\n",
                  WiFi.SSID(i).c_str(),
                  cocok ? "  <-- SSID kamu" : "",
                  WiFi.RSSI(i));
  }

  if (!ditemukan) {
    Serial.println("[WIFI] SSID kamu TIDAK terlihat. Kemungkinan 5GHz, terlalu jauh, atau nama salah.");
  }
  WiFi.scanDelete();
}

const char* namaStatusWiFi(int st) {
  switch (st) {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "SSID TIDAK DITEMUKAN";
    case WL_SCAN_COMPLETED:  return "SCAN SELESAI";
    case WL_CONNECTED:       return "TERHUBUNG";
    case WL_CONNECT_FAILED:  return "GAGAL (password salah?)";
    case WL_CONNECTION_LOST: return "KONEKSI HILANG";
    case WL_DISCONNECTED:    return "TERPUTUS";
    default:                 return "?";
  }
}

// ========== FIREBASE ==========
bool loginFirebaseAnonymous() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String url =
    "https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" +
    String(FIREBASE_API_KEY);

  if (!http.begin(client, url)) {
    Serial.println("[FIREBASE AUTH] Gagal membuka koneksi");
    return false;
  }

  http.addHeader("Content-Type", "application/json");

  int code = http.POST("{\"returnSecureToken\":true}");

  if (code != 200) {
    Serial.printf("[FIREBASE AUTH] Gagal. HTTP: %d\n", code);
    Serial.println(http.getString());
    http.end();
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error) {
    Serial.println("[FIREBASE AUTH] Respons JSON tidak valid");
    http.end();
    return false;
  }

  firebaseIdToken = doc["idToken"].as<String>();

  http.end();

  if (firebaseIdToken.length() == 0) {
    Serial.println("[FIREBASE AUTH] Token kosong");
    return false;
  }

  Serial.println("[FIREBASE AUTH] Anonymous login berhasil");
  return true;
}

bool kirimTelemetriSekali(
  int adc,
  float suhu,
  float hum,
  bool statusPompa
) {
  if (firebaseIdToken.length() == 0) {
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String recordId = String(millis());

  String url =
    String(FIREBASE_DATABASE_URL) +
    "/telemetry/" +
    String(DEVICE_ID) +
    "/" +
    recordId +
    ".json?auth=" +
    firebaseIdToken;

  if (!http.begin(client, url)) {
    Serial.println("[FIREBASE] Gagal membuka koneksi database");
    return false;
  }

  http.addHeader("Content-Type", "application/json");

  String sSuhu = isnan(suhu) ? "null" : String(suhu, 1);
  String sHum = isnan(hum) ? "null" : String(hum, 1);

  String body =
    "{"
      "\"device_id\":\"" + String(DEVICE_ID) + "\","
      "\"created_at\":" + String(waktuUnixMs()) + ","
      "\"adc\":" + String(adc) + ","
      "\"persen\":" + String(hitungPersenTanah(adc)) + ","
      "\"suhu\":" + sSuhu + ","
      "\"hum\":" + sHum + ","
      "\"pompa\":" + String(statusPompa ? "true" : "false") + ","
      "\"mode\":\"auto\","
      "\"state\":\"IDLE\""
    "}";

  int code = http.PUT(body);

  Serial.printf("[FIREBASE] Kirim telemetry. HTTP: %d\n", code);

  if (code != 200) {
    Serial.println(http.getString());
  }

  http.end();

  return code == 200;
}

void kirimTelemetri(
  int adc,
  float suhu,
  float hum,
  bool statusPompa
) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[FIREBASE] WiFi terputus, data tidak dikirim");
    return;
  }

  if (firebaseIdToken.length() == 0) {
    if (!loginFirebaseAnonymous()) {
      return;
    }
  }

  bool berhasil = kirimTelemetriSekali(
    adc,
    suhu,
    hum,
    statusPompa
  );

  if (!berhasil) {
    Serial.println("[FIREBASE] Token mungkin kedaluwarsa, login ulang");

    firebaseIdToken = "";

    if (loginFirebaseAnonymous()) {
      kirimTelemetriSekali(
        adc,
        suhu,
        hum,
        statusPompa
      );
    }
  }
}

// Dipanggil tiap putaran loop: sambung ulang Wi-Fi bila putus dan
// minta sinkronisasi NTP satu kali setelah Wi-Fi tersambung.
void jagaKoneksi() {
  static unsigned long terakhirCoba = 0;
  static bool ntpDiminta = false;
  static bool pernahTerhubung = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!pernahTerhubung) {
      pernahTerhubung = true;
      Serial.println("[WIFI] Terhubung. IP: " + WiFi.localIP().toString());
    }
    if (!ntpDiminta) {
      ntpDiminta = true;
      configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
      Serial.println("[WAKTU] sinkronisasi NTP diminta");
    }
    return;
  }

  if (pernahTerhubung) {
    pernahTerhubung = false;
    Serial.println("[WIFI] Terputus.");
  }

  if (millis() - terakhirCoba > JEDA_COBA_WIFI_MS) {
    terakhirCoba = millis();
    Serial.printf("[WIFI] mencoba sambung ulang... status: %s\n",
                  namaStatusWiFi(WiFi.status()));
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}

// ========== SETUP ==========
void setup() {
  // Deteksi brownout sengaja DIAKTIFKAN agar masalah daya terlihat.
  // Kalau alat sering reset sendiri, perbaiki suplai daya (kabel/adaptor 5V 1A+).
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(500);

  digitalWrite(RELAY_PIN, POMPA_OFF);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, POMPA_OFF);

  analogReadResolution(12);
  analogSetPinAttenuation(MOISTURE_PIN, ADC_11db);
  dht.begin();

  Serial.println();
  Serial.printf("[WIFI] SSID  : [%s] (%d karakter)\n", WIFI_SSID, (int)strlen(WIFI_SSID));
  Serial.printf("[WIFI] PASS  : %d karakter\n", (int)strlen(WIFI_PASS));

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  cetakDaftarJaringan();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("[WIFI] Menghubungkan (angka = kode status)");

  unsigned long startWiFi = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startWiFi < TUNGGU_WIFI_AWAL_MS
  ) {
    delay(500);
    Serial.printf(" [%d]", WiFi.status());
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WIFI] Terhubung");
    Serial.println("[WIFI] IP: " + WiFi.localIP().toString());

    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("[WAKTU] Sinkronisasi NTP");
    time_t sekarang = time(nullptr);
    unsigned long mulaiNtp = millis();

    while (sekarang < 1700000000 && millis() - mulaiNtp < 10000) {
      delay(250);
      Serial.print(".");
      sekarang = time(nullptr);
    }

    Serial.println();

    if (sekarang >= 1700000000) {
      Serial.println("[WAKTU] NTP berhasil");
    } else {
      Serial.println("[WAKTU] NTP gagal, timestamp sementara tidak akurat");
    }

    loginFirebaseAnonymous();
  } else {
    Serial.printf("[WIFI] Gagal terhubung. Status: %s\n",
                  namaStatusWiFi(WiFi.status()));
    Serial.println("[WIFI] Alat tetap berjalan otomatis; akan mencoba sambung ulang.");
  }
}

// ========== LOOP ==========
void loop() {
  jagaKoneksi();

  int adcTanah = bacaTanahMedian();
  float suhu = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.printf(
    "\n[SENSOR] ADC: %d | Suhu: %.1f C | RH: %.1f%% | WiFi: %s\n",
    adcTanah,
    suhu,
    hum,
    namaStatusWiFi(WiFi.status())
  );

  if (adcTanah >= ADC_BATAS_KERING) {
    Serial.printf(
      "[AKSI] Tanah kering. Pompa ON selama %d detik\n",
      DURASI_SIRAM_DETIK
    );

    kirimTelemetri(
      adcTanah,
      suhu,
      hum,
      true
    );

    digitalWrite(RELAY_PIN, POMPA_ON);
    delay(DURASI_SIRAM_DETIK * 1000);

    digitalWrite(RELAY_PIN, POMPA_OFF);

    Serial.printf(
      "[AKSI] Pompa OFF. Menunggu resap %d detik\n",
      JEDA_RESAP_DETIK
    );

    kirimTelemetri(
      adcTanah,
      suhu,
      hum,
      false
    );

    delay(JEDA_RESAP_DETIK * 1000);
  } else {
    Serial.println(
      "[AKSI] Tanah lembap. Pompa OFF dan standby"
    );

    kirimTelemetri(
      adcTanah,
      suhu,
      hum,
      false
    );

    delay(JEDA_CEK_RUTIN_DETIK * 1000);
  }
}
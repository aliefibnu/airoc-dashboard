# Dokumentasi Contoh Kode Robot Leanbot (AIROC 2026)

Repositori ini berisi kumpulan kode contoh, modul pembelajaran, dan misi robot **Leanbot** untuk persiapan kompetisi AIROC 2026. Seluruh folder dan file telah direstrukturisasi dan dinamai secara deskriptif sesuai fungsionalitas kode di dalamnya.

---

## 📁 Struktur Direktori

```text
Examples-master/
├── 01_Arduino_Basics/                          # Dasar pemrograman C++ & Arduino
├── 02_Buzzer/                                  # Kontrol audio & buzzer Leanbot
├── 03_RGB_LED/                                 # Kontrol pencahayaan RGB LED Neopixel
├── 04_Touch_Sensor/                            # Sensor sentuh kapasitif (TB1A, TB1B, TB2A, TB2B)
│   ├── 01_Logic_Conditionals/                  # Struktur logika percabangan (if, else, AND, OR)
│   └── 02_Sensor_Features_and_Control/         # Fitur pembacaan sensor & kontrol robot/buzzer
├── 05_Motion/                                  # Kontrol gerak motor stepper & odometri
├── 06_Gripper/                                 # Kontrol servo capit/gripper
├── 07_Line_Sensor/                             # Sensor garis inframerah (4-channel IR Line)
├── 08_Integrated_Projects/                     # Proyek terintegrasi & sistem menu
│   └── Legacy_Backup/                          # Cadangan kode lama
└── 09_Missions_and_DigitalTwin/                # Misi tantangan & simulasi Digital Twin
```

---

## 📖 Rincian Isi Setiap Modul & File Kode

### 1. `01_Arduino_Basics/` (Dasar Pemrograman Arduino)
Modul ini mencakup fondasi pemrograman mikrokontroler menggunakan komunikasi serial dan I/O digital dasar:
- [`01_LED_Blink.ino`](01_Arduino_Basics/01_LED_Blink.ino): Program kedip (blink) LED bawaan pin 13 dengan interval 1 detik.
- [`02_Serial_Add_Numbers.ino`](01_Arduino_Basics/02_Serial_Add_Numbers.ino): Pembuatan fungsi `addNumbers(a, b)` dan mencetak hasil penjumlahan ke Serial Monitor (9600 baud).
- [`03_Serial_Calculate_Rectangle_Area.ino`](01_Arduino_Basics/03_Serial_Calculate_Rectangle_Area.ino): Input interaktif Serial untuk menerima panjang dan lebar, menghitung luas persegi panjang, lalu menampilkan hasilnya.
- [`04_Serial_Traffic_Light_Control.ino`](01_Arduino_Basics/04_Serial_Traffic_Light_Control.ino): Simulasi lampu lalu lintas via input Serial (1 = Merah, 2 = Kuning, 3 = Hijau) dengan bunyi buzzer dan lampu RGB.

---

### 2. `02_Buzzer/` (Kontrol Audio & Nada)
Modul untuk mempelajari berbagai cara membunyikan buzzer bawaan Leanbot (`LBPIN_BUZZER` / `Leanbot.tone`):
- [`01_Buzzer_DigitalWrite_Delay.ino`](02_Buzzer/01_Buzzer_DigitalWrite_Delay.ino): Menghasilkan frekuensi suara secara manual via `digitalWrite(HIGH/LOW)` dan `delay(10)`.
- [`02_Buzzer_Continuous_Tone.ino`](02_Buzzer/02_Buzzer_Continuous_Tone.ino): Membunyikan nada kontinu frekuensi 500 Hz menggunakan `Leanbot.tone(500)`.
- [`03_Buzzer_Alternating_Tones.ino`](02_Buzzer/03_Buzzer_Alternating_Tones.ino): Bergantian antara nada 500 Hz dan 100 Hz dengan jeda 1 detik secara berulang.
- [`04_Buzzer_Tone_And_NoTone.ino`](02_Buzzer/04_Buzzer_Tone_And_NoTone.ino): Membunyikan nada 500 Hz selama 1 detik kemudian mematikannya menggunakan `Leanbot.noTone()`.
- [`05_Buzzer_Tone_With_Duration.ino`](02_Buzzer/05_Buzzer_Tone_With_Duration.ino): Membunyikan nada frekuensi 500 Hz dengan durasi otomatis 1000 ms (`Leanbot.tone(500, 1000)`).

---

### 3. `03_RGB_LED/` (Indikator Lampu RGB)
Modul untuk mengendalikan susunan LED RGB WS2812B pada Leanbot:
- [`01_RGB_Center_LED_Blink.ino`](03_RGB_LED/01_RGB_Center_LED_Blink.ino): Mengedipkan LED tengah (`ledO`) dengan warna Merah dan mati secara bergantian.
- [`02_RGB_Sequential_Blue_For_Loop.ino`](03_RGB_LED/02_RGB_Sequential_Blue_For_Loop.ino): Menyalakan ke-7 LED satu per satu secara berurutan berwarna Biru menggunakan perulangan `for`.
- [`03_RGB_Fill_All_Colors.ino`](03_RGB_LED/03_RGB_Fill_All_Colors.ino): Mengisi seluruh LED sekaligus (`fillColor`) dengan warna Hijau lalu berganti ke Biru.
- [`04_RGB_Brightness_Fade.ino`](03_RGB_LED/04_RGB_Brightness_Fade.ino): Efek memudarkan (fade) kecerahan lampu putih dari tingkat 0 hingga 50 menggunakan `setBrightness()`.
- [`05_RGB_Bitmap_Patterns.ino`](03_RGB_LED/05_RGB_Bitmap_Patterns.ino): Menampilkan pola bitmap kustom pada susunan LED: senyum (smiley), cemberut (frowning), panah kiri, dan panah kanan.

---

### 4. `04_Touch_Sensor/` (Sensor Sentuh Kapasitif)
Leanbot memiliki 4 tombol sentuh: `TB1A` (kiri depan), `TB1B` (kanan depan), `TB2A` (kiri belakang), `TB2B` (kanan belakang). Modul ini terbagi menjadi dua subkategori:

#### `01_Logic_Conditionals/` (Logika Pemrograman dengan Tombol Sentuh)
- [`01_Touch_Logic_If.ino`](04_Touch_Sensor/01_Logic_Conditionals/01_Touch_Logic_If.ino): Percabangan tunggal `if` — jika TB1A disentuh, motor bergerak.
- [`02_Touch_Logic_Nested_If_Without_AND.ino`](04_Touch_Sensor/01_Logic_Conditionals/02_Touch_Logic_Nested_If_Without_AND.ino): Percabangan bertingkat (nested if) tanpa operator logika `&&`.
- [`03_Touch_Logic_Separate_If_Without_OR.ino`](04_Touch_Sensor/01_Logic_Conditionals/03_Touch_Logic_Separate_If_Without_OR.ino): Dua blok `if` terpisah untuk mendeteksi TB1A atau TB1B tanpa operator `||`.
- [`04_Touch_Logic_If_Else.ino`](04_Touch_Sensor/01_Logic_Conditionals/04_Touch_Logic_If_Else.ino): Percabangan `if ... else` — jalan jika disentuh, berhenti jika dilepas.
- [`05_Touch_Logic_With_AND_Operator.ino`](04_Touch_Sensor/01_Logic_Conditionals/05_Touch_Logic_With_AND_Operator.ino): Kondisi gabungan dengan operator `&&` (kedua tombol disentuh bersamaan).
- [`06_Touch_Logic_With_OR_Operator.ino`](04_Touch_Sensor/01_Logic_Conditionals/06_Touch_Logic_With_OR_Operator.ino): Kondisi gabungan dengan operator `||` (salah satu tombol disentuh).
- [`07_Touch_Logic_If_ElseIf_Else.ino`](04_Touch_Sensor/01_Logic_Conditionals/07_Touch_Logic_If_ElseIf_Else.ino): Percabangan bertingkat multi-kondisi (`if ... else if ... else`).

#### `02_Sensor_Features_and_Control/` (Fitur Sensor & Kontrol Interaktif)
- [`01_Touch_Read_Control_Builtin_LED.ino`](04_Touch_Sensor/02_Sensor_Features_and_Control/01_Touch_Read_Control_Builtin_LED.ino): Membaca nilai digital TB1A dan menyalakan LED bawaan board secara langsung.
- [`02_Touch_Robot_Direction_Control.ino`](04_Touch_Sensor/02_Sensor_Features_and_Control/02_Touch_Robot_Direction_Control.ino): Kendali arah gerak robot dengan 4 tombol (TB1A belok kiri, TB1B belok kanan, TB2A maju, TB2B mundur).
- [`03_Touch_ReadBits_Serial_Display.ino`](04_Touch_Sensor/02_Sensor_Features_and_Control/03_Touch_ReadBits_Serial_Display.ino): Membaca status 4 tombol sentuh sekaligus dalam format bit (`readBits()`) dan menampilkannya sebagai `.` dan `#` di Serial.
- [`04_Touch_OnPress_Event_Detection.ino`](04_Touch_Sensor/02_Sensor_Features_and_Control/04_Touch_OnPress_Event_Detection.ino): Deteksi event klik (edge-triggered) menggunakan `LbTouch.onPress()` untuk setiap tombol.
- [`05_Touch_Dynamic_Buzzer_Pitch_Control.ino`](04_Touch_Sensor/02_Sensor_Features_and_Control/05_Touch_Dynamic_Buzzer_Pitch_Control.ino): Modulasi frekuensi dan delay nada buzzer secara dinamis melalui sentuhan tombol TB1A dan TB1B.

---

### 5. `05_Motion/` (Gerak Motor Stepper & Odometri)
Leanbot menggunakan motor stepper dengan kontrol presisi tinggi per langkah (steps) dan satuan milimeter/derajat:
- [`01_Motion_Basic_Movements_Demo.ino`](05_Motion/01_Motion_Basic_Movements_Demo.ino): Berbagai variasi manuver gerak (maju lurus, mundur, belok kurva kiri, putar di tempat, dan putar pivot).
- [`02_Motion_Run_Distance_Mm.ino`](05_Motion/02_Motion_Run_Distance_Mm.ino): Bergerak maju sejauh +100 mm, berhenti halus (`stopAndWait()`), lalu mundur -100 mm.
- [`03_Motion_Rotation_Encoder_Steps.ino`](05_Motion/03_Motion_Rotation_Encoder_Steps.ino): Rotasi 90 derajat sebanyak 4 kali ke kanan dan 4 kali ke kiri berbasis hitungan langkah encoder (1600 steps).
- [`04_Motion_Rotation_In_Degrees.ino`](05_Motion/04_Motion_Rotation_In_Degrees.ino): Rotasi presisi menggunakan API derajat langsung (`runRotationDeg(+90)` dan `-90`).
- [`05_Motion_WaitUntil_Distance_Checkpoints.ino`](05_Motion/05_Motion_WaitUntil_Distance_Checkpoints.ino): Gerak lurus 100 mm dengan checkpoint setiap 25 mm menggunakan `waitUntilDistanceMm()` disertai bunyi beep konfirmasi.
- [`06_Motion_WaitUntil_Rotation_Steps.ino`](05_Motion/06_Motion_WaitUntil_Rotation_Steps.ino): Berputar 360 derajat terbagi dalam 4 kuadran rotasi encoder dengan bunyi beep per 90 derajat.
- [`07_Motion_WaitUntil_Rotation_Degrees.ino`](05_Motion/07_Motion_WaitUntil_Rotation_Degrees.ino): Berputar 360 derajat terbagi dalam kelipatan sudut derajat (`waitUntilRotationDeg()`) dengan bunyi beep.

---

### 6. `06_Gripper/` (Kontrol Servo Capit)
Modul untuk mengoperasikan mekanisme gripper/capit Leanbot:
- [`01_Gripper_Basic_Open_Close.ino`](06_Gripper/01_Gripper_Basic_Open_Close.ino): Gerakan standar membuka (`open()`) dan menutup (`close()`) capit.
- [`02_Gripper_Shake_And_Custom_Angle.ino`](06_Gripper/02_Gripper_Shake_And_Custom_Angle.ino): Aksi lanjutan gripper mencakup getaran (`shake()`) dan penutupan dengan sudut derajat kustom (`close(angle)`).

---

### 7. `07_Line_Sensor/` (Sensor Garis Inframerah)
Modul untuk bar sensor inframerah 4-channel (`LbIRLine`):
- [`01_Line_Sensor_Read_State_Serial.ino`](07_Line_Sensor/01_Line_Sensor_Read_State_Serial.ino): Membaca nilai biner dari 4 sensor garis pada threshold 50 dan mencetaknya ke Serial Monitor.
- [`02_Line_Following_Bitmask_Logic.ino`](07_Line_Sensor/02_Line_Following_Bitmask_Logic.ino): Algoritma pengikut garis (line follower) berbasis pola bit biner (`0b0110`, `0b0010`, dll.) untuk menyesuaikan kecepatan motor kiri dan kanan.
- [`03_Line_Sensor_Calibration_With_RGB.ino`](07_Line_Sensor/03_Line_Sensor_Calibration_With_RGB.ino): Kalibrasi manual ambang batas hitam/putih dengan tombol TB1A dan visualisasi status sensor langsung pada lampu LED RGB.
- [`04_Line_Following_With_Calibration_And_Recovery.ino`](07_Line_Sensor/04_Line_Following_With_Calibration_And_Recovery.ino): Sistem pengikut garis otonom lengkap dengan proses kalibrasi awal dan logika pemulihan (recovery) ketika garis sempat hilang.

---

### 8. `08_Integrated_Projects/` (Proyek & Demo Lengkap Terintegrasi)
Proyek terintegrasi yang menggabungkan banyak sensor, aktuator, komunikasi, dan audio:
- [`01_Integrated_LineFollower_Obstacle_Melody.ino`](08_Integrated_Projects/01_Integrated_LineFollower_Obstacle_Melody.ino): Robot line follower cerdas dengan kalibrasi otomatis, deteksi rintangan ultrasonik, dan memainkan melodi tema Super Mario saat misi selesai.
- [`02_Interactive_Multi_Demo_Menu.ino`](08_Integrated_Projects/02_Interactive_Multi_Demo_Menu.ino): Sistem menu interaktif di Serial Monitor dan tombol sentuh (TB2B untuk memilih, TB2A untuk konfirmasi):
  - [1] Demo Motor
  - [2] Demo Ping Ultrasonik
  - [3] Demo Line Following
- [`03_Bluetooth_Serial_Remote_Control.ino`](08_Integrated_Projects/03_Bluetooth_Serial_Remote_Control.ino): Kendali jarak jauh (RC) penuh melalui Bluetooth/Serial dengan dukungan perintah maju, mundur, belok, kecepatan 0–9, klakson, lampu depan LED, dan buka/tutup gripper.
- [`04_DualMode_UltrasonicTracking_And_TouchControl.ino`](08_Integrated_Projects/04_DualMode_UltrasonicTracking_And_TouchControl.ino): Program dual-mode:
  - Tekan **TB1A**: Mode pelacak objek ultrasonik (*Follow Object*) dengan gradasi warna RGB sesuai jarak.
  - Tekan **TB1B**: Mode kendali gerak arah robot berbasis sentuhan dengan indikator visual panah RGB.
- `Legacy_Backup/`: Cadangan implementasi terdahulu yang menggunakan pustaka perangkat keras dasar (*raw library* `FastLED`, `NewPing`, dan `MotorL`).

---

### 9. `09_Missions_and_DigitalTwin/` (Misi Kompetisi & Simulasi Digital Twin)
Kumpulan program untuk menyelesaikan misi arena fisik maupun arena virtual Digital Twin (kompetisi AIROC):
- [`01_Mission_Touch_Trigger_Run_150mm.ino`](09_Missions_and_DigitalTwin/01_Mission_Touch_Trigger_Run_150mm.ino): Memulai misi ketika TB1A dan TB1B ditekan bersamaan, maju 150 mm, berhenti, lalu mengakhiri misi (`LbMission.end()`).
- [`02_DigitalTwin_Mission_1_1_Straight_Forward_Backward.ino`](09_Missions_and_DigitalTwin/02_DigitalTwin_Mission_1_1_Straight_Forward_Backward.ino): Misi Digital Twin `ms1.1` — robot maju 100 mm lalu mundur kembali 100 mm.
- [`03_DigitalTwin_Mission_11_1_Turn_And_FollowLine.ino`](09_Missions_and_DigitalTwin/03_DigitalTwin_Mission_11_1_Turn_And_FollowLine.ino): Misi `ms11.1` — kalibrasi garis, rotasi sudut siku C-D, mengikuti garis hitam hingga lingkaran B, dan maju sejauh 40 mm.
- [`04_DigitalTwin_Mission_11_2_FollowLine_Carefully.ino`](09_Missions_and_DigitalTwin/04_DigitalTwin_Mission_11_2_FollowLine_Carefully.ino): Misi `ms11.2` — kalibrasi garis dan susur garis hitam presisi hingga lingkaran B.
- [`05_DigitalTwin_Mission_12_1_FollowLine_Standard_Speed.ino`](09_Missions_and_DigitalTwin/05_DigitalTwin_Mission_12_1_FollowLine_Standard_Speed.ino): Misi `ms12.1` — susur garis dengan kecepatan standar (1000 steps/s).
- [`06_DigitalTwin_Mission_12_1_Fast_FollowLine_Fast_Speed.ino`](09_Missions_and_DigitalTwin/06_DigitalTwin_Mission_12_1_Fast_FollowLine_Fast_Speed.ino): Misi `ms12.1` kecepatan tinggi (1200 steps/s) untuk waktu penyelesaian arena yang optimal.

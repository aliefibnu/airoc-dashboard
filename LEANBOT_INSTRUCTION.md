# Tabel Final Command System Leanbot

Format command string:

```
String command =
  "SPEED:5;SCAN_KANAN:10,90;DEKATI_OBJEK:6;CAPIT;MUNDUR:8;BUNYI:2;STOP";
```

- `;` memisahkan setiap instruksi.
- `:` memisahkan nama instruksi dan parameter.
- `,` memisahkan beberapa parameter.
- Instruksi tanpa parameter cukup ditulis namanya.
- Semua command di bawah merupakan spesifikasi yang perlu diimplementasikan di interpreter C++, silahkan lihat dokumentasi leanbot di folder ./leanbot_code_examples.

## 1. Gerakan dasar

| Instruksi | Parameter | Fungsi                                           |
| --------- | --------- | ------------------------------------------------ |
| `MAJU`    | `cm`      | Bergerak maju sejauh jarak tertentu              |
| `MUNDUR`  | `cm`      | Bergerak mundur sejauh jarak tertentu            |
| `KANAN`   | `deg`     | Rotasi di tempat ke kanan sebesar sudut tertentu |
| `KIRI`    | `deg`     | Rotasi di tempat ke kiri sebesar sudut tertentu  |
| `STOP`    | —         | Menghentikan semua gerakan motor                 |
| `SPEED`   | `level`   | Mengatur level kecepatan motor                   |
| `TUNGGU`  | `ms`      | Menunggu selama durasi tertentu                  |

## 2. Ultrasonic dan pencarian objek

| Instruksi      | Parameter             | Fungsi                                                       |
| -------------- | --------------------- | ------------------------------------------------------------ |
| `UKUR_JARAK`   | —                     | Membaca jarak ultrasonic dan menampilkan hasilnya ke serial  |
| `SCAN_KANAN`   | `jarak_cm,sudut_maks` | Scan kanan hingga objek terdeteksi atau batas sudut tercapai |
| `SCAN_KIRI`    | `jarak_cm,sudut_maks` | Scan kiri hingga objek terdeteksi atau batas sudut tercapai  |
| `DEKATI_OBJEK` | `jarak_akhir_cm`      | Mendekati objek sampai jarak target                          |
| `JAUHI_OBJEK`  | `jarak_akhir_cm`      | Menjauh dari objek sampai jarak target                       |
| `CEK_OBJEK`    | `jarak_cm`            | Memeriksa apakah objek terdeteksi                            |
| `BATAS_SCAN`   | `sudut_deg`           | Menetapkan batas sudut pencarian                             |

## 3. Gripper dan manipulasi balok

| Instruksi | Parameter | Fungsi          |
| --------- | --------- | --------------- |
| `CAPIT`   | —         | Menutup gripper |
| `LEPAS`   | —         | Membuka gripper |

## 4. Koreksi posisi dan manuver zona

| Instruksi     | Parameter | Fungsi                                           |
| ------------- | --------- | ------------------------------------------------ |
| `GESER_KANAN` | `cm`      | Melakukan manuver untuk menggeser balok ke kanan |
| `GESER_KIRI`  | `cm`      | Melakukan manuver untuk menggeser balok ke kiri  |

## 5. Buzzer, LED, dan feedback

| Instruksi     | Parameter             | Fungsi                                                            |
| ------------- | --------------------- | ----------------------------------------------------------------- |
| `BUNYI`       | `jumlah`              | Membunyikan buzzer sebanyak jumlah tertentu                       |
| `BEEP`        | `frekuensi,durasi_ms` | Membunyikan buzzer berdasarkan frekuensi dan durasi jika didukung |
| `LED`         | `mode`                | Mengatur mode LED                                                 |
| `LED_WARNA`   | `warna`               | Mengatur warna LED jika hardware mendukung                        |
| `LED_KEDIP`   | `jumlah,interval_ms`  | Membuat LED berkedip                                              |
| `LAPOR_JARAK` | —                     | Menampilkan pembacaan ultrasonic ke serial                        |

## 9. Timeout, batas gerakan, dan keselamatan

| Instruksi        | Parameter | Fungsi                                      |
| ---------------- | --------- | ------------------------------------------- |
| `EMERGENCY_STOP` | —         | Menghentikan semua aktuator secepat mungkin |
| `END`            | —         | Mengakhiri eksekusi command string          |

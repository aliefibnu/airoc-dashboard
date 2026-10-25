# Tabel Final Command System Leanbot

## Format Transmisi Instruksi

Seluruh instruksi yang dikirimkan ke robot Leanbot melalui Bluetooth BLE maupun Serial USB WAJIB diawali dengan prefiks `CODE;`.

Format transmisi multi-instruksi:

```
String command = "CODE;SPEED:5;SCAN_KANAN:10,90;DEKATI_OBJEK:6;CAPIT;MUNDUR:8;BUNYI:2;STOP";
```

Format transmisi single instruksi:

```
String command = "CODE;MAJU:10";
```

Format emergency stop:

```
String command = "CODE;EMERGENCY_STOP";
```

### Aturan Protokol Transmisi:
1. **Prefiks Wajib `CODE;`**: Robot HANYA memproses data yang memiliki awalan `CODE;`. Data serial tanpa awalan `CODE;` akan otomatis diabaikan dan dibuang dari buffer.
2. **Bebas Filter New Line**: Sistem transmisi TIDAK bergantung pada filter atau karakter baris baru (`\n` atau `\r`). Deteksi didasarkan pada awalan `CODE;`, pemisah instruksi titik koma (`;`), serta jeda transmisi (timeout chunk buffer).
3. **Pemisah Antar Instruksi (`;`)**: Tanda titik koma digunakan untuk memisahkan satu instruksi dengan instruksi berikutnya dalam satu paket transmisi.
4. **Pemisah Perintah dan Parameter (`:`)**: Tanda titik dua memisahkan nama perintah dengan parameternya.
5. **Pemisah Multi-Parameter (`,`)**: Tanda koma memisahkan lebih dari satu nilai parameter pada perintah yang membutuhkan beberapa argumen.
6. **Instruksi Tanpa Parameter**: Cukup dituliskan nama instruksinya saja tanpa titik dua (contoh: `CAPIT`, `LEPAS`, `STOP`, `UKUR_JARAK`).

---

## 1. Gerakan Dasar

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `MAJU` | `cm` | Bergerak maju sejauh jarak tertentu (default 10 cm) |
| `MUNDUR` | `cm` | Bergerak mundur sejauh jarak tertentu (default 10 cm) |
| `KANAN` | `deg` | Rotasi di tempat ke kanan sebesar sudut tertentu (default 90°) |
| `KIRI` | `deg` | Rotasi di tempat ke kiri sebesar sudut tertentu (default 90°) |
| `STOP` | — | Menghentikan semua gerakan motor stepper seketika |
| `SPEED` | `level` | Mengatur kecepatan motor (level 1-10 dipetakan ke 400-2000 steps/s) |
| `TUNGGU` | `ms` | Menunda eksekusi instruksi berikutnya selama durasi tertentu (ms) |

---

## 2. Ultrasonic dan Pencarian Objek

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `UKUR_JARAK` | — | Membaca jarak sensor ultrasonic dan mengirimkan `JARAK:<cm>` ke serial |
| `SCAN_KANAN` | `jarak_cm,sudut_maks` | Berputar ke kanan hingga objek terdeteksi (<= jarak_cm) atau batas sudut tercapai. Mengirimkan `OBJEK:<cm>` atau `OBJEK:TIDAK_ADA` |
| `SCAN_KIRI` | `jarak_cm,sudut_maks` | Berputar ke kiri hingga objek terdeteksi (<= jarak_cm) atau batas sudut tercapai. Mengirimkan `OBJEK:<cm>` atau `OBJEK:TIDAK_ADA` |
| `DEKATI_OBJEK` | `jarak_akhir_cm` | Bergerak maju mendekati objek hingga jarak target tercapai |
| `JAUHI_OBJEK` | `jarak_akhir_cm` | Bergerak mundur menjauhi objek hingga jarak target tercapai |
| `CEK_OBJEK` | `jarak_cm` | Memeriksa objek dalam jarak tertentu. Jika ada: LED Hijau + 3 beep; Jika tidak ada: LED Merah + 1 beep |
| `BATAS_SCAN` | `sudut_deg` | Menetapkan batas sudut default rotasi pencarian scan |
| `LAPOR_JARAK` | — | Menampilkan hasil pembacaan jarak ultrasonic ke serial monitor |

---

## 3. Gripper dan Manipulasi Balok

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `CAPIT` | — | Menutup servo gripper untuk mencapit objek |
| `LEPAS` | — | Membuka servo gripper untuk melepaskan objek |
| `SERVO` | `sudut` | Mengatur posisi sudut servo capit secara spesifik (0-180°) |

---

## 4. Buzzer, LED, dan Feedback

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `BUNYI` | `jumlah` | Membunyikan buzzer sebanyak jumlah ketukan tertentu |
| `BEEP` | `frekuensi,durasi_ms` | Membunyikan buzzer dengan frekuensi (Hz) dan durasi (ms) tertentu |
| `LED` | `mode` | Mengatur mode LED: `1`/`ON`, `0`/`OFF`, `2`/`SMILE`, `3`/`FROWN`, `4`/`LEFT`, `5`/`RIGHT` |
| `LED_WARNA` | `warna` | Mengatur warna seluruh LED RGB (MERAH, HIJAU, BIRU, KUNING, PUTIH, OFF, dll.) |
| `LED_KEDIP` | `jumlah,interval_ms` | Mengedipkan LED Kuning sebanyak jumlah pengulangan dengan interval tertentu |

---

## 5. Line Follower

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `FOLLOW_LINE` | `jumlah_simpangan` | Mengikuti garis hitam menggunakan 4-channel IR line sensor hingga mencapai jumlah persimpangan tertentu (default 1) |
| `MAJU_LINE` | `max_cm` | Bergerak maju hingga 2 sensor infrared tengah (`0b0110`) mendeteksi garis hitam |
| `MUNDUR_LINE` | `max_cm` | Bergerak mundur hingga 2 sensor infrared tengah (`0b0110`) mendeteksi garis hitam |
| `STOP_LINE` | — | Menghentikan mode line follower dan mematikan motor |
| `KALIBRASI` | — | Melakukan kalibrasi manual sensor infrared garis dengan tombol sentuh TB1A (3 tahap: putih, hitam, konfirmasi) |

---

## 6. Keselamatan dan Konfigurasi

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `EMERGENCY_STOP` | — | Menghentikan seluruh motor seketika, membunyikan indikator, dan mengosongkan antrean perintah |
| `END` | — | Menghentikan motor dan menandai akhir rangkaian instruksi |
| `CONFIG` | `parameter,nilai` | Mengatur nilai baseline/default (contoh: `CONFIG:MAJU,15`, `CONFIG:SPEED,6`) |

# Tabel Final Command System Leanbot

## Format Transmisi Instruksi

Seluruh instruksi yang dikirimkan ke robot Leanbot melalui Bluetooth BLE maupun Serial USB diawali dengan prefiks `CODE;`.

Format transmisi multi-instruksi (Batch):

```
String command = "CODE;SPEED:5;MAJU:10;BELOK_KANAN:90;MAJU_LINE:50;FOLLOW_LINE:1;BUNYI:2;END";
```

Format transmisi single instruksi:

```
String command = "CODE;MAJU:10,5";
```

Format emergency stop:

```
String command = "CODE;EMERGENCY_STOP";
```

### Aturan Protokol Transmisi:
1. **Prefiks Wajib `CODE;`**: Robot memproses data yang memiliki awalan `CODE;`. Data serial tanpa awalan `CODE;` akan otomatis diabaikan, kecuali kata kunci darurat (`STOP`, `EMERGENCY`).
2. **Pemisah Antar Instruksi (`;`)**: Tanda titik koma digunakan untuk memisahkan satu instruksi dengan instruksi berikutnya dalam satu paket transmisi.
3. **Pemisah Perintah dan Parameter (`:`)**: Tanda titik dua memisahkan nama perintah dengan parameternya.
4. **Pemisah Multi-Parameter (`,`)**: Tanda koma memisahkan lebih dari satu nilai parameter pada perintah yang membutuhkan beberapa argumen.
5. **Eksekusi Batch Otomatis**: Instruksi panjang dipecah per batch (maksimum 12-15 instruksi per batch) dan otomatis diakhiri dengan instruksi `END`.
6. **Non-Blocking Serial Emergency Interruption**: Selama misi berlangsung, robot secara periodik membaca UART stream sehingga perintah `EMERGENCY_STOP` atau `STOP` dapat langsung membatalkan eksekusi gerakan seketika.
7. **Tombol Sentuh**: Tombol fisik sentuh `TB1A` hanya digunakan khusus untuk kalibrasi manual sensor infrared garis (`KALIBRASI`).

---

## 1. Gerakan Dasar (Motion)

> [!NOTE]
> Perbedaan **Rotate** vs **Belok (Pivot)**:
> - **Rotate (`KANAN`/`KIRI`)**: Poros rotasi berada tepat di tengah robot (roda kiri dan roda kanan berputar berlawanan arah).
> - **Belok (`BELOK_KANAN`/`BELOK_KIRI`)**: Poros rotasi berada di salah satu ban. Pada `BELOK_KANAN`, ban kanan diam (0) dan ban kiri bergerak maju (`+spd`). Pada `BELOK_KIRI`, ban kiri diam (0) dan ban kanan bergerak maju (`+spd`).

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `MAJU` | `cm[,speed]` | Bergerak maju sejauh jarak tertentu (default 10 cm, speed mengikuti kecepatan global) |
| `MUNDUR` | `cm[,speed]` | Bergerak mundur sejauh jarak tertentu (default 10 cm, speed mengikuti kecepatan global) |
| `KANAN` | `deg[,speed]` | Rotate di tempat ke kanan poros tengah (default 90°) |
| `KIRI` | `deg[,speed]` | Rotate di tempat ke kiri poros tengah (default 90°) |
| `BELOK_KANAN` | `deg[,speed]` | Belok ke kanan dengan poros pada ban kanan (default 90°) |
| `BELOK_KIRI` | `deg[,speed]` | Belok ke kiri dengan poros pada ban kiri (default 90°) |
| `STOP` | — | Menghentikan semua gerakan motor stepper seketika |
| `SPEED` | `level` | Mengatur kecepatan motor global (level 1-10 dipetakan ke 400-2000 steps/s) |
| `TUNGGU` | `ms` | Menunda eksekusi instruksi berikutnya selama durasi tertentu (ms) |

---

## 2. Gripper Mekanik

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `CAPIT` | — | Menutup servo gripper untuk mencapit objek |
| `LEPAS` | — | Membuka servo gripper untuk melepaskan objek |

---

## 3. Buzzer & LED Feedback

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `BUNYI` | `jumlah` | Membunyikan buzzer sebanyak jumlah ketukan tertentu (default 1) |
| `LED_WARNA` | `warna` | Mengatur warna seluruh LED RGB (dipilih via dropdown: PUTIH, MERAH, HIJAU, BIRU, KUNING, CYAN, MAGENTA, ORANGE, UNGU, OFF) |
| `LED_KEDIP` | `jumlah,interval_ms` | Mengedipkan LED Kuning sebanyak jumlah pengulangan dengan interval tertentu |

---

## 4. Line Follower & Line Detection

> [!IMPORTANT]
> **Kriteria Deteksi Garis (Line Detected)**:
> Garis dianggap terdeteksi jika:
> 1. Dua sensor infrared tengah aktif (`0b0110`), ATAU
> 2. Tiga atau lebih sensor infrared mendeteksi hitam (`aktif >= 3`, contoh: `0b0111`, `0b1110`, `0b1111`, `0b1101`, `0b1011`).

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `FOLLOW_LINE` | `jumlah_simpangan[,speed]` | Mengikuti garis hitam menggunakan 4-channel IR line sensor hingga mencapai persimpangan (default 1 simpangan) |
| `MAJU_LINE` | `max_cm[,speed]` | Bergerak maju hingga garis terdeteksi (default 50 cm) |
| `MUNDUR_LINE` | `max_cm[,speed]` | Bergerak mundur hingga garis terdeteksi (default 50 cm) |
| `STOP_LINE` | — | Menghentikan mode line follower dan mematikan motor |
| `KALIBRASI` | — | Melakukan kalibrasi manual sensor infrared garis dengan tombol sentuh TB1A (3 tahap: putih, hitam, konfirmasi) |

---

## 5. Keselamatan dan Konfigurasi Krusial

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `EMERGENCY_STOP` | — | Menghentikan seluruh motor seketika, menyalakan indikator merah, dan mengosongkan antrean perintah |
| `END` | — | Menandai akhir rangkaian atau batch instruksi dan menghentikan sisa gerakan motor |
| `CONFIG` | `parameter,nilai` | Mengatur nilai baseline/default krusial (`SPEED`, `MAJU`, `MUNDUR`, `KANAN`, `KIRI`, `BELOK_KANAN`, `BELOK_KIRI`, `FOLLOW_LINE`, `MAJU_LINE`, `MUNDUR_LINE`, `TUNGGU`, `BUNYI`) |

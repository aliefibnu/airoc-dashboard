# Tabel Final Command System Leanbot

## Format Transmisi Instruksi

Protokol transmisi mendukung sistem multi-batch (Batch 1 - 10) dengan konsep "terima seluruh batch terlebih dahulu, baru mulai eksekusi".

Format pengunggahan batch (Batch 1 sampai 10, maksimal 12 instruksi per batch):

```
BATCH:1;SPEED:5;MAJU:10;BELOK_KANAN:90;MAJU_LINE:50;FOLLOW_LINE:1;BUNYI:2;END
BATCH:2;FOLLOW_LINE:2,5;MAJU:15,5;CAPIT;END
```

Format memulai eksekusi seluruh batch misi:

```
START_MISSION
```

Format reset/pembersihan seluruh batch variabel:

```
CLEAR
```

Format emergency stop:

```
EMERGENCY_STOP
```

Format single instruksi (langsung dieksekusi):

```
CODE;MAJU:10,5;END
```

### Aturan Protokol Transmisi:
1. **Sistem Multi-Batch (Batch 1 - 10)**: Robot menyimpan instruksi di variabel array batch 1 sampai 10 (masing-masing maksimal 12 instruksi).
2. **Await All Instructions**: Seluruh batch dikirim dan disimpan ke variabel robot terlebih dahulu sebelum robot mulai bergerak.
3. **Mulai Eksekusi (`START_MISSION`)**: Robot mengeksekusi instruksi dari variabel Batch 1. Setelah selesai, robot memeriksa apakah Batch berikutnya memiliki instruksi; jika ya, dijalankan; jika kosong/dilewati, melanjutkan ke batch berikutnya hingga Batch 10.
4. **Pembersihan Otomatis Pasca Eksekusi**: Setelah seluruh instruksi dalam semua batch selesai dijalankan (atau dibatalkan), robot secara otomatis mereset seluruh variabel Batch 1 - 10 ke 0 agar tidak ada instruksi/batch yang tertinggal di memori robot untuk misi berikutnya.
5. **Pemisah Antar Instruksi (`;`)**: Tanda titik koma memisahkan instruksi dalam batch.
6. **Pemisah Perintah dan Parameter (`:`)**: Tanda titik dua memisahkan nama perintah dan parameternya.
7. **Reset Darurat (`EMERGENCY_STOP`)**: Menghentikan motor seketika dan mereset/mengosongkan seluruh variabel Batch 1 - 10 sehingga robot langsung bersih dan siap menerima instruksi baru.
8. **Non-Blocking Serial Emergency Interruption**: Selama misi berlangsung, robot secara periodik membaca UART stream sehingga perintah `EMERGENCY_STOP` atau `STOP` dapat langsung membatalkan eksekusi gerakan seketika.
9. **Tombol Sentuh**: Tombol fisik sentuh `TB1A` hanya digunakan khusus untuk kalibrasi manual sensor infrared garis (`KALIBRASI`).

---

## 1. Gerakan Dasar (Motion)

> [!NOTE]
> Perbedaan **Rotate** vs **Belok (Pivot)**:
> - **Rotate (`KANAN`/`KIRI`)**: Poros rotasi berada tepat di tengah robot (roda kiri dan roda kanan berputar berlawanan arah).
> - **Belok (`BELOK_KANAN`/`BELOK_KIRI`)**: Poros rotasi berada di salah satu ban. Pada `BELOK_KANAN`, ban kanan diam (0) dan ban kiri bergerak (maju `+spd` pada mode `MAJU` atau mundur `-spd` pada mode `MUNDUR`). Pada `BELOK_KIRI`, ban kiri diam (0) dan ban kanan bergerak (maju `+spd` pada mode `MAJU` atau mundur `-spd` pada mode `MUNDUR`).

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `MAJU` | `cm[,speed]` | Bergerak maju sejauh jarak tertentu (default 10 cm, speed mengikuti kecepatan global) |
| `MUNDUR` | `cm[,speed]` | Bergerak mundur sejauh jarak tertentu (default 10 cm, speed mengikuti kecepatan global) |
| `KANAN` | `deg[,speed]` | Rotate di tempat ke kanan poros tengah (default 90°) |
| `KIRI` | `deg[,speed]` | Rotate di tempat ke kiri poros tengah (default 90°) |
| `BELOK_KANAN` | `deg[,speed][,mode]` | Belok ke kanan poros ban kanan (default 90°, mode: `MAJU` / `MUNDUR`) |
| `BELOK_KIRI` | `deg[,speed][,mode]` | Belok ke kiri poros ban kiri (default 90°, mode: `MAJU` / `MUNDUR`) |
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
| `FOLLOW_LINE` | `jumlah_simpangan[,speed][,mode]` | Mengikuti garis hitam menggunakan 4-channel IR line sensor hingga persimpangan (default 1 simpangan, mode: `MAJU` / `MUNDUR`) |
| `MAJU_LINE` | `max_cm[,speed]` | Bergerak maju hingga garis terdeteksi (default 50 cm) |
| `MUNDUR_LINE` | `max_cm[,speed]` | Bergerak mundur hingga garis terdeteksi (default 50 cm) |
| `STOP_LINE` | — | Menghentikan mode line follower dan mematikan motor |
| `KALIBRASI` | — | Melakukan kalibrasi manual sensor infrared garis dengan tombol sentuh TB1A (3 tahap: putih, hitam, konfirmasi) |

---

## 5. Keselamatan dan Akhir Batch

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `EMERGENCY_STOP` | — | Menghentikan seluruh motor seketika, menyalakan indikator merah, dan mengosongkan antrean perintah |
| `END` | — | Menandai akhir rangkaian atau batch instruksi dan menghentikan sisa gerakan motor |

---

## 6. IoT & Status Misi

> [!NOTE]
> Format Komunikasi Serial / BLE untuk IoT:
> Robot mencetak output baris dengan prefix `IOT : ` berisi payload JSON:
> `IOT : {"key":"key_unique_untuk masing2 misi","done":true}`

| Instruksi | Parameter | Fungsi |
| --------- | --------- | ------ |
| `MISI_DONE` | `misi_key[,status]` | Menandai status selesai suatu misi ke IoT Dashboard. Robot mencetak `IOT : {"key":"<misi_key>","done":<true/false>}` (default status: 1/true). |

### Daftar Key Misi Unik:
- **Neutral (3 Misi)**: `neutral_ungu`, `neutral_hijau`, `neutral_biru`
- **Analysis (2 Misi)**: `analysis_normal`, `analysis_mystery`
- **Containment (2 Misi)**: `containment_merah`, `containment_kuning`
- **Back to CRL (2 Misi)**: `crl_leanbot_a`, `crl_leanbot_b`

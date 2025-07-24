# 🧠 New Mogi Server & Client

**Dibuat oleh: Andey Koiwai**

Proyek ini merupakan sistem komunikasi antara ESP32 (client) dan Flask server (Python) untuk memproses input audio dan menghasilkan respons berbasis AI menggunakan API dari **Google Gemini** dan **Groq**.

---

## 📁 Struktur Proyek

```
newmogiserver/
├── server/     # Aplikasi Flask (Python)
│   ├── NEWSERVER9.py
│   ├── requirements.txt
│   ├── .env.example
├── client/     # Firmware untuk ESP32
│   └── MOGI_V01_2804_2025_UPDATED.ino
```

---

## 🚀 Cara Menjalankan Server (Python/Flask)

### 1. Siapkan file `.env` di folder `server/`

```
GEMINI_API_KEY=isi_api_key_gemini_kamu
GROQ_API_KEY=isi_api_key_groq_kamu
```

### 2. Install dependensi

```bash
pip install -r requirements.txt
```

### 3. Jalankan server

```bash
cd server
python NEWSERVER9.py
```

Server akan aktif di: `http://0.0.0.0:8888`

> Pastikan port dan alamat IP sesuai dengan client ESP32.

---

🔌 Alat yang Dibutuhkan:

Proyek ini menggunakan berbagai komponen untuk membuat sistem ESP32 S3 berbasis audio dan AI. Berikut daftar lengkap perangkat keras:
🧠 Komponen Utama:

    ESP32-S3 N16R8 (Microcontroller utama)

    Modul Mikrofon INMP441 (I2S Digital Microphone)

    Amplifier MAX98357A (I2S ke speaker)

    Speaker kecil (mono, 3W)

🔭 Sensor dan Tampilan:

    Sensor Jarak VL53L0X (I2C)

    OLED GC9A01 (SPI, layar bundar)

🔧 Aktuator dan Motor:

    2 Set Motor Gearbox N20 + Roda

    L298N Mini Motor Driver (kendali 2 motor DC)

🔋 Daya dan Pengisian:

    2x Baterai 18650 (2S, 7.4V)

    Step-down MP1584EN (Buck converter 3.3V / 5V)

    BMS 2S Balanced 20A (untuk proteksi pengisian baterai Li-Ion)

    Pengisi daya USB-C 2S Li-Ion Charger (8.4V, 2A)

🧰 Komponen Pendukung:

    Kabel Jumper

    PCB Dot Matrix (lubang) – untuk sambungan semi permanen

    Switch / Saklar

    Pin Header, Resistor, dll (jika diperlukan)

### Konfigurasi Penting dalam Kode:

```cpp
const char* server = "http://192.168.x.x:8888";
```

> Ganti dengan IP lokal tempat Flask server berjalan

ESP32 akan:

* Merekam suara melalui I2S
* Mengirim file WAV ke Flask server
* Menerima respon dari AI (Gemini/Groq)

---

## 🧠 Fitur Aplikasi Flask (Server)

* Menerima file audio `.wav` via POST request
* Mengubah audio menjadi teks dengan SpeechRecognition
* Menghasilkan balasan menggunakan Gemini atau Groq
* Menyimpan log, status, atau data interaksi ke TinyDB / JSON

---

## 🔐 Keamanan

* API key **tidak disimpan langsung** di file Python
* Menggunakan `os.environ.get()` untuk membaca dari environment
* File `.env` disertakan sebagai contoh, dan **tidak dipush ke GitHub**

---

## 📦 Dependensi Server

* Python 3.8+
* Flask
* gTTS
* SpeechRecognition
* tinydb
* requests
* google-generativeai
* python-dotenv

> Semua sudah didefinisikan di `requirements.txt`

---

## 🙋 FAQ

**Q: Apakah bisa jalan di Replit, Render, atau Koyeb?**
A: Bisa! Cukup upload `server/` ke repo GitHub, dan deploy dari sana. Tambahkan ENV `GEMINI_API_KEY` dan `GROQ_API_KEY` di dashboard platform tersebut.

**Q: Di mana file audio disimpan?**
A: Secara default di local storage server (`recording.wav`, `output.mp3`). Kamu bisa ubah ke penyimpanan eksternal seperti S3 jika di hosting.

---

## 🤝 Lisensi dan Donasi

Proyek ini **gratis digunakan untuk pembelajaran dan pengembangan**.
Jika kamu merasa proyek ini bermanfaat, kamu bisa memberi dukungan:

☕ Donasi melalui **transfer BCA**
No. Rekening: **5745008264**
Atas Nama: **Dewi Lestari**

📫 Hubungi via email jika ingin kolaborasi, kontribusi, atau konsultasi teknis.

**Copyright © 2025 Andey Koiwai.**

---

## 📬 Kontak

* GitHub: [@andeykoiwai](https://github.com/andeykoiwai)  
* Email: andeykoiwai@gmail.com  
* WhatsApp: +62 899 8210 011  
  ![QR WhatsApp](./Gambar/api.qrserver.png)  
  [Klik untuk chat langsung](https://wa.me/628998210011)  
* Facebook: Takako Andey  
* TikTok: [@mogirobotic](https://www.tiktok.com/@mogirobotic)  
* YouTube: [@mogirobotik](https://www.youtube.com/@mogirobotik)

## 🖼️ Dokumentasi Visual

### 🔧 Skema Rangkaian (Fritzing)
![Skema Fritzing](./Gambar/MOGIFritzingdiagram.png)

---

### 🧰 Setting Arduino IDE
![Setting Arduino IDE 1](./Gambar/arduinosetup1.png)
![Setting Arduino IDE 2](./Gambar/arduinosetup2.png)

---

### 🌐 Tampilan Menu WebMogi
![Menu WebMogi](./Gambar/webmogimain.png)
![Menu WebMogi Wifi](./Gambar/webmogiwifi.png)
![Menu WebMogi Setting 1](./Gambar/webmogiconfig1.png)
![Menu WebMogi Setting 2](./Gambar/webmogiconfig2.png)
![Menu WebMogi MP3 1](./Gambar/webmogimp3menu1.png)
![Menu WebMogi MP3 2](./Gambar/webmogimp3menu2.png)
![Menu WebMogi Teman](./Gambar/webmogiteman.png)
---

> Terima kasih sudah menggunakan New Mogi Server & Client! Semoga bermanfaat untuk riset, robotika, AI, atau proyek IoT kamu 🚀

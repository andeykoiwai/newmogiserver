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

## 🔌 Cara Menjalankan Client (ESP32)

### Alat yang Dibutuhkan:

* ESP32 (S3, WROOM, dll)
* Modul mikrofon (INMP441/I2S)
* Arduino IDE atau PlatformIO

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
* Email: (andeykoiwai@gmail.com)
* Telegram: update

---

> Terima kasih sudah menggunakan New Mogi Server & Client! Semoga bermanfaat untuk riset, robotika, AI, atau proyek IoT kamu 🚀

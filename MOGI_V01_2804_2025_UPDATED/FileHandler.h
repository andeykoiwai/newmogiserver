/*
  🤖 MOGI - Mini AI Robot Project
  https://github.com/andeykoiwai/newmogiserver

  🤝 Lisensi dan Donasi
  Proyek ini GRATIS untuk pembelajaran dan pengembangan.

  Jika kamu merasa proyek ini bermanfaat, dukung kami:

  ☕ Donasi via BCA
  No. Rekening : 5745008264
  Atas Nama    : Dewi Lestari
  QRIS         : lihat gambar di /Gambar/Qiris.jpg

  📫 Kontak:
  Email    : andeykoiwai@gmail.com
  WhatsApp : +62 899 8210 011

  © 2025 Andey Koiwai
*/

#ifndef FileHandler_h
#define FileHandler_h

#include <FFat.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "WebConfig.h"

class FileHandler {
  public:
    FileHandler();
    void upload(String namafile, String url);
    void download(String namafile, String url);
    bool ServerStatus(String serverurl_);
    void parseResponse(String response);
    void setMogiConfig(const MogiConfig& config);
    String getserver_user();
    String getesp_user();
    bool isUpload();
    bool isDownload();
    bool isInQuiz();
    String getQuizType();
    
    int responweb;

  private:
    bool uploadStatus;
    bool downloadStatus;
    bool isServerOn;
    bool in_quiz;
    String server_user;
    String esp_user;
    String quiz_type;
    MogiConfig mogiConfig;
};

#endif

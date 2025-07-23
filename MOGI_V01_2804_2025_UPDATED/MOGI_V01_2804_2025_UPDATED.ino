/*
Mogi Robot - Main Control Program
===============================

Features:
- TFT Animation with clock and date display
- Voice capabilities 
- WiFi configuration system
- Configurable robot name
- MP3 file upload support
- Mogi configuration options:
  * Name
  * Serial number
  * Eye parameters (size, blink, breath)
  * Colors and dimensions
- update serial number ask for quiz
- Quiz support (math and English)
- Update Sprite agar tidak fliker
- update pesan baru dari server
Server URLs:
- Cloud: http://andeykoiwai.pythonanywhere.com
         https://mogi-flask-server-production.up.railway.app/  
- Local: http://192.168.0.100:8888

Last Updated: 2025
toch nya sudah berjalan nilainnya tinggal di atur ya, yang masih dalam perbaikan nilai toch
sama di tambah juga di webnya agar nilai toch nya bisa di atur dari sana

*/

#include <WiFi.h>
#include <WebServer.h>
#include "WebConfig.h"
#include "Microphone.h"
#include "FileHandler.h"

//#----- calling mogi ------
#define EIDSP_QUANTIZE_FILTERBANK   0
#include <MOGIONLINE_inferencing.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "driver/i2s.h"
//#----- end calling mogi --

//#----- animasi baru (mata robot)----------
#include <TFT_eSPI.h>
#include "mata_robot_gc9a01.h"

TFT_eSPI display = TFT_eSPI();
roboEyes eyes;

//#----- end animasi-------
//#----- Speker ----------
#include "Audio.h"
#include <driver/i2s.h>
#include "FS.h"
#include "FFat.h"
//#----memory cek------
#include "esp_heap_caps.h"
bool animasidansuara = false;
void printHeapInfo() {
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);
}
//#--------------------
// ======================= Perbaikan: Servo, Motor, VL53L0X, Touch =======================
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>

// VL53L0X
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
#define SDA_PIN 17
#define SCL_PIN 18

// Servo
Servo myServo;
#define SERVO_PIN 14

// Motor A
#define IN1 15
#define IN2 16

// Motor B
#define IN3 47
#define IN4 21

// Touch sensor - PERBAIKAN THRESHOLD
#define TOUCH_PIN T7
// #define HARD_TOUCH_THRESHOLD 5000   // Threshold untuk sentuhan sangat keras
// #define NORMAL_TOUCH_THRESHOLD 4000 // Threshold untuk sentuhan normal
// #define GENTLE_TOUCH_THRESHOLD 1000  // Threshold untuk sentuhan lembut
int HARD_TOUCH_THRESHOLD = 5000;
int NORMAL_TOUCH_THRESHOLD = 4000;
int GENTLE_TOUCH_THRESHOLD = 1000;
#define CARESS_DURATION 1000         // Durasi minimum untuk dianggap usapan (ms)

// Global variables untuk robot movement
bool robotMovementEnabled = true;
bool obstacleDetected = false;
bool motorAktif = true; // default true, nanti diambil dari config
unsigned long lastMovementCheck = 0;
unsigned long lastTouchCheck = 0;
unsigned long touchStartTime = 0;
bool isBeingCaressed = false;
bool isSmiling = false;
bool isAngry = false;
bool isSad = false;
int consecutiveTouchCount = 0;
int hardTouchCount = 0;
const unsigned long MOVEMENT_CHECK_INTERVAL = 400; // ms
const unsigned long TOUCH_CHECK_INTERVAL = 200; // ms

// Variabel untuk kalibrasi otomatis
int touchBaseline = 0;
bool touchCalibrated = false;
int calibrationSamples = 0;
unsigned long calibrationSum = 0;
bool modeSeeTouch = false;
bool modeSeeJarak = false;
//------------------------------------------
#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC 6

Audio audio;
String endplaying = "";
//#----- END Speker ----------

//#----- set calling mogi struktur -----
/** Audio buffers, pointers and selectors */
typedef struct {
  int16_t *buffer;
  uint8_t buf_ready;
  uint32_t buf_count;
  uint32_t n_samples;
} inference_t;

static inference_t inference;
static const uint32_t sample_buffer_size = 2048;
static signed short sampleBuffer[sample_buffer_size];
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static bool record_status = true;
bool callmogi = true;
//#----- end calling mogi struktur -----

//#-----------standar setting -------
WebConfig internet_config("mogi");
Microphone mic;
FileHandler hendelfile;
String tulisan ="";
bool endplayingbool = true;
bool textanimasi = true;
bool newMessage = false;
//#--------- end standar setting -----

// Modifikasi untuk setup() function
void setup() {
  // setup_robot_movement(); // Pindahkan ini ke paling atas
  delay(5000);
  Serial.begin(115200);
  ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
  
  // Disable robot movement di awal saat setup
  // enableRobotMovement(false);
  
  internet_config.begin();
  MogiConfig config = internet_config.getMogiConfig();
  hendelfile.setMogiConfig(config);
  // Set threshold touch dari config
  HARD_TOUCH_THRESHOLD = config.hardTouchThreshold;
  NORMAL_TOUCH_THRESHOLD = config.normalTouchThreshold;
  GENTLE_TOUCH_THRESHOLD = config.gentleTouchThreshold;
  // Set mode debug dari config
  modeSeeTouch = config.modeSeeTouch;
  modeSeeJarak = config.modeSeeJarak;
  // Set motorAktif dari config
  motorAktif = config.motorAktif;
  
  //#-------- Speker ---------
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(30);
  //#-------- EnD ----------
  
  //#-------- animasi baru ------
  display.init();
  display.setRotation(0);
  display.fillScreen(TFT_BLACK);
  eyes.begin(240, 240, 100);  // Ukuran layar 240x240, 40 FPS 75 |10
  eyes.open();
  eyes.setMainColor(config.eyeColor);
  eyes.setWidth(config.eyeWidth, config.eyeWidth);
  eyes.setHeight(config.eyeHeight, config.eyeHeight);
  eyes.setBorderradius(config.borderRadius, config.borderRadius);
  eyes.setSpacebetween(config.spaceBetween);
  eyes.setMood(DEFAULT);
  eyes.setPosition(DEFAULT);
  eyes.setAutoblinker(config.autoBlinker, config.autoBlinkerTime, config.autoBlinkerVariation);
  eyes.setBreathing(config.breathing, config.breathingSpeed, config.breathingAmount);
  xTaskCreate(eyeAnimationTask, "Eye Animation Task", 4096, NULL, 2, NULL);
  
  if(WiFi.localIP().toString()!="0.0.0.0"){
    if (internet_config.cekStatusWifi()) {
      eyes.resyncTime();
      eyes.setTimeDisplay(true);
    } else {
      eyes.setTimeDisplay(false);
    }
    eyes.setIdleMode(true, 5, 3);     // Gerakan idle setiap 5±3 detik
    // eyes.setText("Tunggu Sebentar");
    // Ganti pengecekan file mp3 berdasarkan alias
    String namaFileMp3 = internet_config.getMp3FilenameByAlias("opening"); // Ganti "Nama Alias MP3" sesuai alias yang diinginkan
    if (namaFileMp3 != "" && FFat.exists("/" + namaFileMp3)) {
      playMusic(namaFileMp3);
      while(endplaying!=namaFileMp3){
        audio.loop();
      }
      if(endplaying==namaFileMp3){
        // delay(1000);
        eyes.setText("tunggu 5 detik lagi ya", internet_config.getMogiConfig().textColor);
        // setup_robot_movement();
        // eyes.setText(internet_config.getMogiConfig().name, internet_config.getMogiConfig().textColor);
        xTaskCreate(callingMogi, "Mogi Inference Task", 4096 , NULL, 1, NULL);
        setup_robot_movement();
        animasidansuara = true;
        enableRobotMovement(true);
        delay(1000);
        eyes.setText(" IP "+WiFi.localIP().toString());
        delay(1000);
        eyes.setText(internet_config.getMogiConfig().name, internet_config.getMogiConfig().textColor);
      }
    }
    else{
      delay(1000);
      eyes.setText(" Masuk "+WiFi.localIP().toString()+" di Browser / mogi.local dan upload mp3 file");
      enableRobotMovement(true);
    }
  }
  else{
    // deteksi awalan belum terinstall
    eyes.setPosition(N);
    eyes.setText("untuk pertamakali cari wifi mogi, setelah konek ke wifi mogi, masuk ke browser 192.168.4.1 jika terkoneksi IP "+ WiFi.localIP().toString() +" upload file di browser dengan mengetik ip ini");
    // Disable robot movement jika belum ada koneksi
    enableRobotMovement(false);
  }
}

// Modifikasi untuk loop() function
void loop() {  
  audio.loop();
  internet_config.handleClient();
  // Update mode debug setiap loop agar jika config berubah via web, langsung ikut
  MogiConfig config = internet_config.getMogiConfig();
  modeSeeTouch = config.modeSeeTouch;
  modeSeeJarak = config.modeSeeJarak;
  // Update motorAktif setiap loop
  motorAktif = config.motorAktif;
  commandConsol();

  if(!callmogi){
    komunikasi_ESP_Server();
  }

  if(!endplayingbool){
    if(endplaying=="output_" + String(internet_config.getMogiConfig().serialNumber) + ".mp3"){
      eyes.setMood(DEFAULT);
      endplayingbool = true;
    }
  }
  
  // Tambahkan pengecekan pesan baru
  checkNewMessages();
  
  // Tidak perlu memanggil loop_robot_movement() karena sudah menggunakan FreeRTOS task
  // loop_robot_movement(); // HAPUS INI
}

void textAnimasi(String text, uint16_t color){
  if(textanimasi && !newMessage){
    eyes.setText(text, color);
  }
}
// Task untuk animasi mata
void eyeAnimationTask(void *pvParameters) {
  while (1) {
    eyes.update();
    // off sementara
    /*
    static unsigned long lastMoodChange = 0;
    if (millis() - lastMoodChange > 50000) { // Ganti mood setiap 10 detik
      lastMoodChange = millis();
      int mood = random(4); // Pilih mood acak
      
      switch(mood) {
        case 0:
          eyes.setMood(DEFAULT);
          textAnimasi(internet_config.getMogiConfig().name, internet_config.getMogiConfig().textColor);
          break;
        case 1:
          eyes.setMood(TIRED);
          if(animasidansuara){
            playMusic("sedih.mp3");
            textAnimasi("Hiks", internet_config.getMogiConfig().textColor);
          }
          break;
        case 2:
          eyes.setMood(ANGRY);
          if(animasidansuara){
            playMusic("marah.mp3");
            textAnimasi("hemm", internet_config.getMogiConfig().textColor);
          }
          break;
        case 3:
          eyes.setMood(HAPPY);
          if(animasidansuara){
            playMusic("geli.mp3");
            textAnimasi("Geli", internet_config.getMogiConfig().textColor);
          }
          break;
      }
      
      // Trigger animasi acak
      if(random(100) > 70) {
        if(random(2)) {
          eyes.anim_confused();
          if(animasidansuara){
            playMusic("kaget.mp3");
          }
        } else {
          eyes.anim_laugh();
          if(animasidansuara){
            playMusic("geli.mp3");
          }
        }
      }
    }
    */
    vTaskDelay(20 / portTICK_PERIOD_MS); // Delay untuk mengontrol FPS
  }
}

// command consol buat download dll / test
// Modifikasi untuk commandConsol()
// Modifikasi commandConsol untuk menambah perintah test mood
void commandConsol(){
  if(Serial.available()){   
    tulisan = Serial.readStringUntil('\n');
    tulisan.trim();
    if(tulisan == "data"){
      internet_config.getFileList();
    }
    else if(tulisan == "rekam"){
      callmogi = false;
      record_status = false;
      enableRobotMovement(false);
      delay(100);
      delay(100);
      delay(100);
      mic.startRecording();
    }
    // else if(tulisan == "debug_touch"){
    //   // Toggle debug mode untuk touch sensor
    //   static bool debugMode = false;
    //   debugMode = !debugMode;
    //   if (debugMode) {
    //     Serial.println("Debug touch sensor enabled");
    //     xTaskCreate([](void*){ 
    //       while(true) { 
    //         debugTouchSensor(); 
    //         vTaskDelay(100 / portTICK_PERIOD_MS); 
    //       } 
    //     }, "TouchDebug", 2048, NULL, 1, NULL);
    //   } else {
    //     Serial.println("Debug touch sensor disabled");
    //   }
    // }
    else if(tulisan == "calibrate_touch"){
      Serial.println("Memulai kalibrasi touch sensor...");
      calibrateTouchSensor();
    }
    else if(tulisan == "touch_info"){
      Serial.println("=== Touch Sensor Info ===");
      Serial.print("Calibrated: ");
      Serial.println(touchCalibrated ? "Yes" : "No");
      Serial.print("Baseline: ");
      Serial.println(touchBaseline);
      Serial.print("Current Value: ");
      int currentTouchValue = (int)touchRead(TOUCH_PIN);  // Cast to int
      Serial.println(currentTouchValue);
      Serial.print("Difference: ");
      Serial.println(abs(touchBaseline - currentTouchValue));  // Now both are int
      Serial.println("========================");
    }
    else if(tulisan == "reset_touch"){
      Serial.println("Reset touch sensor settings...");
      touchCalibrated = false;
      touchBaseline = 0;
      resetTouchVariables();
      Serial.println("Touch sensor direset. Gunakan 'calibrate_touch' untuk kalibrasi ulang.");
    }
    else if(tulisan == "test_smile"){
      Serial.println("Testing robot smile...");
      robotSmile();
    }
    else if(tulisan == "test_angry"){
      Serial.println("Testing robot angry...");
      robotAngry();
    }
    else if(tulisan == "test_sad"){
      Serial.println("Testing robot sad...");
      robotSad();
    }
    else if(tulisan == "reset_mood"){
      Serial.println("Resetting mood to normal...");
      returnToNormalMood();
    }
    else if(tulisan == "cekwifi"){
      internet_config.cekStatusWifi();
    }
    else if(tulisan == "cekserver"){
      hendelfile.ServerStatus(internet_config.getServerUrl());
    }
    else if(tulisan == "upload"){
      hendelfile.upload("recording.wav",internet_config.getServerUrl());
    }
    else if(tulisan == "esp_user"){
      Serial.println(hendelfile.getesp_user());
    }
    else if(tulisan == "server_user"){
      Serial.println(hendelfile.getserver_user());
    }
    else if(tulisan == "download") {
        String outputFile = "output_" + String(internet_config.getMogiConfig().serialNumber) + ".mp3";
        hendelfile.download(outputFile, internet_config.getServerUrl());
    }
    else if(tulisan == "robot_on") {
        enableRobotMovement(true);
        Serial.println("Robot movement enabled");
    }
    else if(tulisan == "robot_off") {
        enableRobotMovement(false);
        Serial.println("Robot movement disabled");
    }
    // else if(tulisan == "downloadall") {
    //     callmogi = false;
    //     record_status = false;
    //     enableRobotMovement(false);
    //     eyes.setAutoblinker(false);
    //     eyes.setIdleMode(false);
    //     eyes.setText("Sedang Mendownload");
    //     delay(200);
        
    //     const char* files[] = {
    //         "nama.mp3",
    //         "noserver.mp3", 
    //         "nowifi.mp3",
    //         "rekam.mp3",
    //         "upload.mp3",
    //         "download.mp3",
    //         "error.mp3",
    //         "geli.mp3",
    //         "kaget.mp3",
    //         "bosen.mp3",
    //         "sedih.mp3",
    //         "marah.mp3"
    //     };
        
    //     for(const char* file : files) {
    //         hendelfile.download(file, internet_config.getServerUrl());
    //         delay(200);
    //         eyes.setText("Download " + String(file));
    //     }
        
    //     eyes.setText("Download Selesai");
    //     Serial.println("Download Selesai");
    //     delay(1000);
    //     ESP.restart();
    // }
    // else if(tulisan == "mode_touch_on"){
    //   modeseetoucth = true;
    //   Serial.println("Mode touchDiff aktif!");
    // }
    // else if(tulisan == "mode_touch_off"){
    //   modeseetoucth = false;
    //   Serial.println("Mode touchDiff nonaktif!");
    // }
    else{
      Serial.println("Perintah tidak dikenali.");
      Serial.println("Perintah tambahan: robot_on, robot_off, debug_touch, test_smile, test_angry, test_sad, reset_mood");
    }
  }
}

/*
membuat logic speker plasy speker
*/
void playMusic(String namamusik) {
    String namamusik_ = namamusik;    
    if (FFat.exists("/" + namamusik_)) {
        audio.stopSong(); 
        Serial.println("playing. "+ namamusik_);
        audio.connecttoFS(FFat, ("/" + namamusik_).c_str());
    } else {
        Serial.println("tidak menemukan filemusik.");
    }
}

// Fungsi pembantu untuk play mp3 berdasarkan alias
void playByAlias(String alias) {
    String namaFileMp3 = internet_config.getMp3FilenameByAlias(alias);
    if (namaFileMp3 != "" && FFat.exists("/" + namaFileMp3)) {
        playMusic(namaFileMp3);
        while(endplaying != namaFileMp3){
            audio.loop();
        }
    } else {
        Serial.println("File alias " + alias + " tidak ditemukan!");
    }
}


// Add this updated komunikasi_ESP_Server function to the client code

// Updated komunikasi_ESP_Server function with English learning support
  // Updated komunikasi_ESP_Server function dengan robot movement integration
void komunikasi_ESP_Server(){
    String namaFileMp3 = "";
    ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
    if(internet_config.cekStatusWifi()){
      if(hendelfile.ServerStatus(internet_config.getServerUrl())){
        //cara memanggil suara di taks wajib seperti ini
        textanimasi = false;
        callmogi = false;
        record_status = false;
        animasidansuara = false;
        
        // DISABLE robot movement saat recording
        enableRobotMovement(false);
        
        eyes.setMood(HAPPY);
        eyes.anim_laugh();
        eyes.setText("Mendengarkan... ", TFT_GREEN);
        namaFileMp3 = internet_config.getMp3FilenameByAlias("rekam");
        playMusic(namaFileMp3);
        while(endplaying!=namaFileMp3){
          audio.loop();
        }
        // sampai sini
        if(endplaying==namaFileMp3){
          // ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
          mic.startRecording();
        }
        
        if(!mic.isRecordingActive() && endplaying==namaFileMp3){
          namaFileMp3 = internet_config.getMp3FilenameByAlias("upload");
          playMusic(namaFileMp3);
          eyes.setMood(TIRED);
          eyes.anim_confused();
          eyes.setText("Memproses...", TFT_YELLOW);
          // ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
          while(endplaying!=namaFileMp3){
            // menunggu menjadi upload
            audio.loop();
          }
          // namaFileMp3 = internet_config.getMp3FilenameByAlias("upload");
          if(endplaying==namaFileMp3){
            eyes.setMood(DEFAULT);
            hendelfile.upload("recording.wav",internet_config.getServerUrl());
            if(hendelfile.responweb!=200){
              namaFileMp3 = internet_config.getMp3FilenameByAlias("error");
              playMusic(namaFileMp3);
              while(endplaying!=namaFileMp3){
                audio.loop();
              }
              eyes.setMood(ANGRY);
              Serial.println("upload error");
              // RE-ENABLE robot movement setelah error
              // enableRobotMovement(true);
              return;
            }
            // mencoba merapihkan dari upload
            else if(hendelfile.responweb==200){
              if(!hendelfile.isUpload() && endplaying==namaFileMp3){
                namaFileMp3 = internet_config.getMp3FilenameByAlias("download");
                playMusic(namaFileMp3);
                while(endplaying!=namaFileMp3){
                  // menunggu menjadi download
                  audio.loop();
                }
                if(endplaying==namaFileMp3){
                  eyes.setMood(TIRED);
                  eyes.anim_confused();
                  // Use serial number for output file
                  String outputFile = "output_" + String(internet_config.getMogiConfig().serialNumber) + ".mp3";
                  hendelfile.download(outputFile, internet_config.getServerUrl());
                  if(!hendelfile.isDownload() && endplaying==namaFileMp3){
                    eyes.setMood(HAPPY);
                    eyes.anim_laugh();
                    eyes.setPosition(N);
                    eyes.setIdleMode(false);
                    // TETAP DISABLE robot movement saat playing audio response
                    // enableRobotMovement(false);
                    // Check if we're in a quiz session
                    if(hendelfile.isInQuiz()){
                      // Set different colors based on quiz type
                      uint16_t quizColor;
                      if(hendelfile.getQuizType() == "math") {
                        quizColor = TFT_CYAN;
                      } else if(hendelfile.getQuizType() == "english") {
                        quizColor = TFT_MAGENTA;
                      } else {
                        quizColor = TFT_WHITE;
                      }
                      eyes.setText("Quiz: " + hendelfile.getserver_user(), quizColor);
                      String outputQuizFile = "output_" + String(internet_config.getMogiConfig().serialNumber) + ".mp3";
                      playMusic(outputQuizFile);
                      while(endplaying!=outputQuizFile){
                        audio.loop();
                      }
                      // If we've finished the quiz, return to normal mode
                      if(hendelfile.getserver_user().indexOf("Kuis selesai!") >= 0 || 
                         hendelfile.getserver_user().indexOf("Quiz completed!") >= 0) {
                        if(endplaying==outputQuizFile){
                          xTaskCreate(callingMogi, "Mogi Inference Task", 4096 , NULL, 1, NULL);
                          eyes.clearText();
                          eyes.setIdleMode(true);
                          endplayingbool = false;
                          Serial.println("kembali memanggil mogi");
                          animasidansuara = false;
                          // RE-ENABLE robot movement setelah quiz selesai
                          // enableRobotMovement(true);
                        }
                      } else {
                        // Continue with next quiz question
                        // After playing the question, immediately start listening for answer
                        if(endplaying==outputQuizFile){
                          delay(500); // Short pause before recording
                          // Set text based on quiz type
                          if(hendelfile.getQuizType() == "math") {
                            eyes.setText("Jawab pertanyaan...", TFT_GREEN);
                          } else if(hendelfile.getQuizType() == "english") {
                            eyes.setText("Answer the question...", TFT_GREEN);
                          }
                          // Recursive call to handle next question
                          komunikasi_ESP_Server();
                        }
                      }
                    } else {
                      // Normal conversation mode
                      eyes.setText(hendelfile.getserver_user(), TFT_WHITE);
                      String outputFile = "output_" + String(internet_config.getMogiConfig().serialNumber) + ".mp3";
                      playMusic(outputFile);
                      while(endplaying!=outputFile){
                        audio.loop();
                      }
                      if(endplaying==outputFile){
                        xTaskCreate(callingMogi, "Mogi Inference Task", 4096 , NULL, 1, NULL);
                        eyes.clearText();
                        eyes.setIdleMode(true);
                        endplayingbool = false;
                        Serial.println("kembali memanggil mogi");
                        animasidansuara = true;
                        textanimasi = true;
                        // RE-ENABLE robot movement setelah selesai bicara
                        enableRobotMovement(true);
                      }
                    }
                  }
                }
                else{
                  namaFileMp3 = internet_config.getMp3FilenameByAlias("error");
                  playMusic(namaFileMp3);
                  while(endplaying!=namaFileMp3){
                    audio.loop();
                  }
                  eyes.setMood(ANGRY);
                  eyes.anim_confused();
                  eyes.setText("komunikasi terputus..", TFT_RED);
                  Serial.println("download error");
                  // RE-ENABLE robot movement setelah error
                  // enableRobotMovement(true);
                }
              }
            }
            else{
              namaFileMp3 = internet_config.getMp3FilenameByAlias("error");
              playMusic(namaFileMp3);
              while(endplaying!=namaFileMp3){
                audio.loop();
              }
              eyes.setMood(ANGRY);
              eyes.anim_confused();
              eyes.setText("komunikasi terputus..", TFT_RED);
              Serial.println("upload error");
              // RE-ENABLE robot movement setelah error
              // enableRobotMovement(true);
            }
          }
        }
      }
      else{
        namaFileMp3 = internet_config.getMp3FilenameByAlias("noserver");
        Serial.println("tidak terhubung ke server");  
        playMusic(namaFileMp3);
        eyes.setMood(TIRED);
        eyes.setText("tidak terhubung ke server", TFT_RED);
        // DISABLE robot movement saat playing error sound
        enableRobotMovement(false);
        while(endplaying!=namaFileMp3){
          audio.loop();
        }
        xTaskCreate(callingMogi, "Mogi Inference Task", 4096 , NULL, 1, NULL);
        // RE-ENABLE robot movement setelah error sound
        enableRobotMovement(true);
      }
    }
    else{
      namaFileMp3 = internet_config.getMp3FilenameByAlias("nowifi");
      Serial.println("Wifi tidak terhubung");
      eyes.setMood(TIRED);
      eyes.anim_confused();
      eyes.setText("Wifi tidak terhubung", TFT_RED);
      // DISABLE robot movement saat playing error sound
      enableRobotMovement(false);
      playMusic(namaFileMp3);
      while(endplaying!=namaFileMp3){
        audio.loop();
      }
      xTaskCreate(callingMogi, "Mogi Inference Task", 4096 , NULL, 1, NULL);
      // RE-ENABLE robot movement setelah error sound
      enableRobotMovement(true);
    }
    Serial.print("Tanya Jawab ");
    Serial.println("Done .");
    ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
}

// untuk mengecek bahwa audio benar di play
void audio_eof_mp3(const char *info){  //end of file
  endplaying = info;
  Serial.print("end mp3 ");
  Serial.println(endplaying);
}
/* AWAL CALLING MOGI*/
void setcallMogi(){
  // Serial.println("Edge Impulse Inferencing Demo");

  // // Summary of inferencing settings
  ei_printf("Inferencing settings:\n");
  ei_printf("\tInterval: ");
  ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
  ei_printf(" ms.\n");
  ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
  ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

  ei_printf("\nStarting continuous inference in 2 seconds...\n");

  if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false) {
      ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
      return;
  }

  ei_printf("Recording...\n");
}

// Modifikasi untuk callingMogi task
void callingMogi(void *pvParameters){
  record_status = true;
  callmogi = true;
  
  // Enable robot movement saat dalam mode listening
  enableRobotMovement(true);
  
  setcallMogi();
  while(callmogi){
    bool m = microphone_inference_record();
    if (!m) {
        ei_printf("ERR: Failed to record audio...\n");
        enableRobotMovement(true); // Pastikan robot movement enable saat error
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        enableRobotMovement(true); // Pastikan robot movement enable saat error
        return;
    }

    // Hanya tampilkan prediksi untuk "mogi"
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
//      ei_printf("    %s: ", result.classification[ix].label);
//      ei_printf_float(result.classification[ix].value);
//      ei_printf("\n");
      if (strcmp(result.classification[ix].label, "mogi") == 0){
        if(result.classification[ix].value > 0.7){
          callmogi = false;
          record_status = false;
          enableRobotMovement(false);
          // Robot movement akan di-disable di komunikasi_ESP_Server
          vTaskDelay(200);
          vTaskDelete(NULL);
        }
      }
    }
  }
  if(!callmogi){
    callmogi = false;
    record_status = false;
    enableRobotMovement(false);
    vTaskDelay(200);
    vTaskDelete(NULL);
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

static void audio_inference_callback(uint32_t n_bytes){
  for(int i = 0; i < n_bytes>>1; i++) {
      inference.buffer[inference.buf_count++] = sampleBuffer[i];

      if(inference.buf_count >= inference.n_samples) {
        inference.buf_count = 0;
        inference.buf_ready = 1;
      }
  }
}

static void capture_samples(void* arg) {
  const int32_t i2s_bytes_to_read = (uint32_t)arg;
  size_t bytes_read = i2s_bytes_to_read;

  while (record_status) {
      // read data at once from i2s
      i2s_read((i2s_port_t)1, (void*)sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);

      if (bytes_read <= 0) {
          ei_printf("Error in I2S read : %d", bytes_read);
      }
      else {
          if (bytes_read < i2s_bytes_to_read) {
              ei_printf("Partial I2S read");
          }

          // Scale the data (otherwise the sound is too quiet)
          for (int x = 0; x < i2s_bytes_to_read/2; x++) {
              sampleBuffer[x] = (int16_t)(sampleBuffer[x]) * 8;
          }

          if (record_status) {
              audio_inference_callback(i2s_bytes_to_read);
          }
          else {
              break;
          }
      }
  }
  vTaskDelete(NULL);
}

static bool microphone_inference_start(uint32_t n_samples){
  inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

  if(inference.buffer == NULL) {
      return false;
  }

  inference.buf_count  = 0;
  inference.n_samples  = n_samples;
  inference.buf_ready  = 0;

  ei_sleep(100);

  record_status = true;

  xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void*)sample_buffer_size, 10, NULL);

  return true;
}


static bool microphone_inference_record(void){
  bool ret = true;

  while (inference.buf_ready == 0) {
      delay(10);
  }

  inference.buf_ready = 0;
  return ret;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr){
  numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);
  return 0;
}

// Fungsi untuk mengecek pesan baru
void checkNewMessages() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 30000) { // Cek setiap 30 detik
    lastCheck = millis();
    
    // Cek pesan baru dari server
    internet_config.checkNewMessagesFromServer();
    
    // Dapatkan jumlah pesan yang belum dibaca
    int unreadCount = 0;
    for (int i = 0; i < internet_config.getMessageCount(); i++) {
      if (!internet_config.getMessage(i).isRead) {
        unreadCount++;
      }
    }
    
    if (unreadCount > 0) {
      eyes.setMood(HAPPY);
      eyes.anim_laugh();
      eyes.setText("Ada " + String(unreadCount) + " pesan baru!", TFT_GREEN);
      playByAlias("notif");
      newMessage = true;
      // delay(2000);
      //eyes.clearText();
    }else{
      newMessage = false;
    }
  }
}

// Fungsi untuk kalibrasi otomatis touch sensor
void calibrateTouchSensor() {
  Serial.println("Kalibrasi touch sensor dimulai...");
  eyes.setText("Kalibrasi touch sensor dimulai...", internet_config.getMogiConfig().textColor);
  vTaskDelay(50 / portTICK_PERIOD_MS);
  Serial.println("Jangan sentuh sensor selama 5 detik!");
  eyes.setText("Jangan sentuh sensor selama 5 detik!", internet_config.getMogiConfig().textColor);
  
  unsigned long calibrationSum = 0;  // Change to unsigned long for larger values
  calibrationSamples = 0;
  
  // Ambil sampel selama 5 detik
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    int touchValue = (int)touchRead(TOUCH_PIN);  // Cast to int consistently
    calibrationSum += touchValue;
    calibrationSamples++;
    vTaskDelay(50 / portTICK_PERIOD_MS);
    
    // Tampilkan progress
    if (calibrationSamples % 20 == 0) {
      Serial.print("Kalibrasi: ");
      Serial.print((millis() - startTime) / 1000 + 1);
      Serial.print("/5 detik, Nilai: ");
      Serial.println(touchValue);
    }
  }
  
  // Hitung baseline
  touchBaseline = (int)(calibrationSum / calibrationSamples);  // Cast result to int
  touchCalibrated = true;
  
  Serial.print("Kalibrasi selesai! Baseline: ");
  Serial.println(touchBaseline);
  Serial.println("Touch sensor siap digunakan");
  
  // Update tampilan mata
  // eyes.setText("Touch sensor siap!", TFT_GREEN);
  eyes.setText(String(touchBaseline), TFT_GREEN);
  vTaskDelay(500 / portTICK_PERIOD_MS);
}

// === FreeRTOS task for robot movement ===
// Modifikasi fungsi robotMovementTask untuk menangani mood yang lebih kompleks
void robotMovementTask(void *parameter) {
  while (true) {
    // Hanya jalankan robot movement jika tidak sedang recording atau playing audio
    
    if (robotMovementEnabled && callmogi && endplayingbool) {
      // Check obstacle detection (VL53L0X) dengan interval yang lebih besar
      // Tapi tidak jika sedang marah atau sedih
      if (millis() - lastMovementCheck > MOVEMENT_CHECK_INTERVAL && !isAngry && !isSad) {
        lastMovementCheck = millis();
        checkObstacle();
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    //  Check touch sensor dengan interval yang lebih kecil
      if (millis() - lastTouchCheck > TOUCH_CHECK_INTERVAL) {
        lastTouchCheck = millis();
        checkTouch();
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    } else {
      // Jika robot movement disabled, stop semua motor
      motorStop();
    }
    // Delay yang cukup untuk tidak mengganggu task lain
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
  vTaskDelay(10 / portTICK_PERIOD_MS);
}

// Fungsi tambahan untuk debug touch sensor dengan mood
// Modifikasi fungsi debugTouchSensor untuk menggunakan baseline
// void debugTouchSensor() {
//   static unsigned long lastDebugTime = 0;
//   if (millis() - lastDebugTime > 1000) { // Debug setiap 1 detik
//     lastDebugTime = millis();
//     int touchValue = (int)touchRead(TOUCH_PIN);  // Cast to int
//     int touchDifference = touchCalibrated ? abs(touchBaseline - touchValue) : 0;
    
//     Serial.print("Touch Debug - Raw: ");
//     Serial.print(touchValue);
//     Serial.print(", Baseline: ");
//     Serial.print(touchBaseline);
//     Serial.print(", Diff: ");
//     Serial.print(touchDifference);
//     Serial.print(", Caressed: ");
//     Serial.print(isBeingCaressed ? "Yes" : "No");
//     Serial.print(", Smiling: ");
//     Serial.print(isSmiling ? "Yes" : "No");
//     Serial.print(", Angry: ");
//     Serial.print(isAngry ? "Yes" : "No");
//     Serial.print(", Sad: ");
//     Serial.print(isSad ? "Yes" : "No");
//     Serial.print(", Hard Touch Count: ");
//     Serial.println(hardTouchCount);
//   }
// }

// Fungsi untuk mengecek obstacle
void checkObstacle() {
  VL53L0X_RangingMeasurementData_t measure;
  
  // Timeout untuk pembacaan sensor
  if (lox.rangingTest(&measure, false)) {
    return; // Skip jika timeout
  }

  if (measure.RangeStatus != 4) {
    int jarak = measure.RangeMilliMeter;
    
    // Hanya print jika ada perubahan signifikan
    static int lastDistance = 0;
    if (abs(jarak - lastDistance) > 50) {
      Serial.print("Jarak: ");
      Serial.println(jarak);
      lastDistance = jarak;
    }

    if (modeSeeJarak) {
      textAnimasi("Jarak "+String(jarak), TFT_GREEN);
    }

    if (jarak < 100) {
      // Obstacle sangat dekat - stop dan mundur
      eyes.anim_confused();
      String namaFileMp3 = internet_config.getMp3FilenameByAlias("kaget");
      if (FFat.exists("/" + namaFileMp3)) {
        playMusic(namaFileMp3);
        unsigned long start = millis();
        while(endplaying != namaFileMp3 && millis() - start < 5000) {
          audio.loop();
          vTaskDelay(10 / portTICK_PERIOD_MS);
        }
      } else {
        Serial.println("File sedih tidak ditemukan.");
      }
      vTaskDelay(100 / portTICK_PERIOD_MS);
      obstacleDetected = true;
      myServo.write(90); // Servo ke tengah
      motorStop();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorBackward();
      vTaskDelay(300 / portTICK_PERIOD_MS);
      motorStop();
    } else if (jarak < 200 && motorAktif) {
      // Obstacle sedang - belok kanan
      eyes.anim_confused();
      obstacleDetected = true;
      myServo.write(45);
      motorTurnRight();
    } else if (jarak < 300 && motorAktif) {
      // Obstacle jauh - maju pelan
      eyes.anim_confused();
      obstacleDetected = false;
      myServo.write(90);
      motorForward();
    } else {
      // Tidak ada obstacle - normal
      obstacleDetected = false;
      eyes.clearText();
      myServo.write(90);
      motorStop();
    }
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

// Fungsi untuk mengecek touch sensor
// Fungsi untuk mengecek touch sensor - VERSI PERBAIKAN
void checkTouch() {
  if (!touchCalibrated) {
    textAnimasi("belum dikalibrasi", TFT_GREEN);
    Serial.println("Skip jika belum dikalibrasi");
    return; // Skip jika belum dikalibrasi
  }
  
  // Pastikan kedua nilai dalam tipe data yang sama
  int touchValue = (int)touchRead(TOUCH_PIN);  // Cast ke int
  int touchDifference = abs(touchBaseline - touchValue);  // Sekarang keduanya int
  if (modeSeeTouch) {
    textAnimasi("toucthDiff "+String(touchDifference), TFT_GREEN);
  }
  // Debug output yang lebih informatif
  // static unsigned long lastDebugTime = 0;
  // if (millis() - lastDebugTime > 1000) {
  //   lastDebugTime = millis();
  //   Serial.print("Touch - Raw: ");
  //   Serial.print(touchValue);
  //   Serial.print(", Baseline: ");
  //   Serial.print(touchBaseline);
  //   Serial.print(", Difference: ");
  //   Serial.print(touchDifference);
  //   Serial.print(", Status: ");
    
  //   if (touchDifference > HARD_TOUCH_THRESHOLD) {
  //     Serial.println("VERY HARD");
  //   } else if (touchDifference > NORMAL_TOUCH_THRESHOLD) {
  //     Serial.println("HARD");
  //   } else if (touchDifference > GENTLE_TOUCH_THRESHOLD) {
  //     Serial.println("GENTLE");
  //   } else {
  //     Serial.println("NO TOUCH");
  //   }

  // }
  
  // Logika deteksi sentuhan berdasarkan perbedaan dari baseline
  if (touchDifference > HARD_TOUCH_THRESHOLD) {
    hardTouchCount++;
    Serial.println("Sentuhan sangat keras: Robot marah!");
    
    // Robot marah dan mundur
    robotAngry();
    if(motorAktif){
      motorTurnRight();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorTurnLeft();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorTurnRight();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorTurnLeft();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorBackward();
      vTaskDelay(300 / portTICK_PERIOD_MS);
      motorStop();
      motorForward();
      vTaskDelay(300 / portTICK_PERIOD_MS);
      motorStop();
    }
    
    // Reset semua variabel
    resetTouchVariables();
    
    // Delay tambahan untuk menghindari multiple trigger
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  // Cek untuk sentuhan keras biasa (sedih)
  else if (touchDifference > NORMAL_TOUCH_THRESHOLD) {
    Serial.println("Sentuhan keras: Robot sedih");
    
    // Robot sedih dan mundur
    robotSad();
    if(motorAktif){
      motorTurnRight();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorTurnLeft();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorTurnRight();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorTurnLeft();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      motorBackward();
      vTaskDelay(300 / portTICK_PERIOD_MS);
      motorStop();
      motorForward();
      vTaskDelay(300 / portTICK_PERIOD_MS);
      motorStop();
    }
    
    // Reset variabel usapan
    resetTouchVariables();
    
    // Delay tambahan untuk menghindari multiple trigger
    vTaskDelay(800 / portTICK_PERIOD_MS);
  }
  // Cek untuk usapan (sentuhan lembut)
  else if (touchDifference > GENTLE_TOUCH_THRESHOLD) {
    // Sentuhan lembut terdeteksi
    if (!isBeingCaressed) {
      touchStartTime = millis();
      isBeingCaressed = true;
      consecutiveTouchCount = 1;
      Serial.println("Mulai diusap...");
    } else {
      consecutiveTouchCount++;
      
      // Jika sudah diusap cukup lama, robot tersenyum
      if (millis() - touchStartTime > CARESS_DURATION && consecutiveTouchCount > 2) {
        if (!isSmiling) {
          Serial.println("Robot tersenyum karena diusap!");
          robotSmile();
          isSmiling = true;
        }
      }
    }
  }
  // Tidak ada sentuhan yang signifikan
  else {
    // Reset jika tidak ada sentuhan
    if (isBeingCaressed) {
      unsigned long caressDuration = millis() - touchStartTime;
      if (caressDuration < CARESS_DURATION) {
        Serial.println("Usapan terlalu singkat");
      }
      isBeingCaressed = false;
      touchStartTime = 0;
      consecutiveTouchCount = 0;
      
      // Kembalikan ke mood normal setelah beberapa saat
      if (isSmiling) {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Tersenyum selama 3 detik
        returnToNormalMood();
      }
    }
    
    // Reset mood marah/sedih setelah beberapa saat
    if (isAngry || isSad) {
      static unsigned long moodResetTime = 0;
      if (moodResetTime == 0) {
        moodResetTime = millis();
      }
      
      if (millis() - moodResetTime > 5000) { // Reset setelah 5 detik
        returnToNormalMood();
        moodResetTime = 0;
      }
    }
  }
}

// Fungsi baru untuk robot marah - tambahkan fungsi ini
void robotAngry() {
  if (!animasidansuara) return; // Hanya bereaksi jika animasi dan suara aktif
  // Cegah overlap
  if (isAngry) return;

  eyes.setMood(ANGRY);
  eyes.anim_confused();
  textAnimasi("Jangan kasar ", TFT_RED);

  String namaFileMp3 = internet_config.getMp3FilenameByAlias("marah");
  if (FFat.exists("/" + namaFileMp3)) {
    playMusic(namaFileMp3);
    unsigned long start = millis();
    while(endplaying != namaFileMp3 && millis() - start < 5000) {
      audio.loop();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  } else {
    Serial.println("File marah tidak ditemukan.");
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // Gerakan servo menunjukkan ketidaksenangan
  if (robotMovementEnabled) {
    for (int i = 0; i < 4; i++) {
      myServo.write(45);
      vTaskDelay(150 / portTICK_PERIOD_MS);
      myServo.write(135);
      vTaskDelay(150 / portTICK_PERIOD_MS);
    }
    myServo.write(90); // Kembali ke posisi tengah
  }

  isAngry = true;
  isSad = false;
  isSmiling = false;

  // Reset mood setelah 2 detik
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  returnToNormalMood();
}

// Fungsi baru untuk robot sedih - tambahkan fungsi ini
void robotSad() {
  if (!animasidansuara) return; // Hanya bereaksi jika animasi dan suara aktif
  // Cegah overlap
  if (isSad) return;

  eyes.setMood(TIRED);
  eyes.anim_confused();
  textAnimasi("Aduh, sakit... ", TFT_BLUE);

  String namaFileMp3 = internet_config.getMp3FilenameByAlias("sedih");
  if (FFat.exists("/" + namaFileMp3)) {
    playMusic(namaFileMp3);
    unsigned long start = millis();
    while(endplaying != namaFileMp3 && millis() - start < 5000) {
      audio.loop();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  } else {
    Serial.println("File sedih tidak ditemukan.");
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // Gerakan servo menunjukkan kesedihan (gerakan lambat)
  if (robotMovementEnabled) {
    for (int i = 0; i < 2; i++) {
      myServo.write(70);
      vTaskDelay(400 / portTICK_PERIOD_MS);
      myServo.write(110);
      vTaskDelay(400 / portTICK_PERIOD_MS);
    }
    myServo.write(90); // Kembali ke posisi tengah
  }

  isSad = true;
  isAngry = false;
  isSmiling = false;

  // Reset mood setelah 2 detik
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  returnToNormalMood();
}

// Fungsi untuk robot tersenyum - sudah ada, tapi tambahkan reset variabel
void robotSmile() {
  if (!animasidansuara) return; // Hanya tersenyum jika animasi dan suara aktif
  // Cegah overlap
  if (isSmiling) return;

  eyes.setMood(HAPPY);
  eyes.anim_laugh();
  textAnimasi("Senang diusap ", TFT_YELLOW);

  String namaFileMp3 = internet_config.getMp3FilenameByAlias("happy");
  if (FFat.exists("/" + namaFileMp3)) {
    playMusic(namaFileMp3);
    unsigned long start = millis();
    while(endplaying != namaFileMp3 && millis() - start < 5000) {
      audio.loop();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  } else {
    Serial.println("File happy tidak ditemukan.");
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // Gerakan servo untuk menunjukkan kegembiraan
  if (robotMovementEnabled) {
    for (int i = 0; i < 3; i++) {
      myServo.write(60);
      vTaskDelay(200 / portTICK_PERIOD_MS);
      myServo.write(120);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    myServo.write(90); // Kembali ke posisi tengah
  }

  isSmiling = true;
  isAngry = false;
  isSad = false;

  // Reset mood setelah 2 detik
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  returnToNormalMood();
}

// Fungsi untuk reset semua variabel touch
void resetTouchVariables() {
  isBeingCaressed = false;
  touchStartTime = 0;
  consecutiveTouchCount = 0;
  isSmiling = false;
}

// Fungsi untuk kembali ke mood normal
void returnToNormalMood() {
  eyes.setMood(DEFAULT);
  eyes.setText(internet_config.getMogiConfig().name, internet_config.getMogiConfig().textColor);
  isSmiling = false;
  isAngry = false;
  isSad = false;
  Serial.println("Kembali ke mood normal");
}


// Setup tambahan
void setup_robot_movement() {
  // Initialize I2C dengan pin yang sudah ditentukan
  Wire.begin(SDA_PIN, SCL_PIN,100000);
  
  // Initialize VL53L0X dengan error handling
  if (!lox.begin()) {
    Serial.println("VL53L0X tidak terdeteksi");
    robotMovementEnabled = false; // Disable robot movement jika sensor tidak ada
    eyes.anim_confused();
    textAnimasi("VL53L0X tidak terdeteksi", TFT_GREEN);
    delay(1000);
  } else {
    Serial.println("VL53L0X berhasil diinisialisasi");
    eyes.anim_confused();
    textAnimasi("VL53L0X berhasil diinisialisasi", TFT_GREEN);
    delay(1000);
  }

  // Initialize servo
  myServo.attach(SERVO_PIN);
  myServo.write(90); // Posisi tengah

  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Stop semua motor di awal
  motorStop();
  delay(1000);
  calibrateTouchSensor();
  
  // Create robot movement task dengan priority rendah
  if(touchCalibrated){
    xTaskCreate(robotMovementTask, "RobotMove", 4096 , NULL, 1, NULL);
  }
}

// Fungsi untuk enable/disable robot movement
void enableRobotMovement(bool enable) {
  robotMovementEnabled = enable;
  if (!enable) {
    motorStop();
  }
}

// Motor functions yang sudah diperbaiki
void motorForward() {
  if (!robotMovementEnabled) return;
  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void motorBackward() {
  if (!robotMovementEnabled) return;
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void motorStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void motorTurnRight() {
  if (!robotMovementEnabled) return;
  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH); // Perbaikan: motor kanan mundur untuk belok kanan
}

void motorTurnLeft() {
  if (!robotMovementEnabled) return;
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH); // Perbaikan: motor kiri mundur untuk belok kiri
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Fungsi tambahan untuk kontrol yang lebih halus
void motorForwardSlow() {
  if (!robotMovementEnabled) return;
  
  // Implementasi PWM untuk gerakan lambat bisa ditambahkan di sini
  motorForward();
}

void motorTurnRightSlow() {
  if (!robotMovementEnabled) return;
  
  // Implementasi PWM untuk belok lambat bisa ditambahkan di sini
  motorTurnRight();
}

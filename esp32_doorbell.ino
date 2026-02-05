#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"
#include "SD_MMC.h"
#include "FS.h"
#include "WiFi.h"

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Pin definitions
#define BUTTON_PIN 13
#define LED_PIN 12
#define BUZZER_PIN 14

// Camera pins for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Motion detection variables
unsigned long lastMotionTime = 0;
bool motionDetected = false;
int motionThreshold = 20; // Adjustable sensitivity
camera_fb_t *lastFrame = NULL;

// Recording variables
bool isRecording = false;
File videoFile;
int frameCount = 0;
bool sdCardAvailable = false;

// Button debounce
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 500;

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

// Doorbell chime using passive buzzer
void playDingDong() {
  // "Ding"
  tone(BUZZER_PIN, 800, 200);
  delay(250);
  // "Dong"
  tone(BUZZER_PIN, 600, 300);
  delay(350);
  noTone(BUZZER_PIN);
}

// Button interrupt handler
void IRAM_ATTR buttonPressed() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonPress > debounceDelay) {
    lastButtonPress = currentTime;
    playDingDong();
  }
}

// Simple motion detection based on frame difference
bool detectMotion(camera_fb_t *fb) {
  if (!lastFrame || lastFrame->len != fb->len) {
    if (lastFrame) {
      esp_camera_fb_return(lastFrame);
    }
    lastFrame = fb;
    return false;
  }
  
  int diffPixels = 0;
  int totalPixels = fb->len / 100; // Sample every 100 pixels for speed
  
  for (int i = 0; i < fb->len; i += 100) {
    if (abs((int)fb->buf[i] - (int)lastFrame->buf[i]) > 25) {
      diffPixels++;
    }
  }
  
  esp_camera_fb_return(lastFrame);
  lastFrame = fb;
  
  float motionPercent = (diffPixels * 100.0) / totalPixels;
  return motionPercent > motionThreshold;
}

// HTML for the web interface
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32-CAM Doorbell</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      color: #eee;
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
    }
    h1 {
      text-align: center;
      color: #00d4ff;
      margin-bottom: 30px;
      font-size: 2.5em;
      text-shadow: 0 0 10px rgba(0, 212, 255, 0.5);
    }
    .video-container {
      background: #0f3460;
      border-radius: 15px;
      padding: 20px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
      margin-bottom: 20px;
    }
    #stream {
      width: 100%;
      border-radius: 10px;
      background: #000;
    }
    .controls {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 15px;
      margin-bottom: 20px;
    }
    .btn {
      padding: 15px 25px;
      border: none;
      border-radius: 10px;
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    .btn-primary {
      background: linear-gradient(135deg, #00d4ff, #0099cc);
      color: white;
    }
    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(0, 212, 255, 0.4);
    }
    .btn-danger {
      background: linear-gradient(135deg, #ff4757, #cc3333);
      color: white;
    }
    .btn-danger:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(255, 71, 87, 0.4);
    }
    .btn-success {
      background: linear-gradient(135deg, #2ecc71, #27ae60);
      color: white;
    }
    .btn-success:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(46, 204, 113, 0.4);
    }
    .status-panel {
      background: #0f3460;
      border-radius: 15px;
      padding: 20px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
    }
    .status-item {
      display: flex;
      justify-content: space-between;
      padding: 10px;
      margin: 5px 0;
      background: rgba(255, 255, 255, 0.05);
      border-radius: 8px;
    }
    .status-label {
      color: #00d4ff;
      font-weight: 600;
    }
    .indicator {
      display: inline-block;
      width: 12px;
      height: 12px;
      border-radius: 50%;
      margin-left: 10px;
    }
    .indicator.active {
      background: #2ecc71;
      box-shadow: 0 0 10px #2ecc71;
    }
    .indicator.inactive {
      background: #e74c3c;
    }
    #motion-alert {
      background: #e74c3c;
      color: white;
      padding: 15px;
      border-radius: 10px;
      margin-bottom: 20px;
      display: none;
      animation: pulse 1s infinite;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.7; }
    }
    .slider-container {
      background: rgba(255, 255, 255, 0.05);
      padding: 15px;
      border-radius: 8px;
      margin: 10px 0;
    }
    input[type="range"] {
      width: 100%;
      height: 5px;
      background: #ddd;
      outline: none;
      border-radius: 5px;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 20px;
      height: 20px;
      background: #00d4ff;
      cursor: pointer;
      border-radius: 50%;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔔 ESP32-CAM Doorbell</h1>
    
    <div id="motion-alert">
      <strong>⚠️ MOTION DETECTED!</strong>
    </div>
    
    <div class="video-container">
      <img id="stream" src="">
    </div>
    
    <div class="controls">
      <button class="btn btn-primary" onclick="toggleLED()">Toggle LEDs</button>
      <button class="btn btn-primary" onclick="captureSnapshot()">📸 Snapshot</button>
      <button class="btn btn-success" id="recordBtn" onclick="toggleRecording()">⏺️ Start Recording</button>
      <button class="btn btn-danger" onclick="testBuzzer()">🔔 Test Buzzer</button>
    </div>
    
    <div class="status-panel">
      <h2 style="color: #00d4ff; margin-bottom: 15px;">System Status</h2>
      <div class="status-item">
        <span class="status-label">Stream:</span>
        <span>Active <span class="indicator active" id="stream-indicator"></span></span>
      </div>
      <div class="status-item">
        <span class="status-label">LEDs:</span>
        <span id="led-status">Off <span class="indicator inactive" id="led-indicator"></span></span>
      </div>
      <div class="status-item">
        <span class="status-label">Motion Detection:</span>
        <span id="motion-status">Monitoring <span class="indicator active"></span></span>
      </div>
      <div class="status-item">
        <span class="status-label">Recording:</span>
        <span id="record-status">Stopped <span class="indicator inactive" id="record-indicator"></span></span>
      </div>
      <div class="status-item">
        <span class="status-label">SD Card:</span>
        <span id="sd-status">Checking... <span class="indicator inactive" id="sd-indicator"></span></span>
      </div>
      
      <div class="slider-container">
        <label class="status-label">Motion Sensitivity: <span id="sensitivity-value">20</span>%</label>
        <input type="range" min="5" max="50" value="20" id="sensitivity" oninput="updateSensitivity(this.value)">
      </div>
    </div>
  </div>

  <script>
    let ledOn = false;
    let recording = false;
    
    // Start video stream
    document.getElementById('stream').src = window.location.origin + ':81/stream';
    
    // Check motion status periodically
    setInterval(checkMotion, 1000);
    setInterval(checkStatus, 2000);
    
    function checkMotion() {
      fetch('/motion')
        .then(response => response.json())
        .then(data => {
          if (data.motion) {
            document.getElementById('motion-alert').style.display = 'block';
            setTimeout(() => {
              document.getElementById('motion-alert').style.display = 'none';
            }, 3000);
          }
        });
    }
    
    function checkStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          // Update SD card status
          const sdStatus = document.getElementById('sd-status');
          const sdIndicator = document.getElementById('sd-indicator');
          if (data.sd_available) {
            sdStatus.innerHTML = 'Available <span class="indicator active" id="sd-indicator"></span>';
            sdIndicator.className = 'indicator active';
          } else {
            sdStatus.innerHTML = 'Not Available <span class="indicator inactive" id="sd-indicator"></span>';
            sdIndicator.className = 'indicator inactive';
          }
        });
    }
    
    function toggleLED() {
      fetch('/led/toggle')
        .then(response => response.json())
        .then(data => {
          ledOn = data.led_on;
          const ledStatus = document.getElementById('led-status');
          const ledIndicator = document.getElementById('led-indicator');
          if (ledOn) {
            ledStatus.innerHTML = 'On <span class="indicator active" id="led-indicator"></span>';
            ledIndicator.className = 'indicator active';
          } else {
            ledStatus.innerHTML = 'Off <span class="indicator inactive" id="led-indicator"></span>';
            ledIndicator.className = 'indicator inactive';
          }
        });
    }
    
    function captureSnapshot() {
      window.open('/capture', '_blank');
    }
    
    function toggleRecording() {
      const endpoint = recording ? '/record/stop' : '/record/start';
      fetch(endpoint)
        .then(response => response.json())
        .then(data => {
          if (data.success) {
            recording = !recording;
            const btn = document.getElementById('recordBtn');
            const recordStatus = document.getElementById('record-status');
            const recordIndicator = document.getElementById('record-indicator');
            
            if (recording) {
              btn.textContent = '⏹️ Stop Recording';
              btn.className = 'btn btn-danger';
              recordStatus.innerHTML = 'Recording <span class="indicator active" id="record-indicator"></span>';
              recordIndicator.className = 'indicator active';
            } else {
              btn.textContent = '⏺️ Start Recording';
              btn.className = 'btn btn-success';
              recordStatus.innerHTML = 'Stopped <span class="indicator inactive" id="record-indicator"></span>';
              recordIndicator.className = 'indicator inactive';
              alert('Recording saved: ' + data.filename);
            }
          } else {
            alert('Error: ' + data.message);
          }
        });
    }
    
    function testBuzzer() {
      fetch('/buzzer/test');
    }
    
    function updateSensitivity(value) {
      document.getElementById('sensitivity-value').textContent = value;
      fetch('/motion/sensitivity?value=' + value);
    }
    
    // Initial status check
    checkStatus();
  </script>
</body>
</html>
)rawliteral";

// HTTP handler for root page
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// HTTP handler for video stream
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];

  res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
  if (res != ESP_OK) {
    return res;
  }

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if (fb->format != PIXFORMAT_JPEG) {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = NULL;
        if (!jpeg_converted) {
          Serial.println("JPEG compression failed");
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }
    
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 13);
    }
    
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    
    if (res != ESP_OK) {
      break;
    }
  }
  
  return res;
}

// LED toggle handler
static esp_err_t led_handler(httpd_req_t *req) {
  static bool ledState = false;
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  
  httpd_resp_set_type(req, "application/json");
  char json[50];
  snprintf(json, sizeof(json), "{\"led_on\":%s}", ledState ? "true" : "false");
  return httpd_resp_send(req, json, strlen(json));
}

// Snapshot capture handler
static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=snapshot.jpg");
  
  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

// Motion detection handler
static esp_err_t motion_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  char json[50];
  snprintf(json, sizeof(json), "{\"motion\":%s}", motionDetected ? "true" : "false");
  motionDetected = false; // Reset after reading
  return httpd_resp_send(req, json, strlen(json));
}

// Buzzer test handler
static esp_err_t buzzer_handler(httpd_req_t *req) {
  playDingDong();
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, "{\"success\":true}", 17);
}

// Motion sensitivity handler
static esp_err_t sensitivity_handler(httpd_req_t *req) {
  char buf[100];
  int ret = httpd_req_get_url_query_str(req, buf, sizeof(buf));
  if (ret == ESP_OK) {
    char param[32];
    if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK) {
      motionThreshold = atoi(param);
    }
  }
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, "{\"success\":true}", 17);
}

// Status handler
static esp_err_t status_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  char json[100];
  snprintf(json, sizeof(json), "{\"sd_available\":%s,\"recording\":%s}", 
           sdCardAvailable ? "true" : "false",
           isRecording ? "true" : "false");
  return httpd_resp_send(req, json, strlen(json));
}

// Recording start handler
static esp_err_t record_start_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  
  if (!sdCardAvailable) {
    return httpd_resp_send(req, "{\"success\":false,\"message\":\"SD card not available\"}", 52);
  }
  
  if (isRecording) {
    return httpd_resp_send(req, "{\"success\":false,\"message\":\"Already recording\"}", 48);
  }
  
  // Create filename with timestamp
  char filename[50];
  snprintf(filename, sizeof(filename), "/video_%lu.mjpeg", millis());
  
  videoFile = SD_MMC.open(filename, FILE_WRITE);
  if (!videoFile) {
    return httpd_resp_send(req, "{\"success\":false,\"message\":\"Failed to create file\"}", 52);
  }
  
  isRecording = true;
  frameCount = 0;
  
  return httpd_resp_send(req, "{\"success\":true}", 16);
}

// Recording stop handler
static esp_err_t record_stop_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  
  if (!isRecording) {
    return httpd_resp_send(req, "{\"success\":false,\"message\":\"Not recording\"}", 44);
  }
  
  char filename[100];
  strcpy(filename, videoFile.name());
  videoFile.close();
  isRecording = false;
  
  char json[150];
  snprintf(json, sizeof(json), "{\"success\":true,\"filename\":\"%s\",\"frames\":%d}", filename, frameCount);
  
  return httpd_resp_send(req, json, strlen(json));
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t led_uri = {
    .uri = "/led/toggle",
    .method = HTTP_GET,
    .handler = led_handler,
    .user_ctx = NULL
  };

  httpd_uri_t capture_uri = {
    .uri = "/capture",
    .method = HTTP_GET,
    .handler = capture_handler,
    .user_ctx = NULL
  };

  httpd_uri_t motion_uri = {
    .uri = "/motion",
    .method = HTTP_GET,
    .handler = motion_handler,
    .user_ctx = NULL
  };

  httpd_uri_t buzzer_uri = {
    .uri = "/buzzer/test",
    .method = HTTP_GET,
    .handler = buzzer_handler,
    .user_ctx = NULL
  };

  httpd_uri_t sensitivity_uri = {
    .uri = "/motion/sensitivity",
    .method = HTTP_GET,
    .handler = sensitivity_handler,
    .user_ctx = NULL
  };

  httpd_uri_t status_uri = {
    .uri = "/status",
    .method = HTTP_GET,
    .handler = status_handler,
    .user_ctx = NULL
  };

  httpd_uri_t record_start_uri = {
    .uri = "/record/start",
    .method = HTTP_GET,
    .handler = record_start_handler,
    .user_ctx = NULL
  };

  httpd_uri_t record_stop_uri = {
    .uri = "/record/stop",
    .method = HTTP_GET,
    .handler = record_stop_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &led_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &motion_uri);
    httpd_register_uri_handler(camera_httpd, &buzzer_uri);
    httpd_register_uri_handler(camera_httpd, &sensitivity_uri);
    httpd_register_uri_handler(camera_httpd, &status_uri);
    httpd_register_uri_handler(camera_httpd, &record_start_uri);
    httpd_register_uri_handler(camera_httpd, &record_stop_uri);
  }

  config.server_port = 81;
  config.ctrl_port = 32769;
  
  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
  
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  Serial.println("\n\n\n");
  Serial.println("========================================");
  Serial.println("    ESP32-CAM SMART DOORBELL v1.0");
  Serial.println("========================================");
  Serial.println("System Information:");
  Serial.printf("  Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("  CPU Frequency: %dMHz\n", ESP.getCpuFreqMHz());
  Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("  PSRAM: %s\n", psramFound() ? "Available" : "Not found");
  Serial.println("========================================");
  Serial.println("Initializing system...");
  Serial.println();

  // Configure pins
  Serial.println("Configuring GPIO pins...");
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("  GPIO 12 - LEDs (Output)");
  Serial.println("  GPIO 13 - Button (Input)");
  Serial.println("  GPIO 14 - Buzzer (Output)");
  Serial.println("✓ GPIO configured\n");

  // Attach button interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressed, FALLING);
  Serial.println("✓ Button interrupt attached\n");

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize camera
  Serial.println("Initializing camera...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("========================================");
    Serial.println("✗ CAMERA INIT FAILED!");
    Serial.println("========================================");
    Serial.printf("Error code: 0x%x\n", err);
    Serial.println("Please check:");
    Serial.println("1. Camera module is connected properly");
    Serial.println("2. Ribbon cable is inserted correctly");
    Serial.println("3. Power supply is adequate (5V 2A)");
    Serial.println("========================================");
    return;
  }
  Serial.println("✓ Camera initialized successfully\n");

  // Initialize SD card
  Serial.println("Checking for SD card...");
  if (!SD_MMC.begin()) {
    Serial.println("✗ SD Card Mount Failed (or not inserted)");
    Serial.println("  Recording features will be disabled\n");
    sdCardAvailable = false;
  } else {
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("✗ No SD card detected");
      Serial.println("  Recording features will be disabled\n");
      sdCardAvailable = false;
    } else {
      Serial.println("✓ SD Card initialized successfully");
      sdCardAvailable = true;
      uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
      Serial.printf("  Card Size: %lluMB\n", cardSize);
      Serial.printf("  Card Type: %s\n", 
        cardType == CARD_MMC ? "MMC" : 
        cardType == CARD_SD ? "SD" : 
        cardType == CARD_SDHC ? "SDHC" : "UNKNOWN");
      Serial.println();
    }
  }

  // Connect to WiFi
  Serial.println("\n========================================");
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Serial.println("========================================");
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("========================================");
    Serial.println("✓ WiFi CONNECTED!");
    Serial.println("========================================");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Access doorbell at: http://");
    Serial.println(WiFi.localIP());
    Serial.println("========================================\n");
  } else {
    Serial.println("========================================");
    Serial.println("✗ WiFi CONNECTION FAILED!");
    Serial.println("========================================");
    Serial.println("Please check:");
    Serial.println("1. SSID is correct");
    Serial.println("2. Password is correct");
    Serial.println("3. Router is 2.4GHz (not 5GHz)");
    Serial.println("========================================");
    while(1); // Stop here if WiFi fails
  }

  // Start web server
  startCameraServer();
  
  Serial.println("========================================");
  Serial.println("✓ SYSTEM READY!");
  Serial.println("========================================");
  Serial.println("Features Available:");
  Serial.println("  • Live Video Streaming");
  Serial.println("  • Motion Detection");
  Serial.println("  • Snapshot Capture");
  Serial.println("  • LED Control");
  Serial.println("  • Doorbell Button");
  if (sdCardAvailable) {
    Serial.println("  • Video Recording");
  }
  Serial.println();
  Serial.println("Web Interface:");
  Serial.print("  http://");
  Serial.println(WiFi.localIP());
  Serial.println("========================================\n");
  
  // Welcome chime
  playDingDong();
  Serial.println("System startup complete! 🔔\n");
}

void loop() {
  // Capture frame for motion detection
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb) {
    // Check for motion
    if (detectMotion(fb)) {
      motionDetected = true;
      Serial.println("Motion detected!");
    }
    
    // If recording, save frame
    if (isRecording && sdCardAvailable && videoFile) {
      // Write JPEG data
      videoFile.write(fb->buf, fb->len);
      frameCount++;
      
      // Optional: limit recording time or file size
      if (frameCount > 3000) { // ~5 minutes at 10fps
        videoFile.close();
        isRecording = false;
        Serial.println("Recording stopped - max frames reached");
      }
    }
    
    esp_camera_fb_return(fb);
  }
  
  delay(100); // Adjust for desired frame rate
}

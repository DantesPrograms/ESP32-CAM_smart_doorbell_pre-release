# ESP32-CAM Doorbell - Setup Guide

## 📋 Components Required

### Main Components:
- **ESP32-CAM** (AI-Thinker module)
- **FTDI Programmer** or USB-to-Serial adapter (for uploading code)
- **2x White LEDs** (5mm)
- **Passive Buzzer** (3-5V)
- **Push Button** (momentary, normally open)
- **MicroSD Card** (optional, for video recording)
- **5V Power Supply** (2A recommended)

### Resistors & Components:
- **2x 220Ω Resistors** (for LEDs)
- **1x 10kΩ Resistor** (pull-up for button)
- **Jumper wires**
- **Breadboard** (for prototyping)

---

## 🔌 Pin Connections

### ESP32-CAM Pinout:

```
┌─────────────────────────────────┐
│        ESP32-CAM (Top View)     │
│                                 │
│  [Camera]                       │
│                                 │
│  GND  5V  GPIO12 GPIO13 GPIO15  │
│  IO14 IO2 IO4   GND     VCC     │
└─────────────────────────────────┘
```

### Wiring Diagram:

**Button:**
- One side → GPIO 13
- Other side → GND
- 10kΩ pull-up resistor from GPIO 13 to 3.3V (internal pull-up can also be used)

**LEDs (both in parallel):**
- LED Anode (+) → GPIO 12 → 220Ω resistor
- LED Cathode (-) → GND

**Passive Buzzer:**
- Positive (+) → GPIO 14
- Negative (-) → GND

**Power:**
- 5V → 5V pin
- GND → GND pin

**SD Card:** (Insert into built-in SD card slot)
- Uses GPIO 2, 4, 12, 13, 14, 15 (managed automatically)
- Note: GPIO 14 shared with buzzer - this works fine as SD only uses it during init

---

## 💻 Software Setup

### 1. Install Arduino IDE
- Download from: https://www.arduino.cc/en/software
- Install version 2.0 or higher

### 2. Add ESP32 Board Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "esp32" and install "esp32 by Espressif Systems"

### 3. Select Board and Port
- **Board:** AI Thinker ESP32-CAM
- **Port:** Select the COM port of your FTDI adapter

### 4. Required Libraries
All libraries come pre-installed with ESP32 board support:
- esp_camera
- WiFi
- esp_http_server
- SD_MMC
- FS

---

## 📤 Upload Process

### Wiring for Programming:
```
FTDI → ESP32-CAM
VCC (5V) → 5V
GND → GND
TX → U0R (GPIO 3)
RX → U0T (GPIO 1)
GPIO 0 → GND (for programming mode)
```

### Steps:
1. Connect FTDI to ESP32-CAM with GPIO 0 connected to GND
2. Open `esp32_doorbell.ino` in Arduino IDE
3. **IMPORTANT:** Update WiFi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
4. Click **Upload** button
5. Wait for "Connecting......" message
6. Press the RESET button on ESP32-CAM
7. Wait for upload to complete
8. **Disconnect GPIO 0 from GND**
9. Press RESET button to run the program

### Troubleshooting Upload Issues:
- If "Connecting......" hangs, press and hold RESET, then release
- Ensure GPIO 0 is connected to GND during upload
- Check TX/RX are not swapped
- Try reducing upload speed: Tools → Upload Speed → 115200

---

## 🚀 First Boot

### What to Expect:
1. Power up the ESP32-CAM (disconnect FTDI, use 5V power supply)
2. Open Serial Monitor (115200 baud)
3. You should see:
   ```
   ESP32-CAM Doorbell Starting...
   Connecting to WiFi....
   WiFi connected
   Camera Ready! Access at: http://192.168.x.x
   SD Card initialized successfully
   SD Card Size: 8GB
   ```
4. You'll hear a "ding-dong" welcome chime
5. Note the IP address shown

---

## 🌐 Web Interface Usage

### Access:
1. Open a web browser on any device connected to the same WiFi
2. Navigate to: `http://192.168.x.x` (use the IP from Serial Monitor)

### Features:

**Live Video Stream:**
- Automatically loads on the main page
- Real-time MJPEG stream at configurable quality

**Toggle LEDs:**
- Click to turn LEDs ON/OFF manually
- LEDs illuminate when viewing the stream

**Capture Snapshot:**
- Takes a single high-quality JPEG image
- Opens in a new tab for download/save

**Start/Stop Recording:** (requires SD card)
- Records video in MJPEG format
- Manual control - press to start, press again to stop
- Filename includes timestamp
- Shows frame count when stopped

**Test Buzzer:**
- Plays the "ding-dong" chime on demand
- Same sound as pressing the physical button

**Motion Detection:**
- Continuously monitors for movement
- Displays alert banner when motion detected
- Adjustable sensitivity slider (5-50%)

**System Status Panel:**
- Stream status
- LED status
- Motion detection status
- Recording status
- SD card availability

---

## 🎥 Recording to SD Card

### SD Card Requirements:
- **Format:** FAT32
- **Size:** Up to 32GB recommended
- **Class:** Class 10 or higher for smooth recording

### Format SD Card:
**Windows:**
1. Right-click SD card → Format
2. File system: FAT32
3. Allocation size: Default
4. Click Start

**Mac:**
1. Open Disk Utility
2. Select SD card
3. Click Erase
4. Format: MS-DOS (FAT)
5. Scheme: Master Boot Record

**Linux:**
```bash
sudo mkfs.vfat -F 32 /dev/sdX1
```

### Recording Process:
1. Insert formatted SD card into ESP32-CAM slot (power off first)
2. Power on - verify "SD Card initialized" in Serial Monitor
3. Access web interface
4. Click "Start Recording" button
5. Record your video
6. Click "Stop Recording"
7. Note the filename displayed (e.g., "video_123456.mjpeg")

### Retrieve Recordings:
1. Power off ESP32-CAM
2. Remove SD card
3. Insert into computer
4. Files are in root directory: `video_XXXXXX.mjpeg`

### Play MJPEG Files:
- **VLC Media Player:** File → Open → Select .mjpeg file
- **ffmpeg conversion to MP4:**
  ```bash
  ffmpeg -i video_123456.mjpeg -c:v libx264 output.mp4
  ```

---

## ⚙️ Configuration & Customization

### Adjust Camera Settings:
In `setup()` function, modify:
```cpp
config.frame_size = FRAMESIZE_UXGA;  // Options: QVGA, VGA, SVGA, XGA, SXGA, UXGA
config.jpeg_quality = 10;             // 0-63 (lower = better quality)
```

### Motion Detection Sensitivity:
- Adjust via web interface slider (5-50%)
- Or in code:
```cpp
int motionThreshold = 20;  // Default 20%, higher = less sensitive
```

### Buzzer Tones:
Modify `playDingDong()` function:
```cpp
void playDingDong() {
  tone(BUZZER_PIN, 800, 200);  // Frequency (Hz), Duration (ms)
  delay(250);
  tone(BUZZER_PIN, 600, 300);
  delay(350);
  noTone(BUZZER_PIN);
}
```

### Recording Frame Limit:
```cpp
if (frameCount > 3000) {  // Change 3000 to desired max frames
```

---

## 🔧 Advanced Features

### Enable Camera Settings:
Add to web interface for:
- Brightness
- Contrast
- Saturation
- Special effects

Example:
```cpp
sensor_t * s = esp_camera_sensor_get();
s->set_brightness(s, 0);     // -2 to 2
s->set_contrast(s, 0);       // -2 to 2
s->set_saturation(s, 0);     // -2 to 2
```

### Multiple Doorbells:
- Change web server ports in code
- Access via: `http://IP:PORT`

### Night Vision:
- ESP32-CAM supports IR LEDs
- Replace white LEDs with IR LEDs
- Camera will capture in low light

---

## 🐛 Troubleshooting

### Camera Init Failed:
- Check all camera module connections
- Ensure sufficient power (5V 2A)
- Try lowering frame size/quality

### WiFi Won't Connect:
- Verify SSID and password
- Check 2.4GHz WiFi (5GHz not supported)
- Ensure signal strength is adequate

### SD Card Not Detected:
- Remove and reinsert card
- Verify FAT32 format
- Try different SD card
- Check Serial Monitor for error messages

### Brownout Detector Reset:
- Occurs with insufficient power
- Use 5V 2A power supply
- Add 100µF capacitor across power pins

### Motion Detection Too Sensitive:
- Increase threshold via web slider
- Reduce to 30-40% for less sensitivity

### Recording Choppy:
- Use faster SD card (Class 10+)
- Reduce frame size
- Lower JPEG quality (higher number)

### LEDs Too Dim:
- Check resistor values (220Ω max)
- Use brighter/higher quality LEDs
- Parallel connection distributes current

---

## 📊 Performance Specs

- **Video Resolution:** Up to 1600×1200 (UXGA)
- **Frame Rate:** ~10-15 FPS (depends on quality/size)
- **Streaming:** MJPEG over HTTP
- **Recording Format:** MJPEG (convert to MP4 with ffmpeg)
- **Motion Detection:** Frame difference algorithm
- **Power Consumption:** ~200-300mA (active), ~100mA (idle)
- **WiFi Range:** Standard 802.11 b/g/n
- **Operating Voltage:** 5V
- **Operating Temperature:** -20°C to 85°C

---

## 🔒 Security Notes

**Important:** This is a basic implementation for home/learning use.

For production deployment, consider:
- **Authentication:** Add password protection to web interface
- **HTTPS:** Use SSL/TLS encryption
- **Network:** Place on isolated VLAN
- **Firmware:** Keep ESP32 board package updated

---

## 📝 Bill of Materials (BOM)

| Component | Quantity | Est. Cost |
|-----------|----------|-----------|
| ESP32-CAM Module | 1 | $10-15 |
| FTDI Programmer | 1 | $5-10 |
| White LEDs (5mm) | 2 | $0.50 |
| Passive Buzzer | 1 | $1-2 |
| Push Button | 1 | $0.50 |
| 220Ω Resistors | 2 | $0.10 |
| 10kΩ Resistor | 1 | $0.05 |
| MicroSD Card (8-32GB) | 1 | $5-10 |
| 5V 2A Power Supply | 1 | $5-8 |
| Breadboard | 1 | $3-5 |
| Jumper Wires | Set | $2-5 |
| **Total** | | **~$32-60** |

---

## 🎯 Future Enhancements

- Add MQTT for smart home integration
- Implement push notifications
- Face detection/recognition
- Cloud storage integration
- Battery backup
- Weatherproof enclosure design
- Multiple camera support
- Time-lapse recording

---

## 📚 Resources

- ESP32-CAM Documentation: https://github.com/espressif/esp32-camera
- Arduino ESP32: https://docs.espressif.com/projects/arduino-esp32/
- Troubleshooting: https://randomnerdtutorials.com/esp32-cam-troubleshooting-guide/

---

**Made with ❤️ for the maker community!**

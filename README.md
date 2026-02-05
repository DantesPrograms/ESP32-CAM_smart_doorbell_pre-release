# 🔔 ESP32-CAM Smart Doorbell

A feature-rich smart doorbell system built with ESP32-CAM, featuring live video streaming, motion detection, manual recording, and a sleek dark-themed web interface.

![ESP32-CAM](https://img.shields.io/badge/ESP32--CAM-AI--Thinker-blue)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D)
![License](https://img.shields.io/badge/License-MIT-green)

## ✨ Features

- 📹 **Live Video Streaming** - Real-time MJPEG stream accessible via web browser
- 🚨 **Motion Detection** - Intelligent frame-difference motion detection with adjustable sensitivity
- 💾 **SD Card Recording** - Manual video recording in MJPEG format
- 💡 **LED Illumination** - Two white LEDs for enhanced visibility
- 🔔 **Doorbell Chime** - Passive buzzer plays "ding-dong" melody when button pressed
- 📸 **Snapshot Capture** - High-quality JPEG image capture on demand
- 🌙 **Dark Theme UI** - Beautiful, responsive web interface with gradient design
- 📱 **Mobile Friendly** - Works seamlessly on desktop, tablet, and mobile devices
- ⚙️ **Real-time Status** - Live monitoring of system components and recording state

## 🎥 Demo

Access the web interface to:
- View live camera feed
- Toggle LEDs on/off
- Capture snapshots
- Start/stop recording (with SD card)
- Adjust motion sensitivity
- Test doorbell buzzer
- Monitor system status

## 🛠️ Hardware Requirements

### Core Components
- **ESP32-CAM** (AI-Thinker module)
- **FTDI Programmer** (for code upload)
- **2x White LEDs** (5mm)
- **Passive Buzzer** (3-5V)
- **Push Button** (momentary)
- **MicroSD Card** (optional, for recording)
- **5V 2A Power Supply**

### Electronic Components
- 2x 220Ω resistors (for LEDs)
- 1x 10kΩ resistor (pull-up for button)
- Jumper wires
- Breadboard

**Total Cost:** ~$32-60 USD

## 📐 Wiring Diagram

```
ESP32-CAM Connections:
├─ GPIO 13 → Push Button → GND
├─ GPIO 12 → 220Ω → LED 1 (+) → GND
│           └─ 220Ω → LED 2 (+) → GND
├─ GPIO 14 → Buzzer (+) → GND
├─ 5V → Power Supply (+)
└─ GND → Power Supply (-)
```

**For detailed wiring diagrams, see [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)**

## 🚀 Quick Start

### 1. Install Arduino IDE
Download and install [Arduino IDE 2.0+](https://www.arduino.cc/en/software)

### 2. Add ESP32 Board Support
1. Open Arduino IDE → File → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board → Boards Manager → Install "esp32 by Espressif Systems"

### 3. Configure WiFi
Edit `esp32_doorbell.ino`:
```cpp
const char* ssid = "YOUR_WIFI_SSID";        // Your WiFi network name
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

### 4. Upload Code
1. Connect FTDI programmer to ESP32-CAM:
   - FTDI VCC → ESP32 5V
   - FTDI GND → ESP32 GND
   - FTDI TX → ESP32 U0R
   - FTDI RX → ESP32 U0T
   - **ESP32 GPIO 0 → GND** (programming mode)

2. Select board: **Tools → Board → AI Thinker ESP32-CAM**
3. Select port: **Tools → Port → (your COM port)**
4. Click **Upload**
5. Press RESET button when "Connecting..." appears
6. **After upload, disconnect GPIO 0 from GND**
7. Press RESET to run

### 5. Access Web Interface
1. Open Serial Monitor (115200 baud)
2. Note the IP address displayed (e.g., `192.168.1.100`)
3. Open browser and navigate to `http://192.168.1.100`
4. Enjoy your smart doorbell!

## 📱 Web Interface

The web interface provides:

### Live Controls
- **Toggle LEDs** - Turn camera illumination on/off
- **Capture Snapshot** - Take a high-res JPEG photo
- **Start/Stop Recording** - Record video to SD card
- **Test Buzzer** - Play the doorbell chime

### System Status
- Stream status indicator
- LED on/off status
- Motion detection status
- Recording status
- SD card availability

### Advanced Features
- Motion sensitivity slider (5-50%)
- Real-time motion alerts
- Frame counter during recording
- Responsive layout for all devices

## 💾 SD Card Recording

### Setup
1. Format SD card as **FAT32**
2. Insert into ESP32-CAM SD slot (power off first)
3. Power on and verify "SD Card initialized" message

### Recording
1. Click **"Start Recording"** in web interface
2. Record your video
3. Click **"Stop Recording"**
4. Files saved as `video_XXXXXX.mjpeg`

### Playback
- **VLC Media Player**: File → Open File
- **Convert to MP4**:
  ```bash
  ffmpeg -i video_123456.mjpeg -c:v libx264 output.mp4
  ```

**Recommended SD Card:** Class 10, 8-32GB, FAT32 format

## ⚙️ Configuration

### Camera Quality
```cpp
config.frame_size = FRAMESIZE_UXGA;  // QVGA, VGA, SVGA, XGA, SXGA, UXGA
config.jpeg_quality = 10;             // 0-63 (lower = better quality)
```

### Motion Sensitivity
Adjust via web interface slider or in code:
```cpp
int motionThreshold = 20;  // 5-50 (higher = less sensitive)
```

### Buzzer Melody
Customize in `playDingDong()`:
```cpp
tone(BUZZER_PIN, 800, 200);  // Frequency, Duration
delay(250);
tone(BUZZER_PIN, 600, 300);
```

### Recording Limits
```cpp
if (frameCount > 3000) {  // Max frames (~5 min at 10fps)
```

## 🔧 Troubleshooting

### Camera Init Failed
- ✅ Check camera module is firmly connected
- ✅ Use 5V 2A power supply (insufficient power is common issue)
- ✅ Lower frame size: `FRAMESIZE_VGA`

### WiFi Won't Connect
- ✅ Verify SSID and password are correct
- ✅ Ensure 2.4GHz WiFi (5GHz not supported)
- ✅ Check signal strength

### SD Card Not Detected
- ✅ Format as FAT32
- ✅ Try different SD card (Class 10+)
- ✅ Check Serial Monitor for errors

### Brownout Detector
- ✅ Use quality 5V 2A power supply
- ✅ Add 100µF capacitor across power pins
- ✅ Avoid long/thin power wires

### Upload Failed
- ✅ GPIO 0 must be connected to GND during upload
- ✅ Press RESET when "Connecting..." appears
- ✅ Check TX/RX connections aren't swapped
- ✅ Try lower upload speed: Tools → Upload Speed → 115200

**For detailed troubleshooting, see [SETUP_GUIDE.md](SETUP_GUIDE.md)**

## 📊 Performance Specifications

| Specification | Value |
|---------------|-------|
| Max Resolution | 1600×1200 (UXGA) |
| Frame Rate | 10-15 FPS |
| Streaming Format | MJPEG over HTTP |
| Recording Format | MJPEG (convertible to MP4) |
| Motion Detection | Frame difference algorithm |
| Power Consumption | 200-300mA (active), 100mA (idle) |
| WiFi | 802.11 b/g/n (2.4GHz) |
| Operating Voltage | 5V DC |
| Operating Temperature | -20°C to 85°C |

## 📁 Project Structure

```
esp32-cam-doorbell/
├── esp32_doorbell.ino      # Main Arduino sketch
├── README.md               # This file
├── SETUP_GUIDE.md          # Detailed setup instructions
└── WIRING_DIAGRAM.md       # Circuit diagrams and wiring
```

## 🔒 Security Considerations

**⚠️ Important:** This is a basic implementation for home/educational use.

For production deployment:
- ✅ Add password authentication to web interface
- ✅ Implement HTTPS/SSL encryption
- ✅ Use isolated VLAN for camera
- ✅ Keep firmware updated
- ✅ Change default credentials

## 🎯 Future Enhancements

Possible improvements:
- [ ] MQTT integration for Home Assistant
- [ ] Push notifications (Telegram, email)
- [ ] Face detection/recognition
- [ ] Cloud storage integration (Google Drive, AWS S3)
- [ ] Battery backup with deep sleep
- [ ] Weatherproof enclosure design
- [ ] Two-way audio communication
- [ ] Time-lapse recording mode
- [ ] Multiple camera support
- [ ] Person detection using TensorFlow Lite

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest new features
- Submit pull requests
- Share your builds

## 📄 License

This project is open source and available under the MIT License.

## 🙏 Acknowledgments

- ESP32 community for excellent documentation
- Arduino team for the IDE and libraries
- Random Nerd Tutorials for ESP32-CAM guides
- All makers who share their projects online

## 📚 Additional Resources

- [ESP32-CAM Official Documentation](https://github.com/espressif/esp32-camera)
- [Arduino ESP32 Guide](https://docs.espressif.com/projects/arduino-esp32/)
- [Random Nerd Tutorials - ESP32-CAM](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/)
- [ESP32 Forum](https://www.esp32.com/)

## 📞 Support

Having issues? Check these resources:
1. Read [SETUP_GUIDE.md](SETUP_GUIDE.md) for detailed instructions
2. Review [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md) for correct connections
3. Check the troubleshooting section above
4. Search existing issues on GitHub
5. Create a new issue with:
   - Detailed description
   - Serial Monitor output
   - Photos of your setup

## 🌟 Show Your Support

If you found this project helpful:
- ⭐ Star this repository
- 🔄 Share with other makers
- 📸 Post your build on social media
- 💬 Leave feedback

---

**Built with ❤️ for the maker community**

*Happy making! 🚀*

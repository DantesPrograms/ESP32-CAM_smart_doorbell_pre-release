# ESP32-CAM Doorbell - Wiring Diagram

## Complete Circuit Diagram

```
                                    ESP32-CAM MODULE
                           ┌────────────────────────────────┐
                           │                                │
                           │         [Camera Lens]          │
                           │                                │
                           │  ┌──────────────────────────┐  │
                           │  │   AI-Thinker ESP32-CAM   │  │
                           │  └──────────────────────────┘  │
                           │                                │
        5V Power ────────► │ 5V                        GND  │ ◄──── GND
                           │                                │
                           │                                │
    ┌─────────────────────►│ GPIO 13 (Button)              │
    │                      │                                │
    │  ┌──────────────────►│ GPIO 12 (LEDs)                │
    │  │                   │                                │
    │  │  ┌───────────────►│ GPIO 14 (Buzzer)              │
    │  │  │                │                                │
    │  │  │                │ GND                            │◄──┐
    │  │  │                │                                │   │
    │  │  │                │ [SD Card Slot]                 │   │
    │  │  │                │  Insert microSD card here      │   │
    │  │  │                └────────────────────────────────┘   │
    │  │  │                                                     │
    │  │  │                                                     │
    │  │  │    BUTTON CIRCUIT                                  │
    │  │  │    ┌──────────┐                                    │
    └──┼──┼────┤  Button  ├────────────────────────────────────┘
       │  │    └──────────┘    (Momentary Push Button)
       │  │
       │  │    LED CIRCUIT
       │  │    ┌────────────┐
       │  └────┤   220Ω     ├──┐
       │       └────────────┘  │
       │                       │
       │       LED 1           │
       │       ┌───────┐       │
       │    ┌──┤  ───▶ ├──┐   │
       │    │  └───────┘  │   │
       │    │    White    │   │
       │    │             │   │
       │    │  ┌────────┐ │   │
       │    └──┤  220Ω  ├─┘   │
       │       └────────┘     │
       │                      │
       │       LED 2          │
       │       ┌───────┐      │
       │    ┌──┤  ───▶ ├──┐  │
       │    │  └───────┘  │  │
       │    │    White    │  │
       │    └─────────────┴──┴─────── GND
       │
       │    BUZZER CIRCUIT
       │    ┌──────────────┐
       └────┤ Passive      │
            │ Buzzer   (+) │
            └──────────────┘
                   │
                   │ (-)
                   │
                   └──────────────────────── GND


POWER SUPPLY
┌─────────────┐
│   5V 2A     │
│ Power       │
│ Adapter     │
└─────────────┘
   │      │
   │      └──────────────────────────────── GND (Common Ground)
   │
   └─────────────────────────────────────── 5V


PROGRAMMING CONNECTIONS (FTDI)
┌──────────────┐              ┌────────────────┐
│     FTDI     │              │   ESP32-CAM    │
│              │              │                │
│   VCC (5V)   ├─────────────►│ 5V             │
│   GND        ├─────────────►│ GND            │
│   TX         ├─────────────►│ U0R (GPIO 3)   │
│   RX         ├◄─────────────┤ U0T (GPIO 1)   │
│              │              │                │
└──────────────┘              │ GPIO 0 ──────► GND (for programming)
                              │                │
                              └────────────────┘

NOTE: After programming, disconnect GPIO 0 from GND!
```

---

## Breadboard Layout (Top View)

```
                 Breadboard
    ┌────────────────────────────────────┐
    │  +  -                         +  - │
    │  ═  ═                         ═  ═ │
    │  │  │                         │  │ │
    │  │  │     ESP32-CAM           │  │ │
    │  │  │    ┌──────────┐         │  │ │
    │  │  │    │          │         │  │ │
    │  ●──┼────┤ 5V   GND ├─────────┼──● │  Power Rails
    │     │    │          │         │    │
    │     │    │ IO12 IO13│         │    │
    │     │    │          │         │    │
    │     │    │ IO14     │         │    │
    │     │    └──────────┘         │    │
    │     │         │               │    │
    │     │         │ Button        │    │
    │     │    ┌────●────┐          │    │
    │     │    │         │          │    │
    │     │    └─────────●──────────┘    │
    │     │                               │
    │     │    LED 1   LED 2              │
    │     │     ┌─┐     ┌─┐              │
    │     ●─220Ω┤▶├─┬─220Ω┤▶├─┐          │
    │     │     └─┘ │   └─┘   │          │
    │     │         └─────────●──────────┤
    │     │                              │
    │     │    Buzzer                    │
    │     │    ┌─────┐                   │
    │     ●────┤  ~  ├───●───────────────┤
    │          └─────┘                   │
    │                                    │
    └────────────────────────────────────┘

Legend:
  ● = Connection point
  ═ = Power/Ground rail
  ┤▶├ = LED
  ~ = Buzzer
```

---

## Pin Function Summary

| GPIO | Function | Component | Notes |
|------|----------|-----------|-------|
| 12 | Output | White LEDs | Both LEDs in parallel |
| 13 | Input | Push Button | Internal pull-up enabled |
| 14 | Output | Passive Buzzer | PWM for tones |
| 0 | Program Mode | - | Connect to GND only during upload |
| 1 (U0T) | Serial TX | FTDI RX | For programming |
| 3 (U0R) | Serial RX | FTDI TX | For programming |
| 2,4,12,13,14,15 | SD Card | SD Slot | Auto-managed by SD_MMC |

---

## Component Specifications

### LEDs:
- **Type:** White 5mm LED
- **Forward Voltage:** ~3.2V
- **Current:** 20mA per LED
- **Resistor:** 220Ω (standard value)
- **Connection:** Parallel (both share same GPIO)

### Button:
- **Type:** Tactile momentary switch
- **Rating:** 12V 50mA minimum
- **Debounce:** Handled in software (500ms)
- **Pull-up:** Internal (INPUT_PULLUP mode)

### Buzzer:
- **Type:** Passive buzzer (not active!)
- **Voltage:** 3-5V
- **Frequency Range:** 20Hz - 20kHz
- **Connection:** Direct to GPIO (no resistor needed)
- **Note:** Must be passive type for tone() function

### Power Supply:
- **Voltage:** 5V DC
- **Current:** 2A minimum recommended
- **Connector:** Can use USB power, barrel jack, or direct wires
- **Stability:** Use quality supply to prevent brownout

### SD Card:
- **Format:** FAT32
- **Capacity:** Up to 32GB recommended
- **Speed Class:** Class 10 or UHS-1
- **Connection:** Built-in slot on ESP32-CAM

---

## Assembly Steps

### Step 1: Prepare Components
1. Identify all components
2. Check LED polarity (longer leg = positive)
3. Identify buzzer polarity (usually marked + or has red wire)

### Step 2: Mount ESP32-CAM
1. Place ESP32-CAM on breadboard
2. Ensure pins are properly seated
3. Camera should face outward

### Step 3: Connect Power Rails
1. Connect 5V supply to positive (+) rail
2. Connect GND to negative (-) rail
3. Wire ESP32-CAM 5V to + rail
4. Wire ESP32-CAM GND to - rail

### Step 4: Install Button
1. Place button across breadboard gap
2. Connect one side to GPIO 13
3. Connect other side to GND rail
4. (Optional: Add external 10kΩ pull-up to 3.3V)

### Step 5: Install LEDs
1. Insert LED 1 with anode (long leg) toward resistor
2. Connect 220Ω resistor from GPIO 12 to LED anode
3. Connect LED cathode to GND rail
4. Repeat for LED 2 in parallel

### Step 6: Install Buzzer
1. Identify buzzer positive (+) terminal
2. Connect buzzer + to GPIO 14
3. Connect buzzer - to GND rail

### Step 7: Insert SD Card (Optional)
1. Format SD card to FAT32
2. Insert into SD slot on ESP32-CAM
3. Ensure card is fully seated

### Step 8: Double-Check Connections
1. Verify all power connections
2. Check LED polarity
3. Confirm buzzer polarity
4. Ensure no short circuits

---

## Testing Checklist

- [ ] 5V power supply connected correctly
- [ ] All GND connections to common ground
- [ ] LED orientation correct (not reversed)
- [ ] Buzzer polarity correct
- [ ] Button properly connected to GPIO 13
- [ ] SD card formatted and inserted (if using)
- [ ] No loose wires or shorts
- [ ] Camera module secure

---

## Common Wiring Mistakes

❌ **Wrong LED Polarity**
- LEDs only work one way!
- Long leg (anode) must go to GPIO side
- Short leg (cathode) to GND

❌ **Active vs Passive Buzzer**
- Must use PASSIVE buzzer
- Active buzzers only make one tone
- Passive needed for melody

❌ **Insufficient Power**
- Use at least 2A power supply
- ESP32-CAM draws significant current
- Weak supply causes brownout resets

❌ **Missing Resistors**
- Always use current-limiting resistors for LEDs
- 220Ω is standard for 5V supply
- Without resistor, LEDs will burn out

❌ **GPIO 0 Left Connected**
- GPIO 0 to GND is ONLY for programming
- Must disconnect after upload
- Device won't run normally if still grounded

---

## Enclosure Recommendations

For permanent installation, consider:

1. **3D Printed Case**
   - ESP32-CAM has many free designs on Thingiverse
   - Include cutouts for camera, button, LEDs
   - Ventilation holes for heat dissipation

2. **Weatherproof Box**
   - IP65 rated junction box
   - Clear acrylic window for camera
   - Cable glands for wiring

3. **Mounting**
   - Use standoffs to prevent shorts
   - Secure with hot glue or screws
   - Ensure camera has clear view

---

## Safety Notes

⚠️ **Electrical Safety**
- Disconnect power before wiring changes
- Use proper gauge wire for current
- Ensure no exposed connections
- Keep away from water unless weatherproofed

⚠️ **Component Ratings**
- Don't exceed component voltage ratings
- LEDs: Max 3.5V forward voltage
- ESP32-CAM: 5V input only (not 12V!)
- Buzzer: Check datasheet for max voltage

---

**Visual Reference Photos:**
For real photos of ESP32-CAM pinouts and proper connections, search online for:
- "ESP32-CAM pinout diagram"
- "ESP32-CAM breadboard wiring"
- "ESP32-CAM AI-Thinker pins"

Many excellent photo guides available from Random Nerd Tutorials and other maker sites!

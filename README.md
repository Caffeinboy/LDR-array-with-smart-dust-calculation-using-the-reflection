# Smart Dust calculation using the reflection by the LDR array

An intelligent solar panel cleaning system focused on **accurate dust detection using a 3-LDR + 3-LED reflective sensing array**, combined with automatic cleaning using an ESP32 (Master) and Arduino UNO (Slave).

Unlike simple time-based cleaning systems, this project uses an **LED-LDR reflection mechanism** to determine actual dust accumulation on the solar panel surface before triggering cleaning. This improves water efficiency, reduces unnecessary motor usage, and increases overall system reliability.

The ESP32 continuously monitors panel cleanliness, sends live updates to Blynk IoT, and triggers the cleaning cycle only when real dust buildup is confirmed.

---

# Core Concept — LDR + LED Dust Detection Array

This project is primarily built around a **3-point reflective dust sensing system**.

## How It Works

Each sensing unit contains:

* 1 LED (light source)
* 1 LDR (light dependent resistor)

Total:

* 3 LEDs
* 3 LDRs

The LEDs are placed to illuminate the solar panel surface, while the LDRs measure the reflected light intensity.

---

## Reflection-Based Dust Detection Principle

### Clean Panel

When the panel is clean:

* More LED light reflects properly
* LDR receives stronger reflected light
* Reflection value is HIGH
* Dust percentage remains LOW

---

### Dusty Panel

When dust accumulates:

* Reflection reduces significantly
* LDR receives less reflected light
* Reflection value becomes LOW
* Dust percentage increases

---

## Dust Calculation Formula

```text id="dustformula01"
Dust % = ((Clean Reference - Current Reflection) / Clean Reference) × 100
```

This allows the system to clean only when genuinely required instead of using fixed timers.

This is the most important mechanism of the entire project.

---

# Project Architecture

---

# ESP32 (Master Controller)

Responsible for:

* 3-LDR + 3-LED dust sensing array
* Reflection-based dust percentage calculation
* EMA smoothing for stable readings
* False trigger filtering
* Automatic calibration using Blynk (V20)
* EEPROM storage for clean reference values
* Sending `START` command to Arduino UNO
* Receiving `DIST:value` and `DONE`
* Blynk cloud monitoring and live status updates

The ESP32 acts as the **decision-making brain** of the system.

---

# Arduino UNO (Slave Controller)

Responsible for:

* Receiving `START` command from ESP32
* Pump relay control (D10)
* Pump ON for 5 seconds first
* Pump OFF after spray cycle
* Motor forward movement using L298N
* Ultrasonic distance-based end detection
* Automatic reverse return cycle
* Sending real-time distance updates
* Sending `DONE` after full cleaning cycle

The Arduino UNO acts as the **execution controller** for heavy-load devices.

---

# Full Working Logic

---

## Step 1 — Reflection Measurement

ESP32 performs continuous measurement using:

* LEDs OFF → ambient light reading
* LEDs ON → reflected light reading

The difference gives:

```text id="reflectionformula01"
Reflection = LED ON Reading − Ambient Reading
```

This removes false readings caused by room light or sunlight variation.

---

## Step 2 — EMA Filtering

An Exponential Moving Average (EMA) is applied to:

* smooth fluctuations
* remove false spikes
* improve stability

This prevents accidental triggering.

---

## Step 3 — Stable Dust Verification

Cleaning starts only if:

* Dust % exceeds threshold (65%)
* Multiple stable readings are confirmed

This avoids false cleaning cycles caused by temporary noise.

---

## Step 4 — ESP32 Sends START

ESP32 sends:

```text id="startcmd01"
START
```

to Arduino UNO through Serial2.

---

## Step 5 — Pump First

UNO starts:

* Pump ON for 5 seconds

This ensures water is sprayed before motor movement begins.

This improves cleaning efficiency and reduces dry friction.

---

## Step 6 — Motor Cleaning Cycle

After pump stops:

* Motor moves forward
* Ultrasonic sensor detects end position
* Motor reverses automatically
* Returns to home/start position

---

## Step 7 — Completion

UNO sends:

```text id="donecmd01"
DONE
```

back to ESP32.

ESP32 then:

* resumes dust detection
* resets filters safely
* starts cooldown protection

---

# Hardware Used

---

## Controllers

* ESP32 Dev Board
* Arduino UNO

---

## Dust Detection System

* 3 × LDR Sensors
* 3 × LEDs
* Current limiting resistors

---

## Motion System

* L298N Motor Driver
* DC Motor

---

## Cleaning System

* Water Pump
* Relay Module

---

## Position Detection

* HC-SR04 Ultrasonic Sensor

---

## Other Components

* Power Supply
* Wiring
* Solar Panel Prototype Frame

---

# Pin Configuration

---

# ESP32 Connections

| Component         |    Pin |
| ----------------- | -----: |
| LDR1              | GPIO34 |
| LDR2              | GPIO35 |
| LDR3              | GPIO32 |
| LED1              | GPIO25 |
| LED2              | GPIO26 |
| LED3              | GPIO27 |
| UNO TX → ESP32 RX | GPIO16 |
| UNO RX ← ESP32 TX | GPIO17 |

---

# Arduino UNO Connections

| Component         | Pin |
| ----------------- | --: |
| L298N ENA         |  D9 |
| L298N IN1         |  D8 |
| L298N IN2         |  D7 |
| Pump Relay        | D10 |
| HC-SR04 TRIG      |  D2 |
| HC-SR04 ECHO      |  D3 |
| ESP32 TX → UNO RX |  RX |
| ESP32 RX ← UNO TX |  TX |

---

# Blynk Virtual Pins

| Virtual Pin | Function                  |
| ----------- | ------------------------- |
| V1          | System Status             |
| V2          | Dust Percentage           |
| V4          | Distance Monitoring       |
| V20         | Manual Calibration Button |

---

# Major Features

* Reflection-based dust detection
* 3-point LDR sensing array
* LED-assisted reflective measurement
* Indoor calibration support
* Ambient light cancellation
* EMA smoothing
* False trigger prevention
* Stable threshold verification
* Pump-first power optimization
* Automatic forward + reverse cleaning
* EEPROM calibration memory
* Serial master-slave architecture
* Blynk live monitoring
* Cooldown protection

---

# Key Advantages

Compared to timer-based systems:

* Cleans only when required
* Saves water
* Saves motor power
* Prevents unnecessary cleaning
* Improves system lifespan
* More reliable in real-world conditions
* Better for large solar installations

Most importantly:

## It detects actual dust instead of assuming dust.

This is the strongest technical advantage of the project.

---

# Future Improvements

* Weather prediction integration
* Rain detection system
* GSM alert notifications
* Solar battery optimization
* AI-based dust prediction
* Multi-panel synchronized cleaning

---

# Author

Developed for Smart Solar Panel Maintenance using IoT and Embedded Systems.

The system uses a reflection-based LED-LDR sensing array as its primary intelligence layer, making the cleaning process efficient, power-optimized, and suitable for practical real-world deployment.

---

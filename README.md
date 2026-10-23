# Health-ESP32

### An ESP32-Based IoT Health Monitoring and Patient Assistance System

Health-ESP32 is an embedded IoT project designed to monitor patient-related and environmental parameters, provide real-time alerts, and offer a web-based interface for doctors and family members.

Built with **ESP32-S2, C++, Arduino, and FreeRTOS**, the system combines sensor monitoring, an OLED interface, a local HTTP web server, and patient assistance features in a single embedded platform.

The project explores the integration of embedded systems, IoT, real-time task scheduling, and web-based monitoring for healthcare-oriented applications.

## Key Features

- **Environmental Monitoring:** Monitors room temperature and humidity using DHT22 sensors.
- **Heart Rate Monitoring:** Simulates heart rate measurements using an analog input, enabling development and testing without a dedicated heart rate sensor.
- **Air Quality Monitoring:** Reads analog values from an MQ-series gas sensor and classifies air quality into predefined levels.
- **Real-Time Alerts:** Detects threshold violations and provides visual and audible notifications.
- **Web-Based Dashboard:** Hosts a local web interface directly on the ESP32.
- **Role-Based Interfaces:** Provides separate dashboards for doctors and family members.
- **Patient Assistance:** Includes nurse and family contact request features.
- **Medication Reminders:** Supports configurable medication reminder times.
- **Doctor–Family Communication:** Provides message and prescription management features.
- **OLED Interface:** Displays monitoring information, system status, and alerts on a 128 × 64 OLED screen.
- **Wi-Fi Connectivity:** Supports wireless connectivity and automatic reconnection attempts.
- **Time Synchronization:** Uses Network Time Protocol (NTP) to obtain the current time.

## System Architecture

The system consists of four main components:

| Component | Responsibility |
|---|---|
| ESP32-S2 | Main controller and execution platform |
| Sensors and analog inputs | Environmental monitoring, gas-level readings, and simulated heart rate |
| OLED display and physical buttons | Local monitoring and menu navigation |
| Embedded web server | Browser-based dashboards and patient assistance interfaces |

The ESP32 collects sensor readings, evaluates predefined alert conditions, updates the local display, and serves monitoring information through its HTTP server.

FreeRTOS tasks handle major system responsibilities independently, while mutexes coordinate access to shared resources.

## Technology Stack

**Embedded Development**
- C++
- Arduino Framework
- ESP32-S2
- FreeRTOS

**Communication and Networking**
- Wi-Fi
- HTTP
- Embedded Web Server
- NTP Time Synchronization

**Display and Hardware Interfaces**
- I2C
- SSD1306 OLED Display
- DHT22 Temperature and Humidity Sensors
- Analog-to-Digital Conversion (ADC)
- MQ-Series Gas Sensor Interface

**Development and Simulation**
- PlatformIO
- Wokwi

## Hardware Configuration

The current firmware defines the following hardware interfaces:

| Component | GPIO / Configuration |
|---|---|
| Room DHT22 | GPIO 17 |
| Patient-area DHT22 | GPIO 16 |
| Simulated heart rate input | GPIO 10 |
| MQ gas sensor analog output | GPIO 18 |
| Buzzer | GPIO 33 |
| Left navigation button | GPIO 15 |
| Right navigation button | GPIO 14 |
| Enter button | GPIO 13 |
| OLED display | I2C, address `0x3C` |

**Important:** Verify the GPIO mapping against your actual ESP32-S2 board and circuit before connecting hardware. Pin availability and restrictions can vary by board.

The heart rate input currently uses a potentiometer to simulate readings. The DHT22 sensors monitor environmental temperature and humidity; they do not measure actual body temperature.

## Software Architecture

The firmware uses FreeRTOS to separate monitoring, user interaction, and network communication into dedicated tasks.

The current implementation includes:

- `webServerTask`: Processes incoming HTTP client requests.
- `displayTask`: Updates the OLED interface and handles display alerts.
- `wifiTask`: Monitors Wi-Fi connectivity and manages reconnection and time synchronization.
- `roomDhtTask`: Reads room temperature and humidity.
- `patientDhtTask`: Reads patient-area temperature and humidity.
- `heartRateTask`: Generates simulated heart rate readings.
- `mqSensorTask`: Reads gas sensor values and evaluates air quality.
- `buttonTask`: Processes physical button inputs and menu navigation.

Mutexes are used to coordinate access to shared data and resources.

This design separates major responsibilities and provides a foundation for extending the system with additional sensors and monitoring capabilities.

## Alert and Assistance System

The firmware evaluates sensor readings against configurable thresholds.

Examples include:

- High or low room temperature.
- High or low room humidity.
- High or low patient-area temperature and humidity.
- Elevated gas sensor readings.
- High simulated heart rate.

When applicable, the system can display an alert, activate the buzzer, and record alert information through the firmware's logging mechanisms.

The system also provides patient assistance interfaces for requesting help from a nurse or contacting family members.

**Note:** These thresholds are application-defined values, not clinically validated medical thresholds.

## Web Interface

The ESP32 hosts its own HTTP server, making the dashboards accessible through a web browser on the same network.

The firmware defines routes for:

| Endpoint | Purpose |
|---|---|
| `/` | Login interface |
| `/doctor-dashboard` | Doctor dashboard |
| `/family-dashboard` | Family dashboard |
| `/prescriptions` | Prescription management |
| `/messages` | Message management |
| `/pill-settings` | Medication reminder settings |
| `/weekly-report` | Weekly report interface |
| `/call-nurse` | Nurse assistance request |
| `/contact-nurse` | Nurse contact action |
| `/contact-family` | Family contact action |
| `/emergency` | Emergency handling |

Additional endpoints support logout, alert counter resets, and other system controls.

These interfaces are served directly by the ESP32 rather than requiring a separate web server.

## Getting Started

### Prerequisites

Install the following tools:

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE](https://platformio.org/)
- [Git](https://git-scm.com/)

Alternatively, the firmware can be adapted for development with the Arduino IDE.

### 1. Clone the Repository

```bash
git clone https://github.com/Sobhankhedry/Health-ESP32.git
cd Health-ESP32
```

### 2. Open the Project

Open the repository folder in Visual Studio Code with the PlatformIO extension installed.

The project configuration is provided in `platformio.ini`.

### 3. Install Dependencies

PlatformIO manages the libraries declared in `platformio.ini`, including:

- Adafruit SSD1306
- Adafruit GFX Library
- Adafruit BusIO
- Adafruit DHT Sensor Library
- Adafruit Unified Sensor

The ESP32 board platform and Arduino framework are also configured through PlatformIO.

### 4. Configure Wi-Fi

Before running the firmware on your own network, update the Wi-Fi credentials in `src/main.cpp`:

```cpp
const char *ssid = "YOUR_WIFI_NAME";
const char *password = "YOUR_WIFI_PASSWORD";
```

The current configuration uses the `Wokwi-GUEST` network, which is intended for the Wokwi simulation environment.

### 5. Build and Upload

In PlatformIO:

1. Select the configured `esp32-s2-saola-1` environment.
2. Build the project.
3. Connect a compatible ESP32-S2 board.
4. Upload the firmware.
5. Open the Serial Monitor at `115200` baud.

### 6. Access the Dashboard

After a successful Wi-Fi connection, obtain the ESP32's local IP address from the Serial Monitor.

Open the following address in a browser connected to the same network:

```text
http://<ESP32-IP-ADDRESS>/
```

The login interface should be available when the web server has initialized successfully.

## Project Structure

```text
Health-ESP32/
├── include/                 # Header files and interface definitions
├── lib/                     # Project-specific libraries
├── src/
│   ├── main.cpp             # Main firmware and system logic
│   ├── login.h              # Login interface
│   ├── doctor_dashboard.h   # Doctor dashboard
│   └── family_dashboard.h   # Family dashboard
├── test/                    # Test directory
├── platformio.ini           # PlatformIO configuration
├── diagram.json             # Circuit diagram / simulation configuration
├── web_demo.html            # Web interface demonstration
├── wifi_test.cpp            # Wi-Fi test code
└── README.md
```

The repository also contains Wokwi-related configuration files and project documentation.

## Security Considerations

This project is intended for educational and prototyping purposes.

Before deploying it in a real environment:

- Replace hardcoded demonstration credentials with a secure authentication mechanism.
- Implement proper session management and authorization.
- Protect sensitive patient information.
- Use appropriate encryption and access controls.
- Validate all user-submitted data.
- Avoid exposing the embedded web server directly to the public internet.

The current login implementation should not be considered production-grade authentication.

## Limitations

The current implementation has several limitations:

- Heart rate is simulated rather than measured using a dedicated physiological sensor.
- Environmental sensor readings do not constitute a medical assessment.
- Gas sensor readings require appropriate calibration before they can be interpreted as meaningful gas concentrations.
- The embedded web interface is intended for local-network use.
- Alert thresholds require application-specific validation.
- The system is not a certified medical device.

These limitations provide opportunities for future hardware integration, software improvements, and more comprehensive testing.

## Future Improvements

Potential extensions include:

- Integration of a dedicated heart rate and SpO₂ sensor.
- Addition of a calibrated air quality sensor.
- Secure authentication and role-based authorization.
- Persistent storage for measurements and alert history.
- Remote notifications and caregiver communication.
- Improved data visualization and historical trend analysis.
- Automated firmware testing and hardware integration tests.
- Evaluation of system responsiveness under concurrent workloads.

## License

This project is distributed under the MIT License. See the [`LICENSE`](LICENSE) file for details.

## Author

**Sobhan Khedry**

- GitHub: [@Sobhankhedry](https://github.com/Sobhankhedry)
- Project: [Health-ESP32](https://github.com/Sobhankhedry/Health-ESP32)

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT_U.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "time.h"

// Include HTML templates
#include "login.h"
#include "doctor_dashboard.h"
#include "family_dashboard.h"


// 'Screenshot 2025-07-10 151413', 47x46px
const unsigned char epd_bitmap_Screenshot_2025_07_10_151413 [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xfe, 0xff, 0x87, 0x9f, 0xf3, 
    0xe3, 0xfe, 0xfe, 0x7f, 0xf7, 0xdf, 0xfc, 0xfe, 0xfd, 0xff, 0xfb, 0xbf, 0xff, 0x7e, 0xf3, 0xff, 
    0xfd, 0x7f, 0xff, 0x9e, 0xe7, 0xff, 0xfc, 0x7f, 0xff, 0xce, 0xef, 0xff, 0xfe, 0xff, 0xff, 0xee, 
    0xdf, 0xff, 0xfe, 0xff, 0xff, 0xf6, 0xbf, 0xff, 0xfe, 0xff, 0xff, 0xfa, 0xbf, 0xff, 0xff, 0xff, 
    0xff, 0xfa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfa, 0x78, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x7c, 0x00, 
    0x7f, 0xff, 0xff, 0xfc, 0x7c, 0x00, 0x1f, 0xff, 0xff, 0xfc, 0x7e, 0x00, 0x0f, 0xff, 0xff, 0xfc, 
    0x7e, 0x04, 0x07, 0xff, 0xff, 0xfc, 0x7f, 0x03, 0x03, 0xfc, 0x00, 0x3c, 0x7f, 0x01, 0x83, 0xf0, 
    0x00, 0x7c, 0x7f, 0x80, 0xc3, 0xf0, 0x00, 0x7c, 0xbf, 0x80, 0x63, 0xe0, 0x40, 0xfa, 0xbf, 0xc0, 
    0x33, 0xe1, 0x81, 0xfa, 0xbf, 0xe0, 0x1b, 0xc6, 0x03, 0xfa, 0xdf, 0xe0, 0x19, 0xcc, 0x03, 0xf6, 
    0xdf, 0xf8, 0x0d, 0xd8, 0x07, 0xf6, 0xef, 0xfe, 0x0c, 0xb0, 0x0f, 0xee, 0xef, 0xff, 0xfe, 0xb0, 
    0x1f, 0xee, 0xf7, 0xff, 0xfe, 0x7f, 0xff, 0xde, 0xfb, 0xff, 0xfe, 0x7f, 0xff, 0xbe, 0xf9, 0xff, 
    0xfe, 0x7f, 0xff, 0x3e, 0xfd, 0xff, 0xfe, 0x7f, 0xfe, 0x7e, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xfe, 
    0xff, 0x7f, 0xfe, 0xff, 0xfd, 0xfe, 0xff, 0xbf, 0xfe, 0xff, 0xfb, 0xfe, 0xff, 0xdf, 0xff, 0xff, 
    0xf7, 0xfe, 0xff, 0xef, 0xff, 0xff, 0xef, 0xfe, 0xff, 0xf3, 0xff, 0xff, 0x9f, 0xfe, 0xff, 0xf9, 
    0xff, 0xff, 0x3f, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xff, 0x7f, 0xfd, 0xff, 0xfe, 
    0xff, 0xff, 0x9f, 0xf3, 0xff, 0xfe, 0xff, 0xff, 0xef, 0xef, 0xff, 0xfe, 0xff, 0xff, 0xf3, 0x9f, 
    0xff, 0xfe, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfe
};

// Time management
struct TimeData {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} systemTime;

unsigned long lastTimeUpdate = 0;

String doctorPassword = "doc";
String familyPassword = "fam";

// Login system variables
bool isLoggedIn = false;
String currentUserType = ""; // "doctor" or "family"
unsigned long loginTime = 0;
const unsigned long LOGIN_TIMEOUT = 3600000; // 1 hour timeout

// Button   pins
#define LEFT_BUT 15
#define RIGHT_BUT 14
#define ENTER_BUT 13

// Menu states
enum MenuState {
    MENU_ROOM,
    MENU_PATIENT,
    MENU_HEARTRATE,  // New heart rate menu item
    MENU_AIRQUALITY, // New air quality menu item
    MENU_NURSE,      // New contact nurse menu item
    MENU_FAMILY,     // New call family menu item
    MENU_PILLS,      // New pill reminder menu item
    MENU_SETTINGS,
    VIEW_ROOM,
    VIEW_PATIENT,
    VIEW_HEARTRATE,  // New heart rate view state
    VIEW_AIRQUALITY, // New air quality view state
    VIEW_NURSE,      // New contact nurse view state
    VIEW_FAMILY,     // New call family view state
    VIEW_PILLS,      // New pill reminder view state
    VIEW_SETTINGS
};

// Menu configuration
#define MAX_MENU_ITEMS 8        // Maximum number of menu items (increased to 8)
#define MENU_ITEMS_PER_PAGE 3   // Number of items shown at once
#define MENU_ITEM_HEIGHT 12     // Height of each menu item in pixels

// Menu structure
struct MenuItem {
    const char* name;
    const char* icon;
    MenuState state;
};

// Global menu state
MenuState currentMenu = MENU_ROOM;
int currentMenuScroll = 0;      // Current scroll position

// Pill menu variables
bool editingPillTime = false;   // Whether we're in pill time editing mode
bool editingHours = true;       // True = editing hours, False = editing minutes

// Removed pill system UI variables

// Define menu items
const MenuItem menuItems[MAX_MENU_ITEMS] = {
    {"Room Temp", "\x0F", MENU_ROOM},        // ☼ Sun symbol for room temperature
    {"Patient", "\x02", MENU_PATIENT},       // ☺ Smiling face for patient
    {"Heart Rate", "\x03", MENU_HEARTRATE},  // ♥ Heart symbol for heart rate
    {"Air Quality", "\x14", MENU_AIRQUALITY}, // § Symbol for air quality
    {"Call Nurse", "\x05", MENU_NURSE},      // ♣ Club symbol for nurse call
    {"Call Family", "\x04", MENU_FAMILY},    // ♦ Diamond symbol for family call
    {"Pills", "\x0E", MENU_PILLS},           // ♫ Musical note for pill reminder
    {"Information", "\x1F", MENU_SETTINGS},     // ⚙ Gear for settings
    // You can add more menu items here
};

const int NUM_MENU_ITEMS = 8;  // Current number of menu items (increased to 8)

// Task handles
TaskHandle_t webServerTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t roomDhtTaskHandle = NULL;
TaskHandle_t patientDhtTaskHandle = NULL;
TaskHandle_t heartRateTaskHandle = NULL;
TaskHandle_t mqSensorTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;

// Mutex for protecting shared resources
SemaphoreHandle_t displayMutex;
SemaphoreHandle_t wifiMutex;
SemaphoreHandle_t dhtMutex;
SemaphoreHandle_t menuMutex;
SemaphoreHandle_t heartRateMutex;
SemaphoreHandle_t mqSensorMutex;

// Shared sensor data structure
struct SensorData {
    float temperature;
    float humidity;
    unsigned long lastUpdate;
};

// Heart rate data structure
struct HeartRateData {
    int bpm;
    unsigned long lastUpdate;
    bool isValid;
};

// MQ sensor data structure
struct MQSensorData {
    int gasLevel;      
    float gasPercent; 
    unsigned long lastUpdate;
    bool isValid;
};

// Pill reminder data structure
struct PillData {
    int hours;     
    int minutes;   
    bool isSet;    
    bool alertShown; 
    unsigned long lastCheck; 
};

// Removed pill dispensing system structures and data

struct {
    SensorData room;
    SensorData patient;
    HeartRateData heartRate;
    MQSensorData airQuality;
} sensorData;

// Medical prescription data structure
struct PrescriptionData {
    String medication;
    String dosage;
    String frequency;
    String duration;
    String instructions;
    String doctorName;
    String timestamp;
    bool isValid;
};

// Message data structure
struct MessageData {
    String content;
    String senderType;  // "doctor" or "family"
    String timestamp;
    bool isValid;
    bool isRead;
};

// Alert counter structure
struct AlertCounters {
    int heartRateHigh;
    int roomTempHigh;
    int roomTempLow;
    int roomHumidHigh;
    int roomHumidLow;
    int patientTempHigh;
    int patientTempLow;
    int patientHumidHigh;
    int patientHumidLow;
    int airQualityWarning;
    int airQualityDanger;
};

// Initialize alert counters
AlertCounters alertCounters = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Alert cooldown structure - track last alert time for each condition
struct AlertCooldowns {
    unsigned long heartRateHigh;
    unsigned long roomTempHigh;
    unsigned long roomTempLow;
    unsigned long roomHumidHigh;
    unsigned long roomHumidLow;
    unsigned long patientTempHigh;
    unsigned long patientTempLow;
    unsigned long patientHumidHigh;
    unsigned long patientHumidLow;
    unsigned long airQualityWarning;
    unsigned long airQualityDanger;
};

// Initialize alert cooldowns (all start at 0 - no cooldown initially)
AlertCooldowns alertCooldowns = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Alert cooldown period - 30 minutes (30 * 60 * 1000 = 1800000 milliseconds)
const unsigned long ALERT_COOLDOWN_PERIOD = 1800000; // 30 minutes

bool locationUpdateInProgress = false; // Prevent concurrent location updates

// Message system arrays (simple storage)
#define MAX_PRESCRIPTIONS 10
#define MAX_MESSAGES 20
PrescriptionData prescriptions[MAX_PRESCRIPTIONS];
MessageData messages[MAX_MESSAGES];
int prescriptionCount = 0;
int messageCount = 0;
bool hasNewMessage = false;  // Flag for notifications

// Pill reminder instance
PillData pillReminder = {12, 0, true, false, 0}; // Default to 12:00, ALWAYS active

// Removed pill system data instance



// DHT22 Configuration
#define ROOM_DHT_PIN 17     // Room DHT22 data pin GPIO17
#define PATIENT_DHT_PIN 16  // Patient DHT22 data pin GPIO16
#define DHTTYPE DHT22       // DHT22 (AM2302) sensor type

// Heart Rate Configuration (Potentiometer simulation)
#define HEART_RATE_PIN 10   // Potentiometer pin GPIO1 (ADC1_CH0)

// MQ Gas Sensor Configuration
#define MQ_SENSOR_PIN 18    // MQ sensor analog pin GPIO18 (connected to AOUT in wokwi)


// Buzzer Configuration
#define BUZZER_PIN 33       // Buzzer pin GPIO33 (matches wokwi diagram)

// Create two DHT instances
DHT roomDht(ROOM_DHT_PIN, DHTTYPE);
DHT patientDht(PATIENT_DHT_PIN, DHTTYPE);

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1        // Reset pin (-1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // OLED I2C Address

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 🌐 اطلاعات WiFi - اینجا اطلاعات شبکه خودتون رو بنویسید
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// NTP Time Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600 * 3.5;  // UTC+3:30 for Iran
const int daylightOffset_sec = 0;       // No daylight saving

// � Serial Monitor Configuration for Alert Logging
// Heart rate alerts will be logged to serial monitor

// Alert variables
bool heartRateAlertLogged = false;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN = 60000; // 1 minute cooldown between alerts

// Temperature and Humidity Alert Thresholds
// Room Temperature Alerts
const float ROOM_TEMP_HIGH = 32.0;    // °C - Room too hot
const float ROOM_TEMP_LOW = 16.0;     // °C - Room too cold
const float ROOM_HUMID_HIGH = 75.0;   // % - Room too humid
const float ROOM_HUMID_LOW = 25.0;    // % - Room too dry

// Patient Area Temperature Alerts (these are environmental readings, not body temperature)
const float BODY_TEMP_HIGH = 30.0;    // °C - Patient area too hot
const float BODY_TEMP_LOW = 18.0;     // °C - Patient area too cold
const float BODY_HUMID_HIGH = 80.0;   // % - Patient area too humid
const float BODY_HUMID_LOW = 30.0;    // % - Patient area too dry

// MQ Gas Sensor Alert Thresholds
const int MQ_SAFE_LEVEL = 1000;       // Raw reading - Safe air quality
const int MQ_WARNING_LEVEL = 2000;    // Raw reading - Warning level
const int MQ_DANGER_LEVEL = 3000;     // Raw reading - Dangerous level

// Alert tracking variables
bool roomTempAlertLogged = false;
bool roomHumidAlertLogged = false;
bool bodyTempAlertLogged = false;
bool bodyHumidAlertLogged = false;
unsigned long lastRoomTempAlert = 0;
unsigned long lastRoomHumidAlert = 0;
unsigned long lastBodyTempAlert = 0;
unsigned long lastBodyHumidAlert = 0;

// Emergency alert variables
bool emergencyAlert = false;
unsigned long emergencyAlertTime = 0;
const unsigned long EMERGENCY_ALERT_DURATION = 3000; // 3 seconds full-screen alert
bool fullScreenAlert = false;
String emergencyAlertType = ""; // Track type of emergency alert

// Environmental alert variables
bool environmentalAlert = false;
unsigned long environmentalAlertTime = 0;
const unsigned long ENVIRONMENTAL_ALERT_DURATION = 4000; // 4 seconds environmental alert (longer duration)
String currentEnvironmentalAlert = "";
bool environmentalAlertActive = false;

// Alert state tracking for status bar icons
struct AlertStates {
    bool roomTempHigh = false;
    bool roomTempLow = false;
    bool roomHumidHigh = false;
    bool roomHumidLow = false;
    bool patientTempHigh = false;
    bool patientTempLow = false;
    bool patientHumidHigh = false;
    bool patientHumidLow = false;
    bool airQualityWarning = false;
    bool airQualityDanger = false;
} alertStates;


WebServer server(80);

// Function declarations
void logHeartRateAlert(int heartRate);
void soundBuzzerAlarm();
void soundEnvironmentalAlert();
String getRealDateTime();
String getRealTimeFormatted();
String getPersianDate();
void logRoomTempAlert(float temperature, bool isHigh);
void logRoomHumidityAlert(float humidity, bool isHigh);
void logBodyTempAlert(float temperature, bool isHigh);
void logBodyHumidityAlert(float humidity, bool isHigh);
void logAirQualityAlert(int gasLevel, bool isDanger);
void checkRoomAlerts(float temp, float humid);
void checkBodyAlerts(float temp, float humid);
void checkAirQualityAlerts(int gasLevel);
void clearAlertStates(float roomTemp, float roomHumid, float patientTemp, float patientHumid);
void checkPillReminder(); // Check if it's time for pill reminder
void displayStatusBar();
void resetAlertCounters(); // Reset alert counters for new monitoring period
// Message and prescription management functions
void addPrescription(String medication, String dosage, String frequency, String duration, String instructions, String doctorName);
void addMessage(String content, String senderType);
String getPrescriptionsHTML();
String getMessagesHTML();
int getUnreadMessageCount();
void handleWeeklyReport(); // Weekly report handler for doctors
// Removed pill system function declarations
String getPillSystemStatus();

// Nurse call system function
void sendNurseAlert() {
    // Get current timestamp
    String realDateTime = getRealDateTime();
    char timestamp[50];
    
    if (realDateTime != "Time not set") {
        strncpy(timestamp, realDateTime.c_str(), sizeof(timestamp) - 1);
        timestamp[sizeof(timestamp) - 1] = '\0';
    } else {
        // Fallback to system uptime if NTP time not available
        unsigned long uptime = millis() / 1000;
        uint8_t hours = uptime / 3600;
        uint8_t minutes = (uptime % 3600) / 60;
        uint8_t seconds = uptime % 60;
        snprintf(timestamp, sizeof(timestamp), "Uptime: %02d:%02d:%02d", hours, minutes, seconds);
    }
    
    // Send detailed alert to serial monitor
    Serial.println("");
    Serial.println("==========================================");
    Serial.println("🚨 NURSE CALL ALERT 🚨");
    Serial.println("==========================================");
    Serial.print("⏰ Time: ");
    Serial.println(timestamp);
    Serial.println("👤 Patient is requesting assistance");
    Serial.println("📍 Location: Patient Room");
    Serial.println("🔔 Priority: Normal");
    Serial.println("💡 Action Required: Please check on patient");
    Serial.println("🏥 Health Monitor System");
    Serial.println("==========================================");
    Serial.println("");
}

// Family call system function
void sendFamilyAlert() {
    // Get current timestamp
    String realDateTime = getRealDateTime();
    char timestamp[50];
    
    if (realDateTime != "Time not set") {
        strncpy(timestamp, realDateTime.c_str(), sizeof(timestamp) - 1);
        timestamp[sizeof(timestamp) - 1] = '\0';
    } else {
        // Fallback to system uptime if NTP time not available
        unsigned long uptime = millis() / 1000;
        uint8_t hours = uptime / 3600;
        uint8_t minutes = (uptime % 3600) / 60;
        uint8_t seconds = uptime % 60;
        snprintf(timestamp, sizeof(timestamp), "Uptime: %02d:%02d:%02d", hours, minutes, seconds);
    }
    
    // Send detailed alert to serial monitor
    Serial.println("");
    Serial.println("==========================================");
    Serial.println("📞 FAMILY CALL ALERT 📞");
    Serial.println("==========================================");
    Serial.print("⏰ Time: ");
    Serial.println(timestamp);
    Serial.println("👤 Patient wants to contact family");
    Serial.println("📍 Location: Patient Room");
    Serial.println("🔔 Priority: Normal");
    Serial.println("💡 Action Required: Please call patient's family");
    Serial.println("👨‍👩‍👧‍👦 Family notification requested");
    Serial.println("🏥 Health Monitor System");
    Serial.println("==========================================");
    Serial.println("");
}

// Login handler for authentication
void handleLogin() {
    if (server.method() == HTTP_POST) {
        String userType = server.arg("userType");
        String password = server.arg("password");
        
        bool loginSuccess = false;
        
        if (userType == "doctor" && password == doctorPassword) {
            loginSuccess = true;
            currentUserType = "doctor";
        } else if (userType == "family" && password == familyPassword) {
            loginSuccess = true;
            currentUserType = "family";
        }
        
        if (loginSuccess) {
            isLoggedIn = true;
            loginTime = millis();
            
           
            
            // Redirect to appropriate dashboard
            server.sendHeader("Location", "/" + currentUserType + "-dashboard");
            server.send(302);
        } else {
            // Login failed - redirect back to login with error
            server.sendHeader("Location", "/?error=1");
            server.send(302);
        }
    }
}

// Logout handler
void handleLogout() {
    isLoggedIn = false;
    currentUserType = "";
    loginTime = 0;
    
    
    server.sendHeader("Location", "/");
    server.send(302);
}

// Doctor dashboard handler
void handleDoctorDashboard() {
    // Check if user is logged in as doctor
    if (!isLoggedIn || currentUserType != "doctor") {
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    
    // Check login timeout
    if (millis() - loginTime > LOGIN_TIMEOUT) {
        isLoggedIn = false;
        currentUserType = "";
        server.sendHeader("Location", "/?timeout=1");
        server.send(302);
        return;
    }
    
    // Use enhanced template and replace placeholders with actual data
    String html = DOCTOR_DASHBOARD_HTML;
    
    // Replace placeholders with current data
    html.replace("%TIME%", getRealTimeFormatted());
    html.replace("%ROOM_TEMP%", String(sensorData.room.temperature, 1));
    html.replace("%ROOM_HUMIDITY%", String(sensorData.room.humidity, 1));
    html.replace("%PATIENT_TEMP%", String(sensorData.patient.temperature, 1));
    html.replace("%PATIENT_HUMIDITY%", String(sensorData.patient.humidity, 1));
    html.replace("%HEART_RATE%", String(sensorData.heartRate.bpm));
    
    // Gas/Air Quality information  
    html.replace("%GAS_LEVEL%", String(sensorData.airQuality.gasPercent, 1));
    String airQualityStatus = "Good";
    if (sensorData.airQuality.gasLevel >= 3000) airQualityStatus = "Poor";
    else if (sensorData.airQuality.gasLevel >= 2000) airQualityStatus = "Moderate";
    html.replace("%AIR_QUALITY%", airQualityStatus);
    
    // Pill reminder info
    String pillTime = String(pillReminder.hours < 10 ? "0" : "") + String(pillReminder.hours) + ":";
    pillTime += String(pillReminder.minutes < 10 ? "0" : "") + String(pillReminder.minutes);
    html.replace("%NEXT_PILL_TIME%", pillTime);
    html.replace("%PILLS_TAKEN%", "Not tracked"); // Can implement pill counter later
    html.replace("%PILL_STATUS%", "Active");
    
    // System status
    html.replace("%WIFI_STATUS%", "Connected");
    html.replace("%UPTIME%", String(millis() / 1000) + " seconds");
    html.replace("%LAST_UPDATE%", getRealTimeFormatted());
    
    // Add message notification badge
    int unreadCount = getUnreadMessageCount();
    String messageBadge = "";
    if (unreadCount > 0) {
        messageBadge = "<span class='notification-badge'>" + String(unreadCount) + "</span>";
    }
    html.replace("%MESSAGE_BADGE%", messageBadge);
    
    server.send(200, "text/html", html);
}

// Family dashboard handler
void handleFamilyDashboard() {
    // Check if user is logged in as family
    if (!isLoggedIn || currentUserType != "family") {
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    
    // Check login timeout
    if (millis() - loginTime > LOGIN_TIMEOUT) {
        isLoggedIn = false;
        currentUserType = "";
        server.sendHeader("Location", "/?timeout=1");
        server.send(302);
        return;
    }
    
    // Use enhanced template and replace placeholders with actual data
    String html = FAMILY_DASHBOARD_HTML;
    
    // Replace placeholders with current data
    html.replace("%TIME%", getRealTimeFormatted());
    html.replace("%ROOM_TEMP%", String(sensorData.room.temperature, 1));
    html.replace("%ROOM_HUMIDITY%", String(sensorData.room.humidity, 1));
    html.replace("%PATIENT_TEMP%", String(sensorData.patient.temperature, 1));
    html.replace("%PATIENT_HUMIDITY%", String(sensorData.patient.humidity, 1));
    html.replace("%HEART_RATE%", String(sensorData.heartRate.bpm));
    
    // Gas/Air Quality information
    html.replace("%GAS_LEVEL%", String(sensorData.airQuality.gasPercent, 1));
    String airQualityStatus = "Good";
    if (sensorData.airQuality.gasLevel >= 3000) airQualityStatus = "Poor";
    else if (sensorData.airQuality.gasLevel >= 2000) airQualityStatus = "Moderate";
    html.replace("%AIR_QUALITY%", airQualityStatus);
    
    // Patient status based on heart rate
    String patientStatus = "Good";
    if (sensorData.heartRate.bpm > 100) patientStatus = "Monitored";
    if (sensorData.heartRate.bpm > 120) patientStatus = "Alert";
    html.replace("%PATIENT_STATUS%", patientStatus);
    
    // Pill reminder info
    String pillTime = String(pillReminder.hours < 10 ? "0" : "") + String(pillReminder.hours) + ":";
    pillTime += String(pillReminder.minutes < 10 ? "0" : "") + String(pillReminder.minutes);
    html.replace("%NEXT_PILL_TIME%", pillTime);
    html.replace("%PILLS_TAKEN%", "Not tracked");
    html.replace("%PILL_STATUS%", "Active");
    
    // Comfort status
    String comfortStatus = "Good";
    if (sensorData.room.temperature < 18 || sensorData.room.temperature > 26) comfortStatus = "Check Temperature";
    if (sensorData.room.humidity < 30 || sensorData.room.humidity > 70) comfortStatus = "Check Humidity";
    html.replace("%COMFORT_STATUS%", comfortStatus);
    
    // Add prescriptions HTML
    html.replace("%PRESCRIPTIONS_HTML%", getPrescriptionsHTML());
    
    // Add message notification badge
    int unreadCount = getUnreadMessageCount();
    String messageBadge = "";
    if (unreadCount > 0) {
        messageBadge = "<span class='notification-badge'>" + String(unreadCount) + "</span>";
    }
    html.replace("%MESSAGE_BADGE%", messageBadge);
    
    server.send(200, "text/html", html);
}

void handleRoot()
{
    // Check if already logged in
    if (isLoggedIn && (millis() - loginTime < LOGIN_TIMEOUT)) {
        // Redirect to appropriate dashboard
        server.sendHeader("Location", "/" + currentUserType + "-dashboard");
        server.send(302);
        return;
    }
    
    // Show login page using external template
    String html = LOGIN_HTML;
    
    server.send(200, "text/html", html);
}

// Doctor dashboard page (enhanced version of current interface)
void handleDoctorPage() {
    // Use template and replace placeholders with actual data
    String html = DOCTOR_DASHBOARD_HTML;
    
    // Replace placeholders with current data
    html.replace("%TIME%", getRealTimeFormatted());
    html.replace("%ROOM_TEMP%", String(sensorData.room.temperature, 1));
    html.replace("%ROOM_HUMIDITY%", String(sensorData.room.humidity, 1));
    html.replace("%PATIENT_TEMP%", String(sensorData.patient.temperature, 1));
    html.replace("%PATIENT_HUMIDITY%", String(sensorData.patient.humidity, 1));
    html.replace("%HEART_RATE%", String(sensorData.heartRate.bpm));
    
    // Pill reminder info
    String pillTime = String(pillReminder.hours < 10 ? "0" : "") + String(pillReminder.hours) + ":";
    pillTime += String(pillReminder.minutes < 10 ? "0" : "") + String(pillReminder.minutes);
    html.replace("%NEXT_PILL_TIME%", pillTime);
    html.replace("%PILLS_TAKEN%", "Not tracked"); // Can implement pill counter later
    html.replace("%PILL_STATUS%", "Active");
    
    // System status
    html.replace("%WIFI_STATUS%", "Connected");
    html.replace("%UPTIME%", String(millis() / 1000) + " seconds");
    html.replace("%LAST_UPDATE%", getRealTimeFormatted());
    
    server.send(200, "text/html", html);
}

// Family dashboard page (simplified version)
void handleFamilyPage() {
    // Use template and replace placeholders with actual data
    String html = FAMILY_DASHBOARD_HTML;
    
    // Replace placeholders with current data
    html.replace("%TIME%", getRealTimeFormatted());
    html.replace("%ROOM_TEMP%", String(sensorData.room.temperature, 1));
    html.replace("%ROOM_HUMIDITY%", String(sensorData.room.humidity, 1));
    html.replace("%PATIENT_TEMP%", String(sensorData.patient.temperature, 1));
    html.replace("%PATIENT_HUMIDITY%", String(sensorData.patient.humidity, 1));
    html.replace("%HEART_RATE%", String(sensorData.heartRate.bpm));
    
    // Patient status based on heart rate
    String patientStatus = "Good";
    if (sensorData.heartRate.bpm > 100) patientStatus = "Monitored";
    if (sensorData.heartRate.bpm > 120) patientStatus = "Alert";
    html.replace("%PATIENT_STATUS%", patientStatus);
    
    // Pill reminder info
    String pillTime = String(pillReminder.hours < 10 ? "0" : "") + String(pillReminder.hours) + ":";
    pillTime += String(pillReminder.minutes < 10 ? "0" : "") + String(pillReminder.minutes);
    html.replace("%NEXT_PILL_TIME%", pillTime);
    html.replace("%PILLS_TAKEN%", "Not tracked");
    html.replace("%PILL_STATUS%", "Active");
    
    // Comfort status
    String comfortStatus = "Good";
    if (sensorData.room.temperature < 18 || sensorData.room.temperature > 26) comfortStatus = "Check Temperature";
    if (sensorData.room.humidity < 30 || sensorData.room.humidity > 70) comfortStatus = "Check Humidity";
    html.replace("%COMFORT_STATUS%", comfortStatus);
    
    server.send(200, "text/html", html);
}

// Function to get the main dashboard content - unused after login implementation
void handleMainDashboard()
{
    // This function is now unused - dashboard access goes through login system
    server.sendHeader("Location", "/");
    server.send(302);
}

// LED handlers with simpler response
void handleLedOn() {
    digitalWrite(LED_BUILTIN, HIGH);
    server.send(200, "text/plain", "LED ON");
}

void handleLedOff() {
    digitalWrite(LED_BUILTIN, LOW);
    server.send(200, "text/plain", "LED OFF");
}

void handleCallNurse() {
    sendNurseAlert();
    
    // Write detailed message to serial monitor
    Serial.println("\n" + String("=").substring(0, 50));
    Serial.println("🚨 EMERGENCY: NURSE CALL ACTIVATED 🚨");
    Serial.println(String("=").substring(0, 50));
    Serial.println("⏰ Time: " + getRealTimeFormatted());
    Serial.println("👤 Patient Status: REQUESTING NURSE ASSISTANCE");
    Serial.println("🏥 Action Required: IMMEDIATE NURSE RESPONSE NEEDED");
    Serial.println("📍 Location: Patient Room");
    Serial.println("🔔 Alert Level: HIGH PRIORITY");
    Serial.println("💡 Instructions: Please send nurse to patient immediately");
    Serial.println(String("=").substring(0, 50) + "\n");
    
    // Send JavaScript response to show popup alert
    server.send(200, "text/html", 
        "<script>"
        "alert('🚨 NURSE ALERTED!\\n\\nNurse has been contacted successfully.\\nCheck serial monitor for details.');"
        "window.history.back();"
        "</script>");
}

void handleEmergency() {
    // Trigger emergency alert on OLED
    emergencyAlert = true;
    emergencyAlertTime = millis();
    emergencyAlertType = "MANUAL"; // Manual emergency trigger
    
    
    // Activate buzzer for emergency alert
    soundBuzzerAlarm();
    
    // Send JavaScript response to show popup alert
    server.send(200, "text/html", 
        "<script>"
        "alert('🚨🚨🚨 EMERGENCY ACTIVATED! 🚨🚨🚨\\n\\nCRITICAL ALERT has been triggered.\\nMedical assistance requested immediately.\\nCheck serial monitor for full details.');"
        "window.history.back();"
        "</script>");
}

// Pill Settings handler for both doctor and family
void handlePillSettings() {
    // Check if user is logged in as doctor or family
    if (!isLoggedIn || (currentUserType != "doctor" && currentUserType != "family")) {
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    
    // Check login timeout
    if (millis() - loginTime > LOGIN_TIMEOUT) {
        isLoggedIn = false;
        currentUserType = "";
        server.sendHeader("Location", "/?timeout=1");
        server.send(302);
        return;
    }
    
    if (server.method() == HTTP_POST) {
        // Handle form submission
        String hours = server.arg("hours");
        String minutes = server.arg("minutes");
        
        int newHours = hours.toInt();
        int newMinutes = minutes.toInt();
        
        // Validate input
        if (newHours >= 0 && newHours <= 23 && newMinutes >= 0 && newMinutes <= 59) {
            pillReminder.hours = newHours;
            pillReminder.minutes = newMinutes;
            pillReminder.isSet = true;
            pillReminder.alertShown = false; // Reset alert flag
            
           
            
            // Redirect back with success message
            server.send(200, "text/html", 
                "<script>"
                "alert('💊 PILL TIME UPDATED!\\n\\nPill reminder set to " + 
                String(newHours < 10 ? "0" : "") + String(newHours) + ":" + 
                String(newMinutes < 10 ? "0" : "") + String(newMinutes) + 
                "\\nPatient will be notified at this time.');"
                "window.history.back();"
                "</script>");
        } else {
            // Invalid input
            server.send(200, "text/html", 
                "<script>"
                "alert('❌ INVALID TIME!\\n\\nPlease enter valid hours (0-23) and minutes (0-59).');"
                "window.history.back();"
                "</script>");
        }
        return;
    }
    
    // Show pill settings form
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>💊 Pill Settings</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            padding: 20px;
        }
        .settings-container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            border-radius: 25px;
            padding: 40px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.3);
            color: #333;
            animation: slideIn 0.8s ease-out;
            max-width: 500px;
            width: 100%;
        }
        @keyframes slideIn {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .pill-icon { font-size: 3em; text-align: center; margin-bottom: 20px; }
        h1 { font-size: 2em; margin-bottom: 20px; color: #4CAF50; text-align: center; }
        .current-time {
            background: linear-gradient(45deg, #4CAF50, #45a049);
            color: white;
            padding: 15px;
            border-radius: 15px;
            text-align: center;
            margin-bottom: 25px;
            font-size: 1.5em;
            font-weight: bold;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            font-weight: bold;
            color: #555;
        }
        .time-inputs {
            display: flex;
            gap: 15px;
            align-items: center;
            justify-content: center;
        }
        input[type="number"] {
            width: 80px;
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 10px;
            font-size: 1.2em;
            text-align: center;
            font-weight: bold;
        }
        input[type="number"]:focus {
            outline: none;
            border-color: #4CAF50;
            box-shadow: 0 0 10px rgba(76, 175, 80, 0.3);
        }
        .colon {
            font-size: 2em;
            font-weight: bold;
            color: #666;
        }
        .btn-group {
            display: flex;
            gap: 15px;
            margin-top: 30px;
        }
        .btn {
            flex: 1;
            padding: 12px 24px;
            border: none;
            border-radius: 15px;
            font-size: 1.1em;
            font-weight: bold;
            cursor: pointer;
            transition: transform 0.3s ease;
            text-decoration: none;
            text-align: center;
            display: inline-block;
        }
        .btn:hover { transform: translateY(-2px); }
        .btn-primary {
            background: linear-gradient(45deg, #4CAF50, #45a049);
            color: white;
        }
        .btn-secondary {
            background: linear-gradient(45deg, #666, #555);
            color: white;
        }
        .user-info {
            text-align: center;
            margin-bottom: 20px;
            color: #666;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="settings-container">
        <div class="pill-icon">💊</div>
        <h1>Pill Reminder Settings</h1>
        <div class="user-info">Logged in as: )rawliteral" + currentUserType + R"rawliteral(</div>
        
        <div class="current-time">
            Current Setting: )rawliteral" + 
            String(pillReminder.hours < 10 ? "0" : "") + String(pillReminder.hours) + ":" +
            String(pillReminder.minutes < 10 ? "0" : "") + String(pillReminder.minutes) + R"rawliteral(
        </div>
        
        <form method="POST">
            <div class="form-group">
                <label>Set New Pill Reminder Time:</label>
                <div class="time-inputs">
                    <input type="number" name="hours" min="0" max="23" value=")rawliteral" + String(pillReminder.hours) + R"rawliteral(" required>
                    <span class="colon">:</span>
                    <input type="number" name="minutes" min="0" max="59" value=")rawliteral" + String(pillReminder.minutes) + R"rawliteral(" required>
                </div>
            </div>
            
            <div class="btn-group">
                <button type="submit" class="btn btn-primary">💊 Update Time</button>
                <a href="/)rawliteral" + currentUserType + R"rawliteral(-dashboard" class="btn btn-secondary">🏠 Back</a>
            </div>
        </form>
    </div>
</body>
</html>)rawliteral";
    
    server.send(200, "text/html", html);
}

// Medical Prescription handler (doctor only)
void handlePrescriptions() {
    // Check if user is logged in as doctor
    if (!isLoggedIn || currentUserType != "doctor") {
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    
    // Check login timeout
    if (millis() - loginTime > LOGIN_TIMEOUT) {
        isLoggedIn = false;
        currentUserType = "";
        server.sendHeader("Location", "/?timeout=1");
        server.send(302);
        return;
    }
    
    if (server.method() == HTTP_POST) {
        // Handle prescription submission
        String medication = server.arg("medication");
        String dosage = server.arg("dosage");
        String frequency = server.arg("frequency");
        String duration = server.arg("duration");
        String instructions = server.arg("instructions");
        String doctorName = server.arg("doctorName");
        
        // Validate input
        if (medication.length() > 0 && dosage.length() > 0 && frequency.length() > 0 && duration.length() > 0) {
            addPrescription(medication, dosage, frequency, duration, instructions, doctorName);
            
            // Log to serial monitor
            Serial.println("\n" + String("=").substring(0, 50));
            Serial.println("💊 NEW PRESCRIPTION ISSUED");
            Serial.println(String("=").substring(0, 50));
            Serial.println("⏰ Time: " + getRealTimeFormatted());
            Serial.println("👨‍⚕️ Doctor: " + (doctorName.length() > 0 ? doctorName : "Not specified"));
            Serial.println("💊 Medication: " + medication);
            Serial.println("📏 Dosage: " + dosage);
            Serial.println("🕐 Frequency: " + frequency);
            Serial.println("⏳ Duration: " + duration);
            if (instructions.length() > 0) {
                Serial.println("📝 Instructions: " + instructions);
            }
            Serial.println("👤 Patient Status: Prescription Added Successfully");
            Serial.println(String("=").substring(0, 50) + "\n");
            
            // Redirect back with success message
            server.send(200, "text/html", 
                "<script>"
                "alert('✅ PRESCRIPTION ISSUED!\\n\\nMedical prescription has been successfully added.\\nFamily members can now view it in their dashboard.');"
                "window.location.href='/doctor-dashboard';"
                "</script>");
            return;
        } else {
            server.send(200, "text/html", 
                "<script>"
                "alert('❌ VALIDATION ERROR\\n\\nPlease fill in all required fields:\\n- Medication name\\n- Dosage\\n- Frequency\\n- Duration');"
                "window.history.back();"
                "</script>");
            return;
        }
    }
    
    // Show prescription form
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>💊 Medical Prescription</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            padding: 20px;
        }
        .prescription-container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            border-radius: 25px;
            padding: 40px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.3);
            color: #333;
            animation: slideIn 0.8s ease-out;
            max-width: 600px;
            width: 100%;
        }
        @keyframes slideIn {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .prescription-icon { font-size: 3em; text-align: center; margin-bottom: 20px; }
        h1 { font-size: 2em; margin-bottom: 20px; color: #2196F3; text-align: center; }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            font-weight: bold;
            color: #555;
        }
        input, textarea, select {
            width: 100%;
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            font-size: 1em;
            transition: border-color 0.3s;
        }
        input:focus, textarea:focus, select:focus {
            outline: none;
            border-color: #2196F3;
        }
        textarea {
            height: 80px;
            resize: vertical;
        }
        .required { color: #f44336; }
        .form-row {
            display: flex;
            gap: 15px;
        }
        .form-row .form-group {
            flex: 1;
        }
        .btn-group {
            display: flex;
            gap: 15px;
            margin-top: 30px;
        }
        .btn {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 15px;
            font-size: 1.1em;
            cursor: pointer;
            text-decoration: none;
            text-align: center;
            transition: transform 0.3s ease;
        }
        .btn:hover { transform': translateY(-2px); }
        .btn-primary {
            background: linear-gradient(45deg, #2196F3, #1976D2);
            color: white;
        }
        .btn-secondary {
            background: linear-gradient(45deg, #757575, #424242);
            color: white;
        }
    </style>
</head>
<body>
    <div class="prescription-container">
        <div class="prescription-icon">💊</div>
        <h1>Medical Prescription</h1>
        <form method="post">
            <div class="form-group">
                <label for="doctorName">Doctor Name:</label>
                <input type="text" id="doctorName" name="doctorName" placeholder="Enter doctor's name">
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label for="medication">Medication <span class="required">*</span>:</label>
                    <input type="text" id="medication" name="medication" required placeholder="Enter medication name">
                </div>
                <div class="form-group">
                    <label for="dosage">Dosage <span class="required">*</span>:</label>
                    <input type="text" id="dosage" name="dosage" required placeholder="e.g., 500mg">
                </div>
            </div>
            <div class="form-row">
                <div class="form-group">
                    <label for="frequency">Frequency <span class="required">*</span>:</label>
                    <select id="frequency" name="frequency" required>
                        <option value="">Select frequency...</option>
                        <option value="Once daily">Once daily</option>
                        <option value="Twice daily">Twice daily</option>
                        <option value="Three times daily">Three times daily</option>
                        <option value="Four times daily">Four times daily</option>
                        <option value="Every 4 hours">Every 4 hours</option>
                        <option value="Every 6 hours">Every 6 hours</option>
                        <option value="Every 8 hours">Every 8 hours</option>
                        <option value="As needed">As needed</option>
                    </select>
                </div>
                <div class="form-group">
                    <label for="duration">Duration <span class="required">*</span>:</label>
                    <input type="text" id="duration" name="duration" required placeholder="e.g., 7 days, 2 weeks">
                </div>
            </div>
            <div class="form-group">
                <label for="instructions">Special Instructions:</label>
                <textarea id="instructions" name="instructions" placeholder="Additional instructions for the patient..."></textarea>
            </div>
            <div class="btn-group">
                <button type="submit" class="btn btn-primary">💊 Issue Prescription</button>
                <a href="/doctor-dashboard" class="btn btn-secondary">🔙 Back to Dashboard</a>
            </div>
        </form>
    </div>
</body>
</html>)rawliteral";
    
    server.send(200, "text/html", html);
}

// Messaging handler (both doctor and family)
void handleMessages() {
    // Check if user is logged in
    if (!isLoggedIn || (currentUserType != "doctor" && currentUserType != "family")) {
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    
    // Check login timeout
    if (millis() - loginTime > LOGIN_TIMEOUT) {
        isLoggedIn = false;
        currentUserType = "";
        server.sendHeader("Location", "/?timeout=1");
        server.send(302);
        return;
    }
    
    if (server.method() == HTTP_POST) {
        // Handle message submission
        String message = server.arg("message");
        
        if (message.length() > 0) {
            addMessage(message, currentUserType);
            
            // Log to serial monitor
            Serial.println("\n" + String("=").substring(0, 50));
            Serial.println("💬 NEW MESSAGE RECEIVED");
            Serial.println(String("=").substring(0, 50));
            Serial.println("⏰ Time: " + getRealTimeFormatted());
            String senderCapitalized = currentUserType;
            senderCapitalized.setCharAt(0, toupper(senderCapitalized.charAt(0)));
            Serial.println("👤 From: " + senderCapitalized);
            Serial.println("📝 Message: " + message);
            Serial.println("🔔 Status: Message delivered successfully");
            Serial.println(String("=").substring(0, 50) + "\n");
            
            // Redirect back with success message
            String redirectUrl = (currentUserType == "doctor") ? "/doctor-dashboard" : "/family-dashboard";
            server.send(200, "text/html", 
                "<script>"
                "alert('✅ MESSAGE SENT!\\n\\nYour message has been delivered successfully.');"
                "window.location.href='" + redirectUrl + "';"
                "</script>");
            return;
        } else {
            server.send(200, "text/html", 
                "<script>"
                "alert('❌ VALIDATION ERROR\\n\\nPlease enter a message before sending.');"
                "window.history.back();"
                "</script>");
            return;
        }
    }
    
    // Show messaging interface
    String senderTitle = (currentUserType == "doctor") ? "Doctor" : "Family";
    String recipientTitle = (currentUserType == "doctor") ? "Family" : "Doctor";
    String backUrl = (currentUserType == "doctor") ? "/doctor-dashboard" : "/family-dashboard";
    
    // Mark messages from other user type as read when viewing the message center
    for (int i = 0; i < messageCount; i++) {
        if (messages[i].isValid && !messages[i].isRead) {
            // Only mark messages from other user type as read
            if ((currentUserType == "doctor" && messages[i].senderType == "family") ||
                (currentUserType == "family" && messages[i].senderType == "doctor")) {
                messages[i].isRead = true;
            }
        }
    }
    
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>💬 Messages</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: white;
            padding: 20px;
        }
        .messages-container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            border-radius: 25px;
            padding: 30px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.3);
            color: #333;
            animation: slideIn 0.8s ease-out;
            max-width: 800px;
            margin: 0 auto;
        }
        @keyframes slideIn {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .messages-icon { font-size: 3em; text-align: center; margin-bottom: 20px; }
        h1 { font-size: 2em; margin-bottom: 20px; color: #9C27B0; text-align: center; }
        .chat-container {
            background: #f5f5f5;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 25px;
            max-height: 300px;
            overflow-y: auto;
        }
        .message {
            background: white;
            padding: 15px;
            border-radius: 10px;
            margin-bottom: 15px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        .message-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 8px;
        }
        .message-sender {
            font-weight: bold;
            color: #9C27B0;
        }
        .message-time {
            font-size: 0.9em;
            color: #666;
        }
        .message-content {
            line-height: 1.5;
        }
        .no-messages {
            text-align: center;
            color: #666;
            font-style: italic;
            padding: 40px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            font-weight: bold;
            color: #555;
        }
        textarea {
            width: 100%;
            padding: 15px;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            font-size: 1em;
            transition: border-color 0.3s;
            height: 100px;
            resize: vertical;
        }
        textarea:focus {
            outline: none;
            border-color: #9C27B0;
        }
        .btn-group {
            display: flex;
            gap: 15px;
            margin-top: 20px;
        }
        .btn {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 15px;
            font-size: 1.1em;
            cursor: pointer;
            text-decoration: none;
            text-align: center;
            transition: transform 0.3s ease;
        }
        .btn:hover { transform: translateY(-2px); }
        .btn-primary {
            background: linear-gradient(45deg, #9C27B0, #7B1FA2);
            color: white;
        }
        .btn-secondary {
            background: linear-gradient(45deg, #757575, #424242);
            color: white;
        }
    </style>
    <script>
        // Auto-scroll chat container to bottom to show newest messages
        window.onload = function() {
            var chatContainer = document.querySelector('.chat-container');
            chatContainer.scrollTop = chatContainer.scrollHeight;
        };
    </script>
</head>
<body>
    <div class="messages-container">
        <div class="messages-icon">💬</div>
        <h1>Message Center</h1>
        
        <div class="chat-container">)rawliteral" + getMessagesHTML() + R"rawliteral(</div>
        
        <form method="post">
            <div class="form-group">
                <label for="message">Send Message to )rawliteral" + recipientTitle + R"rawliteral(:</label>
                <textarea id="message" name="message" required placeholder="Type your message here..."></textarea>
            </div>
            <div class="btn-group">
                <button type="submit" class="btn btn-primary">📤 Send Message</button>
                <a href=")rawliteral" + backUrl + R"rawliteral(" class="btn btn-secondary">🔙 Back to Dashboard</a>
            </div>
        </form>
    </div>
</body>
</html>)rawliteral";
    
    server.send(200, "text/html", html);
}

// Weekly Report handler (doctor only)
void handleWeeklyReport() {
    // Check if user is logged in as doctor
    if (!isLoggedIn || currentUserType != "doctor") {
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    
    // Check login timeout
    if (millis() - loginTime > LOGIN_TIMEOUT) {
        isLoggedIn = false;
        currentUserType = "";
        server.sendHeader("Location", "/?timeout=1");
        server.send(302);
        return;
    }
    
    // Generate weekly report
    String currentDate = getRealDateTime();
    String reportDate = (currentDate != "Time not set") ? currentDate : "System Uptime: " + String(millis() / 1000) + " seconds";
    
    // Calculate average values (simplified - in real implementation, you'd store historical data)
    float avgRoomTemp = sensorData.room.temperature;
    float avgRoomHumid = sensorData.room.humidity;
    float avgPatientTemp = sensorData.patient.temperature;
    float avgPatientHumid = sensorData.patient.humidity;
    int avgHeartRate = sensorData.heartRate.bpm;
    float avgAirQuality = sensorData.airQuality.gasPercent;
    
    // Generate alerts summary using alert counters
    String alertsSummary = "";
    int alertCount = 0;
    
    // Heart rate alerts
    if (alertCounters.heartRateHigh > 0) {
        alertsSummary += "<li>High heart rate alerts: " + String(alertCounters.heartRateHigh) + " times (>120 BPM)</li>";
        alertCount += alertCounters.heartRateHigh;
    }
    
    // Room temperature alerts
    if (alertCounters.roomTempHigh > 0) {
        alertsSummary += "<li>Room temperature too high: " + String(alertCounters.roomTempHigh) + " times (>26°C)</li>";
        alertCount += alertCounters.roomTempHigh;
    }
    if (alertCounters.roomTempLow > 0) {
        alertsSummary += "<li>Room temperature too low: " + String(alertCounters.roomTempLow) + " times (<18°C)</li>";
        alertCount += alertCounters.roomTempLow;
    }
    
    // Room humidity alerts
    if (alertCounters.roomHumidHigh > 0) {
        alertsSummary += "<li>Room humidity too high: " + String(alertCounters.roomHumidHigh) + " times (>70%)</li>";
        alertCount += alertCounters.roomHumidHigh;
    }
    if (alertCounters.roomHumidLow > 0) {
        alertsSummary += "<li>Room humidity too low: " + String(alertCounters.roomHumidLow) + " times (<30%)</li>";
        alertCount += alertCounters.roomHumidLow;
    }
    
    // Patient area temperature alerts
    if (alertCounters.patientTempHigh > 0) {
        alertsSummary += "<li>Patient area too hot: " + String(alertCounters.patientTempHigh) + " times (>30°C)</li>";
        alertCount += alertCounters.patientTempHigh;
    }
    if (alertCounters.patientTempLow > 0) {
        alertsSummary += "<li>Patient area too cold: " + String(alertCounters.patientTempLow) + " times (<18°C)</li>";
        alertCount += alertCounters.patientTempLow;
    }
    
    // Patient area humidity alerts
    if (alertCounters.patientHumidHigh > 0) {
        alertsSummary += "<li>Patient area too humid: " + String(alertCounters.patientHumidHigh) + " times (>70%)</li>";
        alertCount += alertCounters.patientHumidHigh;
    }
    if (alertCounters.patientHumidLow > 0) {
        alertsSummary += "<li>Patient area too dry: " + String(alertCounters.patientHumidLow) + " times (<30%)</li>";
        alertCount += alertCounters.patientHumidLow;
    }
    
    // Air quality alerts
    if (alertCounters.airQualityDanger > 0) {
        alertsSummary += "<li>Air quality DANGER level: " + String(alertCounters.airQualityDanger) + " times (critical gas levels)</li>";
        alertCount += alertCounters.airQualityDanger;
    }
    if (alertCounters.airQualityWarning > 0) {
        alertsSummary += "<li>Air quality WARNING level: " + String(alertCounters.airQualityWarning) + " times (elevated gas levels)</li>";
        alertCount += alertCounters.airQualityWarning;
    }
    
    if (alertCount == 0) {
        alertsSummary = "<li>No alerts during monitoring period</li>";
    }
    
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>📊 Weekly Health Report</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: white;
            color: #333;
            line-height: 1.6;
            padding: 0;
        }
        
        .report-container {
            max-width: 210mm; /* A4 width */
            margin: 0 auto;
            background: white;
            min-height: 297mm; /* A4 height */
            padding: 20mm;
            box-shadow: 0 0 20px rgba(0,0,0,0.1);
        }
        
        .report-header {
            text-align: center;
            border-bottom: 3px solid #667eea;
            padding-bottom: 20px;
            margin-bottom: 30px;
        }
        
        .report-title {
            font-size: 24px;
            color: #667eea;
            font-weight: bold;
            margin-bottom: 10px;
        }
        
        .report-subtitle {
            font-size: 16px;
            color: #666;
            margin-bottom: 5px;
        }
        
        .report-date {
            font-size: 14px;
            color: #888;
        }
        
        .patient-info {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 30px;
            border-left: 5px solid #667eea;
        }
        
        .patient-info h2 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 18px;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
        }
        
        .info-item {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid #eee;
        }
        
        .info-label {
            font-weight: 600;
            color: #555;
        }
        
        .info-value {
            color: #333;
            font-weight: bold;
        }
        
        .section {
            margin-bottom: 30px;
        }
        
        .section-title {
            font-size: 18px;
            color: #667eea;
            border-bottom: 2px solid #eee;
            padding-bottom: 10px;
            margin-bottom: 20px;
            font-weight: bold;
        }
        
        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .metric-card {
            background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
            padding: 20px;
            border-radius: 10px;
            border: 1px solid #dee2e6;
            text-align: center;
        }
        
        .metric-title {
            font-size: 14px;
            color: #666;
            margin-bottom: 10px;
            font-weight: 600;
        }
        
        .metric-value {
            font-size: 24px;
            font-weight: bold;
            color: #667eea;
            margin-bottom: 5px;
        }
        
        .metric-unit {
            font-size: 12px;
            color: #888;
        }
        
        .status-good { color: #28a745; }
        .status-warning { color: #ffc107; }
        .status-danger { color: #dc3545; }
        
        .alerts-section {
            background: #fff3cd;
            border: 1px solid #ffeaa7;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
        }
        
        .alerts-title {
            color: #856404;
            font-weight: bold;
            margin-bottom: 15px;
            font-size: 16px;
        }
        
        .alerts-list {
            list-style: none;
            padding: 0;
        }
        
        .alerts-list li {
            padding: 8px 0;
            border-bottom: 1px solid #f8d7da;
            color: #721c24;
        }
        
        .alerts-list li:last-child {
            border-bottom: none;
        }
        
        .summary-section {
            background: #d1ecf1;
            border: 1px solid #bee5eb;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
        }
        
        .summary-title {
            color: #0c5460;
            font-weight: bold;
            margin-bottom: 15px;
            font-size: 16px;
        }
        
        .summary-text {
            color: #0c5460;
            line-height: 1.8;
        }
        
        .footer {
            text-align: center;
            padding-top: 20px;
            border-top: 1px solid #eee;
            color: #666;
            font-size: 12px;
        }
        
        .action-buttons {
            position: fixed;
            top: 20px;
            right: 20px;
            display: flex;
            gap: 10px;
            z-index: 1000;
        }
        
        .btn {
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            text-decoration: none;
            font-weight: bold;
            transition: all 0.3s ease;
        }
        
        .btn-primary {
            background: #667eea;
            color: white;
        }
        
        .btn-secondary {
            background: #6c757d;
            color: white;
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        }
        
        @media print {
            .action-buttons { display: none; }
            .report-container { 
                box-shadow: none; 
                margin: 0;
                padding: 15mm;
            }
            body { background: white; }
        }
        
        @page {
            size: A4;
            margin: 20mm;
        }
    </style>
</head>
<body>
    <div class="action-buttons">
        <button class="btn btn-primary" onclick="window.print()">🖨️ Print Report</button>
        <a href="/doctor-dashboard" class="btn btn-secondary">🔙 Back to Dashboard</a>
    </div>
    
    <div class="report-container">
        <div class="report-header">
            <div class="report-title">📊 WEEKLY HEALTH MONITORING REPORT</div>
            <div class="report-subtitle">Patient Health & Environmental Analysis</div>
            <div class="report-date">Generated: )rawliteral" + reportDate + R"rawliteral(</div>
        </div>
        
        <div class="patient-info">
            <h2>👤 Patient Information</h2>
            <div class="info-grid">
                <div class="info-item">
                    <span class="info-label">Patient ID:</span>
                    <span class="info-value">PATIENT-001</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Monitoring Period:</span>
                    <span class="info-value">7 Days</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Report Type:</span>
                    <span class="info-value">Weekly Summary</span>
                </div>
            </div>
        </div>
        
        <div class="section">
            <div class="section-title">📈 Vital Signs Summary</div>
            <div class="metrics-grid">
                <div class="metric-card">
                    <div class="metric-title">Average Heart Rate</div>
                    <div class="metric-value )rawliteral" + String(avgHeartRate > 100 ? "status-warning" : "status-good") + R"rawliteral(">)rawliteral" + String(avgHeartRate) + R"rawliteral(</div>
                    <div class="metric-unit">BPM</div>
                </div>
                <div class="metric-card">
                    <div class="metric-title">Patient Area Temperature</div>
                    <div class="metric-value )rawliteral" + String((avgPatientTemp > 30 || avgPatientTemp < 18) ? "status-warning" : "status-good") + R"rawliteral(">)rawliteral" + String(avgPatientTemp, 1) + R"rawliteral(</div>
                    <div class="metric-unit">°C</div>
                </div>
                <div class="metric-card">
                    <div class="metric-title">Patient Area Humidity</div>
                    <div class="metric-value )rawliteral" + String((avgPatientHumid > 70 || avgPatientHumid < 30) ? "status-warning" : "status-good") + R"rawliteral(">)rawliteral" + String(avgPatientHumid, 1) + R"rawliteral(</div>
                    <div class="metric-unit">%</div>
                </div>
            </div>
        </div>
        
        <div class="section">
            <div class="section-title">🏠 Environmental Conditions</div>
            <div class="metrics-grid">
                <div class="metric-card">
                    <div class="metric-title">Room Temperature</div>
                    <div class="metric-value )rawliteral" + String((avgRoomTemp > 26 || avgRoomTemp < 18) ? "status-warning" : "status-good") + R"rawliteral(">)rawliteral" + String(avgRoomTemp, 1) + R"rawliteral(</div>
                    <div class="metric-unit">°C</div>
                </div>
                <div class="metric-card">
                    <div class="metric-title">Room Humidity</div>
                    <div class="metric-value )rawliteral" + String((avgRoomHumid > 70 || avgRoomHumid < 30) ? "status-warning" : "status-good") + R"rawliteral(">)rawliteral" + String(avgRoomHumid, 1) + R"rawliteral(</div>
                    <div class="metric-unit">%</div>
                </div>
                <div class="metric-card">
                    <div class="metric-title">Air Quality Index</div>
                    <div class="metric-value )rawliteral" + String(avgAirQuality > 50 ? "status-warning" : "status-good") + R"rawliteral(">)rawliteral" + String(avgAirQuality, 1) + R"rawliteral(</div>
                    <div class="metric-unit">%</div>
                </div>
            </div>
        </div>
        
        <div class="section">
            <div class="alerts-section">
                <div class="alerts-title">⚠️ Alerts & Notifications Summary</div>
                <ul class="alerts-list">)rawliteral" + alertsSummary + R"rawliteral(</ul>
            </div>
        </div>
        
        <div class="section">
            <div class="summary-section">
                <div class="summary-title">📋 Clinical Summary</div>
                <div class="summary-text">
                    <p><strong>Overall Health Status:</strong> )rawliteral" + String(alertCount == 0 ? "Stable - No significant concerns detected during monitoring period." : "Requires Attention - " + String(alertCount) + " alert(s) detected during monitoring period.") + R"rawliteral(</p>
                    
                    <p><strong>Cardiovascular:</strong> Average heart rate of )rawliteral" + String(avgHeartRate) + R"rawliteral( BPM )rawliteral" + String(avgHeartRate > 100 ? "indicates elevated heart rate requiring monitoring." : "is within normal range.") + R"rawliteral(</p>
                    
                    <p><strong>Environmental Comfort:</strong> Room maintained at )rawliteral" + String(avgRoomTemp, 1) + R"rawliteral(°C with )rawliteral" + String(avgRoomHumid, 1) + R"rawliteral(% humidity. )rawliteral" + String((avgRoomTemp >= 18 && avgRoomTemp <= 26 && avgRoomHumid >= 30 && avgRoomHumid <= 70) ? "Environmental conditions are optimal for patient comfort." : "Environmental adjustments may be needed for optimal comfort.") + R"rawliteral(</p>
                    
                    <p><strong>Recommendations:</strong> )rawliteral" + String(alertCount == 0 ? "Continue current monitoring protocol. Patient environment and vitals are stable." : "Increase monitoring frequency and consider clinical intervention for elevated parameters.") + R"rawliteral(</p>
                </div>
            </div>
        </div>
        
        <div class="section">
            <div class="section-title">💊 Medication Compliance</div>
            <div class="info-grid">
                <div class="info-item">
                    <span class="info-label">Scheduled Pill Time:</span>
                    <span class="info-value">)rawliteral" + String(pillReminder.hours < 10 ? "0" : "") + String(pillReminder.hours) + ":" + String(pillReminder.minutes < 10 ? "0" : "") + String(pillReminder.minutes) + R"rawliteral(</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Reminder Status:</span>
                    <span class="info-value">)rawliteral" + String(pillReminder.isSet ? "Active" : "Inactive") + R"rawliteral(</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Compliance Rate:</span>
                    <span class="info-value">Tracking Available</span>
                </div>
            </div>
        </div>
        
        <div class="footer">
            <p>📋 Generated by Health Monitor System | 🏥 Patient Care Dashboard</p>
            <p>For medical emergencies, contact healthcare provider immediately</p>
            <p>Report ID: RPT-)rawliteral" + String(millis()) + R"rawliteral( | System Version: 1.0</p>
        </div>
    </div>
</body>
</html>)rawliteral";
    
    server.send(200, "text/html", html);
}

// Reset Alert Counters handler (doctor only)
void handleResetCounters() {
    // Check if user is logged in as doctor
    if (!isLoggedIn || currentUserType != "doctor") {
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    
    // Check login timeout
    if (millis() - loginTime > LOGIN_TIMEOUT) {
        isLoggedIn = false;
        currentUserType = "";
        server.sendHeader("Location", "/?timeout=1");
        server.send(302);
        return;
    }
    
    // Reset all alert counters
    resetAlertCounters();
    
    // Send success response
    server.send(200, "text/html", 
        "<script>"
        "alert('📊 ALERT COUNTERS RESET!\\n\\nNew monitoring period started.\\nAll alert counters have been reset to zero.');"
        "window.location.href='/doctor-dashboard';"
        "</script>");
}

void handleCallFamily() {
    // Check if user is logged in as family - they shouldn't contact themselves
    if (isLoggedIn && currentUserType == "family") {
        // Return 404 Not Found - Family members shouldn't call themselves
        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>404 - Not Found</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            text-align: center;
        }
        .error-container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            border-radius: 25px;
            padding: 50px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.3);
            color: #333;
            animation: slideIn 0.8s ease-out;
        }
        @keyframes slideIn {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .error-icon { font-size: 4em; margin-bottom: 20px; }
        h1 { font-size: 2.5em; margin-bottom: 20px; color: #ff6b6b; }
        p { font-size: 1.2em; margin-bottom: 20px; line-height: 1.6; color: #666; }
        .back-btn {
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 15px;
            font-size: 1.1em;
            cursor: pointer;
            text-decoration: none;
            display: inline-block;
            margin-top: 20px;
            transition: transform 0.3s ease;
        }
        .back-btn:hover { transform: translateY(-2px); }
    </style>
</head>
<body>
    <div class="error-container">
        <div class="error-icon">🚫</div>
        <h1>404 - Not Found</h1>
        <p><strong>This page is not available.</strong></p>
        <p>Family members cannot call themselves.</p>
        <p>Use the "Call Nurse" button if you need assistance.</p>
        <a href="/" class="back-btn">🏠 Back to Dashboard</a>
    </div>
</body>
</html>)rawliteral";
        
        server.send(404, "text/html", html);
        return;
    }
    
    // For doctors - allow calling family
    sendFamilyAlert();
    
    // Write detailed message to serial monitor for doctor calling family
    Serial.println("\n" + String("=").substring(0, 50));
    Serial.println("📞 DOCTOR: FAMILY CALL ACTIVATED 📞");
    Serial.println(String("=").substring(0, 50));
    Serial.println("⏰ Time: " + getRealTimeFormatted());
    Serial.println("👨‍⚕️ Doctor Status: CONTACTING PATIENT'S FAMILY");
    Serial.println("🏥 Action Required: FAMILY NOTIFICATION NEEDED");
    Serial.println("📍 Location: Medical Facility");
    Serial.println("🔔 Alert Level: NORMAL PRIORITY");
    Serial.println("💡 Instructions: Please contact patient's family members");
    Serial.println("👨‍👩‍👧‍👦 Family communication requested by doctor");
    Serial.println(String("=").substring(0, 50) + "\n");
    
    // Send JavaScript response to show popup alert
    server.send(200, "text/html", 
        "<script>"
        "alert('📞 FAMILY CONTACTED!\\n\\nFamily has been notified successfully.\\nCheck serial monitor for details.');"
        "window.history.back();"
        "</script>");
}



// LED handlers with simpler response
void handleLEDOn()
{
  digitalWrite(LED_BUILTIN, HIGH);
  server.sendHeader("Location", "/");
  server.send(303); // See Other
}

void handleLEDOff()
{
  digitalWrite(LED_BUILTIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303); // See Other
}

// Contact Nurse handler for web interface
void handleContactNurse()
{
  // Send nurse alert to serial monitor (same as button press)
  sendNurseAlert();
  
  // Send success response
  server.send(200, "text/plain", "Nurse contacted successfully");
}

// Contact Family handler for web interface
void handleContactFamily()
{
  // Check if user is logged in as family - they shouldn't contact themselves
  if (isLoggedIn && currentUserType == "family") {
    server.send(404, "text/plain", "Not Found - Family members cannot call themselves");
    return;
  }
  
  // Send family alert to serial monitor (same as button press)
  sendFamilyAlert();
  
  // Send success response
  server.send(200, "text/plain", "Family contacted successfully");
}

// 📝 Heart Rate Alert Serial Logger
void logHeartRateAlert(int heartRate) {
    // Check 30-minute cooldown period for heart rate alerts
    unsigned long currentTime = millis();
    
    // If less than 30 minutes since last heart rate alert, skip
    if (alertCooldowns.heartRateHigh != 0 && (currentTime - alertCooldowns.heartRateHigh) < ALERT_COOLDOWN_PERIOD) {
        return;
    }
    
    // Also check if a full-screen alert is already active
    if (fullScreenAlert && emergencyAlert) {
        return; // Don't trigger a new alert while one is already active
    }
    
    Serial.println("");
    Serial.println("==================================");
  
    
    // Create timestamp string with real date/time
    String realDateTime = getRealDateTime();
    char timestamp[50];
    
    if (realDateTime != "Time not set") {
        // Use real date and time
        snprintf(timestamp, sizeof(timestamp), "%s", realDateTime.c_str());
    } else {
        // Fallback to uptime if NTP failed
        unsigned long uptime = millis() / 1000;
        uint8_t uptimeHours = uptime / 3600;
        uint8_t uptimeMinutes = (uptime % 3600) / 60;
        uint8_t uptimeSeconds = uptime % 60;
        snprintf(timestamp, sizeof(timestamp), "Uptime: %02d:%02d:%02d", 
                 uptimeHours, uptimeMinutes, uptimeSeconds);
    }
    
    // Log detailed alert to serial monitor
    Serial.println("🚨 CRITICAL HEART RATE ALERT 🚨");
    Serial.println("====================================");
    Serial.print("⏰ Time: ");
    Serial.println(timestamp);
    Serial.print("❤️ Heart Rate: ");
    Serial.println(heartRate);
    Serial.println("⚠️  Status: DANGEROUS (Above 120 BPM threshold)");
    Serial.println("🔧 Health Monitor System");
    Serial.println("==========================================");
    

    // Update cooldown timestamp
    alertCooldowns.heartRateHigh = currentTime;
    
    // Trigger visual emergency alert and buzzer (only if not already active)
    emergencyAlert = true;
    fullScreenAlert = true;
    emergencyAlertTime = millis();
    emergencyAlertType = "HEART"; // Set alert type for heart rate
    
    // Increment heart rate alert counter
    alertCounters.heartRateHigh++;
    
}

// 🌡️ Room Temperature Alert Logger
void logRoomTempAlert(float temperature, bool isHigh) {
    // Check 30-minute cooldown period for specific alert type
    unsigned long currentTime = millis();
    unsigned long lastAlertTime = isHigh ? alertCooldowns.roomTempHigh : alertCooldowns.roomTempLow;
    
    // If less than 30 minutes since last alert of this type, skip
    if (lastAlertTime != 0 && (currentTime - lastAlertTime) < ALERT_COOLDOWN_PERIOD) {
        return;
    }
    
    // Trigger visual alert and sound
    environmentalAlert = true;
    environmentalAlertTime = millis();
    if (isHigh) {
        currentEnvironmentalAlert = "Room TOO hot";
        alertStates.roomTempHigh = true;
        alertStates.roomTempLow = false;
        // Increment room temperature high counter
        alertCounters.roomTempHigh++;
        // Update cooldown timestamp
        alertCooldowns.roomTempHigh = currentTime;
    } else {
        currentEnvironmentalAlert = "Room too cold";
        alertStates.roomTempLow = true;
        alertStates.roomTempHigh = false;
        // Increment room temperature low counter
        alertCounters.roomTempLow++;
        // Update cooldown timestamp
        alertCooldowns.roomTempLow = currentTime;
    }
    environmentalAlertActive = true;
}

// 💧 Room Humidity Alert Logger
void logRoomHumidityAlert(float humidity, bool isHigh) {
    // Check 30-minute cooldown period for specific alert type
    unsigned long currentTime = millis();
    unsigned long lastAlertTime = isHigh ? alertCooldowns.roomHumidHigh : alertCooldowns.roomHumidLow;
    
    // If less than 30 minutes since last alert of this type, skip
    if (lastAlertTime != 0 && (currentTime - lastAlertTime) < ALERT_COOLDOWN_PERIOD) {
        return;
    }
    
    // Trigger visual alert and sound
    environmentalAlert = true;
    environmentalAlertTime = millis();
    if (isHigh) {
        currentEnvironmentalAlert = "Room TOO humid";
        alertStates.roomHumidHigh = true;
        alertStates.roomHumidLow = false;
        // Increment room humidity high counter
        alertCounters.roomHumidHigh++;
        // Update cooldown timestamp
        alertCooldowns.roomHumidHigh = currentTime;
    } else {
        currentEnvironmentalAlert = "Room too dry";
        alertStates.roomHumidLow = true;
        alertStates.roomHumidHigh = false;
        // Increment room humidity low counter
        alertCounters.roomHumidLow++;
        // Update cooldown timestamp
        alertCooldowns.roomHumidLow = currentTime;
    }
    environmentalAlertActive = true;
}

// 🌡️ Patient Area Temperature Alert Logger
void logBodyTempAlert(float temperature, bool isHigh) {
    // Check 30-minute cooldown period for specific alert type
    unsigned long currentTime = millis();
    unsigned long lastAlertTime = isHigh ? alertCooldowns.patientTempHigh : alertCooldowns.patientTempLow;
    
    // If less than 30 minutes since last alert of this type, skip
    if (lastAlertTime != 0 && (currentTime - lastAlertTime) < ALERT_COOLDOWN_PERIOD) {
        return;
    }
    
    // Trigger visual alert and sound
    environmentalAlert = true;
    environmentalAlertTime = millis();
    if (isHigh) {
        currentEnvironmentalAlert = "Patient TOO hot";
        alertStates.patientTempHigh = true;
        alertStates.patientTempLow = false;
        // Increment patient temperature high counter
        alertCounters.patientTempHigh++;
        // Update cooldown timestamp
        alertCooldowns.patientTempHigh = currentTime;
    } else {
        currentEnvironmentalAlert = "Patient too cold";
        alertStates.patientTempLow = true;
        alertStates.patientTempHigh = false;
        // Increment patient temperature low counter
        alertCounters.patientTempLow++;
        // Update cooldown timestamp
        alertCooldowns.patientTempLow = currentTime;
    }
    environmentalAlertActive = true;
}


void logBodyHumidityAlert(float humidity, bool isHigh) {
    // Check 30-minute cooldown period for specific alert type
    unsigned long currentTime = millis();
    unsigned long lastAlertTime = isHigh ? alertCooldowns.patientHumidHigh : alertCooldowns.patientHumidLow;
    
    // If less than 30 minutes since last alert of this type, skip
    if (lastAlertTime != 0 && (currentTime - lastAlertTime) < ALERT_COOLDOWN_PERIOD) {
        return;
    }
    
    // Trigger visual alert and sound
    environmentalAlert = true;
    environmentalAlertTime = millis();
    if (isHigh) {
        currentEnvironmentalAlert = "Patient TOO humid";
        alertStates.patientHumidHigh = true;
        alertStates.patientHumidLow = false;
        // Increment patient humidity high counter
        alertCounters.patientHumidHigh++;
        // Update cooldown timestamp
        alertCooldowns.patientHumidHigh = currentTime;
    } else {
        currentEnvironmentalAlert = "Patient too dry";
        alertStates.patientHumidLow = true;
        alertStates.patientHumidHigh = false;
        // Increment patient humidity low counter
        alertCounters.patientHumidLow++;
        // Update cooldown timestamp
        alertCooldowns.patientHumidLow = currentTime;
    }
    environmentalAlertActive = true;
}

// 🌬️ Air Quality Alert Logger
void logAirQualityAlert(int gasLevel, bool isDanger) {
    // Check 30-minute cooldown period for specific alert type
    unsigned long currentTime = millis();
    unsigned long lastAlertTime = isDanger ? alertCooldowns.airQualityDanger : alertCooldowns.airQualityWarning;
    
    // If less than 30 minutes since last alert of this type, skip
    if (lastAlertTime != 0 && (currentTime - lastAlertTime) < ALERT_COOLDOWN_PERIOD) {
        return;
    }
    
    // Trigger visual alert and sound
    environmentalAlert = true;
    environmentalAlertTime = millis();
    if (isDanger) {
        currentEnvironmentalAlert = "Air DANGER!";
        alertStates.airQualityDanger = true;
        alertStates.airQualityWarning = false;
        // Increment air quality danger counter
        alertCounters.airQualityDanger++;
        // Update cooldown timestamp
        alertCooldowns.airQualityDanger = currentTime;
    } else {
        currentEnvironmentalAlert = "Air Warning!";
        alertStates.airQualityWarning = true;
        alertStates.airQualityDanger = false;
        // Increment air quality warning counter
        alertCounters.airQualityWarning++;
        // Update cooldown timestamp
        alertCooldowns.airQualityWarning = currentTime;
    }
    environmentalAlertActive = true;
}

// 🔍 Room Environment Alert Checker
void checkRoomAlerts(float temp, float humid) {
    // Check room temperature
    if (temp >= ROOM_TEMP_HIGH) {
        logRoomTempAlert(temp, true);  // High temperature
    } else if (temp <= ROOM_TEMP_LOW) {
        logRoomTempAlert(temp, false); // Low temperature
    }
    
    // Check room humidity
    if (humid >= ROOM_HUMID_HIGH) {
        logRoomHumidityAlert(humid, true);  // High humidity
    } else if (humid <= ROOM_HUMID_LOW) {
        logRoomHumidityAlert(humid, false); // Low humidity
    }
    
    // Clear alert states if conditions return to normal
    clearAlertStates(temp, humid, 0, 0); // Room data only
}

// 🔍 Patient Area Environment Alert Checker
void checkBodyAlerts(float temp, float humid) {
    // Check patient area temperature
    if (temp >= BODY_TEMP_HIGH) {
        logBodyTempAlert(temp, true);  // Patient area too hot
    } else if (temp <= BODY_TEMP_LOW) {
        logBodyTempAlert(temp, false); // Patient area too cold
    }
    
    // Check patient area humidity
    if (humid >= BODY_HUMID_HIGH) {
        logBodyHumidityAlert(humid, true);  // Patient area too humid
    } else if (humid <= BODY_HUMID_LOW) {
        logBodyHumidityAlert(humid, false); // Patient area too dry
    }
    
    // Clear alert states if conditions return to normal
    clearAlertStates(0, 0, temp, humid); // Patient data only
}

// 🔍 Air Quality Alert Checker
void checkAirQualityAlerts(int gasLevel) {
    // Check air quality levels
    if (gasLevel >= MQ_DANGER_LEVEL) {
        logAirQualityAlert(gasLevel, true);  // Dangerous level
    } else if (gasLevel >= MQ_WARNING_LEVEL) {
        logAirQualityAlert(gasLevel, false); // Warning level
    } else if (gasLevel < MQ_SAFE_LEVEL) {
        // Clear air quality alerts when level is safe
        alertStates.airQualityWarning = false;
        alertStates.airQualityDanger = false;
    }
}

// Function to reset alert counters (for new monitoring period)
void resetAlertCounters() {
    // Reset alert counters
    alertCounters.heartRateHigh = 0;
    alertCounters.roomTempHigh = 0;
    alertCounters.roomTempLow = 0;
    alertCounters.roomHumidHigh = 0;
    alertCounters.roomHumidLow = 0;
    alertCounters.patientTempHigh = 0;
    alertCounters.patientTempLow = 0;
    alertCounters.patientHumidHigh = 0;
    alertCounters.patientHumidLow = 0;
    alertCounters.airQualityWarning = 0;
    alertCounters.airQualityDanger = 0;
    
    // Reset alert cooldowns (allow immediate alerts after reset)
    alertCooldowns.heartRateHigh = 0;
    alertCooldowns.roomTempHigh = 0;
    alertCooldowns.roomTempLow = 0;
    alertCooldowns.roomHumidHigh = 0;
    alertCooldowns.roomHumidLow = 0;
    alertCooldowns.patientTempHigh = 0;
    alertCooldowns.patientTempLow = 0;
    alertCooldowns.patientHumidHigh = 0;
    alertCooldowns.patientHumidLow = 0;
    alertCooldowns.airQualityWarning = 0;
    alertCooldowns.airQualityDanger = 0;
    
  
}


void checkPillReminder() {
    // Pill reminder is always active, no need to check isSet
    
    // Get current time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        
        return; // Can't get time, skip check
    }
    
    int currentHour = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;
    
    // Debug output every minute to show pill checking
    static int lastDebugMinute = -1;
    if (currentMinute != lastDebugMinute) {
        lastDebugMinute = currentMinute;
    }
    
    // Check if current time matches pill time
    if (currentHour == pillReminder.hours && currentMinute == pillReminder.minutes) {
        // Only show alert once per minute (to avoid spam)
        if (!pillReminder.alertShown) {
            
            // Trigger visual alert
            emergencyAlert = true;
            fullScreenAlert = true;
            emergencyAlertTime = millis();
            emergencyAlertType = "PILL"; // Set alert type for pill reminder
            
            // Sound buzzer alarm
            soundBuzzerAlarm();
            
            pillReminder.alertShown = true;
        }
    } else {
        // Reset alert flag when time changes
        pillReminder.alertShown = false;
    }
}

// Update system time
void updateSystemTime() {
    unsigned long currentMillis = millis();
    unsigned long elapsed = currentMillis - lastTimeUpdate;
    
    if (elapsed >= 1000) {  // Update every second
        systemTime.seconds++;
        if (systemTime.seconds >= 60) {
            systemTime.seconds = 0;
            systemTime.minutes++;
            if (systemTime.minutes >= 60) {
                systemTime.minutes = 0;
                systemTime.hours++;
                if (systemTime.hours >= 24) {
                    systemTime.hours = 0;
                }
            }
        }
        lastTimeUpdate = currentMillis;
    }
}

// Get real date and time string
String getRealDateTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Time not set";
    }
    
    char dateTimeStr[30];
    strftime(dateTimeStr, sizeof(dateTimeStr), "%Y/%m/%d %H:%M:%S", &timeinfo);
    return String(dateTimeStr);
}

// Get formatted real time for display
String getRealTimeFormatted() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "N/A";
    }
    
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    return String(timeStr);
}

// Convert Gregorian date to Persian (Shamsi) calendar
String getPersianDate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Time not set";
    }
    
    // Simple Persian calendar conversion
    int gYear = timeinfo.tm_year + 1900;
    int gMonth = timeinfo.tm_mon + 1;
    int gDay = timeinfo.tm_mday;
    
    // Basic conversion algorithm (simplified)
    int pYear = gYear - 621;
    if (gMonth < 3 || (gMonth == 3 && gDay < 21)) {
        pYear--;
    }
    
    // Approximate Persian month and day calculation
    int pMonth, pDay;
    
    // Simplified conversion based on Gregorian month
    if (gMonth >= 3 && gMonth <= 5) {
        // Spring months (فروردین، اردیبهشت، خرداد)
        pMonth = gMonth - 2;
        pDay = gDay;
        if (gMonth == 3 && gDay < 21) {
            pMonth = 12;
            pDay = gDay + 10;
            pYear--;
        }
    } else if (gMonth >= 6 && gMonth <= 8) {
        // Summer months (تیر، مرداد، شهریور)
        pMonth = gMonth - 2;
        pDay = gDay;
    } else if (gMonth >= 9 && gMonth <= 11) {
        // Autumn months (مهر، آبان، آذر)
        pMonth = gMonth - 2;
        pDay = gDay;
    } else {
        // Winter months (دی، بهمن، اسفند)
        pMonth = gMonth + 10;
        pDay = gDay;
        if (pMonth > 12) {
            pMonth -= 12;
        }
    }
    
    // Ensure valid ranges
    if (pMonth > 12) pMonth = 12;
    if (pMonth < 1) pMonth = 1;
    if (pDay > 31) pDay = 31;
    if (pDay < 1) pDay = 1;
    
    // Format in YYYY/MM/DD
    char persianDateStr[20];
    snprintf(persianDateStr, sizeof(persianDateStr), "%04d/%02d/%02d", 
             pYear, pMonth, pDay);
    
    return String(persianDateStr);
}

// Message and prescription management functions
void addPrescription(String medication, String dosage, String frequency, String duration, String instructions, String doctorName) {
    if (prescriptionCount < MAX_PRESCRIPTIONS) {
        prescriptions[prescriptionCount].medication = medication;
        prescriptions[prescriptionCount].dosage = dosage;
        prescriptions[prescriptionCount].frequency = frequency;
        prescriptions[prescriptionCount].duration = duration;
        prescriptions[prescriptionCount].instructions = instructions;
        prescriptions[prescriptionCount].doctorName = doctorName;
        prescriptions[prescriptionCount].timestamp = getRealDateTime();
        prescriptions[prescriptionCount].isValid = true;
        prescriptionCount++;
    }
}

void addMessage(String content, String senderType) {
    if (messageCount < MAX_MESSAGES) {
        messages[messageCount].content = content;
        messages[messageCount].senderType = senderType;
        messages[messageCount].timestamp = getRealDateTime();
        messages[messageCount].isValid = true;
        messages[messageCount].isRead = false;
        messageCount++;
        // Message notifications are handled via web dashboard only
    }
}

String getPrescriptionsHTML() {
    if (prescriptionCount == 0) {
        return "<div class='no-prescriptions'>📝 No prescriptions available</div>";
    }
    
    String html = "";
    for (int i = prescriptionCount - 1; i >= 0; i--) { // Show newest first
        if (prescriptions[i].isValid) {
            html += "<div class='prescription'>";
            html += "<div class='prescription-header'>";
            html += "<div class='prescription-medication'>💊 " + prescriptions[i].medication + "</div>";
            html += "<div class='prescription-time'>" + prescriptions[i].timestamp + "</div>";
            html += "</div>";
            if (prescriptions[i].doctorName.length() > 0) {
                html += "<div class='prescription-doctor'>👨‍⚕️ Dr. " + prescriptions[i].doctorName + "</div>";
            }
            html += "<div class='prescription-details'>";
            html += "<strong>📏 Dosage:</strong> " + prescriptions[i].dosage + "<br>";
            html += "<strong>🕐 Frequency:</strong> " + prescriptions[i].frequency + "<br>";
            html += "<strong>⏳ Duration:</strong> " + prescriptions[i].duration;
            if (prescriptions[i].instructions.length() > 0) {
                html += "<br><strong>📝 Instructions:</strong> " + prescriptions[i].instructions;
            }
            html += "</div></div>";
        }
    }
    return html;
}

String getMessagesHTML() {
    if (messageCount == 0) {
        return "<div class='no-messages'>💭 No messages yet</div>";
    }
    
    String html = "";
    for (int i = 0; i < messageCount; i++) { // Show oldest first, newest at bottom
        if (messages[i].isValid) {
            String senderIcon = (messages[i].senderType == "doctor") ? "👨‍⚕️" : "👨‍👩‍👧‍👦";
            String senderName = messages[i].senderType;
            senderName.setCharAt(0, toupper(senderName.charAt(0)));
            
            html += "<div class='message'>";
            html += "<div class='message-header'>";
            html += "<div class='message-sender'>" + senderIcon + " " + senderName + "</div>";
            html += "<div class='message-time'>" + messages[i].timestamp + "</div>";
            html += "</div>";
            html += "<div class='message-content'>" + messages[i].content + "</div>";
            html += "</div>";
        }
    }
    return html;
}

int getUnreadMessageCount() {
    int count = 0;
    for (int i = 0; i < messageCount; i++) {
        if (messages[i].isValid && !messages[i].isRead) {
            // Only count messages from other user type
            if ((currentUserType == "doctor" && messages[i].senderType == "family") ||
                (currentUserType == "family" && messages[i].senderType == "doctor")) {
                count++;
            }
        }
    }
    return count;
}

// Removed pill dispensing system functions

// Room DHT Sensor Task
void roomDhtTask(void * parameter) {
    while(1) {
        if (xSemaphoreTake(dhtMutex, portMAX_DELAY)) {
            float newTemp = roomDht.readTemperature();
            float newHum = roomDht.readHumidity();
            
            if (!isnan(newTemp) && !isnan(newHum)) {
                sensorData.room.temperature = newTemp;
                sensorData.room.humidity = newHum;
                sensorData.room.lastUpdate = millis();
                
                // Check for room environment alerts
                checkRoomAlerts(newTemp, newHum);
            }
            xSemaphoreGive(dhtMutex);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Read every 2 seconds
    }
}

// Patient DHT Sensor Task
void patientDhtTask(void * parameter) {
    while(1) {
        if (xSemaphoreTake(dhtMutex, portMAX_DELAY)) {
            float newTemp = patientDht.readTemperature();
            float newHum = patientDht.readHumidity();
            
            if (!isnan(newTemp) && !isnan(newHum)) {
                sensorData.patient.temperature = newTemp;
                sensorData.patient.humidity = newHum;
                sensorData.patient.lastUpdate = millis();
                
                // Check for body/patient environment alerts
                checkBodyAlerts(newTemp, newHum);
            }
            xSemaphoreGive(dhtMutex);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Read every 2 seconds
    }
}

// Heart Rate Monitoring Task (using potentiometer simulation)
void heartRateTask(void * parameter) {
    static unsigned long lastHeartBeat = 0;
    static bool heartBeatPhase = false;
    
    while(1) {
        if (xSemaphoreTake(heartRateMutex, portMAX_DELAY)) {
            // Read potentiometer value for base heart rate
            int potValue = analogRead(HEART_RATE_PIN);
            
            // Map potentiometer value to heart rate range (60-120 BPM)
            int baseHeartRate = map(potValue, 0, 4095, 60, 120);
            
            // Calculate heart beat interval in milliseconds
            unsigned long heartBeatInterval = 60000 / baseHeartRate; // 60000ms = 1 minute
            
            // Add realistic heart rate variation (±3 BPM)
            unsigned long currentTime = millis();
            if (currentTime - lastHeartBeat >= heartBeatInterval) {
                lastHeartBeat = currentTime;
                heartBeatPhase = !heartBeatPhase;
                
                // Add small random variation to simulate natural heart rate
                int variation = random(-3, 4);
                int heartRate = baseHeartRate + variation;
                
                // Add slight increase during "beat" phase
                if (heartBeatPhase) {
                    heartRate += 1;
                }
                
                // Ensure heart rate stays within realistic bounds (allow up to 180 for emergency detection)
                heartRate = constrain(heartRate, 60, 180);
                
                sensorData.heartRate.bpm = heartRate;
                sensorData.heartRate.lastUpdate = millis();
                sensorData.heartRate.isValid = true;
                
                // 🚨 Emergency Alert for dangerous heart rate
                if (heartRate >= 120) {
                    // Log alert to serial
                    logHeartRateAlert(heartRate);
                }
            }
            
            xSemaphoreGive(heartRateMutex);
        }
        vTaskDelay(250 / portTICK_PERIOD_MS);  // Increased to 250ms to reduce system load
    }
}

// MQ Gas Sensor Monitoring Task
void mqSensorTask(void * parameter) {
    while(1) {
        if (xSemaphoreTake(mqSensorMutex, portMAX_DELAY)) {
            // Read MQ sensor analog value
            int gasReading = analogRead(MQ_SENSOR_PIN);
            
            // Convert to percentage (0-100%)
            float gasPercent = map(gasReading, 0, 4095, 0, 100);
            
            sensorData.airQuality.gasLevel = gasReading;
            sensorData.airQuality.gasPercent = gasPercent;
            sensorData.airQuality.lastUpdate = millis();
            sensorData.airQuality.isValid = true;
            
            // Debug output every 5 seconds
            static unsigned long lastDebugPrint = 0;
            if (millis() - lastDebugPrint > 5000) {
                
                lastDebugPrint = millis();
            }
            
            // Check for air quality alerts
            checkAirQualityAlerts(gasReading);
            
            xSemaphoreGive(mqSensorMutex);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Increased to 2 seconds to reduce system load
    }
}

// 🔊 Buzzer Alarm Function
void soundBuzzerAlarm() {
    // Create an emergency alarm pattern with varying tones
    for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 1000, 200);  // High tone for 200ms
        delay(200);
        tone(BUZZER_PIN, 800, 200);   // Lower tone for 200ms
        delay(200);
    }
    noTone(BUZZER_PIN);  // Ensure buzzer is properly turned off
}

// 🌡️ Environmental Alert Sound Function
void soundEnvironmentalAlert() {
    // Create a milder environmental alert pattern
    for (int i = 0; i < 2; i++) {
        tone(BUZZER_PIN, 600, 150);   // Medium tone for 150ms
        delay(150);
        tone(BUZZER_PIN, 500, 150);   // Lower tone for 150ms
        delay(150);
    }
    noTone(BUZZER_PIN);  // Ensure buzzer is properly turned off
}

// 🌐 Web Server Task
void webServerTask(void * parameter) {
    while(1) {
        if (WiFi.status() == WL_CONNECTED) {
            server.handleClient();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Small delay to prevent watchdog issues
    }
}

// 📺 Display Task
void displayTask(void * parameter) {
    while(1) {
        // Check pill reminder every cycle
        checkPillReminder();
        
        // Use shorter timeout to avoid blocking button responses
        if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(50))) {
            // Check if emergency alert should be displayed
            if (emergencyAlert && fullScreenAlert) {
                // Check if alert duration has passed
                if (millis() - emergencyAlertTime >= EMERGENCY_ALERT_DURATION) {
                    emergencyAlert = false;
                    fullScreenAlert = false;
                    noTone(BUZZER_PIN);  // Ensure buzzer is off
                } else {
                    // Display full-screen emergency alert
                    display.clearDisplay();
                    display.setTextSize(2);
                    display.setTextColor(SSD1306_WHITE);
                    
                    // Flashing effect for emergency
                    bool flash = (millis() / 250) % 2;
                    if (flash) {
                        display.fillScreen(SSD1306_WHITE);
                        display.setTextColor(SSD1306_BLACK);
                    }
                    
                    // Check alert type and display appropriate message
                    if (emergencyAlertType == "PILL") {
                        // Pill reminder alert
                        display.setTextSize(2);
                        display.setCursor(15, 8);
                        display.println(F("PILL"));
                        display.setTextSize(2);
                        display.setCursor(15, 28);
                        display.println(F("TIME!"));
                        
                        // Show pill time and message
                        display.setTextSize(1);
                        display.setCursor(5, 52);
                        display.print(F("Take pill: "));
                        display.print(pillReminder.hours < 10 ? "0" : "");
                        display.print(pillReminder.hours);
                        display.print(F(":"));
                        display.print(pillReminder.minutes < 10 ? "0" : "");
                        display.print(pillReminder.minutes);
                    } else {
                        // Heart rate or other emergency alert
                        display.setTextSize(2);
                        display.setCursor(15, 10);
                        display.println(F("EMERGENCY"));
                        display.setCursor(25, 30);
                        display.println(F("ALERT!"));
                        display.setTextSize(1);
                        display.setCursor(10, 50);
                        if (emergencyAlertType == "HEART") {
                            display.println(F("High Heart Rate!"));
                        } else {
                            display.println(F("Critical Alert!"));
                        }
                    }
                    display.display();
                    
                    // Sound buzzer alarm
                    soundBuzzerAlarm();
                    
                    xSemaphoreGive(displayMutex);
                    vTaskDelay(250 / portTICK_PERIOD_MS);  // Flash every 250ms
                    continue;
                }
            }
            
            // Check for environmental alerts
            if (environmentalAlert && environmentalAlertActive) {
                // Check if alert duration has passed
                if (millis() - environmentalAlertTime >= ENVIRONMENTAL_ALERT_DURATION) {
                    environmentalAlert = false;
                    environmentalAlertActive = false;
                    noTone(BUZZER_PIN);  // Ensure buzzer is off
                } else {
                    // Display environmental alert overlay
                    display.clearDisplay();
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    
                    // Flashing effect for environmental alert
                    bool flash = (millis() / 300) % 2;
                    if (flash) {
                        display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);
                        display.setTextColor(SSD1306_BLACK);
                    }
                    
                    
                    display.setTextColor(SSD1306_WHITE);
                    display.setTextSize(1);
                    display.setCursor(5, 20);
                    display.println(currentEnvironmentalAlert);
                    
                   
                    
                    display.display();
                    
                    // Sound environmental alert
                    soundEnvironmentalAlert();
                    
                    xSemaphoreGive(displayMutex);
                    vTaskDelay(300 / portTICK_PERIOD_MS);  // Flash every 300ms
                    continue;
                }
            }
            
            // Normal display logic based on menu state
            display.clearDisplay();
            
            // Declare realTime variable outside switch statement
            String realTime;
            
            switch (currentMenu) {
                case MENU_ROOM:
                case MENU_PATIENT:
                case MENU_HEARTRATE:
                case MENU_AIRQUALITY:
                case MENU_NURSE:
                case MENU_SETTINGS:
                {
                    // Display menu
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    
                    // Display status bar with alert icons instead of title
                    displayStatusBar();
                    
                    // Display time in top right corner (real Tehran time)
                    display.setCursor(90, 0);
                    String realTimeFormatted = getRealTimeFormatted();
                    if (realTimeFormatted != "N/A") {
                        display.print(realTimeFormatted.substring(0, 5)); // Show HH:MM only
                    } else {
                        // Fallback to system time if NTP not available
                        char timeStr[8];
                        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", 
                                 systemTime.hours, systemTime.minutes);
                        display.print(timeStr);
                    }
                    
                    display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
                    
                    // Display menu items with scrolling
                    int startItem = 0;
                    int maxVisibleItems = 4; // Show maximum 4 items at once
                    
                    // Calculate scroll offset if needed
                    if (currentMenuScroll >= maxVisibleItems) {
                        startItem = currentMenuScroll - maxVisibleItems + 1;
                    }
                    
                    for (int i = 0; i < maxVisibleItems && (startItem + i) < NUM_MENU_ITEMS; i++) {
                        int menuIndex = startItem + i;
                        int y = 15 + (i * 12);
                        
                        // Highlight current selection
                        if (menuIndex == currentMenuScroll) {
                            display.fillRect(0, y-1, SCREEN_WIDTH, 10, SSD1306_WHITE);
                            display.setTextColor(SSD1306_BLACK);
                        } else {
                            display.setTextColor(SSD1306_WHITE);
                        }
                        
                        display.setCursor(5, y);
                        display.print(menuItems[menuIndex].icon);
                        display.print(" ");
                        display.print(menuItems[menuIndex].name);
                    }
                    
                    // Show scroll indicators
                    if (startItem > 0) {
                        // Up arrow
                        display.setCursor(120, 15);
                        display.print("^");
                    }
                    if (startItem + maxVisibleItems < NUM_MENU_ITEMS) {
                        // Down arrow
                        display.setCursor(120, 50);
                        display.print("v");
                    }
                    
                    // (Removed time at bottom; time is now only at top right)
                    break;
                }
                    
                case VIEW_ROOM:
                {
                    // Display room sensor data
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("=== ROOM SENSORS ==="));
                    
                    if (xSemaphoreTake(dhtMutex, 10)) {  // Reduced timeout to prevent blocking
                        bool dataValid = (millis() - sensorData.room.lastUpdate) < 10000;
                        display.setCursor(0, 15);
                        display.print(F("Temp: "));
                        if (dataValid) {
                            display.print(sensorData.room.temperature, 1);
                            display.println(F("C"));
                        } else {
                            display.println(F("N/A"));
                        }
                        
                        display.setCursor(0, 28);
                        display.print(F("Humidity: "));
                        if (dataValid) {
                            display.print(sensorData.room.humidity, 1);
                            display.println(F("%"));
                        } else {
                            display.println(F("N/A"));
                        }
                        xSemaphoreGive(dhtMutex);
                    } else {
                        // If mutex not available, show cached message
                        display.setCursor(0, 15);
                        display.println(F("Temp: Reading..."));
                        display.setCursor(0, 28);
                        display.println(F("Humidity: Reading..."));
                    }
                    
                    display.setCursor(0, 45);
                    display.println(F("Press ENTER to return"));
                    break;
                }
                    
                case VIEW_PATIENT:
                {
                    // Display patient sensor data
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("== PATIENT SENSORS =="));
                    
                    if (xSemaphoreTake(dhtMutex, 10)) {  // Reduced timeout to prevent blocking
                        bool dataValid = (millis() - sensorData.patient.lastUpdate) < 10000;
                        display.setCursor(0, 15);
                        display.print(F("Temp: "));
                        if (dataValid) {
                            display.print(sensorData.patient.temperature, 1);
                            display.println(F("C"));
                        } else {
                            display.println(F("N/A"));
                        }
                        
                        display.setCursor(0, 28);
                        display.print(F("Humidity: "));
                        if (dataValid) {
                            display.print(sensorData.patient.humidity, 1);
                            display.println(F("%"));
                        } else {
                            display.println(F("N/A"));
                        }
                        xSemaphoreGive(dhtMutex);
                    } else {
                        // If mutex not available, show cached message
                        display.setCursor(0, 15);
                        display.println(F("Temp: Reading..."));
                        display.setCursor(0, 28);
                        display.println(F("Humidity: Reading..."));
                    }
                    
                    display.setCursor(0, 45);
                    display.println(F("Press ENTER to return"));
                    break;
                }
                    
                case VIEW_HEARTRATE:
                {
                    // Display heart rate data
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("=== HEART MONITOR ==="));
                    
                    if (xSemaphoreTake(heartRateMutex, 10)) {  // Reduced timeout to prevent blocking
                        bool dataValid = (millis() - sensorData.heartRate.lastUpdate) < 5000;
                        display.setCursor(0, 15);
                        display.print(F("Heart Rate: "));
                        if (dataValid && sensorData.heartRate.isValid) {
                            display.print(sensorData.heartRate.bpm);
                            display.println(F(" BPM"));
                            
                            display.setCursor(0, 28);
                            display.print(F("Status: "));
                            if (sensorData.heartRate.bpm < 60) {
                                display.println(F("Low"));
                            } else if (sensorData.heartRate.bpm >= 120) {
                                display.println(F("CRITICAL!"));
                            } else if (sensorData.heartRate.bpm > 100) {
                                display.println(F("High"));
                            } else {
                                display.println(F("Normal"));
                            }
                        } else {
                            display.println(F("N/A"));
                        }
                        xSemaphoreGive(heartRateMutex);
                    } else {
                        // If mutex not available, show cached message
                        display.setCursor(0, 15);
                        display.println(F("Heart Rate: Reading..."));
                    }
                    
                    display.setCursor(0, 45);
                    display.println(F("Press ENTER to return"));
                    break;
                }
                    
                case VIEW_AIRQUALITY:
                {
                    // Display air quality sensor data
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("=== AIR QUALITY ==="));
                    
                    if (xSemaphoreTake(mqSensorMutex, 10)) {  // Reduced timeout to prevent blocking
                        bool dataValid = (millis() - sensorData.airQuality.lastUpdate) < 10000;
                        display.setCursor(0, 15);
                        display.print(F("Gas Level: "));
                        if (dataValid && sensorData.airQuality.isValid) {
                            display.println(sensorData.airQuality.gasLevel);
                            
                            display.setCursor(0, 28);
                            display.print(F("Percentage: "));
                            display.print(sensorData.airQuality.gasPercent, 1);
                            display.println(F("%"));
                            
                            display.setCursor(0, 38);
                            display.print(F("Status: "));
                            if (sensorData.airQuality.gasLevel >= MQ_DANGER_LEVEL) {
                                display.println(F("DANGER!"));
                            } else if (sensorData.airQuality.gasLevel >= MQ_WARNING_LEVEL) {
                                display.println(F("Warning"));
                            } else {
                                display.println(F("Safe"));
                            }
                        } else {
                            display.println(F("N/A"));
                        }
                        xSemaphoreGive(mqSensorMutex);
                    } else {
                        // If mutex not available, show cached message
                        display.setCursor(0, 15);
                        display.println(F("Air Quality: Reading..."));
                    }
                    
                    display.setCursor(0, 55);
                    display.println(F("Press ENTER to return"));
                    break;
                }
                    
                case VIEW_NURSE:
                {
                    // Display nurse call confirmation
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("=== NURSE ALERT ==="));

                    display.setTextSize(1);
                    display.setCursor(0, 15);
                    display.println(F("Nurse has been"));
                    display.setCursor(0, 28);
                    display.println(F("alerted!"));
                    
                    display.setCursor(0, 45);
                    display.println(F("Help is on the way"));
                    
                    display.setCursor(0, 55);
                    display.println(F("Press ENTER to return"));
                    break;
                }
                    
                case VIEW_FAMILY:
                {
                    // Display family call confirmation
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("=== FAMILY CALL ==="));
                    
                   
                    
                    display.setTextSize(1);
                    display.setCursor(0, 15);
                    display.println(F("Family has been"));
                    display.setCursor(0, 28);
                    display.println(F("notified!"));
                    
                    display.setCursor(0, 45);
                    display.println(F("They will call soon"));
                    
                    display.setCursor(0, 55);
                    display.println(F("Press ENTER to return"));
                    break;
                }
                    
                case VIEW_PILLS:
                {
                    // Display pill reminder settings
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("=== PILL REMINDER ==="));
                    
                    // Show current pill time setting
                    display.setCursor(0, 15);
                    display.print(F("Pill Time: "));
                    
                    // Highlight hours or minutes based on editing mode
                    if (editingPillTime) {
                        if (editingHours) {
                            display.print(F("> "));
                            display.print(pillReminder.hours < 10 ? "0" : "");
                            display.print(pillReminder.hours);
                            display.print(F(" <:"));
                            display.print(pillReminder.minutes < 10 ? "0" : "");
                            display.print(pillReminder.minutes);
                        } else {
                            display.print(pillReminder.hours < 10 ? "0" : "");
                            display.print(pillReminder.hours);
                            display.print(F(":> "));
                            display.print(pillReminder.minutes < 10 ? "0" : "");
                            display.print(pillReminder.minutes);
                            display.print(F(" <"));
                        }
                    } else {
                        display.print(pillReminder.hours < 10 ? "0" : "");
                        display.print(pillReminder.hours);
                        display.print(F(":"));
                        display.print(pillReminder.minutes < 10 ? "0" : "");
                        display.print(pillReminder.minutes);
                    }
                    
                    // Show current real time for reference
                    display.setCursor(0, 28);
                    display.print(F("Now: "));
                    String realTime = getRealTimeFormatted();
                    display.println(realTime);
                
                    break;
                }
                    
                case VIEW_SETTINGS:
                {
                    // Display settings
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println(F("=== Information ==="));
                    
                    // Show Shamsi date and time
                    String realDateTime = getRealDateTime();
                    if (realDateTime != "Time not set") {
                        display.setCursor(0, 15);
                        display.print(F("Shamsi: "));
                        String persianDate = getPersianDate();
                        display.println(persianDate);
                        
                        display.setCursor(0, 25);
                        display.print(F("Time: "));
                        display.println(realDateTime.substring(11)); // HH:MM:SS
                    } else {
                        display.setCursor(0, 15);
                        display.println(F("NTP: Not synced"));
                    }
                    
                    display.setCursor(0, 35);
                    display.print(F("WiFi: "));
                    display.println(WiFi.status() == WL_CONNECTED ? F("Connected") : F("Disconnected"));
                    
                    display.setCursor(0, 45);
                    display.print(F("IP: "));
                    display.println(WiFi.localIP().toString());
                    display.setCursor(0, 55);
                    display.println(F("Press ENTER to return"));
                    break;
                }
                
                // Removed pill system menu cases
            }
            
            display.display();
            xSemaphoreGive(displayMutex);
        }
        // Much longer delay to reduce OLED lag and improve responsiveness
        vTaskDelay(300 / portTICK_PERIOD_MS);  // Update display every 300ms instead of 150ms
    }
}

// 📶 WiFi Management Task
void wifiTask(void * parameter) {
    static unsigned long lastNtpSync = 0;
    const unsigned long NTP_SYNC_INTERVAL = 3600000; // Resync every hour (3600000ms)
    
    while(1) {
        if (xSemaphoreTake(wifiMutex, portMAX_DELAY)) {
            // Check WiFi connection status
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi disconnected, attempting to reconnect...");
                WiFi.begin(ssid, password);
                
                int attempts = 0;
                while (WiFi.status() != WL_CONNECTED && attempts < 10) {
                    delay(500);
                    attempts++;
                }
                
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("WiFi reconnected successfully!");
                    digitalWrite(LED_BUILTIN, HIGH);
                    
                    // Reconfigure time after reconnection
                    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
                    lastNtpSync = millis();
                } else {
                    Serial.println("WiFi reconnection failed");
                    digitalWrite(LED_BUILTIN, LOW);
                }
            } else {
                // WiFi is connected, check if we need to resync time
                if (millis() - lastNtpSync > NTP_SYNC_INTERVAL) {
                    Serial.println("Periodic NTP time resync...");
                    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
                    lastNtpSync = millis();
                }
            }
            xSemaphoreGive(wifiMutex);
        }
        vTaskDelay(30000 / portTICK_PERIOD_MS);  // Check every 30 seconds
    }
}

// 🔘 Button Handler Task
void buttonTask(void * parameter) {
    bool lastLeftState = HIGH;
    bool lastRightState = HIGH;
    bool lastEnterState = HIGH;
    
    // Debouncing variables
    unsigned long lastButtonPress = 0;
    const unsigned long DEBOUNCE_DELAY = 50; // 50ms debounce
    
    // Long press detection for ENTER button
    unsigned long enterPressStartTime = 0;
    bool enterPressed = false;
    const unsigned long LONG_PRESS_DURATION = 1000; // 1 second for long press
    
    while(1) {
        // Get current time for button handling
        unsigned long currentTime = millis();
        
        // Use shorter timeout for mutex to avoid blocking
        if (xSemaphoreTake(menuMutex, pdMS_TO_TICKS(10))) {
            // Read button states (buttons are active LOW due to pull-up)
            bool leftState = digitalRead(LEFT_BUT);
            bool rightState = digitalRead(RIGHT_BUT);
            bool enterState = digitalRead(ENTER_BUT);
            
            // Long press detection for ENTER button
            if (enterState == LOW && lastEnterState == HIGH) {
                // ENTER button just pressed
                enterPressStartTime = currentTime;
                enterPressed = true;
            } else if (enterState == HIGH && lastEnterState == LOW) {
                // ENTER button just released
                enterPressed = false;
            } else if (enterPressed && enterState == LOW && 
                      (currentTime - enterPressStartTime >= LONG_PRESS_DURATION)) {
                // Long press detected - return to main menu from any status page
                if (currentMenu == VIEW_ROOM || currentMenu == VIEW_PATIENT || 
                    currentMenu == VIEW_HEARTRATE || currentMenu == VIEW_AIRQUALITY || 
                    currentMenu == VIEW_NURSE || currentMenu == VIEW_FAMILY || 
                    currentMenu == VIEW_SETTINGS) {
                    currentMenu = MENU_ROOM;
                    enterPressed = false;
                    lastButtonPress = currentTime;
                    xSemaphoreGive(menuMutex);
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    continue;
                } else if (currentMenu == VIEW_PILLS && !editingPillTime) {
                    // Long press on pill page returns to main menu
                    currentMenu = MENU_ROOM;
                    editingPillTime = false; // Reset editing state
                    editingHours = true;     // Reset to hours
                    
                    enterPressed = false;
                    lastButtonPress = currentTime;
                    xSemaphoreGive(menuMutex);
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    continue;
                }
            }
            
            // Debouncing - ignore button presses within debounce period
            if (currentTime - lastButtonPress < DEBOUNCE_DELAY) {
                xSemaphoreGive(menuMutex);
                vTaskDelay(10 / portTICK_PERIOD_MS);
                continue;
            }
            
            bool buttonPressed = false;
            
            // Check if any button is pressed to dismiss environmental alerts
            if ((leftState == LOW && lastLeftState == HIGH) || 
                (rightState == LOW && lastRightState == HIGH) || 
                (enterState == LOW && lastEnterState == HIGH)) {
                
                // Dismiss environmental alert if active
                if (environmentalAlertActive) {
                    environmentalAlert = false;
                    environmentalAlertActive = false;
                    noTone(BUZZER_PIN);  // Stop any alert sound
                    lastButtonPress = currentTime;
                    buttonPressed = true;
                }
            }
            
            // Only process menu navigation if no alert was dismissed
            if (!buttonPressed) {
                // Handle LEFT button (navigate up)
                if (leftState == LOW && lastLeftState == HIGH) {
                    if (currentMenu == MENU_ROOM || currentMenu == MENU_PATIENT || 
                        currentMenu == MENU_HEARTRATE || currentMenu == MENU_AIRQUALITY || 
                        currentMenu == MENU_NURSE || currentMenu == MENU_FAMILY || 
                        currentMenu == MENU_PILLS || currentMenu == MENU_SETTINGS) {
                        currentMenuScroll--;
                        if (currentMenuScroll < 0) {
                            currentMenuScroll = NUM_MENU_ITEMS - 1;
                        }
                        lastButtonPress = currentTime;
                        buttonPressed = true;
                    }
                    // Handle pill time editing (UP button increases value)
                    else if (currentMenu == VIEW_PILLS && editingPillTime) {
                        if (editingHours) {
                            pillReminder.hours++;
                            if (pillReminder.hours > 23) {
                                pillReminder.hours = 0;
                            }
                        } else {
                            pillReminder.minutes++;
                            if (pillReminder.minutes > 59) {
                                pillReminder.minutes = 0;
                            }
                        }
                        lastButtonPress = currentTime;
                        buttonPressed = true;
                    }
                }
                
                // Handle RIGHT button (navigate down)
                if (rightState == LOW && lastRightState == HIGH) {
                    if (currentMenu == MENU_ROOM || currentMenu == MENU_PATIENT || 
                        currentMenu == MENU_HEARTRATE || currentMenu == MENU_AIRQUALITY || 
                        currentMenu == MENU_NURSE || currentMenu == MENU_FAMILY || 
                        currentMenu == MENU_PILLS || currentMenu == MENU_SETTINGS) {
                        currentMenuScroll++;
                        if (currentMenuScroll >= NUM_MENU_ITEMS) {
                            currentMenuScroll = 0;
                        }
                        lastButtonPress = currentTime;
                        buttonPressed = true;
                    }
                    // Handle pill time editing (DOWN button decreases value)
                    else if (currentMenu == VIEW_PILLS && editingPillTime) {
                        if (editingHours) {
                            pillReminder.hours--;
                            if (pillReminder.hours < 0) {
                                pillReminder.hours = 23;
                            }
                        } else {
                            pillReminder.minutes--;
                            if (pillReminder.minutes < 0) {
                                pillReminder.minutes = 59;
                            }
                        }
                        lastButtonPress = currentTime;
                        buttonPressed = true;
                    }
                }
                
                // Handle ENTER button (select/return)
                if (enterState == LOW && lastEnterState == HIGH) {
                    // Removed Serial.print for better performance
                    
                    switch (currentMenu) {
                        case MENU_ROOM:
                        case MENU_PATIENT:
                        case MENU_HEARTRATE:
                        case MENU_AIRQUALITY:
                        case MENU_NURSE:
                        case MENU_FAMILY:
                        case MENU_PILLS:
                        case MENU_SETTINGS:
                            // Enter the selected menu item
                            currentMenu = menuItems[currentMenuScroll].state;
                            if (currentMenu == MENU_ROOM) currentMenu = VIEW_ROOM;
                            else if (currentMenu == MENU_PATIENT) currentMenu = VIEW_PATIENT;
                            else if (currentMenu == MENU_HEARTRATE) currentMenu = VIEW_HEARTRATE;
                            else if (currentMenu == MENU_AIRQUALITY) currentMenu = VIEW_AIRQUALITY;
                            else if (currentMenu == MENU_NURSE) {
                                // Send nurse alert to serial monitor
                                sendNurseAlert();
                                currentMenu = VIEW_NURSE;
                            }
                            else if (currentMenu == MENU_FAMILY) {
                                // Send family alert to serial monitor
                                sendFamilyAlert();
                                currentMenu = VIEW_FAMILY;
                            }
                            else if (currentMenu == MENU_PILLS) {
                                currentMenu = VIEW_PILLS;
                                editingPillTime = false; // Start in view mode
                                editingHours = true;     // Default to hours when entering edit
                            }
                            else if (currentMenu == MENU_SETTINGS) currentMenu = VIEW_SETTINGS;
                            // Removed pill system case
                            break;
                            
                        case VIEW_ROOM:
                        case VIEW_PATIENT:
                        case VIEW_HEARTRATE:
                        case VIEW_AIRQUALITY:
                        case VIEW_NURSE:
                        case VIEW_FAMILY:
                        case VIEW_SETTINGS:
                            // Return to main menu
                            currentMenu = MENU_ROOM;
                            break;
                            
                        case VIEW_PILLS:
                            if (editingPillTime) {
                                // In editing mode, ENTER switches between hours/minutes or exits
                                if (editingHours) {
                                    editingHours = false; // Switch to minutes
                                } else {
                                    // Done editing, exit edit mode and save
                                    editingPillTime = false;
                                    editingHours = true; // Reset for next time
                                    pillReminder.alertShown = false; // Reset alert flag for new time
                                    
                                    
                                }
                            } else {
                                // Not editing, enter edit mode
                                editingPillTime = true;
                                editingHours = true; // Start with hours
                            }
                            break;
                            
                        // Removed pill system cases
                    }
                    lastButtonPress = currentTime;
                    buttonPressed = true;
                }
            }
            
            // Update last button states
            lastLeftState = leftState;
            lastRightState = rightState;
            lastEnterState = enterState;
            
            xSemaphoreGive(menuMutex);
        }
        
        // Increased delay to reduce OLED lag and mutex contention
        vTaskDelay(50 / portTICK_PERIOD_MS);  // Check buttons every 50ms instead of 20ms
    }
}

void updateOLEDStatus() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // Check for full-screen emergency alert first
    if (fullScreenAlert && emergencyAlert && (millis() - emergencyAlertTime) < EMERGENCY_ALERT_DURATION) {
        // Debug output
        static unsigned long lastDebugPrint = 0;
        if (millis() - lastDebugPrint > 1000) {
            lastDebugPrint = millis();
        }
        
        // Full-screen emergency alert
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        
        // Create flashing effect
        if ((millis() / 300) % 2 == 0) {
            display.fillScreen(SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }
        
        // Large warning text
        display.setTextSize(2);
        display.setCursor(15, 8);
        
        if (emergencyAlertType == "PILL") {
            display.println("PILL");
            display.setTextSize(2);
            display.setCursor(15, 28);
            display.println("TIME!");
            
            // Show pill time and message
            display.setTextSize(1);
            display.setCursor(10, 52);
            display.print("Take pill now: ");
            display.print(pillReminder.hours < 10 ? "0" : "");
            display.print(pillReminder.hours);
            display.print(":");
            display.print(pillReminder.minutes < 10 ? "0" : "");
            display.print(pillReminder.minutes);
        } else {
            display.println("CRITICAL");
            
            display.setTextSize(3);
            display.setCursor(25, 28);
            display.println("ALERT");
            
            // Heart rate info
            display.setTextSize(1);
            display.setCursor(15, 52);
            if (xSemaphoreTake(heartRateMutex, portMAX_DELAY)) {
                display.print("Heart Rate: ");
                display.print(sensorData.heartRate.bpm);
                display.println(" BPM");
                xSemaphoreGive(heartRateMutex);
            }
        }
        
        // Sound buzzer alarm
        soundBuzzerAlarm();
        
        display.display();
        return; // Skip normal display update
    }
    
    // Turn off full-screen alert after duration
    if (fullScreenAlert && (millis() - emergencyAlertTime) >= EMERGENCY_ALERT_DURATION) {
        fullScreenAlert = false;
        emergencyAlert = false;
        noTone(BUZZER_PIN); // Properly turn off buzzer tone
        digitalWrite(BUZZER_PIN, LOW); // Ensure pin is low
    }
    
    if (xSemaphoreTake(menuMutex, portMAX_DELAY)) {
        MenuState currentState = currentMenu;
        int scrollPos = currentMenuScroll;
        xSemaphoreGive(menuMutex);
        
        // Show time in top right corner (except on settings page)
        if (currentState != VIEW_SETTINGS) {
            display.setTextSize(1);
            
            // Position adjusted for 24-hour format (no AM/PM needed)
            display.setCursor(90, 0);
            char timeStr[8];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", 
                     systemTime.hours, systemTime.minutes);
            display.print(timeStr);
        }
        
        // Handle menu views
        if (currentState < VIEW_ROOM) {
            // Draw title (slightly moved left to make room for clock)
            display.setTextSize(2);
            display.setCursor(2, 0);
            display.println("Menu");
            display.drawLine(0, 16, display.width(), 16, SSD1306_WHITE);
            
            // Draw menu items
            display.setTextSize(1);
            for (int i = 0; i < MENU_ITEMS_PER_PAGE && (i + scrollPos) < NUM_MENU_ITEMS; i++) {
                int itemY = 20 + (i * MENU_ITEM_HEIGHT);
                display.setCursor(2, itemY);
                
                // Draw selection cursor or space
                if (i + scrollPos == (int)currentState) {
                    display.print(">");
                } else {
                    display.print(" ");
                }
                
                // Draw icon and menu item text
                display.setCursor(12, itemY);
                display.print(menuItems[i + scrollPos].icon);
                display.print(" ");
                display.println(menuItems[i + scrollPos].name);
            }
            
            // Draw scroll indicators if needed
            if (scrollPos > 0) {
                display.fillTriangle(120, 20, 124, 16, 128, 20, SSD1306_WHITE);
            }
            if (scrollPos + MENU_ITEMS_PER_PAGE < NUM_MENU_ITEMS) {
                display.fillTriangle(120, 60, 124, 64, 128, 60, SSD1306_WHITE);
            }
        }
        // Handle detailed views
        else {
            switch(currentState) {
                case VIEW_ROOM:
                    display.setTextSize(2);
                    display.setCursor(5, 5);
                    display.print("\x0F"); // Icon
                    display.println(" Room");
                    display.setTextSize(1);
                    if (xSemaphoreTake(dhtMutex, portMAX_DELAY)) {
                        bool dataValid = (millis() - sensorData.room.lastUpdate) < 10000;
                        if (dataValid) {
                            display.setCursor(10, 25);
                            display.print("Temp: ");
                            display.print(sensorData.room.temperature, 1);
                            display.println("C");
                            display.setCursor(10, 35);
                            display.print("Humidity: ");
                            display.print(sensorData.room.humidity, 1);
                            display.println("%");
                        } else {
                            display.setCursor(10, 30);
                            display.println("No data available");
                        }
                        xSemaphoreGive(dhtMutex);
                    }
                    break;
                    
                case VIEW_PATIENT:
                    display.setTextSize(2);
                    display.setCursor(5, 5);
                    display.print("\x02"); // Icon
                    display.println(" Patient");
                    display.setTextSize(1);
                    if (xSemaphoreTake(dhtMutex, portMAX_DELAY)) {
                        bool dataValid = (millis() - sensorData.patient.lastUpdate) < 10000;
                        if (dataValid) {
                            display.setCursor(10, 25);
                            display.print("Temp: ");
                            display.print(sensorData.patient.temperature, 1);
                            display.println("C");
                            display.setCursor(10, 35);
                            display.print("Humidity: ");
                            display.print(sensorData.patient.humidity, 1);
                            display.println("%");
                        } else {
                            display.setCursor(10, 30);
                            display.println("No data available");
                        }
                        xSemaphoreGive(dhtMutex);
                    }
                    break;

                case VIEW_HEARTRATE:
                    // Smaller title with heart icon
                    display.setTextSize(1);
                    display.setCursor(5, 5);
                    display.print("\x03 Heart Rate");
                    
                    if (xSemaphoreTake(heartRateMutex, portMAX_DELAY)) {
                        bool dataValid = (millis() - sensorData.heartRate.lastUpdate) < 5000;
                        if (dataValid && sensorData.heartRate.isValid) {
                            // Display heart rate with large text
                            display.setTextSize(3);
                            display.setCursor(25, 20);
                            display.print(sensorData.heartRate.bpm);
                            display.setTextSize(1);
                            display.setCursor(85, 30);
                            display.println("BPM");
                            
                            // Add heart beat animation (pulsing heart)
                            static bool heartPulse = false;
                            if (millis() % 1000 < 200) { // Pulse every second for 200ms
                                heartPulse = !heartPulse;
                            }
                            
                            display.setCursor(5, 50);
                            if (heartPulse) {
                                display.print("\x03 \x03 \x03"); // Triple heart when pulsing
                            } else {
                                display.print("\x03   \x03"); // Single hearts
                            }
                            
                            // Add heart rate status
                            display.setCursor(50, 50);
                            if (sensorData.heartRate.bpm < 60) {
                                display.println("Low");
                            } else if (sensorData.heartRate.bpm > 100) {
                                display.println("High");
                            } else {
                                display.println("Normal");
                            }
                            
                            // Show current date and time for reference
                            display.setCursor(5, 58);
                            display.print("Time: ");
                            String realTime = getRealTimeFormatted();
                            display.print(realTime);
                            
                        } else {
                            display.setCursor(10, 30);
                            display.println("No heart rate data");
                        }
                        xSemaphoreGive(heartRateMutex);
                    }
                    break;

                case VIEW_SETTINGS:
                    // Settings title
                    display.setTextSize(2);
                    display.setCursor(5, 0);
                    display.print("\x1F"); // Icon
                    display.println(" Settings");
                    
                    // Draw separator line
                    display.drawLine(0, 16, display.width(), 16, SSD1306_WHITE);
                    
                    // Show detailed time and date
                    display.setTextSize(1);
                    
                    // Show Shamsi date and time from NTP
                    display.setCursor(5, 22);
                    String realDateTime = getRealDateTime();
                    if (realDateTime != "Time not set") {
                        display.print("Shamsi: ");
                        // Display Persian date
                        String persianDate = getPersianDate();
                        display.println(persianDate);
                        
                        display.setCursor(5, 32);
                        display.print("Time: ");
                        // Extract just the time part (HH:MM:SS)
                        display.println(realDateTime.substring(11));
                    } else {
                        display.println("NTP Time: Not synced");
                        
                        // Fallback to system time
                        display.setCursor(5, 32);
                        display.print("Sys Time: ");
                        uint8_t displayHours = systemTime.hours % 12;
                        if (displayHours == 0) displayHours = 12;
                        
                        char timeStr[20];
                        snprintf(timeStr, sizeof(timeStr), "%2d:%02d:%02d %s", 
                                displayHours, systemTime.minutes, systemTime.seconds,
                                systemTime.hours < 12 ? "AM" : "PM");
                        display.println(timeStr);
                    }
                    
                    // Show uptime
                    display.setCursor(5, 42);
                    display.print("Uptime: ");
                    unsigned long uptime = millis() / 1000; // Convert to seconds
                    uint8_t uptimeHours = uptime / 3600;
                    uint8_t uptimeMinutes = (uptime % 3600) / 60;
                    uint8_t uptimeSeconds = uptime % 60;
                    char uptimeStr[20];
                    snprintf(uptimeStr, sizeof(uptimeStr), "%02d:%02d:%02d", 
                            uptimeHours, uptimeMinutes, uptimeSeconds);
                    display.println(uptimeStr);
                    
                    display.setCursor(5, 52);
                    if (WiFi.status() == WL_CONNECTED) {
                        display.print("WiFi RSSI: ");
                        display.print(WiFi.RSSI());
                        display.println("dBm");
                    } else {
                        display.println("WiFi: Disconnected");
                    }
                    break;
            }
        }
    }
    
    // Emergency alert overlay
    if (emergencyAlert && (millis() - emergencyAlertTime) < EMERGENCY_ALERT_DURATION) {
        // Flash emergency alert on screen
        if ((millis() / 500) % 2 == 0) { // Flash every 500ms
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.fillRect(0, 0, SCREEN_WIDTH, 12, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
            display.setCursor(2, 2);
            display.println(" Pill time ");
            display.setTextColor(SSD1306_WHITE);
        }
    } else if (emergencyAlert) {
        // Turn off emergency alert after duration
        emergencyAlert = false;
    }
    
    // Environmental alert overlay (less intrusive than emergency alert)
    if (environmentalAlert && environmentalAlertActive && (millis() - environmentalAlertTime) < ENVIRONMENTAL_ALERT_DURATION) {
        // Flash environmental alert banner at bottom
        if ((millis() / 400) % 2 == 0) { // Flash every 400ms
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.fillRect(0, 52, SCREEN_WIDTH, 12, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
            display.setCursor(2, 54);
            display.println("⚠️ ENV ALERT: " + currentEnvironmentalAlert.substring(0, 12));
            display.setTextColor(SSD1306_WHITE);
        }
    } else if (environmentalAlert) {
        // Turn off environmental alert after duration
        environmentalAlert = false;
        environmentalAlertActive = false;
    }
    
    display.display();
}

// Clear alert states when conditions return to normal
void clearAlertStates(float roomTemp, float roomHumid, float patientTemp, float patientHumid) {
    // Clear room temperature alerts if temperature is back to normal (only if we have room data)
    if (roomTemp != 0 && roomTemp > ROOM_TEMP_LOW && roomTemp < ROOM_TEMP_HIGH) {
        alertStates.roomTempHigh = false;
        alertStates.roomTempLow = false;
    }
    
    // Clear room humidity alerts if humidity is back to normal (only if we have room data)
    if (roomHumid != 0 && roomHumid > ROOM_HUMID_LOW && roomHumid < ROOM_HUMID_HIGH) {
        alertStates.roomHumidHigh = false;
        alertStates.roomHumidLow = false;
    }
    
    // Clear patient temperature alerts if temperature is back to normal (only if we have patient data)
    if (patientTemp != 0 && patientTemp > BODY_TEMP_LOW && patientTemp < BODY_TEMP_HIGH) {
        alertStates.patientTempHigh = false;
        alertStates.patientTempLow = false;
    }
    
    // Clear patient humidity alerts if humidity is back to normal (only if we have patient data)
    if (patientHumid != 0 && patientHumid > BODY_HUMID_LOW && patientHumid < BODY_HUMID_HIGH) {
        alertStates.patientHumidHigh = false;
        alertStates.patientHumidLow = false;
    }
}

// Display status bar with alert icons
void displayStatusBar() {
    int iconX = 0; // Start position for icons
    
    // Room temperature icons
    if (alertStates.roomTempHigh) {
        display.setCursor(iconX, 0);
        display.print("R\x0F"); // R☀️ (room temp high)
        iconX += 18;
    } else if (alertStates.roomTempLow) {
        display.setCursor(iconX, 0);
        display.print("R\x9D"); // R❄️ (room temp low)
        iconX += 18;
    }
    
    // Room humidity icons
    if (alertStates.roomHumidHigh) {
        display.setCursor(iconX, 0);
        display.print("R\x7E"); // R💦 (room humid high)
        iconX += 18;
    } else if (alertStates.roomHumidLow) {
        display.setCursor(iconX, 0);
        display.print("R\x04"); // R🔥 (room humid low)
        iconX += 18;
    }
    
    // Patient temperature icons  
    if (alertStates.patientTempHigh) {
        display.setCursor(iconX, 0);
        display.print("P\x0F"); // P☀️ (patient temp high)
        iconX += 18;
    } else if (alertStates.patientTempLow) {
        display.setCursor(iconX, 0);
        display.print("P\x9D"); // P❄️ (patient temp low)
        iconX += 18;
    }
    
    // Patient humidity icons
    if (alertStates.patientHumidHigh) {
        display.setCursor(iconX, 0);
        display.print("P\x7E"); // P💦 (patient humid high)
        iconX += 18;
    } else if (alertStates.patientHumidLow) {
        display.setCursor(iconX, 0);
        display.print("P\x04"); // P🔥 (patient humid low)
        iconX += 18;
    }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  
  // Initialize time
  systemTime = {0, 0, 0};
  lastTimeUpdate = millis();

  // Create mutexes
  displayMutex = xSemaphoreCreateMutex();
  wifiMutex = xSemaphoreCreateMutex();
  dhtMutex = xSemaphoreCreateMutex();
  menuMutex = xSemaphoreCreateMutex();
  heartRateMutex = xSemaphoreCreateMutex();
  mqSensorMutex = xSemaphoreCreateMutex();

  // Initialize buttons with internal pull-up resistors
  pinMode(LEFT_BUT, INPUT_PULLUP);
  pinMode(RIGHT_BUT, INPUT_PULLUP);
  pinMode(ENTER_BUT, INPUT_PULLUP);
  


  // Initialize DHT sensors
  roomDht.begin();
  patientDht.begin();
  
  // Initialize sensor data structures
  sensorData.room = {0.0f, 0.0f, 0};
  sensorData.patient = {0.0f, 0.0f, 0};
  sensorData.heartRate = {0, 0, false};
  
 

  // Initialize analog pin for heart rate (potentiometer)
  pinMode(HEART_RATE_PIN, INPUT);
  
  // Initialize MQ sensor analog pin
  pinMode(MQ_SENSOR_PIN, INPUT);
  
  // Initialize buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer starts off
  
  // Configure ADC for GPIO34 specifically
  analogSetPinAttenuation(HEART_RATE_PIN, ADC_11db);  // Set specific pin attenuation
  analogReadResolution(12);        // Set ADC resolution to 12 bits (0-4095)

  // Initialize LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize I2C and OLED
  Wire.begin(21, 26);  // SDA = 21, SCL = 26

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    while (true); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen
  display.clearDisplay();          // Clear the buffer
  
  // Display new health logo (47x46 pixels) - positioned higher
  display.drawBitmap(
    ((SCREEN_WIDTH - 47) / 2),   // Center horizontally (47px width)
    4,                         // Position higher on screen (was centered)
    epd_bitmap_Screenshot_2025_07_10_151413, 47, 46, SSD1306_WHITE);
  
  // Add text below the logo
  display.setTextSize(1);          // Small text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 56);        // Position text below logo
  display.println(F("Health is Wealth"));
  display.display();               // Show the display buffer on the screen

  // Show startup logo for 3 seconds before menu takes over
  delay(3000);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH); // LED on = connected

   

   
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Wait for time to be set
    
    int timeAttempts = 0;
    while (!time(nullptr) && timeAttempts < 15) {
        
        delay(1000);
        timeAttempts++;
    }
    Serial.println();
    
    // Check if time was successfully set
    struct tm timeinfo;
    

    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/login", HTTP_POST, handleLogin);
    server.on("/logout", handleLogout);
    server.on("/doctor-dashboard", handleDoctorDashboard);
    server.on("/family-dashboard", handleFamilyDashboard);
    server.on("/pill-settings", HTTP_GET, handlePillSettings);
    server.on("/pill-settings", HTTP_POST, handlePillSettings);
    server.on("/prescriptions", HTTP_GET, handlePrescriptions);
    server.on("/prescriptions", HTTP_POST, handlePrescriptions);
    server.on("/messages", HTTP_GET, handleMessages);
    server.on("/messages", HTTP_POST, handleMessages);
    server.on("/weekly-report", HTTP_GET, handleWeeklyReport);
    server.on("/reset-counters", HTTP_POST, handleResetCounters);
    server.on("/led/on", handleLEDOn);
    server.on("/led/off", handleLEDOff);
    server.on("/call-nurse", handleCallNurse);
    server.on("/emergency", handleEmergency);
    server.on("/contact-nurse", HTTP_POST, handleContactNurse);
    server.on("/contact-family", HTTP_POST, handleContactFamily);
    
    // Start web server
    server.begin();


    // Create RTOS tasks
    xTaskCreatePinnedToCore(webServerTask, "WebServer", 8192, NULL, 2, &webServerTaskHandle, 1);
    xTaskCreatePinnedToCore(displayTask, "Display", 4096, NULL, 1, &displayTaskHandle, 1);
    xTaskCreatePinnedToCore(wifiTask, "WiFi", 4096, NULL, 1, &wifiTaskHandle, 0);
    xTaskCreatePinnedToCore(roomDhtTask, "RoomDHT", 4096, NULL, 1, &roomDhtTaskHandle, 0);
    xTaskCreatePinnedToCore(patientDhtTask, "PatientDHT", 4096, NULL, 1, &patientDhtTaskHandle, 0);
    xTaskCreatePinnedToCore(heartRateTask, "HeartRate", 4096, NULL, 1, &heartRateTaskHandle, 0);
    xTaskCreatePinnedToCore(mqSensorTask, "MQSensor", 4096, NULL, 1, &mqSensorTaskHandle, 0);
    xTaskCreatePinnedToCore(buttonTask, "Buttons", 2048, NULL, 2, &buttonTaskHandle, 0);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("");
    Serial.println("WiFi connection failed!");
    Serial.println("Possible solutions:");
    Serial.println("1. Check WiFi credentials");
    Serial.println("2. Ensure network is 2.4GHz");
    Serial.println("3. Check router distance");
    Serial.println("===============================");
  }
}

void loop() {
  // Main loop is now empty as everything is handled by RTOS tasks
  vTaskDelay(portMAX_DELAY);
}

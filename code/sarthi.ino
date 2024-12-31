#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
TinyGPSPlus gps;
// Create an instance of the MPU6050 sensor
Adafruit_MPU6050 mpu;
const int ir = 10;

// LED Pins
const int LED_PIN_1 = 13;
const int LED_PIN_2 = 12; // Change this pin as needed

// GSM Module Pins
SoftwareSerial gsmSerial(7, 8);  // RX, TX pins for the GSM module

// GPS Module Pins
SoftwareSerial GPS_SoftSerial(4, 3); // Rx, Tx for GPS module

// Thresholds
const float impactThreshold = 100.0;
const int smokeThreshold = 200;

void setup() {
  Serial.begin(115200);
  gsmSerial.begin(9600);
  GPS_SoftSerial.begin(9600);

  // Set up the LEDs as output
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);

  // Attempt to initialize the MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find the MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);

  Serial.println("MPU6050 found!");
}

void loop() {
  int irState = digitalRead(ir);

  if (irState == LOW) {
    // Get MPU6050 sensor events
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Calculate acceleration magnitude
    float accelerationMagnitude = sqrt(a.acceleration.x * a.acceleration.x +
                                       a.acceleration.y * a.acceleration.y +
                                       a.acceleration.z * a.acceleration.z);

    // Read MQ-135 sensor value
    int smokeValue = analogRead(A0);

    // Check for severe impact
    if (accelerationMagnitude > impactThreshold) {
      // Blink LED1
      blinkLED(LED_PIN_1);
      sendSMS("+919770339552","Accident Detected!");

      // Send SMS with GPS coordinates
      sendGPSviaSMS();
      accelerationMagnitude = 0;
    }

    // Check for smoke detection
    if (smokeValue > smokeThreshold) {
      // Blink LED2
      blinkLED(LED_PIN_1);
      sendSMS("+919770339552","Alcohol Detected!");
      // Send SMS with GPS coordinates
      sendGPSviaSMS();
    }

    // Wait a bit before the next read
    delay(10);
  }

  // Wait a bit before the next read
  delay(100); // Adjust the delay as needed
}

// Function to blink LED
void blinkLED(int pin) {
  digitalWrite(pin, HIGH);
  delay(500); // Adjust this delay as needed
  digitalWrite(pin, LOW);
  delay(500); // Adjust this delay as needed
}

// Function to send SMS with GPS coordinates
void sendGPSviaSMS() {
  unsigned long start;
  double lat_val, lng_val;
  smartDelay(1000);
  lat_val = gps.location.lat();
  lng_val = gps.location.lng();
  String link="https://www.google.com/maps/@"+String(lat_val, 6)+","+String(lng_val, 6)+",15z";
  // Send SMS with GPS coordinates
  String message = "GPS Coordinates - Latitude: " + String(lat_val, 6) + ", Longitude: " + String(lng_val, 6);
  sendSMS("+919770339552", link);
}
// Function to send SMS
void sendSMS(String phoneNumber, String message) {
  gsmSerial.println("AT");  // Send AT command to check the communication
  delay(1000);

  gsmSerial.println("AT+CMGF=1");  // Set SMS mode to text mode
  delay(1000);

  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");  // Set the destination phone number
  delay(1000);

  gsmSerial.print(message);
  delay(500);

  gsmSerial.write(26);  // Send Ctrl+Z to indicate the end of the message
  delay(1000);
}
// Rest of the code remains unchanged

// Add the remaining functions and definitions from the GPS code
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (GPS_SoftSerial.available())
      gps.encode(GPS_SoftSerial.read());
  } while (millis() - start < ms);
}

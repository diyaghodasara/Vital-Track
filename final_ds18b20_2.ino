#include <ESP8266WiFi.h>
#include <espnow.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266HTTPClient.h>

// Replace with your network credentials
const char* ssid = "realme X2 Pro";
const char* password = "ca4dd002827c";

// Replace with the MAC Address of the receiver
uint8_t broadcastAddress[] = {0x2C, 0x3A, 0xE8, 0x38, 0xBB, 0xF1};

// Initialize the DS18B20 sensor
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#define LED_PIN  D1
// Structure to send data
typedef struct struct_message {
  float btemperature;
} struct_message;

// Create a struct_message to hold sensor data
struct_message myData;

// Callback function that will be executed when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Data sent to receiver, status = ");
  Serial.println(sendStatus);
}

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);
  
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  // Register callback function
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Add broadcast address of the receiver
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  // Initialize DS18B20 sensor
  sensors.begin(); 
  pinMode(LED_PIN,OUTPUT); 
}

void loop() {
  // Request temperature data from DS18B20 sensor
  sensors.requestTemperatures();

  // Read temperature data from DS18B20 sensor
  float btemperature = sensors.getTempCByIndex(0);

  // Print temperature data
  Serial.print("bTemperature: ");
  Serial.print(btemperature);
  Serial.print(" °C");
 
  myData.btemperature = btemperature;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (btemperature > 35) {
    digitalWrite(LED_PIN, HIGH);  // Turn on the LED
  } else {
    digitalWrite(LED_PIN, LOW);  // Turn off the LED
  }
  

  // Wait for 5 seconds before sending data again
  delay(5000);
}
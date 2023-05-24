#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>


const char* ssid = "realme X2 Pro";
const char* password = "ca4dd002827c";

uint8_t broadcastAddress[] = {0x2C, 0x3A, 0xE8, 0x38, 0xBB, 0xF1};

// Initialize the DHT sensor
#define DHTPIN D2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Structure to send data
typedef struct struct_message {
  float temperature;
  float humidity;
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

  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  // Read DHT sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Print sensor data
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("% ");

  // Send sensor data to receiver
  myData.temperature = temperature;
  myData.humidity = humidity;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));


  // Wait for 5 seconds before sending data again
  delay(5000);
}
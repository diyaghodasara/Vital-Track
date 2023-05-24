
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


// Replace with your network credentials
const char* ssid = "realme X2 Pro";
const char* password = "ca4dd002827c";

// Replace with the MAC Addresses of the senders
uint8_t senderAddress1[] = {0x84, 0xF3, 0xEB, 0xCB, 0x10, 0x8A}; //dht22
uint8_t senderAddress2[] = {0xC8, 0xC9, 0xA3, 0x56, 0x66, 0x76}; //ds18b20

//LED pin
#define LED D1

// Structure to receive data
typedef struct board1 
{
  float temperature;
  float humidity;
} board1;

typedef struct board2 
{
  float btemperature;
} board2;

// Create a struct_message to hold received data
board1 receivedData1;
board2 receivedData2;

// Create an ESP8266WebServer object
ESP8266WebServer server(80);

//Checking the received data is from which sender
void OnDataRecv1(uint8_t * mac, uint8_t *incomingData1, uint8_t len1) 
{
   if (memcmp(mac, senderAddress1, 6) == 0) {
    memcpy(&receivedData1, incomingData1, sizeof(receivedData1)); 
   }
   else if(memcmp(mac, senderAddress2, 6) == 0){
     memcpy(&receivedData2, incomingData1, sizeof(receivedData2)); 
   }
  
}

void setup() 
{
  //setup LED
  pinMode(LED,OUTPUT);

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

  //Giving Controller rights
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Add the first sender
  esp_now_add_peer(senderAddress1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  // Add the second sender
  esp_now_add_peer(senderAddress2, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_register_recv_cb(OnDataRecv1);
  
  // Start the server
  server.begin();
  
}

void loop()
 {

  // Wait for data to be received
  delay(1000);

  // Check if data has been received from sender 1
  if (esp_now_send(senderAddress1, (uint8_t *) &receivedData1, sizeof(receivedData1)) == ERR_OK) 
  {
    // Print received data from sender 1
    Serial.print("Room Temperature from DHT22: ");
    Serial.print(receivedData1.temperature);
    Serial.print(" °C ");
    // Send data to ThingSpeak
    WiFiClient client1;
    String url = "/update?api_key=WUO4GYK47631BZY9&field3=" + String(receivedData1.temperature);

    if (client1.connect("api.thingspeak.com", 80)) 
    {
      client1.print(String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: api.thingspeak.com\r\n" +
      "Connection: close\r\n\r\n");
      Serial.println("Data sent to ThingSpeak");
    }
    client1.stop();

    // Print received data from sender 1
    Serial.print("Room Humidity from DHT22: ");
    Serial.print(receivedData1.humidity);
    Serial.println("% ");
    // Send data to ThingSpeak
    WiFiClient client2;
    String url1 = "/update?api_key=WUO4GYK47631BZY9&field2=" + String(receivedData1.humidity);

    if (client2.connect("api.thingspeak.com", 80)) 
    {
      client2.print(String("GET ") + url1 + " HTTP/1.1\r\n" +
      "Host: api.thingspeak.com\r\n" +
      "Connection: close\r\n\r\n");
      Serial.println("Data sent to ThingSpeak");
      Serial.println(" ");
    }

    client2.stop();
  }
  
  // Serial.println("step 1 chal raha hain"); 
  // Check if data has been received from sender 2
  if (esp_now_send(senderAddress2, (uint8_t *) &receivedData2, sizeof(receivedData2)) == ERR_OK) 
  {
    // Print received data from sender 2
    Serial.print("Body Temperature from ds18b20: ");
    Serial.print(receivedData2.btemperature);
    Serial.print(" °C ");

    // Send data to ThingSpeak
    WiFiClient client;
    String url = "/update?api_key=WUO4GYK47631BZY9&field1=" + String(receivedData2.btemperature);

    if (client.connect("api.thingspeak.com", 80)) 
    {
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: api.thingspeak.com\r\n" +
      "Connection: close\r\n\r\n");
      Serial.println("Data sent to ThingSpeak");
    }
    client.stop();
  }
  //LED blinking 
  if(receivedData2.btemperature>35)
  {
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(500);
  }
      
  // Handle incoming client requests
  server.handleClient();
}
 /*
  Capstone Final Demo
  Mariel Serra, Arielle Ainabe, Nicole Rodriguez
  Last Edited: 03-19-2024
*/
#include "max6675.h"
#include <esp_now.h>
#include <WiFi.h>
#define LED_PIN 4

// MAC Address for transmitter aka ESP32 S3
uint8_t broadcastAddress[] = {0x34, 0x85, 0x18, 0x6D, 0x04, 0xA8}; //34:85:18:6D:04:A8

// Testing values in case sensors fail
float testTemp = 20;
float testHumid = 10;
float testPercent = 45;

// Thermocouple variables
int thermoDO = 16; //Serial Out SO Pin
int thermoCS = 17; // Chip Select CS Pin
int thermoCLK = 18; // Serial Clock SCK Pin
float thermTemp;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);// create instance object for thermocouple

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    float thermTemp;
    float humidity;
    float firePercent; //Percentage for detecting wildfire gas
} struct_message;
// Create a struct_message for displaying data on GUI called readData
struct_message readData;

typedef struct struct_message_new {
  int id; //0 = FEATHER BOARD, 1 = WROOM
  bool sensorPos;
  float dht_humidity;
  float probability;
} struct_message_new;
// Create a struct message for sending back data for sensor plate positioning
struct_message_new sendData;

esp_now_peer_info_t peerInfo;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&readData, incomingData, sizeof(readData));
  // Serial.print("Thermocouple Temperature: ");
  // Serial.println(readData.thermTemp);
  // Serial.print("Humidity: "); //If greater than or equal to 15
  // Serial.println(readData.humidity);
  // Serial.print("Risk of Fire: ");
  // Serial.println(readData.firePercent);
  // Serial.println();
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void updateGUI(float temp, float humidity, float fireRisk)
{
  Serial.print(temp); //Replace with testTemp for testing GUI purposes
  Serial.print(",");
  Serial.print(humidity); //Replace with testHumid for testing GUI purposes
  Serial.print(",");
  Serial.println(fireRisk); //Replace with testPercent for testing GUI purposes

  delay(500);
}
 
void setup() {

  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else{
    Serial.println("Successfully initialized ESP-NOW");
  }

  // Once ESPNow is successfully Init, we will register for send CB to
  // get send packer info
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  else{
    Serial.println("Successfully added peer");
  }
  
  // Do the same for receiving!
  esp_now_register_recv_cb(OnDataRecv);

  //Initialize LED
  pinMode(LED_PIN, OUTPUT);
  sendData.id = 1;
}
 
void loop() 
{
  //thermTemp = thermocouple.readCelsius();
  updateGUI(readData.thermTemp, readData.humidity, readData.firePercent);
  sendData.sensorPos = true;
  digitalWrite(LED_PIN, HIGH);

  if (Serial.available())
  {
    char command = Serial.read();
    if (command == 'T'){
      //move platform
      sendData.sensorPos = false;
      digitalWrite(LED_PIN, LOW);
    }
    // else if (command == 'R'){
    //   Serial.print(readData.thermTemp); //Replace with testTemp for testing GUI purposes
    //   Serial.print(",");
    //   Serial.print(readData.humidity); //Replace with testHumid for testing GUI purposes
    //   Serial.print(",");
    //   Serial.println(readData.firePercent); //Replace with testPercent for testing GUI purposes
    // }
  }
  if (thermTemp > 60){
      sendData.sensorPos = false;
      digitalWrite(LED_PIN, LOW);
    }
  Serial.printf("Sensor Platform: %d \n", sendData.sensorPos);
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}
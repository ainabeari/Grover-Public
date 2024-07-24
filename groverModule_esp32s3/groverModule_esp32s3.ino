/*
  Arielle Ainabe, Mariel Serra and Nicole Rodriguez
  Last edited: 03-19-2024.
  Grover Module Code
*/
#include <esp_now.h>
#include <WiFi.h>
#include "max6675.h"
#include <TMCStepper.h>

#define MOTOR_SIGNAL 15
#define SIGNAL1 16
#define SIGNAL2 14

TaskHandle_t Task1;
TaskHandle_t Task2;

// Thermocouple variables
int thermoDO = 40; //Serial Out SO Pin
int thermoCS = 41; // Chip Select CS Pin
int thermoCLK = 42; // Serial Clock SCK Pin
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);// create instance object for thermocouple

//Receiver MAC Address for ESP32 WROOM
uint8_t broadcastAddressWROOM[] = {0x24, 0x0A, 0xC4, 0x5F, 0x76, 0xE0};
//Receiver MAC Address for ESP32 Featherboard
uint8_t broadcastAddressFEATHER[] = {0x40, 0x22, 0xD8, 0xF3, 0xC5, 0x98}; //40:22:D8:F3:C5:98

// Structure example to send data to the MainControls Unit
// Must match the receiver structure
typedef struct struct_message_sendGUI 
{
  float thermTemp;
  float humidity;
  float firePercent;
} struct_message_sendGUI;
// Create a struct_message for sending to GUI
struct_message_sendGUI sendData;

// Structure example to read data from the ESP featherboard
// Must match the sender structure
typedef struct struct_message {
  int id; //0 = FEATHER BOARD, 1 = WROOM
  bool sensorPos;
  float dht_humidity;
  float probability;
} struct_message;
// Create a struct_message to read the sensor data
struct_message readData;

// Create Structure to hold readings from each board
struct_message feather;
struct_message wroom;

//Create and array with all board structures
struct_message boardsStruct[2] = {feather, wroom};

esp_now_peer_info_t peerInfoFEATHER;
esp_now_peer_info_t peerInfoWROOM;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
// Receive Data from Gas sensor and DHT Sensor
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  // Serial.print("Packet received from: ");
  // snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
  //          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.println(macStr);
  memcpy(&readData, incomingData, sizeof(readData));
  // Serial.printf("Board ID %u: %u bytes\n", readData.id, len);
  
  if (readData.id == 1){
    boardsStruct[0].sensorPos = readData.sensorPos;
    boardsStruct[1].sensorPos = readData.sensorPos;
  }
  else if (readData.id == 0){
    boardsStruct[0].dht_humidity = readData.dht_humidity;
    boardsStruct[0].probability = readData.probability;
    boardsStruct[1].dht_humidity = readData.dht_humidity;
    boardsStruct[1].probability = readData.probability;
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  while(!Serial);
 
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

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer --> Featherboard
  memcpy(peerInfoFEATHER.peer_addr, broadcastAddressFEATHER, 6);
  peerInfoFEATHER.channel = 0;  
  peerInfoFEATHER.encrypt = false;
  // Register peer --> WROOM
  memcpy(peerInfoWROOM.peer_addr, broadcastAddressWROOM, 6);
  peerInfoWROOM.channel = 0;  
  peerInfoWROOM.encrypt = false;
  
  // Add peers     
  if (esp_now_add_peer(&peerInfoFEATHER) != ESP_OK){
    Serial.println("Failed to add SensorPlate peer");
    return;
  } else{
    Serial.println("Successfully added SensorPlate peer");
  }
  if (esp_now_add_peer(&peerInfoWROOM) != ESP_OK){
    Serial.println("Failed to add MainControls peer");
    return;
  } else{
    Serial.println("Successfully added MainControls peer");
  }

  esp_now_register_recv_cb(OnDataRecv);

  pinMode(MOTOR_SIGNAL, OUTPUT);
  pinMode(SIGNAL1, OUTPUT);
  pinMode(SIGNAL2, OUTPUT);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  //delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    //delay(500); 
}

void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;){
    Serial.print("Platform Position: ");
    Serial.println(boardsStruct[1].sensorPos);
    sendData.humidity = boardsStruct[0].dht_humidity; // Get this float value from featherboard
    Serial.print("Humidity: ");
    Serial.println(sendData.humidity);

    sendData.firePercent = boardsStruct[0].probability; // Get this float value from featherboard
    Serial.print("Percentage: ");
    Serial.println(sendData.firePercent);
    // Serial.printf("Sensor Position: %d \n", boardsStruct[1].sensorPos);
    // Serial.printf("Humidity: %d \n", boardsStruct[0].dht_humidity);
    // Serial.printf("Probability: %d \n", boardsStruct[0].probability);

    // Stepper Motor Logic Signals
    // Always keep pins high. Trigger only gets sent when the signal reads low
    digitalWrite(MOTOR_SIGNAL, HIGH);
    digitalWrite(SIGNAL1, HIGH);
    digitalWrite(SIGNAL2, HIGH);

  // Sensor cases detecting a fire
  if (readData.probability > 0.5 && sendData.thermTemp > 35){
    digitalWrite(MOTOR_SIGNAL, LOW); // The main "wildfire" condition. Use candles to activate this condition.
  }
  else if (sendData.thermTemp > 70){
    digitalWrite(MOTOR_SIGNAL, LOW); // Extreme temps - particularily for demo purposes
  }
  
    if (boardsStruct[1].sensorPos == false){
      digitalWrite(MOTOR_SIGNAL, LOW); 
      digitalWrite(SIGNAL1, LOW);
      digitalWrite(SIGNAL2, LOW);
    }
  
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddressWROOM, (uint8_t *) &sendData, sizeof(sendData));
   
    if (result == ESP_OK) {
      // Serial.println("Sent with success");
    }
    else {
      // Serial.println("Error sending the data");
    }
    //delay(250);
  }
}

void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;){
    sendData.thermTemp = thermocouple.readCelsius(); // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
    Serial.print("Thermocouple Temperature: ");
    Serial.println(sendData.thermTemp);
    delay(500);
  }
}
 
void loop() 
{
}


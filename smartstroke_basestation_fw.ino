#include <esp_now.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SMARTSTROKE_UUID "8bbd2160-84fa-43fb-8779-feb9185daba1"
#define SS_TIME_UUID "0a4a5dc5-23ef-46cc-beb4-ff5fa9a5c992"
#define SS_FSR1_UUID "9fefbfec-8a18-4971-93b8-f83240cb85bb"
#define SS_FSR2_UUID "9b92b831-2ae8-4659-80a4-c6d4652baa79"
#define SS_FSR3_UUID "2315637b-84de-4851-88f8-0cbf3aa29f8a"
#define SS_FSR4_UUID "e9c9be95-5bdb-4cb6-9783-084e33e11d41"
#define SS_DEBUG_UUID "b3d4772d-f279-4435-9a6c-e1a5c42472f6"
#include <Arduino.h>

//Recive from Paddle
//My MAC 8c:aa:b5:8c:4a:38
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int id; // must be unique for each sender board
    long int time;
    int ADC; 
}struct_message;

typedef struct handshake_signal {
  int id;
  int num_connect;
}handshake_signal;

int i = 0;
bool Sendflag = true;
esp_now_peer_info_t peerInfo;

uint8_t paddle1Address[] = {0x7c, 0xdf, 0xa1, 0xf2, 0xc5, 0x34};
uint8_t paddle2Address[] = {0x7c, 0xdf, 0xa1, 0xf2, 0xc5, 0x3c};
uint8_t paddle3Address[] = {0x7c, 0xdf, 0xa1, 0xf2, 0xc5, 0x8c};
uint8_t paddle4Address[] = {0x8c, 0xaa, 0xb5, 0x8c, 0x4a, 0x38};
 //Create a struct_message called myData
struct_message PROGMEM myData;
handshake_signal Id_check;
// Create a structure to hold the readings from each board
struct_message Paddle_board1;
struct_message Paddle_board2;
struct_message Paddle_board3;
struct_message Paddle_board4;

// Create an array with all the structures
struct_message boardsStruct[4] = {Paddle_board1, Paddle_board2, Paddle_board3, Paddle_board4};
class MyCallbacks: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
      Serial.println("*********");
      Serial.print("New value: ");
      Serial.println(pCharacteristic->getValue().c_str());
      Serial.println("*********");
  }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onDisconnect(BLEServer* pServer) {
      pServer->startAdvertising(); // restart advertising
      Serial.println("Connection lost, readvertising");
    }
};

long int startTime = 0;
BLEService *pService;
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println();
  WiFi.softAP("ESP32");
  

  WiFi.mode(WIFI_STA);
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  memcpy(peerInfo.peer_addr, paddle1Address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, paddle2Address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, paddle3Address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, paddle4Address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  //Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  //Once ESPNow is successfully Init, we will register for recv CB to
  //get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  Id_check.num_connect = 4;
  while (!Serial);
    Serial.println("Starting BLE work!");

     BLEDevice::init("SmartStroker9001Elite 2A84");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SMARTSTROKE_UUID);
  BLECharacteristic *pDebugCharacteristic = pService->createCharacteristic(
                                         SS_DEBUG_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_WRITE_NR
                                       );
  pDebugCharacteristic->setCallbacks(new MyCallbacks());
  pDebugCharacteristic->setValue("Hello World says Smart Stroke");

  BLECharacteristic *pTimeCharacteristic = pService->createCharacteristic(
                                         SS_TIME_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  BLECharacteristic *pFSR1Characteristic = pService->createCharacteristic(
                                         SS_FSR1_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  BLECharacteristic *pFSR2Characteristic = pService->createCharacteristic(
                                         SS_FSR2_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  BLECharacteristic *pFSR3Characteristic = pService->createCharacteristic(
                                         SS_FSR3_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  BLECharacteristic *pFSR4Characteristic = pService->createCharacteristic(
                                         SS_FSR4_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE);
                                         
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SMARTSTROKE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  startTime = millis();
}
 
void loop() {
  
  // Acess the variables for each board
    Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  if(i >= Id_check.num_connect){
    i = 1;
  }
  else{
    i += 1;
  }
  Id_check.id = i;
  Serial.println("Id_check");
  Serial.println(i);
  
  esp_err_t result1 = esp_now_send(paddle1Address, (uint8_t *) &Id_check, sizeof(Id_check));
  if (result1 == ESP_OK) {      
    Serial.println("Sent with success");
  }
  
  esp_err_t result2 = esp_now_send(paddle2Address, (uint8_t *) &Id_check, sizeof(Id_check));
  if (result2 == ESP_OK) {      
    Serial.println("Sent with success");
  }
  
  esp_err_t result3 = esp_now_send(paddle3Address, (uint8_t *) &Id_check, sizeof(Id_check));
  if (result3 == ESP_OK) {      
    Serial.println("Sent with success");
  }
  
  esp_err_t result4 = esp_now_send(paddle4Address, (uint8_t *) &Id_check, sizeof(Id_check));
  if (result4 == ESP_OK) {      
    Serial.println("Sent with success");
  }
  
  

  // Board 1
  int board_1_time = boardsStruct[0].time;
  int board_1_ADC = boardsStruct[0].ADC;

  // Board 2
  int board_2_time = boardsStruct[1].time;
  int board_2_ADC = boardsStruct[1].ADC;

  // Board 3
  int board_3_time = boardsStruct[2].time;
  int board_3_ADC = boardsStruct[2].ADC;

  // Board 4
  int board_4_time = boardsStruct[3].time;
  int board_4_ADC = boardsStruct[3].ADC;
  
  //BLE to phone
  
  BLECharacteristic *pTimeCharacteristic = pService->getCharacteristic(SS_TIME_UUID);
  double time = (double)(millis() - startTime);
  pTimeCharacteristic->setValue(time);
  BLECharacteristic *pFSR1Characteristic = pService->getCharacteristic(SS_FSR1_UUID);
  double value1 = (((double)board_1_ADC)/4095.0) - 1.0;
  pFSR1Characteristic->setValue(value1);
  BLECharacteristic *pFSR2Characteristic = pService->getCharacteristic(SS_FSR2_UUID);
  double value2 = (((double)board_2_ADC)/4095.0) - 1.0;
  pFSR2Characteristic->setValue(value2);
  BLECharacteristic *pFSR3Characteristic = pService->getCharacteristic(SS_FSR3_UUID);
  double value3 = (((double)board_3_ADC)/4095.0) - 1.0;
  pFSR3Characteristic->setValue(value3);
  BLECharacteristic *pFSR4Characteristic = pService->getCharacteristic(SS_FSR4_UUID);
  double value4 = (((double)board_4_ADC)/4095.0) - 1.0;
  pFSR4Characteristic->setValue(value3);
  delay(50);
}



// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.println(F("Packet received from: ")); 
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].time = myData.time;
  boardsStruct[myData.id-1].ADC = myData.ADC;

  Serial.printf("time: %d \n", boardsStruct[myData.id-1].time);
  Serial.printf("ADC: %d \n", boardsStruct[myData.id-1].ADC);

  Serial.println();
}
 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
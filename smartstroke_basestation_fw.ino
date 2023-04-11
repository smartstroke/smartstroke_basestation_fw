//Base Station to App code

//----------------------------------------
//          Includes
//----------------------------------------
#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>

//----------------------------------------
//          Definitions
//----------------------------------------
#define SERIAL_PLOTTER 1
#define DEBUG_VERBOSE 0

#define LED 2
#define SERIAL_BAUD_RATE 921600

//----------------------------------------
//          Globals
//----------------------------------------
int ledState = LOW;

//----------------------------------------
//          Helper Functions
//----------------------------------------
WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Ready");
}

void handleGet() {
  if (server.hasArg("data")) {
    String data = server.arg("data");
    Serial.println("Data: " + data);
  }
  server.send(200, "text/plain", "Data Received");
}

void handlePost() {
  server.send(200, "text/plain", "Processing Data");
}

void handleUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.println("Receiving data:");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    server.send(200, "text/plain", "Data: ");
  }
}

//Recive from Paddle
//My MAC 8c:aa:b5:8c:4a:38
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;  // must be unique for each sender board
  long int time;
  int AccX;
  int AccY;
  int AccZ;
  int ADC;
  int GyroX;
  int GyroY;
  int GyroZ;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message Paddle_board1;
struct_message Paddle_board2;
struct_message Paddle_board3;

// Create an array with all the structures
struct_message boardsStruct[3] = { Paddle_board1, Paddle_board2, Paddle_board3 };

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac_addr, const uint8_t* incomingData, int len) {
  char macStr[18];
#ifdef DEBUG_VERBOSE
  Serial.print("Packet received from: ");
#endif
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
#ifdef DEBUG_VERBOSE
  Serial.println(macStr);
#endif
  memcpy(&myData, incomingData, sizeof(myData));
#ifdef DEBUG_VERBOSE
  Serial.printf("Board ID %u: %u bytes\r\n", myData.id, len);
#endif
  // Update the structures with the new incoming data
  boardsStruct[myData.id - 1].time = myData.time;
  boardsStruct[myData.id - 1].AccX = myData.AccX;
  boardsStruct[myData.id - 1].AccY = myData.AccY;
  boardsStruct[myData.id - 1].AccZ = myData.AccZ;
  boardsStruct[myData.id - 1].ADC = myData.ADC;
  boardsStruct[myData.id - 1].GyroX = myData.GyroX;
  boardsStruct[myData.id - 1].GyroY = myData.GyroY;
  boardsStruct[myData.id - 1].GyroZ = myData.GyroZ;

#ifdef DEBUG_VERBOSE
  Serial.printf("time:%d ", boardsStruct[myData.id - 1].time);
  Serial.printf("AccX:%d ", boardsStruct[myData.id - 1].AccX);
  Serial.printf("AccY:%d ", boardsStruct[myData.id - 1].AccY);
  Serial.printf("AccZ:%d ", boardsStruct[myData.id - 1].AccZ);
  Serial.printf("ADC:%d ", boardsStruct[myData.id - 1].ADC);
  Serial.printf("GyroX:%d ", boardsStruct[myData.id - 1].GyroX);
  Serial.printf("GyroY:%d ", boardsStruct[myData.id - 1].GyroY);
  Serial.printf("GyroZ:%d ", boardsStruct[myData.id - 1].GyroZ);
  Serial.println();
#endif

#ifdef SERIAL_PLOTTER
  Serial.printf("time:%d ", boardsStruct[myData.id - 1].time);
  Serial.printf("AccX:%d ", boardsStruct[0].AccX);
  Serial.printf("AccY:%d ", boardsStruct[0].AccY);
  Serial.printf("AccZ:%d ", boardsStruct[0].AccZ);
  Serial.printf("AccX2:%d ", boardsStruct[1].AccX);
  Serial.printf("AccY2:%d ", boardsStruct[1].AccY);
  Serial.printf("AccZ2:%d ", boardsStruct[1].AccZ);
  Serial.printf("ADC:%d ", boardsStruct[myData.id - 1].ADC);
  Serial.printf("GyroX:%d ", boardsStruct[myData.id - 1].GyroX);
  Serial.printf("GyroY:%d ", boardsStruct[myData.id - 1].GyroY);
  Serial.printf("GyroZ:%d ", boardsStruct[myData.id - 1].GyroZ);
  Serial.println();
#endif
}

//----------------------------------------
//          Setup
//----------------------------------------
void setup() {
  pinMode(LED, OUTPUT);

  //Initialize Serial Monitor
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println();
  WiFi.softAP("ESP32");
  server.on("/", handleRoot);
  server.on("/get", HTTP_GET, handleGet);
  server.on("/post", HTTP_POST, handlePost, handleUpload);
  server.begin();

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Once ESPNow is successfully Init, we will register for recv CB to
  //get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}

//----------------------------------------
//          Main Loop
//----------------------------------------
void loop() {
  // Acess the variables for each board
  //Serial.println("Loop");
  // Board 1
  int board_1_time = boardsStruct[0].time;
  int board_1_AccX = boardsStruct[0].AccX;
  int board_1_AccY = boardsStruct[0].AccY;
  int board_1_AccZ = boardsStruct[0].AccZ;
  int board_1_ADC = boardsStruct[0].ADC;
  int board_1_GyroX = boardsStruct[0].GyroX;
  int board_1_GyroY = boardsStruct[0].GyroY;
  int board_1_GyroZ = boardsStruct[0].GyroZ;

  // Board 2
  int board_2_time = boardsStruct[1].time;
  int board_2_AccX = boardsStruct[1].AccX;
  int board_2_AccY = boardsStruct[1].AccY;
  int board_2_AccZ = boardsStruct[1].AccZ;
  int board_2_ADC = boardsStruct[1].ADC;
  int board_2_GyroX = boardsStruct[1].GyroX;
  int board_2_GyroY = boardsStruct[1].GyroY;
  int board_2_GyroZ = boardsStruct[1].GyroZ;

  // Board 3
  int board_3_time = boardsStruct[2].time;
  int board_3_AccX = boardsStruct[2].AccX;
  int board_3_AccY = boardsStruct[2].AccY;
  int board_3_AccZ = boardsStruct[2].AccZ;
  int board_3_ADC = boardsStruct[2].ADC;
  int board_3_GyroX = boardsStruct[2].GyroX;
  int board_3_GyroY = boardsStruct[2].GyroY;
  int board_3_GyroZ = boardsStruct[2].GyroZ;
  server.handleClient();
  delay(10000);

  ledState = !ledState;
  digitalWrite(LED, ledState);
}
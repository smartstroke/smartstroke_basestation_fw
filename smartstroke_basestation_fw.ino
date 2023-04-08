#include <esp_now.h>
#include <WiFi.h>
//Base Station to App code
#include <WebServer.h>

#define LED 2
int ledState = LOW;

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
  int id;
  int x;
  int y;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message Paddle_board1;
struct_message Paddle_board2;
struct_message Paddle_board3;

// Create an array with all the structures
struct_message boardsStruct[3] = {Paddle_board1, Paddle_board2, Paddle_board3};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: "); 
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  boardsStruct[myData.id-1].y = myData.y;
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].x);
  Serial.printf("y value: %d \n", boardsStruct[myData.id-1].y);
  Serial.println();
}
 

void setup() {
  pinMode(LED, OUTPUT);

  //Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
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
 
void loop() {
  // Acess the variables for each board
  int board1X = boardsStruct[0].x;
  int board1Y = boardsStruct[0].y;
  int board2X = boardsStruct[1].x;
  int board2Y = boardsStruct[1].y;
  int board3X = boardsStruct[2].x;
  int board3Y = boardsStruct[2].y;
  server.handleClient();  
  delay(10000);  

  ledState = !ledState;
  digitalWrite(LED, ledState); 
}

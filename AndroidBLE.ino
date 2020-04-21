#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "mbedtls/aes.h"

//SplitStringConfig
//String motorID = "5"; //B6304WLW
//String motorID = 2; //A3027BLX
String motorID = "3"; //B3369WMS
//String motorID = 4; //B6422TMA

//Relay
const int RELAY_EN = 2;
String response;
String readdataString; //main captured String
String idMotor; //data String
String idUser;
String condition;

//Decrypt
char * key = "tandebikeskripsi";
unsigned char decipheredTextOutput[16];
unsigned char cipherTextOutput[16];
String hasilmantap = "";

int ind1; // , locations
int ind2;
int ind3;

//BLEConfig
int scanTime = 2; //In seconds
static BLEAddress *pServerAddress;
String ScannedAddress;
static BLEUUID *pServiceUUID;
static BLEUUID *pServiceUUIDValue;
String ScannedUUID;
String ValueUUID;
BLEScan* pBLEScan;
String nameDevices;
String serviceData;
String serviceUUID;
String dataCollect[2];
String Collections;
String datakeSplit;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
//      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());      
////      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
////      Serial.println(pServerAddress.toString().c_str());  
    }
};

//WiFiConfig
const char* ssid = "chay";
const char* password = "19711224";
//const char* ssid = "winddheaaro";
//const char* password = "";

void decrypt(unsigned char * chipherText, char * key, unsigned char * outputBuffer){
 
  mbedtls_aes_context aes;
 
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)chipherText, outputBuffer);
  mbedtls_aes_free( &aes );
}

void setup() {
  Serial.begin(115200);
  pinMode (RELAY_EN, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.println("Scanning Bluetooth...");
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  while(datakeSplit == ""){
    ScanBT();
  }
    delay(12000);
    SendData();
    if(response != "Data not found"){
      loop();
    }else{
    Serial.println("Sukses");
    }
    decryption();
    SplitString();
    if (idMotor == motorID){
      MotorCon();
    }
    else{
     Serial.print("TIDAK DAPAT DIPESAN");

    }
    delay(5000);
    return loop();
}

void ScanBT(){
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  int index = 0;
  int n = foundDevices.getCount();
  Serial.println(n);
  for(uint32_t i = 0; i < n; i++){
    BLEAdvertisedDevice devices = foundDevices.getDevice(i);
    if(devices.haveServiceUUID()){
      if(strcmp(devices.getServiceUUID().toString().c_str(), "00001802-0000-1000-8000-00805f9b34fb") == 0 ){
        serviceUUID = devices.getServiceUUID().toString().c_str();
        nameDevices = devices.getName().c_str();
        while(devices.getServiceData().c_str() > 0 ){
          serviceData = devices.getServiceData().c_str();
          if(dataCollect[index].length() <= 31){
//          Serial.print("ServiceData=");
           Serial.println(serviceData);

            if(serviceData.length() == 10 || serviceData.length() == 11){
              dataCollect[index] = serviceData;
   //           Serial.println("masuk serviceData.length() == 10");

              if(dataCollect[0] == NULL || dataCollect[index] != dataCollect [index-1]){
         //     Serial.println("masuk dataCollect[0] == NULL");
              Serial.println(dataCollect[index]);
              Collections = Collections + dataCollect[index];
                if(Collections.length() == 32){
//                  Serial.println("masuk Collections.length() == 32");
                  datakeSplit = Collections;
                  Serial.println(Collections);
                  for(int i = 0 ; i <= index; i++){
                    dataCollect[i] = "";
                  }
                  Collections = "";
                  break;
                }
              serviceData = "";
              index++;
              }else if(dataCollect[index] == dataCollect[index-1]){
                dataCollect[index] = "";
                serviceData = "";
                Serial.println("Data Duplikat");
                //index--;   
                break;
              }
            }else{
             dataCollect[index] = "";
             Serial.println("Data gak 11");
             break;
            }
         }else{
            Serial.println("Data Ngaco");
            dataCollect[index] = "";
            serviceData = "";
            break;
         }
        }
    }
  }
  }
  //Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  //delay(500);
}

void SplitString(){
    ind1 = hasilmantap.indexOf(':'); //finds location of first ,
    idMotor = hasilmantap.substring(0, ind1); //captures first data String
    ind2 = hasilmantap.indexOf(':', ind1+1 ); //finds location of second ,
    condition = hasilmantap.substring(ind1+1, ind2); //captures second data String
    ind3 = hasilmantap.indexOf(':', ind2+1 );
    idUser = hasilmantap.substring(ind2+1);
  
    Serial.print("idMotor = ");
    Serial.println(idMotor);
    Serial.print("condition = ");
    Serial.println(condition);
    Serial.print("idUser= ");
    Serial.println(idUser);
}

void MotorCon(){
  if(condition == "TurnOnMotor"){
    digitalWrite(RELAY_EN, HIGH);
    Serial.print("Motor Menyala");
  }if(condition == "TurnOffMoto" ){    
    digitalWrite(RELAY_EN, LOW);
    Serial.print("Motor Mati");
  }if(condition == "CheckInMotor" ){
    Serial.print("Check In");
  }else if(condition == "CheckOutMoto" ){
    Serial.print("Check Out");
  }
}

void SendData(){
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
  HTTPClient http;  
  idMotor = 3;
  String postData = " {\n";
      postData += " \"idMotor\" : " + idMotor + ",\n";
      postData += " \"Encryption\" : \"" + datakeSplit + "\"\n";
      postData += " }";

      datakeSplit = "";

  Serial.println(postData);  
  http.begin("https://gotandebike.000webhostapp.com/api/user/checkEncrypt");  //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json");             //Specify content-type header
  int httpResponseCode = http.POST(postData);   //Send the actual POST request
      
    if(httpResponseCode>0){
        response = http.getString();                       //Get the response to the request 
        Serial.println(httpResponseCode);   //Print return code
        Serial.println(response);           //Print request answer
        Serial.println("POST Succesfully");  
        }else{
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        loop();
    }
  http.end();  //Free resources
  }else{
   Serial.println("Error in WiFi connection");
  }
}

void decryption(){
  int x =0;
  for(int i = 0; i<16; i++){
    char charArray[5] = "0";
    String coba = "00"+ response.substring(x,x+2);
    coba.toCharArray(charArray,5);
    int v = x2i(charArray);
    Serial.println(charArray);
    cipherTextOutput[i]=v;
    x=x+2;
  }
  decrypt(cipherTextOutput, key, decipheredTextOutput);

  Serial.println("\nCiphered text:");
 
  for (int i = 0; i < 16; i++) {
 
    char str[3];
    sprintf(str, "%02x", (int)cipherTextOutput[i]);
        Serial.print("|");
        Serial.print(cipherTextOutput[i]);
    Serial.print(".");
    Serial.println(str);
  }
  
  Serial.println("\n\nDeciphered text:");
  for (int i = 0; i < 16; i++) {
    Serial.print((char)decipheredTextOutput[i]);
    hasilmantap += (char)decipheredTextOutput[i];
  }
  Serial.println(hasilmantap);
}

int x2i(char *s)
{
 int x = 0;
 for(;;) {
   char c = *s;
   if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
   }
   else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10;
   }
   else break;
   s++;
 }
 return x;
}

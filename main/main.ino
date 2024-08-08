#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <OneWire.h>
#include <DallasTemperature.h>


//Functions
String water_level();
String water_temperature();
String turbidity_level();
String ph_level();
String pir();
void sendtoFirebase(String path,String data);

//Pins

const int trigPin = 2;
const int echoPin = 4;
const int oneWireBus = 23;
const int turbPin=34;
const int phpin = A0;
const int pirpin = 24;

//vaiables

float maxDepth=12.0;
float distance = 0.0; 
String percentage = "";
float volt=0.0;
float ntu=0.0;
float value;
float ph;
String temp= "";
String turbidity= "";
String phvalue= "";
String piroutput="";
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);


//Firebase paths
String dbName = "test/";
String waterlevelPath = dbName + "waterLevel";
String turbidityPath = dbName + "turbidityLevel";
String temperaturePath = dbName + "temperature";
String pirPath = dbName + "pirStatus";
String phPath = dbName + "phValue";

//Wifi credentials
#define WIFI_SSID "harshiiss"
#define WIFI_PASSWORD "12345678"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDimxt4inJYjOmgn87bbmBUKwafur4gw6Q"

#define URL "aquasense-6f3a7-default-rtdb.asia-southeast1.firebasedatabase.app/" 


FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

void setup(){
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(turbPin,INPUT);
  pinMode(phpin,INPUT);
  pinMode(pirpin,INPUT);
  Serial.begin(115200);
  Serial.println("Pins' setup done");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  config.api_key = API_KEY;


  config.database_url = URL;


  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }


  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(false);
}

void loop(){
  sendtoFirebase(waterlevelPath,water_level());
  sendtoFirebase(temperaturePath,water_temperature());
//  sendtoFirebase(phPath,ph_level());
//  sendtoFirebase(turbidityPath,turbidity_level());
//  sendtoFirebase(pirPath,pir());

}


//Function for firebase data sending
void sendtoFirebase(String path,String data){
  if (Firebase.ready() && signupOK ){
      if (Firebase.RTDB.setString(&fbdo, path, data)){
        Serial.println("Sent: " + data);
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      delay(3300);
}}
//function for waterlevel
String water_level(){
  digitalWrite(trigPin,LOW);
  delayMicroseconds(10);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  distance = pulseIn(echoPin,HIGH);
  distance = (distance*0.034)/2;
  if (distance <= maxDepth){
    distance = ((maxDepth - distance)/maxDepth) * 100;
    percentage = String(distance) + " " + "%";
  }
  else {
    percentage = "Error";
}
  return percentage;
  
}

//function for water temperature
String water_temperature()
{
  sensors.requestTemperatures(); 
  float temperature = sensors.getTempCByIndex(0);
  if(temperature>=22 && temperature<=28)
  temp= String(temperature) + " PERFECT!";
  else if(temperature<22)
  temp= String(temperature) + " LOW!";
  else 
  temp= String(temperature) + " HIGH!";
  return(temp);
}

String turbidity_level(){
    volt = analogRead(turbPin);
    ntu = map(volt,2015,0,0,100);

     if(ntu>=15 && ntu<=24)
    turbidity= String(ntu) + " PERFECT!";
  else if(ntu>90)
   turbidity= String(ntu) + " Danger";
  else 
   turbidity= String(ntu) ;
    return turbidity;
   
}

String ph_level()
{
  value = analogRead(phpin);
  value = value*(3.3/4095);
  ph = 3.3*value;
  if(ph>=6.5 && ph<=8)
  phvalue= String(ph) + " PERFECT";
  else
  phvalue=String(ph) + " NOT IDEAL";
  return phvalue;
}

String pir(){
  if (digitalRead(pirpin) == 1){
    piroutput="Movement detected";
  }
  else {
        piroutput="no movement";
  }
   return piroutput;
  
}
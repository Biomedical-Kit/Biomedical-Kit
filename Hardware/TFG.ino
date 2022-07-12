#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Keypad.h>            
#include "MAX30100_PulseOximeter.h"
#include "MAX30100.h"
#include <LiquidCrystal_I2C.h>
#define REPORTING_PERIOD_MS     1000
////////////////////////////////////////////////////////////CONFIGURACION/////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------MQTT-----------------------------------------------------------------------------------//
const char* ssid =   "";
const char* password =  "";
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char user[32];
char usuario_anonimo [32];
char user2 [32];

PulseOximeter pox;
uint32_t tsLastReport = 0;

//----------------------------------------------------------------TECLADO-------------------------------------------------------------------------------//
const byte Filas = 4;          //Cuatro filas
     const byte Cols = 4;           //Cuatro columnas    
     byte Pins_Cols[] = {26, 25, 33, 32};     //Pines Arduino para las filas {19, 23, 13, 14}
     byte Pins_Filas[] = { 15, 14, 13, 27};     // Pines Arduino para las columnas
     char Teclas [ Filas ][ Cols ] =
          {
              {'1','2','3','a'},
              {'4','5','6','b'},
              {'7','8','9','c'},
              {'*','0','#','d'}
          };
Keypad Teclado1 = Keypad(makeKeymap(Teclas), Pins_Filas, Pins_Cols, Filas, Cols);
//---------------------------------------------------------PANTALLA LCD ------------------------------------------------------------------------------------//
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  // set the LCD address to 0x27 for a 16 chars and 2 line display

///////////////////////////////////////////////////////////////////////////Funciones auxiliares///////////////////////////////////////////////////////////////

//------------------------------------------------------ECG-------------------------------------------------------------//
void electrocardiograma() { 
  if((digitalRead(18) == 1)||(digitalRead(19) == 1)){
    //Serial.println('error');
  }
  else{
    // send the value of analog input:
      float ecgValue = analogRead(34);
      jsonFormat("ecg", ecgValue);
  }
  //Wait for a bit to keep serial data from saturating
  delay(10);
}
//---------------------------------------------------------EMG------------------------------------------------------//
void emg()
{
float emgValue = analogRead(35); 
jsonFormat("emg", emgValue);
Serial.println(emgValue);
delay(3.9);
}
//-------------------------------------------------------Heart Rate-----------------------------------------------//
void heartRate(){
    pox.update(); 
    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        float heartrate = pox.getHeartRate();
        if(heartrate > 20 && heartrate < 200){
            jsonFormat("frecuenciacardiaca", heartrate);
        }                  
        float oxig = pox.getSpO2();
        if(oxig > 40 && oxig < 100){
            jsonFormat("pulsioximetro", oxig);
        }
        tsLastReport = millis(); 
    }
}

//----------------------------------------------LOGIN---------------------------------------------------------//
bool userlogin (){
    bool login_ok = false;
    lcd.clear();
    lcd.setCursor(0, 0);//posicionar cursor en dicha celda
    lcd.print("INTRODUCE USER:");
    strcpy(user,"");
    strcpy(usuario_anonimo,"");
    char actual[2];
    actual[1] = '\0';
    char pulsacion = -1;
    int posicion = 0;
    while (pulsacion !=42){              // Si el valor es 0 es que no se se ha pulsado ninguna tecla     
      pulsacion = Teclado1.getKey();
      if(pulsacion != 0 && pulsacion != 35){
            if(pulsacion !=42){          
              lcd.setCursor(0, 1);
              actual[0] = pulsacion;  
              strcat(user, actual);
              strcat(usuario_anonimo, actual);
              lcd.print(usuario_anonimo);
              usuario_anonimo[posicion]= '*';
              posicion++;
            }
      }
      if(pulsacion == 35){
            strcpy(user,"");
            strcpy(usuario_anonimo,"");
            lcd.clear();
            lcd.setCursor(0, 0);//posicionar cursor en dicha celda
            lcd.print("INTRODUCE USER:");
            lcd.setCursor(0, 1);
            posicion = 0;
      }
    }

    lcd.clear();
    lcd.setCursor(0, 0);//posicionar cursor en dicha celda
    lcd.print("REPITA USER:");
    strcpy(user2,"");
    strcpy(usuario_anonimo,"");
    char actual2[2];
    actual2[1] = '\0';
    pulsacion = -1;
    posicion = 0;
    while (pulsacion !=42){              // Si el valor es 0 es que no se se ha pulsado ninguna tecla     
      pulsacion = Teclado1.getKey();
      if(pulsacion != 0 && pulsacion != 35){
            if(pulsacion !=42){          
              lcd.setCursor(0, 1);
              actual2[0] = pulsacion;  
              strcat(user2, actual2);
              strcat(usuario_anonimo, actual2);
              lcd.print(usuario_anonimo);
              usuario_anonimo[posicion]= '*';
              posicion++;
            }
      }
      if(pulsacion == 35){
            strcpy(user2,"");
            strcpy(usuario_anonimo,"");
            lcd.clear();
            lcd.setCursor(0, 0);//posicionar cursor en dicha celda
            lcd.print("REPITA USER:");
            lcd.setCursor(0, 1);
            posicion = 0;
      }
    }
    if(strncmp(user,user2, 31) == 0){
        login_ok=true;
        lcd.clear();
        lcd.setCursor(0, 0);//posicionar cursor en dicha celda
        lcd.print("LOGIN CORRECTO");
        delay(3000);
    }else{
        login_ok=false;
        lcd.clear();
        lcd.setCursor(0, 0);//posicionar cursor en dicha celda
        lcd.print("LOGIN INCORRECTO");
        delay(3000);
    }
    return login_ok;
}

//---------------------------------------------Setup Wifi----------------------------------------------------------------//

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);//posicionar cursor en dicha celda
    lcd.print("FALLO WIFI");
    delay(3000);
  }
}

//------------------------------------------Wifi Reconnect----------------------------------------------------//
void reconnect() {
  // Loop until we're reconnected
  lcd.clear();
  lcd.setCursor(0, 0);//posicionar cursor en dicha celda
  lcd.print("FALLO MQTT");
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      // Subscribe
      client.subscribe("esp32/output");
      //digitalWrite(mqttledPin, HIGH);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//------------------------------------------------JSON FORMAT-----------------------------------------------//
//Función que transforma los datos de los sensores al formato JSON admitido por elastic
void jsonFormat(const char* topic, float value) {
  char string[64];
  dtostrf(value, 1, 2, string);
  char sensorData[1024] ;
  strcpy(sensorData,"{\"iduser\":\"");
  strcat(sensorData, user);
  strcat(sensorData, "\",\"data\":");
  strcat(sensorData, string);
  strcat(sensorData, "}");
  client.publish(topic, sensorData);
}

////////////////////////////////////////////////////////////////////SETUP/////////////////////////////////////////////////////////////////////////////////
void setup()
{   
    Serial.begin(115200);
//----------------------------------------------WIFI----------------------------------------------// 
    setup_wifi();
    client.setServer(mqtt_server, 13023);
//----------------------------------------ECG----------------------------------------------------//
    pinMode(19, INPUT); // Setup for leads off detection LO +
    pinMode(18, INPUT); // Setup for leads off detection LO -
    pinMode(13, INPUT);
//-----------------------------------------PANTALLA LCD ------------------------------------------//
    // initialize LCD
    lcd.init();
    // turn on LCD backlight                      
    lcd.backlight();//activar led
}

///////////////////////////////////////////////////////////////////////LOOP (MAIN)///////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    pox.shutdown(); //asegura que el pox este apagado y guardando datos basura
    lcd.clear();
    bool isLogIn = userlogin();
    
    char opcion = Teclado1.getKey();
    while (isLogIn){    
//---------------------------------MQTT reconnect----------------------------------------------------//
//Función que comprueba por cada ciclo si el cliente MQTT esta conectado, en caso de no estarlo hace la reconexión
      if (!client.connected()) {
      reconnect();
       }
      client.loop();
//-------------------------------------------Hearth Rate--------------------------------------------//
      switch (opcion){
          case '1': //HEARTH RATE
              lcd.clear();
              lcd.setCursor(0, 0);//posicionar cursor en dicha celda
              lcd.print("PULSIOXIMETRO");
              if (!pox.begin()) {
                  for(;;);
              }
              while (opcion == 0 || opcion == 49){
                  if (!client.connected()) {
                      reconnect();
                      lcd.clear();
                      lcd.setCursor(0, 0);//posicionar cursor en dicha celda
                      lcd.print("PULSIOXIMETRO");
                  }
                  opcion = Teclado1.getKey();
                  heartRate();
              }
              pox.shutdown(); 
              break;
//---------------------------------------------------------------ECG-----------------------------------------------------------------//
          case '2':
              lcd.clear();
              lcd.setCursor(0, 0);//posicionar cursor en dicha celda
              lcd.print("ECG");
              while (opcion == 0 || opcion == 50){
                  if (!client.connected()) {
                      reconnect();
                      lcd.clear();
                      lcd.setCursor(0, 0);//posicionar cursor en dicha celda
                      lcd.print("ECG");
                  }
                  opcion = Teclado1.getKey();
                  electrocardiograma();                
              }
              break;
//-------------------------------------------------------------EMG----------------------------------------------------------------//
          case '3':
              lcd.clear();
              lcd.setCursor(0, 0);//posicionar cursor en dicha celda
              lcd.print("EMG");
              while (opcion == 0 || opcion == 51){
                  if (!client.connected()) {
                      reconnect();
                      lcd.clear();
                      lcd.setCursor(0, 0);//posicionar cursor en dicha celda
                      lcd.print("EMG");
                  }
                  opcion = Teclado1.getKey();
                  emg();
              }
              break;
//--------------------------------------------------Log out-------------------------------------------------------------------//
          case '#':
              lcd.clear();
              lcd.setCursor(0, 0);//posicionar cursor en dicha celda
              lcd.print("LOG OUT");
              isLogIn = false; 
              break;
//-------------------------------------------------Modo espera----------------------------------------------------------------//
          default:
              opcion = 0;
              lcd.clear();
              lcd.setCursor(0, 0);//posicionar cursor en dicha celda
              lcd.print("ESPERANDO MODO");
              while (opcion == 0){
                  if (!client.connected()) {
                      reconnect();
                      lcd.clear();
                      lcd.setCursor(0, 0);//posicionar cursor en dicha celda
                      lcd.print("ESPERANDO MODO");
                  }
                  opcion = Teclado1.getKey();
              }
              break;
      }     
   }
}

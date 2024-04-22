/*
 * revision de pines para el rele,
 * #define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

#define I2C_SDA              21
#define I2C_SCL              22
#define LED_GPIO             13
#define LED_ON               HIGH
#define LED_OFF              LOW

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00
 */
/*
 * se usa un ina219 para voltaje y corriente sin embargo para algunos consumos muy vajo falla en medir voltaje pero para la funcion 4-20 mah es suficiente
 * https://forum.arduino.cc/t/promedio-de-lecturas/482652
 * 
 * 
 * se agrega SD

 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS         5
 *    CMD      MOSI       23
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK        18
 *    VSS      GND
 *    D0       MISO       19
 *    D1       -
 */
// Please select the corresponding model

 #define SIM800L_IP5306_VERSION_20190610
//#define SIM800L_AXP192_VERSION_20200327
// #define SIM800C_AXP192_VERSION_20200609
// #define SIM800L_IP5306_VERSION_20200811

// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS //+++++++++++++++=====================================
#define TINY_GSM_DEBUG          SerialMon

#include "utilities.h"
#define SerialMon Serial
// Set serial for AT commands (to the module)
#define SerialAT  Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800          // Modem is SIM800
#define TINY_GSM_RX_BUFFER      1024   // Set RX buffer to 1Kb

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "bam.entelpcs.cl";
const char user[] = "";
const char pass[] = "";
#include "Adafruit_MQTT.h"


TinyGsmClient client(modem);

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "arebolledo"
#define AIO_KEY         "ddab223d3d734873bc455a0f13237339"

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish counter = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/counter");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/
// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
/*************************** Sketch Code ************************************/

// How many transmission failures in a row we're willing to be ok with before reset
uint8_t txfailures = 0;
#define MAXTXFAILURES 3



//======================================================================================== variables de trabajo
int led = 13;
int rele =14;
int ledStatus = LOW;
int retrys = 3;
int retrysb = 3;
bool gsmC = false;
int BTNA = 33;
int BTNB = 32; 
int val ;
String ultimoM ;
////////////////////////////////////SD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

/////////////////////////////////
// Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// Sleep for 1 hour = 3600 seconds // 1 minuto 60 segundos
RTC_DATA_ATTR int TIME_TO_SLEEP = 10; //el deep sleep se puede guardar como variable para guardar en el reloj asi se puede modificar pero hasta que no se resetea no se pierde
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR boolean recordSleep = false;
RTC_DATA_ATTR boolean firstRecordSleep = true;
//////////variables de nombres para guardar con reloj interno
RTC_DATA_ATTR char filenameCSV[25]=""; //para construir el archivo csv base cada vez que se prenda y guarde

/////////////////////////// SD variables y funciones
boolean sd = false;
boolean firstRecord = true;
void recordNewData();
void storeDataToSDCard(fs::FS &fs, const char * path, const char * message);
////////////////datos para guardarlos 
String nombreCHIP;
String versionP = "GateKeeper V0.01";//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// version del programa
String dato1; //dispositivo o chipID
String dato2; //hora
String dato3; //temp interna
String dato4; //press
String dato5; //hum
String dato6; //alt
String dato7; //bat dispositivo


/////////////////////////////////////////////////////////////////////////////////////////////
#include <Wire.h>
#include <INA226_WE.h>
#define I2C_ADDRESS 0x40

INA226_WE ina226 = INA226_WE(I2C_ADDRESS);
 boolean inaOK = false;

#include <U8g2lib.h>
//8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
 U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW 
 
  float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0; 

////////////////////////////////////////////////////////////////////////////// setup
//
//////////////////////////////////////////////////////////////////////////////
void setup(){
pinMode(BTNA, INPUT_PULLUP);
pinMode(BTNB , INPUT_PULLUP);
    uint32_t currentFrequency;
    // Set console baud rate
    SerialMon.begin(115200);


    
 Serial.println("Test gsm mqtt esp32!");

 display.begin();   
display.setFont(u8g2_font_pressstart2p_8u);
  display.setFont(u8g2_font_t0_11_tf);
  // display.setFontRefHeightExtendedText();
  display.setDrawColor(1);
  display.setFontPosTop();
  display.setFontDirection(0);
  display.drawStr(0, 0, "test ina V 0.1");
  display.sendBuffer(); // transfer internal memory to the display

   ina226.init();
  if (! ina226.init()) {
    Serial.println("Failed to find INA226 chip");
  //  while (1) { delay(10); }
     display.drawStr(0, 20, "ina fail");
         
        display.sendBuffer();          // transfer internal memory to the display
       
  }
  // ina226.waitUntilConversionCompleted(); //if you comment this line the first data might be zero
    
pinMode(led, OUTPUT);
pinMode(rele, OUTPUT);
digitalWrite(rele, LOW); 
////////////////////////////////////////////////////////////////// SD

 if(!SD.begin()){
        Serial.println("Card Mount Failed");
         display.drawStr(0, 10, "SD ERR");
          display.sendBuffer();          // transfer internal memory to the display
       
        delay(1000);
        return;
    }
if(SD.begin()){
        Serial.println("SD OK");
        sd = true;

        display.drawStr(0, 10, "SD OK");
         
        display.sendBuffer();          // transfer internal memory to the display
        delay(2000);
 ///////////////funcion crear csvs distintos segun si hay o no archivo       
 int n = 0;
String NP = "/";
NP = NP+nombreCHIP+"N%03d.csv"; 
Serial.println(NP)  ;
int str_lenNP = NP.length() + 1;  // esta funcion es para asignarle el valor de casillas al array
char npC[str_lenNP]; //este es el char array  
NP.toCharArray(npC, str_lenNP);  
Serial.println(npC);  
snprintf(filenameCSV, sizeof(npC), npC, n); // includes a three-digit sequence number in the file name 
Serial.println(filenameCSV);
 while(SD.exists(filenameCSV)) {
 n++;
 snprintf(filenameCSV, sizeof(npC), npC, n); // includes a
  }
 
  File file = SD.open(filenameCSV,FILE_READ); //crea el archivo
  Serial.println(n);
  Serial.println(filenameCSV);
  file.close();      
  return;
} 
 



    // Start power management
    if (setupPMU() == false) {
        Serial.println("Setting power error");
    }

    // Some start operations
    setupModem();

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    
      modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
 // Some start operations
    setupModem();
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    while(retrys == 0 ){
     static uint32_t retryT;
 if (millis() - retryT >= 10000){ 
  retryT = millis();
  retrys--;
  SerialMon.print(".");
        }
    } 
    gsmC = false;    
  }else{
    gsmC = true;
    }
  if(gsmC == true){
  SerialMon.println(" OK");
  }
  

}

unsigned long t = 0;
uint32_t x=0;
//////////////////////////////////////////////////loop
//
////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
/////////////////////////////////////
int btnA = digitalRead(BTNA);
int btnB = digitalRead(BTNB);
if(btnA == 0){
  val = 4;
digitalWrite(rele,HIGH); 
 Serial.println("btn a");
  }
  if(btnB == 0){
  val = 1;
  digitalWrite(rele, LOW); 
   Serial.println("btnb");
  }

   String modemInfo = modem.getModemInfo();
  DBG("Modem:", modemInfo);
bool res = modem.isNetworkConnected();

int index=modem.newMessageIndex(0);
if(index>0){
String SMS=modem.readSMS(index);
String ID=modem.getSenderID(index);
DBG("new message arrived from :");
DBG(ID);DBG("Says");DBG(SMS);

ultimoM = String(SMS);
 }
  ina226.readAndClearFlags();
  shuntVoltage_mV = ina226.getShuntVoltage_mV();
  busVoltage_V = ina226.getBusVoltage_V();
  current_mA = ina226.getCurrent_mA();
  power_mW = ina226.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
  
   static uint32_t ToShow; //timetofill
 if (millis() - ToShow >= 5000) { //muestra el serial cada 5 segundos
    Serial.print("Shunt Voltage [mV]: "); Serial.println(shuntVoltage_mV);
  Serial.print("Bus Voltage [V]: "); Serial.println(busVoltage_V);
  Serial.print("Load Voltage [V]: "); Serial.println(loadVoltage_V);
  Serial.print("Current[mA]: "); Serial.println(current_mA);
  Serial.print("Bus Power [mW]: "); Serial.println(power_mW);
  if(!ina226.overflow){
    Serial.println("Values OK - no overflow");
  }
  else{
    Serial.println("Overflow! Choose higher current range");
  }
  Serial.println();
 
 }  

   

   display.clearBuffer(); //limpia la pantalla    
 display.setCursor(0, 0);     // Start at top-left corner
 display.print(String(current_mA)+" mA");
 display.setCursor(0, 10);     // Start at top-left corner
  display.print(String(loadVoltage_V)+"V");
  display.setCursor(10, 20);     // Start at top-left corner
  display.print("enviando no");

 display.sendBuffer();

 
   static uint32_t ToSD; //timetofill
 if (millis() - ToSD >= 150000) {
    recordNewData(); ///////////// llama a guardar cada 5 segundos como minimo
    ToSD = millis(); 
 }  

}

//===============================================================
// enbloqueado de datos para guardar el csv
// se consideran generar instancias segun que sensores esten conectados
//===============================================================
void recordNewData(){
// String message;
// ////////////////////////////////////////////////////////////////primer loop para generar header
// if (firstRecord == true && firstRecordSleep  == true  ){
// dato1 = "NOMBRE"; //dispositivo
// dato2 = "TIEMPO PRENDIDO"; //hora relativa al uso del dispositivo
// dato3 = "VOLTAJE BATERIA"; //hora relativa al uso del dispositivo 
// dato4 = "TEMPERATURA INT"; //temp interna
// dato5 ="HUMEDAD INT";     //hum interna
// dato6 = "PRESION INT";     //pression interna
// dato7 = "ALTURA";          //alt
//
//
//  message = String(dato1) //nombre del dispositvo
//  + "," 
//  + String(dato2)  //tiempo
//  + "," 
//  + String(dato3)  //VOLTAJE
//  + "," 
//  + String(dato4)  //TEMP INT
//  + "," 
//  + String(dato5)  //HUM INT
//  + "," 
//  + String(dato6)  // PRESS INT
//  + "," 
//  + String(dato7)  // ALTURA
//
// firstRecord = false; 
// }else{
//
// dato1 = nombreCHIP ; //dispositivo
// dato2 = tiempo; //hora relativa al uso del dispositivo
// dato3 = String(battv); //bat dispositivo
// dato4 = bme.readTemperature(); //temp interna
// dato5 = bme.readHumidity(); //hum interna
// dato6 = bme.readPressure() / 100.0F;//pression interna
// dato7 = bme.readAltitude(SEALEVELPRESSURE_HPA); /nombre del dispositvo
//  + "," 
//  + String(dato2) //tiempo
//  + "," 
//  + String(dato3) //VOLTAJE
//  + "," 
//  + String(dato4) //TEMPINT
//  + "," 
//  + String(dato5) //HUMINT
//  + "," 
//  + String(dato6) //PRESSINT
//  + "," 
//  + String(dato7) //ALTURA
//
// }
//  Serial.println(message);
//  storeDataToSDCard(SD, filenameCSV, message.c_str());
//
}
//===============================================================
// guardando SD
//===============================================================
void storeDataToSDCard(fs::FS &fs, const char * path, const char * message) {
//  Serial.printf("Appending data to file: %s\n", path);
//  File file = fs.open(path, FILE_APPEND);
//  if(!file) {
//    Serial.println("Failed to open file for appending");
//    return;
//  }
//  if(file.println(message)) {
//    Serial.println("Data appended");
//  } else {
//    Serial.println("Append failed");
//  }
//  file.close();
}


// (C) Antonio F. Díaz 2022-2024

#define BLUETOOTH_SUPPORT
//#define UGR_KEYS

#include <HardwareSerial.h>
//#include <SoftwareSerial.h>
//#define PZEM004_SOFTSERIAL
#include <PZEM004Tv30.h>

//#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <EasyButton.h>
//#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>
#include "HttpsOTAUpdate.h"
#include <PubSubClient.h>
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks
#include <esp_task_wdt.h>
#include <esp_system.h>

#include <map>

#ifdef BLUETOOTH_SUPPORT
#include "BluetoothSerial.h"
#endif

HardwareSerial Serx2(1);
#define RXD2 19
#define TXD2 22
#define DISPLAY_SDA 18
#define DISPLAY_SCL 21
#define BUTTON_PIN 5

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDR 0x3C
#define OLED_RESET -1


using namespace std;
std::map <String, String> params;

// for eduroam old certificate
/*
const static char* test_root_ca PROGMEM = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDtzCCAp+gAwIBAgIQDOfg5RfYRv6P5WD8G/AwOTANBgkqhkiG9w0BAQUFADBl\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSQwIgYDVQQDExtEaWdpQ2VydCBBc3N1cmVkIElEIFJv\n" \
    "b3QgQ0EwHhcNMDYxMTEwMDAwMDAwWhcNMzExMTEwMDAwMDAwWjBlMQswCQYDVQQG\n" \
    "EwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3d3cuZGlnaWNl\n" \
    "cnQuY29tMSQwIgYDVQQDExtEaWdpQ2VydCBBc3N1cmVkIElEIFJvb3QgQ0EwggEi\n" \
    "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCtDhXO5EOAXLGH87dg+XESpa7c\n" \
    "JpSIqvTO9SA5KFhgDPiA2qkVlTJhPLWxKISKityfCgyDF3qPkKyK53lTXDGEKvYP\n" \
    "mDI2dsze3Tyoou9q+yHyUmHfnyDXH+Kx2f4YZNISW1/5WBg1vEfNoTb5a3/UsDg+\n" \
    "wRvDjDPZ2C8Y/igPs6eD1sNuRMBhNZYW/lmci3Zt1/GiSw0r/wty2p5g0I6QNcZ4\n" \
    "VYcgoc/lbQrISXwxmDNsIumH0DJaoroTghHtORedmTpyoeb6pNnVFzF1roV9Iq4/\n" \
    "AUaG9ih5yLHa5FcXxH4cDrC0kqZWs72yl+2qp/C3xag/lRbQ/6GW6whfGHdPAgMB\n" \
    "AAGjYzBhMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n" \
    "BBRF66Kv9JLLgjEtUYunpyGd823IDzAfBgNVHSMEGDAWgBRF66Kv9JLLgjEtUYun\n" \
    "pyGd823IDzANBgkqhkiG9w0BAQUFAAOCAQEAog683+Lt8ONyc3pklL/3cmbYMuRC\n" \
    "dWKuh+vy1dneVrOfzM4UKLkNl2BcEkxY5NM9g0lFWJc1aRqoR+pWxnmrEthngYTf\n" \
    "fwk8lOa4JiwgvT2zKIn3X/8i4peEH+ll74fg38FnSbNd67IJKusm7Xi+fT8r87cm\n" \
    "NW1fiQG2SVufAQWbqz0lwcy2f8Lxb4bG+mRo64EtlOtCt/qMHt1i8b5QZ7dsvfPx\n" \
    "H2sMNgcWfzd8qVttevESRmCD1ycEvkvOl77DZypoEd+A5wwzZr8TDRRu838fYxAe\n" \
    "+o0bJW1sj6W3YQGx0qMmoRBxna3iw/nDmVG3KwcIzi7mULKn+gpFL6Lw8g==\n" \
    "-----END CERTIFICATE-----\n";
    */
   // new certificate 2022
  const static char* test_root_ca PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----";


#define DEFAULT_TS_ESP_WDT_S "30"
#define DEFAULT_TS_WIFI_WDT_S "300"
#define DEFAULT_TS_MQTT_WDT_S "3600"
#define DEFAULT_0W_MSG_WDT "0"   // No watchdog for 0W msgs
//#define SAMPLING_POWER_BUFFER 25

const char *release = "1.2.2";
String preferences_version;
//const char *valid_cmds[] = {"get", "set", "help", "info", "update", "reset", "version","scan"};
const char *valid_params[] = {"mode","ssid","password","ssid2","password2",
  "ssid3","user3","password3","anon3", // for eduroam
  "alias","group","interval",
  "max_ts_esp_wdt","max_ts_wifi_wdt","max_ts_mqtt_wdt","max_0w_msg_wdt",
  "mqtt_server","mqtt_port","mqtt_user","mqtt_password","mqtt_topic",
  "mqtt_server2","mqtt_port2","mqtt_user2","mqtt_pass2","mqtt_topic2",
  "url_update","url_update2"
  };

const char *state_string[] = {
  "Idle", "Connecting", "Connected"};
// States: 0=idle, 1=connecting, 2=connected
enum {
  IDLE,
  CONNECTING,
  CONNECTED
};
int wifi_state = IDLE;
bool connecting_wifi = false;
bool auto_send_serial_data = false;
bool auto_send_mqtt_data = true;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
PZEM004Tv30 pzem(Serx2,RXD2,TXD2,PZEM_DEFAULT_ADDR);
Preferences preferences;

EasyButton button1(BUTTON_PIN);

WiFiClient espClient;
PubSubClient client(espClient);
char jsonbuffer[1024];
//DynamicJsonDocument doc(1024);


float voltage=0.0, current=0.0, power=0.0, energy=0.0, frequency=0.0, pf=0.0;
float sum_voltage=0.0, sum_current=0.0, sum_power=0.0, sum_energy=0.0,sum_frequency=0.0,sum_pf=0.0;
float last_power=0.0;
//float partial_power[SAMPLING_POWER_BUFFER];
int interval=0;
int sampling_period;
int samples_per_interval;
int current_samples_per_interval=0;

unsigned long t_menu0=0;
unsigned long t1m;
unsigned long twscan;
unsigned long twmqtt;
unsigned long t_wifi_wdt;
unsigned long t_mqtt_wdt;
unsigned long t_seconds;
unsigned long cc_0w_msg_wdt=0;
unsigned long max_0w_msg_wdt=0;

String mac_ad,vid1,vgroup;
String cmdstr, cmd0,arg1,arg2;
String current_mqtt_topic;
String pending_mqtt_topic,pending_mqtt_result;

int selected_wifi = 0;
int selected_mqtt = 0;
int nn=0;
bool wait_mqtt_flag=false;
bool show_status_flag=false;
bool show_data_flag=false;
bool show_state_flag=true;
bool try_mqtt_flag=false;
bool interval_updated=false;
bool show_vid_flag=false;
int menu=0;
long int tkey=0;


// statistics
int total_mqtt_sent=0;
int total_mqtt_received=0;

int total_mqtt_connect_attempts=0;
int total_mqtt_connects=0;

int total_wifi_connect_attempts=0;
int total_wifi_connects=0;

int total_failed_sampling=0;
int total_successful_sampling=0;

int total_seconds=0;



#ifdef BLUETOOTH_SUPPORT
bool bluetooth_mode=0;
BluetoothSerial SerialBT;
String btcmdstr;
#endif


void  onPressedForDuration4() {
#ifdef BLUETOOTH_SUPPORT
  bluetooth_mode=!bluetooth_mode;
  if (bluetooth_mode) {
    SerialBT.begin(vid1.c_str());
  } else {
    SerialBT.end();
  }
  menu=2;
  show_status_flag=true;
#endif
}




String scanNetworks() {
  String result="{\"networks\":[";
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(300);
//  Serial.print("st:"); Serial.println( WiFi.status() );
  while (WiFi.status() == WL_CONNECTED) {
    delay(500);
  }
//  WiFi.begin();
  delay(500);

  WiFi.scanDelete();
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    if (i>0) result+=",";
    result+="{\"ssid\":\""+String(WiFi.SSID(i))+"\",\"rssi\":"+String(WiFi.RSSI(i))+"}";
//    Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
  }
  result+="]}";
  return result;
}


int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware

void updateFirmware(uint8_t *data, size_t len){
  if (len >0) {
    Update.write(data, len);
    currentLength += len;
    // Print dots while waiting for update to finish
//    Serial.print('.');
  }
  // if current length of written firmware is not equal to total firmware size, repeat
  if(currentLength != totalLength) return;
  Update.end(true);
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
  // Restart ESP32 to see changes 
  ESP.restart();
}

//#define HOST "https://drive.google.com/file/d/1HgQ9N-AuT_jc8-WyfmHhDW137NehdJC2/view?usp=sharing"
//#define HOST "https://drive.google.com/uc?export=download&id=1HgQ9N-AuT_jc8-WyfmHhDW137NehdJC2"
#define HOST "https://drive.ugr.es/index.php/s/SPVpkjPimPnkKoa/download"

void updating_display(float percentaje) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Updating:");
  display.setCursor(0,20);
  display.print(percentaje,0);  // no decimal
  display.println("%");
  display.display();
}

void esp32_ota_update(String url_update) {
HTTPClient client;
int nn=0;
float percentaje=0.0;
float last_percentaje=0.0;

 client.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
 client.begin(url_update.c_str());
  // Get file, just to check if each reachable
  int resp = client.GET();
  Serial.print("Response: ");
  Serial.println(resp);
  // If file is reachable, start downloading
  if(resp > 0){
      // get length of document (is -1 when Server sends no Content-Length header)
      totalLength = client.getSize();
      // transfer to local variable
      int len = totalLength;
      // this is required to start firmware update process
      Update.begin(UPDATE_SIZE_UNKNOWN);
      Serial.printf("FW Size: %u\n",totalLength);
      // create buffer for read
      uint8_t buff[512] = { 0 };
      // get tcp stream
      WiFiClient * stream = client.getStreamPtr();
      // read all data from server
      Serial.println("Updating firmware...");
      while(client.connected() && (len > 0 || len == -1)) {
           // get available data size
           size_t size = stream->available();
           //Serial.print('-');    Serial.print(currentLength); Serial.print('-');
           Serial.print(currentLength); Serial.print("      \r");
           percentaje = (float)currentLength/totalLength*100;
           if (percentaje>=last_percentaje+5.0) {
             last_percentaje=percentaje;
             updating_display(percentaje);
           }

           if(size) {
              int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
              // pass to function
              updateFirmware(buff, c);
              if(len > 0) {
                 len -= c;
              }
           } else {
             if ((totalLength==-1) && (++nn == 1000)) {
               totalLength=currentLength;
              updateFirmware(buff, 0);
             }

           }
           delay(1);
      }
  }else{
    Serial.println("Cannot download firmware file");
  }
  client.end();
}


static HttpsOTAStatus_t otastatus;

void ota_https_update() {
  HttpsOTA.begin(HOST, nullptr, true);
  while (true) {
    otastatus = HttpsOTA.status();
   if(otastatus == HTTPS_OTA_SUCCESS) { 
      Serial.println("OTA update success!");
      ESP.restart();
    } else if(otastatus == HTTPS_OTA_FAIL) { 
        Serial.println("Firmware Upgrade Fail");
        break;
    }
    Serial.print(".");
    delay(500);
  }
}

int getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

void update_sampling_parameters() {
  interval=params["interval"].toInt();
  current_samples_per_interval=0;
  if (interval<=1000) {
    sampling_period=500;
    samples_per_interval=2;
  } else if (interval<=5000 ){
    sampling_period=500;
    samples_per_interval=interval/500;
  } else {
    sampling_period=1000;
    samples_per_interval=interval/1000;
  }
  interval=sampling_period * samples_per_interval;
}

void reset_values() {
  sum_power=0.0;
  sum_energy=0.0;
  sum_voltage=0.0;
  sum_frequency=0.0;
  sum_current=0.0;
  sum_pf=0.0;
  current_samples_per_interval=0;
  last_power = 0.0;
/*  for (int i=0;i<SAMPLING_POWER_BUFFER;i++)
    partial_power[i]=0.0; */
}

void last_reset_condition(String xparam){
  preferences.begin("vampire", false);
  preferences.putString("l_reset", xparam);
  preferences.end();
}

void inc_param(String xparam) {
  int counter;
  preferences.begin("vampire", false);
  counter = preferences.getInt(xparam.c_str(), 0);
  counter++;
  preferences.putInt(xparam.c_str(), counter);
  preferences.end();
}

int get_int_param(String xparam) {
  int counter;
  preferences.begin("vampire", false);
  counter = preferences.getInt(xparam.c_str(), 0);
  preferences.end();
  return counter;
}

String get_str_param(String xparam) {
  String xstr;
  preferences.begin("vampire", false);
  xstr = preferences.getString(xparam.c_str(),"");
  preferences.end();
  return xstr;
}

int int_xversion(String xversion) {
String v1,v2,v3;
    v1=xversion.substring(0,xversion.indexOf('.'));
    xversion.remove(0,xversion.indexOf('.')+1);
    v2=xversion.substring(0,xversion.indexOf('.'));
    xversion.remove(0,xversion.indexOf('.')+1);
    v3=xversion;
    return (v1.toInt()*10000)+(v2.toInt()*100)+v3.toInt();
}

void home_preferences() {
// 
}

#ifdef UGR_KEYS
struct ugr_keys_st {
  string mac ;
  string user;
  string pass;
}  ;

#define UGR_N_KEYS 1
ugr_keys_st ugr_k[UGR_N_KEYS]={
  {.mac="00:11:22:33:44:55",.user="r@.es",.pass="nopasswd"},

};

void ugr_preferences() {
  preferences.putString("ssid3","eduroam");
  preferences.putString("anon3","anonymous2022@ugr.es");
  for (int i=0;i<UGR_N_KEYS;i++) {
    if (WiFi.macAddress() ==ugr_k[i].mac.c_str()) {
      preferences.putString("user3",ugr_k[i].user.c_str());
      preferences.putString("password3",ugr_k[i].pass.c_str());
      break;
    }
  }
}


#endif


void first_preferences() {
  preferences.putString("ssid","");
  preferences.putString("password","");
  preferences.putString("ssid2","");
  preferences.putString("password2","");
  preferences.putString("ssid3","");
  preferences.putString("user3","");
  preferences.putString("password3","");

  preferences.putString("alias","");
  preferences.putString("group","default");
  preferences.putString("interval","5000");
  preferences.putString("mqtt_data","none");
  preferences.putString("serial_data","none");

  preferences.putString("url_update","https://drive.ugr.es/index.php/s/SPVpkjPimPnkKoa/download");
  preferences.putString("url_update2","https://drive.google.com/uc?export=download&id=1HgQ9N-AuT_jc8-WyfmHhDW137NehdJC2");
  preferences.putString("mqtt_server","rlab.ugr.es");
  preferences.putString("mqtt_port","2681");
  preferences.putString("mqtt_user","v"); // place mqtt user
  preferences.putString("mqtt_password","A"); // place mqtt password
  preferences.putString("mqtt_topic","vampire/");
  preferences.putString("max_topic_data","vampire/data/");

  preferences.putString("max_ts_esp_wdt",DEFAULT_TS_ESP_WDT_S);  // number of seconds to wait before restarting the ESP32 watchdog  
  preferences.putString("max_ts_wifi_wdt",DEFAULT_TS_WIFI_WDT_S);  // number of seconds to wait before restarting the soft watchdog
  preferences.putString("max_ts_mqtt_wdt",DEFAULT_TS_MQTT_WDT_S); // number of seconds to wait before restarting the mqtt watchdog
  preferences.putString("max_0w_msg_wdt",DEFAULT_0W_MSG_WDT); // number of seconds to wait before restarting the 0w watchdog")
}

void release_preferences() {
int int_version = int_xversion(preferences.getString("pref_version","0.0.0"));
  if (int_version<10106) {
    preferences.putString("mqtt_topic","vampire/");
    preferences.putString("max_ts_esp_wdt",DEFAULT_TS_ESP_WDT_S);  // number of seconds to wait before restarting the ESP32 watchdog  
    preferences.putString("max_ts_wifi_wdt",DEFAULT_TS_WIFI_WDT_S);  // number of seconds to wait before restarting the soft watchdog
    preferences.putString("max_ts_mqtt_wdt",DEFAULT_TS_MQTT_WDT_S); // number of seconds to wait before restarting the mqtt watchdog
   }
  if (int_version<10108) { // release 1.1.8 update anonymous user
      preferences.putString("anon3","anonymous2022@ugr.es");
  }
  if (int_version<10201) { // release 1.2.1 update 0w_msg_wdt
      preferences.putString("max_0w_msg_wdt","0");
  }

}

void init_preferences() {
  preferences.begin("vampire", false);
  preferences_version = preferences.getString("pref_version", "");
  if (preferences_version == "") { 
    first_preferences();
    preferences.putString("pref_version", release);
  } else if (preferences_version != release) {
      release_preferences();
      preferences.putString("pref_version", release);
  }   
  preferences.end();
}

void update_params() {
  if (params["mqtt_data"]=="off") {
    auto_send_mqtt_data=false;
  }
  if (params["mqtt_data"]=="on") {
    auto_send_mqtt_data=true;
  }
  if (params["serial_data"]=="off") {
    auto_send_serial_data=false;
  }
  if (params["serial_data"]=="on") {
    auto_send_serial_data=true;
  }
  if ((sampling_period==0) || (params["interval"].toInt()!=interval)) {
    update_sampling_parameters();
    interval_updated=true;
  }
}

void load_preferences() {
  preferences.begin("vampire", false);
  for (int i=0;i<sizeof(valid_params)/sizeof(valid_params[0]);i++) {
    params[valid_params[i]] = preferences.getString(valid_params[i],"");
  }
  preferences.end();
  update_params();
}

void setup() {
  int max_ts_esp_wdt;
  Serial.begin(115200);
//  Serx2.begin(9600,SERIAL_8N1,RXD2,TXD2);
  delay(100); 
  Wire.begin(DISPLAY_SDA,DISPLAY_SCL, (uint32_t) 100000);
  delay(100);
  display.begin (SSD1306_SWITCHCAPVCC, OLED_ADDR);
  delay(100);

  display.clearDisplay ();
  display.display ();
  display.setTextSize (2);
  display.setTextColor (WHITE);
  display.setCursor (1,20);
  display.print ("VampireUGR");
  display.setCursor (1,40);
  display.print (release);
  display.display();
  
  WiFi.mode(WIFI_STA);
  mac_ad = WiFi.macAddress();
  vid1="vampire-"+mac_ad.substring(9,11)+mac_ad.substring(12,14)+mac_ad.substring(15,17);
//  EEPROM.begin(EEPROM_SIZE);
  button1.begin();
  button1.onPressedFor(4000,onPressedForDuration4);
  init_preferences();
  load_preferences();
  reset_values();
  max_ts_esp_wdt=preferences.getString("max_ts_esp_wdt").toInt(); // config esp32 wdt
  if (max_ts_esp_wdt<0) max_ts_esp_wdt=0;
  if (max_ts_esp_wdt) esp_task_wdt_init(max_ts_esp_wdt, true);
  t1m=millis();   // init timer values
  t_menu0=millis();
  t_wifi_wdt=millis();
  t_mqtt_wdt=millis();
  t_seconds=millis();
}

void init_sampling_vars() {
  voltage=0.0;
  current=0.0;
  power=0.0;
  energy=0.0;
  frequency=0.0;
  pf=0.0;
}

void sampling() {
  voltage = pzem.voltage();
  if (isnan(voltage)) voltage = 0.0;
  if (voltage==0.0) {
    init_sampling_vars();
  } else {
    power = pzem.power();
    if (isnan(power)) power = 0.0;
    if (power > 5000.0) { // if power > 5000, not real value
      init_sampling_vars();
    } else {
      current = pzem.current();
      if (isnan(current)) current = 0.0;
      energy = pzem.energy();
      if(isnan(energy)) energy = 0.0;
      frequency = pzem.frequency();
      if(isnan(frequency)) frequency = 0.0;
      pf = pzem.pf();
      if (isnan(pf)) pf = 0.0;
    }
  }
}

String sstate() {
String result = state_string[wifi_state];
  switch(wifi_state) {
    case CONNECTING:
      result = result + " " + WiFi.SSID();
      break;
    case CONNECTED:
      switch (selected_wifi) {
        case 0:
          result = result + " " + params["ssid"];
        break;
        case 1:
          result = result + " " + params["ssid2"];
        break;
        case 2:
          result = result + " " + params["ssid3"];
        break;
      }
      break;
  }
  if (result.length()>20) result = result.substring(0,20);
  return result;
}

int nn_status=0;
void show_status() {
  display.clearDisplay();
  display.setTextSize (1);
  display.setCursor (0,0);
  switch(wifi_state) {
    case IDLE:
      display.print("IDLE");
      break;
    case CONNECTING:
      display.print("CONNECTING ");
      display.print(selected_wifi);
      break;
    case CONNECTED:
      display.print("CONNECTED ");
      display.print( getRSSIasQuality(WiFi.RSSI()) );
      display.print("%");
      break;
  }

  display.setCursor (0,10);
  display.print(selected_wifi);
  display.setCursor (0,20);
  display.print(WiFi.status());
  display.setCursor (0,30);
  display.print(++nn_status);
  display.display();
}

void show_data() {
  char ss[32];
  display.clearDisplay ();
  display.setCursor (0,0);
  display.setTextSize (2);
  sprintf(ss,"%.1fW",power);
  display.print (ss);

  display.setTextSize (1);
  display.setCursor (0,20);
  sprintf(ss,"%.3fkWh",energy);
  display.print (ss);

  display.setTextSize (1);
  display.setCursor (0,30);
  sprintf(ss,"%.1fv %.0fHz",voltage, frequency);
  display.print (ss);
#ifdef BLUETOOTH_SUPPORT
  if (bluetooth_mode) display.print(" BT");
#endif
  display.setCursor (0,40);
  if (wifi_state!=CONNECTED) {
    display.print(sstate());
  } else {
    sprintf(ss,"%.3fA %.2fpf",current, pf);
    display.print (ss);
    if (client.connected()) display.print(" MQ");
     else display.print(" --");
  }

  display.setCursor (0,50);
  show_vid_flag=!show_vid_flag;
  if (show_vid_flag) display.print(vid1);
    else display.print(params["alias"]);
  display.print(" ");
  
  switch(wifi_state) {
    case IDLE:
      display.print("ID");
      break;
    case CONNECTING:
      display.print("C..");
      break;
    case CONNECTED:
      display.print( getRSSIasQuality(WiFi.RSSI()) );
      display.print("%");
      break;
  }
  display.display(); 
}

void send_serial_data() {
//  char ss[256];
//  sprintf(ss,"VMP,M,id,id,grp,%.1f,%.3f,%.1f,%.0f,%.3f,%.2f", power, energy, voltage, frequency, current, pf);
  Serial.println(jsonbuffer);
//  sprintf(ss,"VMP,S,id,id,grp,%s",release);
//  Serial.println(ss);
}

void check_keys() {
    button1.read();
    if (button1.wasPressed()) {
      if (++menu>=2) menu=0;
      switch (menu) {
        case 0:
          show_data_flag=true;
          break;
        case 1:
          tkey=millis();
          show_status_flag=true;
          break;
      }
    }
}

void send_data_mqtt() {
String topic = current_mqtt_topic+vid1+"/data";
  client.publish(topic.c_str(), jsonbuffer);
  total_mqtt_sent++;
  if (power!=0.0) {
    cc_0w_msg_wdt=0;
  } else {
    max_0w_msg_wdt=params["max_0w_msg_wdt"].toInt();
    if (max_0w_msg_wdt) {
      cc_0w_msg_wdt++;
      if (cc_0w_msg_wdt>max_0w_msg_wdt) {
        inc_param("0w_counter");
        last_reset_condition("0w_counter");
        ESP.restart();
      }
    }
  }
}

//float last_p=0.0;

void measure2json() {
  char xstr[32];
  sprintf(jsonbuffer,"{\"id\":\"%s\",\"alias\":\"%s\",\"group\":\"%s\",\"power\":%.1f,\"energy\":%.3f,\"voltage\":%.1f,\"frequency\":%.0f,\"current\":%.3f,\"pf\":%.2f}",
    vid1.c_str(), params["alias"].c_str(), 
    vgroup.c_str(), power, energy, voltage, frequency, current, pf);
/*    
  sprintf(xstr,"%.1f",(partial_power[0]+last_p)/2.0);
  last_p=partial_power[0];
  strcat(jsonbuffer,xstr);
  for (int i=1;i<samples_per_interval;i++) {
    sprintf(xstr,",%.1f",(partial_power[i]+last_p)/2.0);
    last_p=partial_power[i];
    strcat(jsonbuffer,xstr);
  }
  strcat(jsonbuffer,"]}"); */
}

void send_data() {
  // calculate mean values
  if (current_samples_per_interval>0) {
    power=sum_power/current_samples_per_interval;
    energy=sum_energy/current_samples_per_interval;
    voltage=sum_voltage/current_samples_per_interval;
    frequency=sum_frequency/current_samples_per_interval;
    current=sum_current/current_samples_per_interval;
    pf=sum_pf/current_samples_per_interval;
  } else {
    power=0.0;
    energy=0.0;
    voltage=0.0;
    frequency=0.0;
    current=0.0;
    pf=0.0;
    last_power=0.0;
  }
  measure2json();
  if ( (auto_send_mqtt_data) && (client.connected()) ) send_data_mqtt(); // as fast as possible, 4 ms
  if (auto_send_serial_data) send_serial_data(); //  < 1 ms
  reset_values();
}

void accum_data() {
  if ((power==0.0) && (last_power!=0.0)) {
    last_power=power;
    return;
  }
  last_power=power;
  sum_power+=power;
  sum_energy+=energy;
  sum_voltage+=voltage;
  sum_frequency+=frequency;
  sum_current+=current;
  sum_pf+=pf;
//  if (current_samples_per_interval < SAMPLING_POWER_BUFFER )
//    partial_power[current_samples_per_interval]=power;
  current_samples_per_interval++;
}

//unsigned long xmillis;
void check_measurement() {
 if (interval_updated) {
    send_data();
    update_sampling_parameters();
    t1m=millis();
    interval_updated=false;
 }
 if (millis()-t1m >= sampling_period) {
    t1m+=sampling_period;
    sampling(); // 60 ms, 100 ms (if device is not connected)
    accum_data();
//    Serial.print(power);
//    Serial.print(" ");
//    Serial.println(millis()-xmillis);
//    xmillis=millis();
    if (current_samples_per_interval>=samples_per_interval) send_data();
  }
}

String remove_leading_spaces(String s) {
  int i=0;
  while (s.length()>0 && s[i]==' ') i++;
  return s.substring(i);
}
String Get_token(String s) {
  int i=s.indexOf(" ");
  if (i==-1) return s;
  return s.substring(0,i);
}

void cmd_set() {
  for (int i=0;i<sizeof(valid_params)/sizeof(valid_params[0]);i++) {
    if (arg1==valid_params[i]) {
      if ((arg1=="ssid") || (arg1=="ssid2")) arg2 = cmdstr;  // ssid is a special case, it can include spaces
      params[arg1]=arg2;
      preferences.begin("vampire", false);
      preferences.putString(arg1.c_str(), arg2);
      preferences.end();
      update_params();
      if ((arg1=="ssid") || (arg1=="ssid2") || (arg1=="password") || (arg1=="password2")  ) { // if ssid changed, disconnect
        WiFi.disconnect();
        wifi_state=IDLE;
        selected_wifi=0;
     }

    }
  }
}

String cmd_get() {
  String result="{";
  if (arg1=="all") {
    for (int i=0;i<sizeof(valid_params)/sizeof(valid_params[0]);i++) {
      if (i>0) result+=",";
//      if (String(valid_params[i]).indexOf("password")!=-1) { // password key detected
//        result+="\""+String(valid_params[i])+"\":\"******\"";
//      }
//      else 
      result+="\""+String(valid_params[i])+"\":"+'"'+params[valid_params[i]]+'"';
    }
  } else {
    if (arg1.indexOf("password")!=-1) { // password key detected
      result+="\""+arg1+"\":\"******\"";
    } else
    result+='"' + arg1 + '"'+": "+'"' + params[arg1] + '"';
  }
  result+="}";
  return result;
}

String cmd_update() {
String url_update;
  url_update = params["url_update"];
  if (arg1.length()>0) {
    if (arg1=="2") url_update=params["url_update2"];
  }
  if (url_update.length()==0) return "no url_update";
  esp32_ota_update(url_update);
  return "OK";
}

String cmd_serial_data() {
  String result="";
  if (arg1=="on") {
    auto_send_serial_data=true;
  } else if (arg1=="off") {
    auto_send_serial_data=false;
  }
  return result;
}
String cmd_mqtt_data() {
  String result="";
  if (arg1=="on") {
    auto_send_mqtt_data=true;
  } else if (arg1=="off") {
    auto_send_mqtt_data=false;
  }
  return result;
}

void clear_counters() {
  preferences.begin("vampire", false);
  preferences.putInt("wifi_wdt_counter", 0);
  preferences.putInt("mqtt_wdt_counter", 0);
  preferences.putInt("0w_counter", 0);
  preferences.putString("l_reset", "");
  preferences.end();
}
 
void cmd_reset() {
    if (arg1=="hard") {
      preferences.begin("vampire", false);
      first_preferences();
      preferences.end();
    } else if (arg1=="counters") {
      clear_counters();
//    } else if (arg1=="home") {
//      preferences.begin("vampire", false);
//      home_preferences();
//      preferences.end();
#ifdef UGR_KEYS
    } else if (arg1=="ugr") {
      preferences.begin("vampire", false);
      ugr_preferences();
      preferences.end();
#endif
    } else if (arg1=="kw") {
      pzem.resetEnergy();
      return;
    }
  ESP.restart();
}

#define APPEND_RESULT_STR(key, value) result += '"' + String(key) + '"' + ": " + '"' + String(value) + '"' + ","
#define APPEND_RESULT_VAL(key, value) result += '"' + String(key) + '"' + ": " + value + ","

String cmd_debug() {
  String result="{";
  APPEND_RESULT_VAL("total_seconds", total_seconds);
  APPEND_RESULT_VAL("total_mqtt_sent", total_mqtt_sent);
  APPEND_RESULT_VAL("total_mqtt_received", total_mqtt_received);
  APPEND_RESULT_VAL("total_mqtt_connect_attempts", total_mqtt_connect_attempts);
  APPEND_RESULT_VAL("total_mqtt_connects", total_mqtt_connects);

  APPEND_RESULT_VAL("total_wifi_connect_attempts", total_wifi_connect_attempts);
  APPEND_RESULT_VAL("total_wifi_connects", total_wifi_connects);

  APPEND_RESULT_VAL("esp32_getFreeHeap", ESP.getFreeHeap());
  APPEND_RESULT_VAL("esp32_getHeapSize", ESP.getHeapSize() );
  APPEND_RESULT_VAL("esp32_getMinFreeHeap", ESP.getMinFreeHeap() );
  APPEND_RESULT_VAL("wifi_wdt_counter", get_int_param("wifi_wdt_counter") );
  APPEND_RESULT_VAL("current_wifi_wdt_counter", (millis()-t_wifi_wdt)/1000 );

  APPEND_RESULT_VAL("mqtt_wdt_counter", get_int_param("mqtt_wdt_counter") );
  APPEND_RESULT_VAL("current_mqtt_wdt_counter", (millis()-t_mqtt_wdt)/1000 );
  APPEND_RESULT_VAL("0w_counter", get_int_param("0w_counter") );
  APPEND_RESULT_STR("l_reset",get_str_param("l_reset"));
  result.remove(result.length()-1);

  return result+"}";
}

String cmd_status() {
  String result="{";
  APPEND_RESULT_STR("id", vid1);

  result += "\"alias\":\"" + params["alias"] + '"';
  result += ",\"release\":\"" + String(release) + '"';
  result += ",\"interval\":\"" + params["interval"] + '"';
  result += ",\"sampling_period\":\"" + String(sampling_period) + '"';
  result += ",\"samples_per_interval\":\"" + String(samples_per_interval) + '"';
  result += ",\"wifi_state\":\"" + sstate() + '"';
  result += ",\"wifi_status\":\"" + String(WiFi.status()) + '"';
  result += ",\"wifi_selected\":\"" + String(selected_wifi) + '"';
  result += ",\"wifi_ssid\":\"" + String(WiFi.SSID()) + '"';
  result += ",\"wifi_rssi\":\"" + String(WiFi.RSSI()) + '"';
  result += ",\"wifi_rssi_quality\":\"" + String(getRSSIasQuality(WiFi.RSSI())) + '%' + '"';

  result += ",\"wifi_ip\":\"" + WiFi.localIP().toString() + '"';
  result += ",\"wifi_gateway\":\"" + WiFi.gatewayIP().toString() + '"';
  result += ",\"wifi_subnet\":\"" + WiFi.subnetMask().toString() + '"';
  result += ",\"wifi_mac\":\"" + WiFi.macAddress() + '"';

  result += ",\"wifi_channel\":\"" + String(WiFi.channel()) + '"';
  result += ",\"wifi_mode\":\"" + String(WiFi.getMode()) + '"';
  result += ",\"auto_send_serial_data\":\"" + String(auto_send_serial_data) + '"';
  result += ",\"mqtt_state\":\"" + String(client.connected()) + '"';
  result += ",\"mqtt_server\":\"" + params["mqtt_server"] + '"';
  result += ",\"mqtt_port\":\"" + params["mqtt_port"] + '"';
  result += ",\"current_mqtt_topic\":\"" + params["mqtt_topic"] + '"';
#ifdef BLUETOOTH_ENABLED
  result += ",\"bluetooth_mode\":\"" + String(bluetooth_mode) + '"';
#endif
  return result+"}";
}

String cmd_help() {
String result;
  result="Command list:\n";
  result+="  help - this help\n";
  result+="  set <param> <value> - set parameter value\n";
  result+="  get <param> <all> - get parameter value, or all values\n";
  result+="  update <1|2> - update firmware from url_update1 or url_update2\n";
  result+="  serial_data <on|off> - enable/disable serial data sending\n";
  result+="  mqtt_data <on|off> - enable/disable mqtt data sending\n";
  result+="  reset <hard|kw> - normal reset or hard:default values or kw: kw counter init=0\n";
  result+="  status - get status\n";
  result+=" <param> list (for get/set):\n";
  for (int i=0;i<sizeof(valid_params)/sizeof(valid_params[0]);i++) {
    if (i!=0) result +=", ";
    result+= valid_params[i];
  }
  return result;
}

String process_cmd() {
  String result="";
  cmdstr=remove_leading_spaces(cmdstr);  // remove leading spaces
  if (cmdstr.length()==0) return result;
  cmd0=Get_token(cmdstr);
  cmdstr=cmdstr.substring(cmd0.length());
  cmdstr=remove_leading_spaces(cmdstr);
  arg1=Get_token(cmdstr);
  cmdstr=cmdstr.substring(arg1.length());
  cmdstr=remove_leading_spaces(cmdstr);
  arg2=Get_token(cmdstr);
//  cmdstr=cmdstr.substring(arg2.length());
//  Serial.print(cmd0); Serial.print(">>"); Serial.print(arg1); Serial.print(">>"); Serial.print(arg2); Serial.println("<<");

  if (cmd0=="scan") {
    return scanNetworks();
  } else 
  if (cmd0=="set") {
    cmd_set();
  } else 
  if (cmd0=="get") {
    return cmd_get();
  } else 
  if (cmd0=="serial_data") {
     return cmd_serial_data();
  } else
    if (cmd0=="mqtt_data") {
     return cmd_mqtt_data();
  } else
  if (cmd0=="status") {
    return cmd_status();
  } else
  if (cmd0=="update") {
    return cmd_update();
  } else
  if (cmd0=="reset") {
    cmd_reset();
  } else
  if (cmd0=="help") {
    return cmd_help();
  } else
  if (cmd0=="debug") {
    return cmd_debug();
  }


//        esp32_ota_update();
     //ota_https_update(); // it requires certificate
  return result;
}

void check_serial() {
  if (Serial.available() > 0) {
   char recbyte = Serial.read();
   if ((recbyte>=32) && (recbyte<127)) {
    Serial.write(recbyte);
    cmdstr+=recbyte;
   } else if (recbyte=='\n') {
     Serial.println();
     if (cmdstr.length()>0) {
       Serial.println(process_cmd());
     }
     cmdstr.clear();
     Serial.print("VMP>");
   } else if (recbyte==8) {
      if (cmdstr.length()>0) {
        cmdstr.remove(cmdstr.length()-1);
        Serial.print("\b \b");
      }
    } 
  }
}

#ifdef BLUETOOTH_SUPPORT
void check_bluetooth() {
  String tmp_cmdstr;
  if (SerialBT.available() > 0) {
   char recbyte = SerialBT.read();
   SerialBT.write(recbyte);

   if (recbyte=='\n') {
     if (btcmdstr.length()>0) {
       tmp_cmdstr = cmdstr;
       cmdstr=btcmdstr;
       SerialBT.println(process_cmd());
       cmdstr=tmp_cmdstr;
     }
     btcmdstr.clear();
     SerialBT.print("VMP>");
   } else 
    if (recbyte==8) {
      if (btcmdstr.length()>0) {
        btcmdstr.remove(btcmdstr.length()-1);
        SerialBT.print("\b \b");
      }
    } else {
      if (recbyte!='\r') btcmdstr+=recbyte;
    }



  }
}
#endif


void xdisplay() {

  switch (menu) {
    case 0:
      if (show_data_flag) {
        show_data_flag=false;
        show_data(); // 27 ms
        t_menu0=millis();  // reset timer
      }
      break;
    case 1:
      if (show_status_flag) {
        display.clearDisplay();
        display.setCursor (0,0);
        display.print(vid1);
        display.print(" ");
        display.print(release);
        display.setCursor (0,10);
        if (WiFi.isConnected()) {
          display.print("WiFi Connected:");
          display.setCursor (0,20);
          display.print(WiFi.SSID());
          display.setCursor (0,30);
          display.print(WiFi.localIP());
          display.setCursor (0,40);
          int rssi = WiFi.RSSI();
          display.print(rssi);
          display.print(" dBm ");
          int quality = getRSSIasQuality(rssi);
          display.print(quality);
          display.print("%");
        }
        else display.print("WiFi Disconnected");
        display.setCursor (0,50);
        if (client.connected()) display.print(" MQTT Ok");
        else display.print(" MQTT --");
        display.display();
        show_status_flag=false;
      }    
    break;
    case 2:
      if (show_status_flag) {
        display.clearDisplay();
        display.setCursor (0,0);
        display.setTextSize(1);
        display.setTextColor(WHITE);
#ifdef BLUETOOTH_SUPPORT
        display.print("Bluetooth ");
        if (bluetooth_mode) display.print("ON");
        else display.print("OFF");
#endif
        display.setCursor(0,20);
        display.print(vid1);
        display.display();
        show_status_flag=false;
        tkey=millis();
      }
    break;
  }  

}

void check_menus() {
  switch (menu)
  {
    case 0:
      if ((!show_data_flag) && (millis()-t_menu0>=2000)) {
        show_data_flag=true;
      }
      break;

    case 1:
    if (millis()-tkey>=5000)
     {
      menu=0;
      show_data_flag=true;
    }
    break;
    case 2:
    if (millis()-tkey>=10000)
     {
      menu=0;
      show_data_flag=true;
    }
  }


}


bool wifi_begin() {

int tries=3;

  do {
    WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
    WiFi.mode(WIFI_STA); //init wifi mode
    
  printf("Conectando...%d<%s><%s>\n",selected_wifi,params["ssid"].c_str(),params["password"].c_str());

    switch(selected_wifi) {
      case 0:
        if (params["ssid"].length()>0) {
          twscan=millis();
          WiFi.begin(params["ssid"].c_str(), params["password"].c_str());
          total_wifi_connect_attempts++;
          return true;
        }
        break;

      case 1:
        if (params["ssid2"].length()>0) {
          twscan=millis();
          WiFi.begin(params["ssid2"].c_str(), params["password2"].c_str());
          total_wifi_connect_attempts++;
          return true;
        }
        break;

      case 2:
        if (params["ssid3"].length()>0) {
          twscan=millis();
          esp_wifi_sta_wpa2_ent_set_ca_cert((uint8_t *)test_root_ca, strlen(test_root_ca) + 1);
  //esp_wifi_sta_wpa2_ent_set_ca_cert((uint8_t *)test_root_ca, strlen(test_root_ca));
          esp_wifi_sta_wpa2_ent_set_identity((const unsigned char *)params["anon3"].c_str(), params["anon3"].length() );
          esp_wifi_sta_wpa2_ent_set_username((const unsigned char *)params["user3"].c_str(), params["user3"].length() );
          esp_wifi_sta_wpa2_ent_set_password((const unsigned char *)params["password3"].c_str(), params["password3"].length() );
    //      esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config settings to default
    //      esp_wifi_sta_wpa2_ent_enable(&config); //set config settings to enable function
          esp_wifi_sta_wpa2_ent_enable(); //set config settings to enable function
          WiFi.begin(params["ssid3"].c_str()); //connect to wifi
          total_wifi_connect_attempts++;
          return true;
        }
        break;
    }
    if (++selected_wifi>=3) selected_wifi=0;  // cycle
  } while (--tries);
  return false;
}


bool check_wl_conected() {
    if ( (wifi_state!=CONNECTED) && (WiFi.status() == WL_CONNECTED)) {
        wifi_state = CONNECTED;
        wait_mqtt_flag=false;
        total_wifi_connects++;
        return true;
    }
    return false;
}

bool check_ssids() {
  if ( (params["ssid"].length()>0) || (params["ssid2"].length()>0) || (params["ssid3"].length()>0) ) {
    return true;
  }
  return false;
}

void check_wifi() {
  switch(wifi_state) {
    case IDLE:
      if ( (check_ssids)() && (!check_wl_conected()) ){
        connecting_wifi = wifi_begin();
         if (connecting_wifi) 
          wifi_state = CONNECTING;
      }
    break;
    case CONNECTING:
      if (!check_wl_conected()) {
        if (millis()-twscan>=20000) {
          wifi_state = IDLE;
          if (++selected_wifi>=3) selected_wifi=0;  // cycle
        } 
      }
    break;
    case CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        wifi_state = IDLE;
      }
    break;
  }
}


void receivedCallback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
  String old_cmdstr = cmdstr;
  total_mqtt_received++;
  cmdstr="";
  for (int i = 0; i < length; i++) {
    cmdstr+= (char)payload[i];
  }
  pending_mqtt_result = process_cmd();
  pending_mqtt_topic = current_mqtt_topic+vid1+"/result";
  cmdstr = old_cmdstr;
}

void try_mqtt_connection() {
  String topic;
  String wtopic;
  client.setServer(params["mqtt_server"].c_str(), params["mqtt_port"].toInt());
  client.setCallback(receivedCallback);
  current_mqtt_topic=params["mqtt_topic"];
  wtopic = current_mqtt_topic+vid1+"/status";
  if (client.connect(vid1.c_str(), params["mqtt_user"].c_str(), params["mqtt_password"].c_str(),
    wtopic.c_str(), 0, true, "offline")) {
    total_mqtt_connects++;
    wait_mqtt_flag=false;
    client.setBufferSize(1024);
    try_mqtt_flag=true;
    topic = current_mqtt_topic+vid1+"/status";
    client.publish(topic.c_str(), "online");
    total_mqtt_sent++;
    topic = current_mqtt_topic+vid1+"/cmd";
    client.subscribe(topic.c_str());
          //client.subscribe(params["topic"].c_str());
    } else {  //conecction failed
      wait_mqtt_flag=true;
      twmqtt=millis();
  }
}

void check_soft_wdt(String xparam,String debug_param, unsigned long t_ref) {
int max_t_wdt = params[xparam].toInt();
  if (max_t_wdt<0) max_t_wdt=0;
  if ((max_t_wdt) && (millis()-t_ref>=max_t_wdt*1000)) {
    inc_param(debug_param);
    ESP.restart();
  }
}


void check_mqtt() {
    if (client.connected()) {      
      client.loop();
      t_mqtt_wdt = millis();
      if (pending_mqtt_topic.length()>0) {
        client.publish(pending_mqtt_topic.c_str(), pending_mqtt_result.c_str());
        total_mqtt_sent++;
        pending_mqtt_topic="";
      }
    } else { // not connected
    // check t_mqtt_wdt
      check_soft_wdt("max_ts_mqtt_wdt","mqtt_wdt_counter",t_mqtt_wdt);

      if (!wait_mqtt_flag) {  // not waiting for reconnect
        try_mqtt_connection();
      } else { // waiting for reconnect
        if (millis()-twmqtt>=10000) {
          wait_mqtt_flag=false;
        }
      }
    }
}

void check_wifi_wdt() {
  if (!check_ssids()) return;  // No SSID configured
  if (WiFi.isConnected()) {
    if (client.connected()) {
      t_wifi_wdt = millis();    
    }

  } else {  // not wifi, is something wrong ?...
    check_soft_wdt("max_ts_wifi_wdt","wifi_wdt_counter",t_wifi_wdt);
  }
}

void check_seconds() {
  if (millis()-t_seconds>=1000) {
    t_seconds+=1000;
    total_seconds++;
  }
}


void loop() {
  esp_task_wdt_reset();
  check_seconds();
  check_wifi();
  if (WiFi.isConnected()) {
      check_mqtt();
  }
  check_wifi_wdt();
  check_keys();
  check_menus();
  check_measurement();
  check_serial();
#ifdef BLUETOOTH_SUPPORT
  if (bluetooth_mode) check_bluetooth();
#endif
  xdisplay();
}
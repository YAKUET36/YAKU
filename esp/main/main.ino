#include "RMaker.h"
#include "WiFi.h"
#include <SimpleTimer.h>
#include "WiFiProv.h"
#include <wifi_provisioning/manager.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>

//------------------------------------------- Definicion de algunos valores por defecto -------------------------------------------//
#define DEFAULT_RELAY_MODE true
#define DHTTYPE  DHT22   //Definimos el modelo del sensor DHT22
#define DHTPIN    23     // Se define el pin D4 del ESP32 para conectar el sensor DHT22
DHT dht(DHTPIN, DHTTYPE, 23); 

Adafruit_SSD1306 display(128, 64, &Wire, -1);
const int tiempo_12hs = (60 * 12) * 60000;

// Valores del sensor LDR
const float max_value_ldr = 1023;
const float min_value_ldr = 0;

//Pressure
SFE_BMP180 bmp180;
double PresionNivelMar = 1013.25; //presion sobre el nivel del mar en mbar

// BLE Credentils 
const char *service_name = "PROV_12345";
const char *pop = "1234567";

//------------------------------------------- Declaracion de pines -------------------------------------------//

static uint8_t gpio_reset = 0;      //boton de resetear
static uint8_t gpio_ground = 35;  //ADC35 para sensor de humedad suelo
static uint8_t gpio_ldr = 34;       //ADC34 para sensor de luminosidad
static uint8_t relay = 2;  // pin que maneja el relay
bool relay_state = true;
bool wifi_connected = 0;

SimpleTimer Timer;
SimpleTimer Water_bomb_Timer;
SimpleTimer delay_5s;

//------------------------------------------- Declaracion de dispositivos -------------------------------------------//

static TemperatureSensor ground("Tierra");     //Sensor de humedad de tierra
static TemperatureSensor ldr("LDR");              //Sensor de luminosidad
static Switch my_switch("Bomba de Agua", &relay);         //Relay
static TemperatureSensor pressure("Presion");     //Sensor de Presion Atmosferica
static TemperatureSensor temp("Temperatura");     //Sensor de Temperatura
static TemperatureSensor humidity("Humedad");    //Sesnor de Humedad ambiente
static TemperatureSensor altitud("Altitud");    //Sesnor de Humedad ambiente

void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
      printQR(service_name, pop, "softap");
#endif
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf("\nConnected to Wi-Fi!\n");
      wifi_connected = 1;
      delay(500);
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
      }
    case ARDUINO_EVENT_PROV_INIT:
      wifi_prov_mgr_disable_auto_stop(10000);
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("Stopping Provisioning!!!");
      wifi_prov_mgr_stop_provisioning();
      break;
  }
}

//------------------------------------------- Callback del relay -------------------------------------------//

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx) {
  const char *device_name = device->getDeviceName();
  Serial.println(device_name);
  const char *param_name = param->getParamName();

  if (strcmp(device_name, "Relay") == 0)   //detectar relay
  {
    if (strcmp(param_name, "Power") == 0)  //detectar si el relay esta prendido
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      relay_state = val.val.b;
      (relay_state == false) ? digitalWrite(relay, LOW) : digitalWrite(relay, HIGH);
      param->updateAndReport(val);
    }
  }
}

void setup() {
  // Iniciamos el Panel OLED
  #ifdef __DEBUG__
    delay(100);
  #endif
    // Iniciar pantalla OLED en la dirección 0x3C
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  #ifdef __DEBUG__
      Serial.println("No se encuentra la pantalla OLED");
  #endif
      while (true);
  }
  //BEGIN´s
  display.clearDisplay();
  dht.begin();

  if (bmp180.begin())
    Serial.println("BMP180 iniciado");
  else
  {
    Serial.println("Error al iniciar el BMP180");
    while(1);
  }

  Serial.begin(115200);
  // Setear gpios
  digitalWrite(gpio_reset, HIGH);
  pinMode(gpio_reset, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, DEFAULT_RELAY_MODE);

  //------------------------------------------- Declaracion del nodo y relay -------------------------------------------//
  Node my_node;
  my_node = RMaker.initNode("YAKU");

  my_switch.addCb(write_callback);  // Callback del relay

  //------------------------------------------- Añadiendo dispositivos al nodo -------------------------------------------//
  my_node.addDevice(ground);
  my_node.addDevice(ldr);
  my_node.addDevice(pressure);
  my_node.addDevice(humidity);
  my_node.addDevice(temp);
  my_node.addDevice(altitud);
  my_node.addDevice(my_switch);

  //RMaker.enableOTA(OTA_USING_PARAMS); // Es opcional
  RMaker.enableTZService();
  RMaker.enableSchedule();

  Serial.printf("\nStarting ESP-RainMaker\n");
  RMaker.start();

  //Intervalo de tiempo para enviar los datos del sensor
  Timer.setInterval(60000);
  Water_bomb_Timer.setInterval(tiempo_12hs);

  WiFi.onEvent(sysProvEvent);

#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif
}

void loop() {
  if (Timer.isReady() && wifi_connected) { // Chequea si el contador termino
    
    Send_Sensor();
    Regado();
    Serial.println("Sending Sensor's Data");
    Timer.reset(); // Resetar el contador
  };
  // Leer GPIO0 (boton externo para resetear)
  if (digitalRead(gpio_reset) == LOW) {  //mantener boton presionado
    Serial.printf("Reset Button Pressed!\n");
    // manejo de rebote clave
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // Si se presiona por mas de 10 segundos se resetea todo
      Serial.printf("Reset to factory.\n");
      wifi_connected = 0;
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi-Fi.\n");
      wifi_connected = 0;
      // Si se presiona entre 3 y 10 segundos se resetea el wifi
      RMakerWiFiReset(2);
    }
  }
  delay(100);
}

void Send_Sensor() {
  char status;
  double T,P,A;

  status = bmp180.startTemperature(); //Inicio de lectura de temperatura
  if (status != 0){   
    delay(status); //Pausa para que finalice la lectura
    status = bmp180.getTemperature(T); //Obtener la temperatura
    if (status != 0){
      status = bmp180.startPressure(3); //Inicio lectura de presión
      if (status != 0){        
        delay(status); //Pausa para que finalice la lectura        
        status = bmp180.getPressure(P,T); //Obtener la presión
        if (status != 0){                  
          A= bmp180.altitude(P,PresionNivelMar); //Calcular altura
        }      
      }     
    }   
  } 

  //------------------------------------------- Pasa el valor de los dispositivos a porcentaje-------------------------------------------//
  float ground_value = map(analogRead(gpio_ground), 339, 600, 0, 100); 
  float ldr_value = analogRead(gpio_ldr);
  float humidity_value = dht.readHumidity(); 
  float temp_value = dht.readTemperature(); 
  float pressure_value = P;
  float altitude_value = A;

  //------------------------------------------- Envia los valores a la Rainmaker -------------------------------------------//
  ground.updateAndReportParam("Temperature", ground_value);
  ldr.updateAndReportParam("Temperature", ldr_value);
  humidity.updateAndReportParam("Temperature", humidity_value); //No se mapea ya que es un porcentaje
  temp.updateAndReportParam("Temperature", temp_value);
  pressure.updateAndReportParam("Temperature", pressure_value);  
  altitud.updateAndReportParam("Temperature", altitude_value); 


  //------------ Muestra por pantalla el valor del dispositivo tanto en su valor original ------------//
  // Dibujar línea horizontal
  display.drawLine(0, 18, display.width(), 18, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.print("YAKU");
  display.setCursor(0, 19);
  display.setTextSize(1);
  display.print("Ground:");
  display.println(ground_value);
  display.print("LDR:");
  display.println(ldr_value);
  display.print("Temperature:");
  display.println(temp_value);
  display.print("Humidity:");
  display.println(humidity_value);
  display.print("Pressure:");
  display.println(P);
  display.print("Altitude:");
  display.println(altitude_value);

  // Enviar a pantalla
  display.display();
}

void Regado()
{
  //------------------------------------------- Este metodo abre la bomba de agua durante 5s cada 12hs -------------------------------------------//
 if(Water_bomb_Timer.isReady()){
    if(analogRead(gpio_ldr) > 1000 && analogRead(gpio_ground) < 250){
      delay_5s.setInterval(5000);
      digitalWrite(relay, HIGH); relay_state = true;
      Serial.println("REGO");
      if(delay_5s.isReady()){
        digitalWrite(relay, LOW); relay_state = false;
        delay_5s.reset();
      }
    }
    Water_bomb_Timer.reset();
  }
}
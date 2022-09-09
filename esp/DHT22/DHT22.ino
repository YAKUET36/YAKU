#include <DHT.h>           //Cargamos la librería DHT
#define DHTTYPE  DHT22   //Definimos el modelo del sensor DHT22
#define DHTPIN    23     // Se define el pin D4 del ESP32 para conectar el sensor DHT22

DHT dht(DHTPIN, DHTTYPE, 23); 
void setup()
{
  Serial.begin(9600);   //Se inicia la comunicación serial 
  Serial.println("Iniciando...");

  dht.begin(); 
}
void loop()
{
 float h = dht.readHumidity(); //Se lee la humedad y se asigna el valor a "h"
 float t = dht.readTemperature(); //Se lee la temperatura y se asigna el valor a "t"

 //Se imprimen las variables
  Serial.print("Humedad: "); 
  Serial.println(h);
  Serial.print("Temperatura: ");
  Serial.println(t);
  delay(200);
 
}

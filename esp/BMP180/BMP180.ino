#include <SFE_BMP180.h>
#include <Wire.h>

SFE_BMP180 bmp180;

double PresionNivelMar = 1013.25; //presion sobre el nivel del mar en mbar

void setup()
{
  Serial.begin(115200);

  if (bmp180.begin())
    Serial.println("BMP180 iniciado");
  else
  {
    Serial.println("Error al iniciar el BMP180");
    while(1);
  }
}

void loop()
{
  char status;
  double T,P,A;
  
  status = bmp180.startTemperature(); //Inicio de lectura de temperatura
  if (status != 0)
  {   
    delay(status); //Pausa para que finalice la lectura
    status = bmp180.getTemperature(T); //Obtener la temperatura
    if (status != 0)
    {
      status = bmp180.startPressure(3); //Inicio lectura de presión
      if (status != 0)
      {        
        delay(status); //Pausa para que finalice la lectura        
        status = bmp180.getPressure(P,T); //Obtener la presión
        if (status != 0)
        {                  
          Serial.print("Temperatura: ");
          Serial.print(T);
          Serial.print(" *C , ");
          Serial.print("Presion: ");
          Serial.print(P);
          Serial.print(" mb , ");     
          
          A= bmp180.altitude(P,PresionNivelMar); //Calcular altura
          Serial.print("Altitud: ");
          Serial.print(A);
          Serial.println(" m");    
        }      
      }      
    }   
  } 
  delay(1000);
}

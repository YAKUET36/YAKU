const int sensorPin = 16;
const int measureInterval = 2500;
volatile int pulseConter;

// YF-S201
const float factorK = 7.5;

// FS300A
//const float factorK = 5.5;

// FS400A
//const float factorK = 3.5;

float volume = 0;
long t0 = 0;


void ISRCountPulse()
{
   pulseConter++;
}

float GetFrequency()
{
   pulseConter = 0;

   interrupts();
   delay(measureInterval);
   noInterrupts();

   return (float)pulseConter * 1000 / measureInterval;
}

void SumVolume(float dV)
{
   volume += dV / 60 * (millis() - t0) / 1000.0;
   t0 = millis();
}

void setup()
{
   Serial.begin(9600);
   attachInterrupt(digitalPinToInterrupt(sensorPin), ISRCountPulse, RISING);
   t0 = millis();
}

void loop()
{
   // obtener frecuencia en Hz
   float frequency = GetFrequency();

   // calcular caudal L/min
   float flow_Lmin = frequency / factorK;
   SumVolume(flow_Lmin);

   Serial.print(" Caudal: ");
   Serial.print(flow_Lmin, 3);
   Serial.print(" (L/min)\tConsumo:");
   Serial.print(volume, 1);
   Serial.println(" (L)");
}
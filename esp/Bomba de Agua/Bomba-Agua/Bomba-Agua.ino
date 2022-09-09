const int bomba = 9;

void setup()
{
  pinMode(bomba, OUTPUT);  //definir pin como salida
}
 
void loop()
{
  digitalWrite(bomba, HIGH);   // poner el Pin en HIGH
  delay(10000);               // esperar 10 segundos
  digitalWrite(bomba, LOW);    // poner el Pin en LOW
  delay(10000);               // esperar 10 segundos
}

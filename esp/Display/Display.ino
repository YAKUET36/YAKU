#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define ANCHO_PANTALLA 128 // ancho pantalla OLED
#define ALTO_PANTALLA 64 // alto pantalla OLED

// Objeto de la clase Adafruit_SSD1306
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
void setup() {
  #ifdef __DEBUG__

  delay(100);
  Serial.println("Iniciando pantalla OLED");
#endif

  // Iniciar pantalla OLED en la dirección 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
#ifdef __DEBUG__
    Serial.println("No se encuentra la pantalla OLED");
#endif
    while (true);
  }
  
  // put your setup code here, to run once:
  display.clearDisplay();
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);

  display.print("Ground");
  Serial.println("Ground");
  display.display();
}

// Librerías
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// Pines
const int PULSADOR_SUBIR = 2; // Pulsador que aumenta el número de paladas por minuto.
const int PULSADOR_BAJAR = 3; // Pulsador que disminuye el número de paladas por minuto.
const int LED_VERDE      = 4; // LED de color azul.
const int LED_ROJO       = 5; // LED de color verde.
const int BUZZER         = 6; // Buzzer.
// const int LCD_BACKLIGHT  = 9; // Backlight del LCD.

// Constantes
const int MAXIMO_PALADAS     = 150;  // Número máximo de paladas por minuto.
const int MINIMO_PALADAS     = 30;   // Número mínimo de paladas por minuto.
const int FACTOR_PALADAS     = 5;    // Valor en el que aumenta/disminuye el número de paladas.
const int DURACION_DESTELLO  = 200;  // Número de milisegundos que dura el destello de los LEDs.
const int VOLUMEN_BUZZER     = 210;  // Volumen con el que suena el buzzer. Valor con el que suena más alto: 210.
const int BRILLO_BACKLIGHT   = 120;  // Brillo del backlight del LCD.
const int TIEMPO_INACTIVIDAD = 5000; // Milisegundos que deben pasar para estar en inactividad.

// Variables
unsigned long intervalo        = 1200;      // Milisegundos que debe haber entre las paladas.
unsigned int  paladas          = 50;        // Número de paladas por minuto incial.
unsigned long ultima_pulsacion = millis();  // Milisegundos que pasaron desde que se inició el programa hasta que se tocó un pulsador.
unsigned long ultimo_destello  = millis();  // Milisegundos que pasaron desde que se inició el programa hasta que se hizo alguna palada.
boolean backlight_on           = true;      // Indica si el display está prendido (1) o apagado (0).
boolean buzzer_on              = false;     // Indica si el buzzer está prendido (1) o apagado (0).
int led_on                     = LED_ROJO;  // Indica qué LED está prendido.
int led_off                    = LED_VERDE; // Indica qué LED está apagado.

// LCD
// pin 13 - Serial clock out (SCLK)
// pin 11 - Serial data out (DIN)
// pin 8  - Data/Command select (D/C)
// pin 12 - LCD chip select (CS)
// pin 10 - LCD reset (RST)
Adafruit_PCD8544 lcd = Adafruit_PCD8544(13, 11, 8, 12, 10);


/*
 * Inicializa los pines, el serial y el lcd.
 */
void setup() {
  // Define pines de entrada y salida
  pinMode(PULSADOR_SUBIR, INPUT);
  pinMode(PULSADOR_BAJAR, INPUT);
  // pinMode(LCD_BACKLIGHT,  OUTPUT);
  pinMode(LED_VERDE,      OUTPUT);
  pinMode(LED_ROJO,       OUTPUT);
  pinMode(BUZZER,         OUTPUT);
  
  // Inicializa los LEDs
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO,  HIGH);

  // Enciende el backlight del LCD
  // analogWrite(LCD_BACKLIGHT, BRILLO_BACKLIGHT);
    
  // Inicializa el serial
  Serial.begin(9600);
  Serial.println("Serial funcionando...");

  // Inicializa el lcd
  lcd.begin();
  lcd.setContrast(50); // Establece el contraste del LCD.
  lcd.clearDisplay();  // Limpia la pantalla y el buffer.

  // Muestra el mensaje de bienvenida
  lcd.setCursor(0,12);
  lcd.setFont(&FreeSansBold9pt7b);
  lcd.println("Hola");
  lcd.println("mostro!");
  lcd.display();
  delay(3000);
  lcd.clearDisplay();

  // Muestra los datos iniciales en el LCD
  actualizarLCD();
} // Fin de setup()


/*
 * Actualiza los datos que se muestran el LCD y el serial.
 */
void actualizarLCD() {
  lcd.clearDisplay();
  
  lcd.setFont();
  lcd.setCursor(0, 0);
  lcd.print("PPM");
  
  lcd.setFont(&FreeSansBold24pt7b);
  lcd.setCursor(0, 45);
  lcd.print(paladas);
  
  lcd.display();
  
  Serial.print("Paladas por minuto: ");
  Serial.println(paladas, DEC);
} // Fin de actualizarLCD()


/*
 * Actualiza el intervalo de tiempo que debe haber entre las paladas.
 */
void actualizarRitmo() {
  intervalo = (long)(60 / (float)paladas * 1000);
  actualizarLCD();
} // Fin de actualizarRitmo()


/*
 * Aumenta el número de paladas por minuto.
 */
void aumentarPaladas() {
  if (paladas == MAXIMO_PALADAS)
    paladas = MINIMO_PALADAS;
  else
    paladas += FACTOR_PALADAS;  

  actualizarRitmo();

  // Detiene el programa hasta que no se suelte el pulsador para evitar ingresar de nuevo a la función
  while (digitalRead(PULSADOR_SUBIR) == HIGH && digitalRead(PULSADOR_BAJAR) == LOW)
    delay(100);
} // Fin de aumentarPaladas()


/*
 * Disminuye el número de paladas por minuto.
 */
void disminuirPaladas() {
  if (paladas == MINIMO_PALADAS)
    paladas = MAXIMO_PALADAS;
  else
    paladas -= FACTOR_PALADAS;
  
  actualizarRitmo();

  // Detiene el programa hasta que no se suelte el pulsador para evitar ingresar de nuevo a la función
  while (digitalRead(PULSADOR_BAJAR) == HIGH && digitalRead(PULSADOR_SUBIR) == LOW)
    delay(100);
} // Fin de disminuirPaladas()


/*
 * Enciende y apaga los LEDs de acuerdo al intervalo de tiempo entre paladas.
 */
void destellar() {
  int led;

  // Apaga el LED y el buzzer encendidos una vez pasada la duración del destello.
  if ((millis() - ultimo_destello) > DURACION_DESTELLO && digitalRead(led_on) == HIGH) {
    digitalWrite(led_on, LOW);
    if (buzzer_on)
      analogWrite(BUZZER, 0);
  }

  // Enciende el LED correspondiente y el buzzer una vez pasado el intervalo entre paladas.
  if ((millis() - ultimo_destello) >= intervalo) {
    digitalWrite(led_off, HIGH);

    if (buzzer_on)
      analogWrite(BUZZER, VOLUMEN_BUZZER);

    // Intercambia el LED encendido por el apagado.
    led     = led_on;
    led_on  = led_off;
    led_off = led;

    ultimo_destello = millis();
  }

  // Apaga el backlight del LCD una vez pasado el tiempo de inactividad.
  if ((millis() - ultima_pulsacion) >= TIEMPO_INACTIVIDAD && backlight_on) {
    // analogWrite(LCD_BACKLIGHT, BRILLO_BACKLIGHT);
    // backlight_on = false;
  }
} // Fin de destellar()


/*
 * Detecta si un pulsador es presionado para aumentar/disminuir el número de
 * paladas por minuto.
 */
void detectarPulsadores() {
  boolean subir = digitalRead(PULSADOR_SUBIR) == HIGH && digitalRead(PULSADOR_BAJAR) == LOW;
  boolean bajar = digitalRead(PULSADOR_SUBIR) == LOW  && digitalRead(PULSADOR_BAJAR) == HIGH;
  boolean ambos = digitalRead(PULSADOR_SUBIR) == HIGH && digitalRead(PULSADOR_BAJAR) == HIGH;

  if (subir) {
    delay(100);

    if (subir) {
      aumentarPaladas();
      ultima_pulsacion = millis();
      // analogWrite(LCD_BACKLIGHT, BRILLO_BACKLIGHT);
      // backlight_on = true;
    }    
  }

  if (bajar) {
    delay(100);

    if (bajar) {
      disminuirPaladas();      
      ultima_pulsacion = millis();
      // analogWrite(LCD_BACKLIGHT, BRILLO_BACKLIGHT);
      // backlight_on = true;
    }
  }

  if (ambos) {
    buzzer_on = !buzzer_on;
    
    if (buzzer_on)
      analogWrite(BUZZER, VOLUMEN_BUZZER);
    else
      analogWrite(BUZZER, 0);

    actualizarLCD();

    Serial.print("Buzzer: ");
    Serial.println(buzzer_on, DEC);

    while (digitalRead(PULSADOR_SUBIR) == HIGH && digitalRead(PULSADOR_BAJAR) == HIGH)
      delay(100);
  }
} // Fin de detectarPulsadores()


/* 
 * Bucle que se repite infinitamente. Enciende de forma intermitente los LEDs y
 * detecta si algún pulsador fue presionado.
 */
void loop() {  
  destellar();
  detectarPulsadores();
} // Fin de loop()

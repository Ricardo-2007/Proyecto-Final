#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// Definiciones de la pantalla OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pines para el potenciómetro y los pulsadores
const int potPin = A0;        // Potenciómetro conectado al pin analógico A0
const int nextButtonPin = 2;  // Pulsador para la siguiente pista conectado al pin digital 2
const int prevButtonPin = 3;  // Pulsador para la pista anterior conectado al pin digital 3

// Pines para el DFPlayer Mini
SoftwareSerial mySerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// Variables
int currentTrack = 1;  // Número de pista actual
int totalTracks = 10;  // Total de pistas almacenadas
int volume = 15;       // Volumen inicial (0 a 30)

// Pines para la tarjeta SD
const int chipSelect = 10;

void setup() {
  // Inicialización de la comunicación serial
  Serial.begin(9600);
  mySerial.begin(9600);

  // Inicializar la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se encuentra la pantalla OLED"));
    for (;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Inicializar DFPlayer Mini
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println(F("DFPlayer no conectado"));
    for (;;);
  }
  myDFPlayer.volume(volume);

  // Mostrar pista inicial en OLED
  mostrarPista(currentTrack);

  // Inicializar la tarjeta SD
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Error en la tarjeta SD"));
    for (;;);
  }
  
  // Reproducir la primera pista
  myDFPlayer.play(currentTrack);
  
  // Configurar los pines de los pulsadores como entradas
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(prevButtonPin, INPUT_PULLUP);
}

void loop() {
  // Leer el estado de los pulsadores
  if (debounce(nextButtonPin)) {
    siguientePista();
  }

  if (debounce(prevButtonPin)) {
    pistaAnterior();
  }

  // Leer el valor del potenciómetro (0 a 1023)
  int potValue = analogRead(potPin);
  
  // Controlar el volumen con el potenciómetro
  int newVolume = map(potValue, 0, 1023, 0, 30);
  if (newVolume != volume) {
    volume = newVolume;
    myDFPlayer.volume(volume);
    mostrarPista(currentTrack);  // Actualizar la pantalla OLED para mostrar el nuevo volumen
  }

  delay(100);  // Pequeño retraso para evitar rebotes de los botones
}

// Función para cambiar a la siguiente pista
void siguientePista() {
  currentTrack++;
  if (currentTrack > totalTracks) currentTrack = 1;  // Reiniciar a la primera pista si se excede
  myDFPlayer.play(currentTrack);
  mostrarPista(currentTrack);  // Mostrar la nueva pista en la pantalla OLED
}

// Función para volver a la pista anterior
void pistaAnterior() {
  currentTrack--;
  if (currentTrack < 1) currentTrack = totalTracks;  // Reiniciar a la última pista si se va por debajo de 1
  myDFPlayer.play(currentTrack);
  mostrarPista(currentTrack);  // Mostrar la nueva pista en la pantalla OLED
}

// Función para mostrar la pista y el volumen en la pantalla OLED
void mostrarPista(int pista) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Reproduciendo pista: ");
  display.println(pista);
  display.setCursor(0, 16);
  display.print("Volumen: ");
  display.println(volume);
  display.display();
}

// Función de debounce para los pulsadores
bool debounce(int buttonPin) {
  static unsigned long lastDebounceTime = 0;
  static int lastButtonState = HIGH;
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
      lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > 50) {
      if (reading != lastButtonState) {
          lastButtonState = reading;
          return reading == LOW; // Devuelve true si el botón está presionado
      }
  }

  return false; // No se detectó un cambio
}

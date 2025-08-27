// === Pines para ESP32 WROOM-32 (30 pines) ===
const int BUTTON_PIN         = 23; // Botón físico
const int LED_STATUS_PIN     = 2;  // LED integrado del ESP32
const int PLUMA_ENTRADA_PIN  = 18; // Pluma de ENTRADA
const int PLUMA_SALIDA_PIN   = 19; // Pluma de SALIDA
const int LED_ENTRADA_PIN    = 21; // LED para pluma ENTRADA
const int LED_SALIDA_PIN     = 22; // LED para pluma SALIDA

// Variables de debounce
int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Variables auxiliares
bool plumaEntradaAbierta = false;
bool plumaSalidaAbierta  = false;
unsigned long tiempoAperturaEntrada = 0;
unsigned long tiempoAperturaSalida  = 0;

const unsigned long DURACION_PLUMA_ENTRADA = 200; // 15s
const unsigned long DURACION_PLUMA_SALIDA  = 10000; // 10s

// Serial
String inputString = "";
boolean stringComplete = false;

void setup() {
  // Configurar pines de salida primero (estado seguro)
  pinMode(PLUMA_ENTRADA_PIN, OUTPUT);
  pinMode(PLUMA_SALIDA_PIN, OUTPUT);
  pinMode(LED_ENTRADA_PIN, OUTPUT);
  pinMode(LED_SALIDA_PIN, OUTPUT);

  // Estado inicial seguro: plumas abajo
  digitalWrite(PLUMA_ENTRADA_PIN, HIGH); // HIGH = circuito abierto → pluma cerrada
  digitalWrite(PLUMA_SALIDA_PIN, HIGH);
  digitalWrite(LED_ENTRADA_PIN, LOW);
  digitalWrite(LED_SALIDA_PIN, LOW);

  // Botón y LED de status
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_STATUS_PIN, OUTPUT);

  Serial.begin(115200);
  inputString.reserve(200);

  // Parpadeo inicial
  digitalWrite(LED_STATUS_PIN, HIGH);
  delay(300);
  digitalWrite(LED_STATUS_PIN, LOW);

  Serial.println("ESP32 Parking System Ready");
}

void loop() {
  processSerialCommands();

  // Leer botón con debounce
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      bool buttonPressed = (buttonState == LOW); // pullup → LOW cuando presionado

      if (buttonPressed && !plumaEntradaAbierta) {
        Serial.println("BUTTON_PRESSED");

        openEntradaGate();
      }
    }
  }

  // Cierre automático
  if (plumaEntradaAbierta && millis() - tiempoAperturaEntrada >= DURACION_PLUMA_ENTRADA) {
    closeEntradaGate();
  }

  if (plumaSalidaAbierta && millis() - tiempoAperturaSalida >= DURACION_PLUMA_SALIDA) {
    closeSalidaGate();
  }

  lastButtonState = reading;
  delay(20);
}

// === FUNCIONES DE COMANDOS SERIALES ===
void processSerialCommands() {
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      if (inputString.length() > 0) stringComplete = true;
    } else inputString += inChar;
  }

  if (stringComplete) {
    inputString.trim();
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}

void processCommand(String command) {
  if (command.equalsIgnoreCase("OPEN_GATE")) openSalidaGate();
  else if (command.equalsIgnoreCase("CLOSE_GATE")) closeSalidaGate();
  else if (command.equalsIgnoreCase("OPEN_ENTRY_GATE")) openEntradaGate();
  else if (command.equalsIgnoreCase("CLOSE_ENTRY_GATE")) closeEntradaGate();
  else if (command.equalsIgnoreCase("STATUS")) sendStatus();
  else Serial.println("Comando no reconocido: " + command);
}

// === ACCIONES DE GATES ===
void openSalidaGate() {
  if (!plumaSalidaAbierta) {
    digitalWrite(PLUMA_SALIDA_PIN, LOW); // Cierra circuito → pluma arriba
    digitalWrite(LED_SALIDA_PIN, HIGH);
    plumaSalidaAbierta = true;
    tiempoAperturaSalida = millis();
    Serial.println("Pluma SALIDA ABIERTA");
  }
}

void closeSalidaGate() {
  if (plumaSalidaAbierta) {
    digitalWrite(PLUMA_SALIDA_PIN, HIGH); // Abre circuito → pluma abajo
    digitalWrite(LED_SALIDA_PIN, LOW);
    plumaSalidaAbierta = false;
    Serial.println("Pluma SALIDA CERRADA");
  }
}

void openEntradaGate() {
  if (!plumaEntradaAbierta) {
    digitalWrite(PLUMA_ENTRADA_PIN, LOW); // Cierra circuito → pluma arriba
    digitalWrite(LED_ENTRADA_PIN, HIGH);
    plumaEntradaAbierta = true;
    tiempoAperturaEntrada = millis();
    Serial.println("Pluma ENTRADA ABIERTA");
  }
}

void closeEntradaGate() {
  if (plumaEntradaAbierta) {
    digitalWrite(PLUMA_ENTRADA_PIN, HIGH); // Abre circuito → pluma abajo
    digitalWrite(LED_ENTRADA_PIN, LOW);
    plumaEntradaAbierta = false;
    Serial.println("Pluma ENTRADA CERRADA");
  }
}

void sendStatus() {
  Serial.println("=== ESTADO DEL SISTEMA ===");
  Serial.print("Pluma ENTRADA: ");
  Serial.println(plumaEntradaAbierta ? "ABIERTA" : "CERRADA");
  Serial.print("Pluma SALIDA: ");
  Serial.println(plumaSalidaAbierta ? "ABIERTA" : "CERRADA");
  Serial.print("Botón: ");
  Serial.println(buttonState == LOW ? "PRESIONADO" : "LIBERADO");
  Serial.println("==========================");
}

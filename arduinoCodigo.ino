/*
 * Sketch Arduino para Sistema de Parking Completo
 * 
 * ENTRADA (Doble control):
 * - Botón físico en pin 2 → Controla pluma de entrada (pin 8)
 * - Comandos seriales → También controla pluma de entrada
 * - Cuando se presiona botón, abre pluma por 15 seg automáticamente
 * - Comandos: "OPEN_ENTRY_GATE" y "CLOSE_ENTRY_GATE"
 * 
 * SALIDA:  
 * - Solo comandos seriales → Controla pluma de salida (pin 9)
 * - Comandos: "OPEN_GATE" y "CLOSE_GATE"
 * 
 * COMANDOS DISPONIBLES:
 * - "OPEN_ENTRY_GATE": Abre pluma entrada
 * - "CLOSE_ENTRY_GATE": Cierra pluma entrada
 * - "OPEN_GATE": Abre pluma salida
 * - "CLOSE_GATE": Cierra pluma salida
 * - "STATUS": Estado del sistema
 * 
 * CONEXIONES:
 * - Botón: Pin 2 y GND
 * - Pluma entrada: Pin 8 
 * - Pluma salida: Pin 9
 */

const int BUTTON_PIN = 2;        // Pin donde está conectado el botón
const int LED_PIN = 13;          // LED integrado para indicar estado
const int PLUMA_ENTRADA_PIN = 8; // Pin para controlar la pluma de ENTRADA
const int PLUMA_SALIDA_PIN = 9;  // Pin para controlar la pluma de SALIDA

// Variables para debounce
int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // 50ms de debounce

// Variable para evitar múltiples mensajes de inicio
bool initialized = false;

// Variable para detectar el tipo de botón
bool buttonTypeDetected = false;
bool buttonConnectsToGND = true; // true = conecta a GND, false = conecta a VCC

// Variables para controlar el estado de la pluma de ENTRADA
bool plumaEntradaAbierta = false;
unsigned long tiempoAperturaEntrada = 0;
const unsigned long DURACION_PLUMA = 15000; // 15 segundos para que baje

// Variables para controlar el estado de la pluma de SALIDA
bool plumaSalidaAbierta = false;
unsigned long tiempoAperturaSalida = 0;
const unsigned long DURACION_PLUMA_SALIDA = 10000; // 10 segundos para pluma salida

// Variables para procesar comandos seriales
String inputString = "";
boolean stringComplete = false;

void setup() {
  // Configurar pines
  pinMode(BUTTON_PIN, INPUT_PULLUP);      // Botón con pull-up interno
  pinMode(LED_PIN, OUTPUT);
  pinMode(PLUMA_ENTRADA_PIN, OUTPUT);     // Pin de la pluma de ENTRADA
  pinMode(PLUMA_SALIDA_PIN, OUTPUT);      // Pin de la pluma de SALIDA
  
  // Inicializar ambas plumas cerradas (HIGH = cables separados)
  digitalWrite(PLUMA_ENTRADA_PIN, HIGH);  // Pluma entrada cerrada
  digitalWrite(PLUMA_SALIDA_PIN, HIGH);   // Pluma salida cerrada
  
  // Inicializar comunicación serial
  Serial.begin(9600);
  
  // Reservar espacio para el string de comandos
  inputString.reserve(200);
  
  // Esperar a que el serial esté listo
  delay(1000);
  
  // Mensaje de inicio (solo una vez)
  if (!initialized) {
    Serial.println("Arduino Parking System Ready");
    Serial.println("ENTRADA: Boton + Serial -> Pluma Pin 8");
    Serial.println("SALIDA: Solo Serial -> Pluma Pin 9");
    Serial.println("Comandos disponibles:");
    Serial.println("- OPEN_ENTRY_GATE / CLOSE_ENTRY_GATE");
    Serial.println("- OPEN_GATE / CLOSE_GATE");
    Serial.println("- STATUS");
    initialized = true;
  }
   
  // Indicador visual de que está listo
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
   
  // Detectar tipo de botón
  int initialReading = digitalRead(BUTTON_PIN);
  Serial.print("Estado inicial del botón: ");
  Serial.println(initialReading == HIGH ? "NO PRESIONADO" : "PRESIONADO");
   
  // Si está presionado al inicio, asumir que conecta a GND
  if (initialReading == LOW) {
    buttonConnectsToGND = true;
    Serial.println("Detectado: Botón conecta a GND (normal)");
  } else {
    buttonConnectsToGND = false;
    Serial.println("Detectado: Botón conecta a VCC (invertido)");
  }
   
  buttonTypeDetected = true;

  Serial.println("Sistema inicializado:");
  Serial.println("- Pluma ENTRADA: CERRADA (Pin 8)");
  Serial.println("- Pluma SALIDA: CERRADA (Pin 9)");
}
 
void loop() {
  // PASO 1: Procesar comandos seriales para pluma de SALIDA
  processSerialCommands();
  
  // PASO 2: Leer el estado del botón para pluma de ENTRADA
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      bool buttonPressed = false;
      if (buttonConnectsToGND) {
        buttonPressed = (buttonState == LOW);
      } else {
        buttonPressed = (buttonState == HIGH);
      }

      if (buttonPressed && !plumaEntradaAbierta) {
        Serial.println("BUTTON_PRESSED");
        digitalWrite(PLUMA_ENTRADA_PIN, LOW);
        Serial.println("PLUMA ENTRADA ACTIVADA - Abriendo...");
        plumaEntradaAbierta = true;
        tiempoAperturaEntrada = millis();
      }

      if (!buttonPressed && !plumaEntradaAbierta && !plumaSalidaAbierta) {
        Serial.println("BUTTON_RELEASED");
      }
    }
  }

  // --- CONTROL DE TIEMPO DE LA PLUMA DE ENTRADA ---
  if (plumaEntradaAbierta) {
    if (millis() - tiempoAperturaEntrada >= DURACION_PLUMA) {
      digitalWrite(PLUMA_ENTRADA_PIN, HIGH);
      Serial.println("PLUMA ENTRADA DESACTIVADA - Cerrando...");
      plumaEntradaAbierta = false;
    }
  }
  
  // --- CONTROL DE TIEMPO DE LA PLUMA DE SALIDA ---
  if (plumaSalidaAbierta) {
    if (millis() - tiempoAperturaSalida >= DURACION_PLUMA_SALIDA) {
      digitalWrite(PLUMA_SALIDA_PIN, HIGH);
      Serial.println("PLUMA SALIDA DESACTIVADA - Cerrando...");
      plumaSalidaAbierta = false;
    }
  }
 
  lastButtonState = reading;
  delay(50);
}

// === FUNCIONES PARA COMANDOS SERIALES ===
void processSerialCommands() {
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }

  if (stringComplete) {
    inputString.trim();
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}

void processCommand(String command) {
  Serial.print("Comando recibido: ");
  Serial.println(command);
  
  if (command.equals("OPEN_GATE")) {
    openSalidaGate();
  } else if (command.equals("CLOSE_GATE")) {
    closeSalidaGate();
  } else if (command.equals("OPEN_ENTRY_GATE")) {
    openEntradaGate();
  } else if (command.equals("CLOSE_ENTRY_GATE")) {
    closeEntradaGate();
  } else if (command.equals("STATUS")) {
    sendStatus();
  } else {
    Serial.println("Comando no reconocido. Comandos disponibles: OPEN_GATE, CLOSE_GATE, OPEN_ENTRY_GATE, CLOSE_ENTRY_GATE, STATUS");
  }
}

void openSalidaGate() {
  if (!plumaSalidaAbierta) {
    Serial.println("Abriendo pluma de SALIDA...");
    digitalWrite(PLUMA_SALIDA_PIN, LOW);
    plumaSalidaAbierta = true;
    tiempoAperturaSalida = millis();
    Serial.println("GATE_OPENED - Pluma salida activada");
  } else {
    Serial.println("La pluma de salida ya está abierta");
  }
}

void closeSalidaGate() {
  if (plumaSalidaAbierta) {
    Serial.println("Cerrando pluma de SALIDA...");
    digitalWrite(PLUMA_SALIDA_PIN, HIGH);
    plumaSalidaAbierta = false;
    Serial.println("GATE_CLOSED - Pluma salida desactivada");
  } else {
    Serial.println("La pluma de salida ya está cerrada");
  }
}

void openEntradaGate() {
  if (!plumaEntradaAbierta) {
    Serial.println("Abriendo pluma de ENTRADA por comando serial...");
    digitalWrite(PLUMA_ENTRADA_PIN, LOW);
    plumaEntradaAbierta = true;
    tiempoAperturaEntrada = millis();
    Serial.println("ENTRY_GATE_OPENED - Pluma entrada activada por comando");
  } else {
    Serial.println("La pluma de entrada ya está abierta");
  }
}

void closeEntradaGate() {
  if (plumaEntradaAbierta) {
    Serial.println("Cerrando pluma de ENTRADA por comando serial...");
    digitalWrite(PLUMA_ENTRADA_PIN, HIGH);
    plumaEntradaAbierta = false;
    Serial.println("ENTRY_GATE_CLOSED - Pluma entrada desactivada por comando");
  } else {
    Serial.println("La pluma de entrada ya está cerrada");
  }
}

void sendStatus() {
  Serial.println("=== ESTADO DEL SISTEMA ===");
  Serial.print("Pluma ENTRADA (Pin 8): ");
  Serial.println(plumaEntradaAbierta ? "ABIERTA" : "CERRADA");
  Serial.print("Pluma SALIDA (Pin 9): ");
  Serial.println(plumaSalidaAbierta ? "ABIERTA" : "CERRADA");
  Serial.print("Botón: ");
  Serial.println(buttonState == LOW ? "PRESIONADO" : "LIBERADO");
  Serial.println("========================");
}

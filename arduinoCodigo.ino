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
 * - Pluma entrada: Pin 8 (LOW=cerrada, HIGH=abierta)
 * - Pluma salida: Pin 9 (LOW=cerrada, HIGH=abierta)
 * - LED entrada: Pin 10 (HIGH=encendido cuando pluma abierta)
 * - LED salida: Pin 11 (HIGH=encendido cuando pluma abierta)
 * - LED status: Pin 13 (LED integrado para indicaciones)
 */

 const int BUTTON_PIN = 2;        // Pin donde está conectado el botón
 const int LED_STATUS_PIN = 13;   // LED integrado para status general
 const int PLUMA_ENTRADA_PIN = 8; // Pin para controlar la pluma de ENTRADA
 const int PLUMA_SALIDA_PIN = 9;  // Pin para controlar la pluma de SALIDA
 
 // Pines para LEDs individuales
 const int LED_ENTRADA_PIN = 10;  // LED para indicar estado de pluma ENTRADA
 const int LED_SALIDA_PIN = 11;   // LED para indicar estado de pluma SALIDA
 
 // Variables para debounce
 int lastButtonState = HIGH;
 int buttonState = HIGH;
 unsigned long lastDebounceTime = 0;
 unsigned long debounceDelay = 50;  // 50ms de debounce
 
 // Variables auxiliares
 bool initialized = false;
 bool buttonTypeDetected = false;
 bool buttonConnectsToGND = true; // true = conecta a GND, false = conecta a VCC
 
 // Estado de la pluma de ENTRADA
 bool plumaEntradaAbierta = false;
 unsigned long tiempoAperturaEntrada = 0;
 const unsigned long DURACION_PLUMA = 15000; // 15 segundos para que baje
 
 // Estado de la pluma de SALIDA
 bool plumaSalidaAbierta = false;
 unsigned long tiempoAperturaSalida = 0;
 const unsigned long DURACION_PLUMA_SALIDA = 10000; // 10 segundos para pluma salida
 
 // Procesamiento de comandos seriales
 String inputString = "";
 boolean stringComplete = false;
 
 void setup() {
   pinMode(BUTTON_PIN, INPUT_PULLUP);
   pinMode(LED_STATUS_PIN, OUTPUT);
   pinMode(PLUMA_ENTRADA_PIN, OUTPUT);
   pinMode(PLUMA_SALIDA_PIN, OUTPUT);
   pinMode(LED_ENTRADA_PIN, OUTPUT);
   pinMode(LED_SALIDA_PIN, OUTPUT);
 
   // Inicializar plumas y LEDs apagados
   digitalWrite(PLUMA_ENTRADA_PIN, LOW);
   digitalWrite(PLUMA_SALIDA_PIN, LOW);
   digitalWrite(LED_ENTRADA_PIN, LOW);
   digitalWrite(LED_SALIDA_PIN, LOW);
 
   Serial.begin(9600);
   inputString.reserve(200);
 
   delay(1000);
 
     if (!initialized) {
    Serial.println("Arduino Parking System Ready");
    initialized = true;
  }
 
   // Parpadeo de status
   digitalWrite(LED_STATUS_PIN, HIGH);
   delay(500);
   digitalWrite(LED_STATUS_PIN, LOW);
 
      // Detectar tipo de botón
   int initialReading = digitalRead(BUTTON_PIN);
   if (initialReading == LOW) {
     buttonConnectsToGND = true;
   } else {
     buttonConnectsToGND = false;
   }
   buttonTypeDetected = true;
 }
 
 void loop() {
   // Procesar comandos seriales
   processSerialCommands();
 
   // Leer botón con debounce
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
 
       // === MANTENIDO DE TU LÓGICA ORIGINAL ===
       if (buttonPressed && !plumaEntradaAbierta) {
         Serial.println("BUTTON_PRESSED");
 
         digitalWrite(PLUMA_ENTRADA_PIN, HIGH);
         digitalWrite(LED_ENTRADA_PIN, HIGH);
 
         Serial.println("PLUMA ENTRADA ACTIVADA - Abriendo...");
 
         plumaEntradaAbierta = true;
         tiempoAperturaEntrada = millis();
       }
 
       // No necesitamos mensaje cuando se libera el botón
     }
   }
 
   // Cierre automático de pluma entrada
   if (plumaEntradaAbierta && millis() - tiempoAperturaEntrada >= DURACION_PLUMA) {
     digitalWrite(PLUMA_ENTRADA_PIN, LOW);
     digitalWrite(LED_ENTRADA_PIN, LOW);
     plumaEntradaAbierta = false;
   }
 
   // Cierre automático de pluma salida
   if (plumaSalidaAbierta && millis() - tiempoAperturaSalida >= DURACION_PLUMA_SALIDA) {
     digitalWrite(PLUMA_SALIDA_PIN, LOW);
     digitalWrite(LED_SALIDA_PIN, LOW);
     plumaSalidaAbierta = false;
   }
 
   lastButtonState = reading;
   delay(50);
 }
 
 // === FUNCIONES PARA COMANDOS SERIALES ===
 void processSerialCommands() {
   while (Serial.available() > 0) {
     char inChar = (char)Serial.read();
     if (inChar == '\n' || inChar == '\r') {
       if (inputString.length() > 0) {
         stringComplete = true;
       }
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
    // Solo log para comandos importantes
    
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
    }
    // Comando no reconocido - no necesitamos log
  }
  
  // === ACCIONES DE GATES ===
  void openSalidaGate() {
    if (!plumaSalidaAbierta) {
      digitalWrite(PLUMA_SALIDA_PIN, HIGH);
      digitalWrite(LED_SALIDA_PIN, HIGH);
      plumaSalidaAbierta = true;
      tiempoAperturaSalida = millis();
    }
  }
  
  void closeSalidaGate() {
    if (plumaSalidaAbierta) {
      digitalWrite(PLUMA_SALIDA_PIN, LOW);
      digitalWrite(LED_SALIDA_PIN, LOW);
      plumaSalidaAbierta = false;
    }
  }
  
  void openEntradaGate() {
    if (!plumaEntradaAbierta) {
      digitalWrite(PLUMA_ENTRADA_PIN, HIGH);
      digitalWrite(LED_ENTRADA_PIN, HIGH);
      plumaEntradaAbierta = true;
      tiempoAperturaEntrada = millis();
    }
  }
  
  void closeEntradaGate() {
    if (plumaEntradaAbierta) {
      digitalWrite(PLUMA_ENTRADA_PIN, LOW);
      digitalWrite(LED_ENTRADA_PIN, LOW);
      plumaEntradaAbierta = false;
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
 
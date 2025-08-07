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
 * - LED RGB: Pines 10, 11, 12
 */

const int BUTTON_PIN = 2;        // Pin donde está conectado el botón
const int LED_PIN = 13;          // LED integrado para indicar estado
const int PLUMA_ENTRADA_PIN = 8; // Pin para controlar la pluma de ENTRADA
const int PLUMA_SALIDA_PIN = 9;  // Pin para controlar la pluma de SALIDA

// Pines para el LED RGB
const int RED_PIN = 10;    // Ajusta estos pines según tu conexión
const int GREEN_PIN = 11;
const int BLUE_PIN = 12;
 
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
const unsigned long DURACION_VERDE = 10000; // 10 segundos LED verde

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
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
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

  // Inicialmente, LED en rojo (ambas plumas cerradas)
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
  
  Serial.println("Sistema inicializado:");
  Serial.println("- Pluma ENTRADA: CERRADA (Pin 8)");
  Serial.println("- Pluma SALIDA: CERRADA (Pin 9)");
 }
 
 void loop() {
  // PASO 1: Procesar comandos seriales para pluma de SALIDA
  processSerialCommands();
  
  // PASO 2: Leer el estado del botón para pluma de ENTRADA
  int reading = digitalRead(BUTTON_PIN);
 
   // Si el estado cambió, resetear el timer de debounce
   if (reading != lastButtonState) {
     lastDebounceTime = millis();
   }
 
   // Si han pasado suficientes ms desde el último cambio
   if ((millis() - lastDebounceTime) > debounceDelay) {
     // Si el estado del botón cambió
     if (reading != buttonState) {
       buttonState = reading;
 
       // Determinar si el botón fue presionado según el tipo de conexión
       bool buttonPressed = false;
 
       if (buttonConnectsToGND) {
         // Botón conecta a GND: presionado = LOW
         buttonPressed = (buttonState == LOW);
       } else {
         // Botón conecta a VCC: presionado = HIGH
         buttonPressed = (buttonState == HIGH);
       }
 
               // Lógica para la pluma de ENTRADA y el LED RGB
        if (buttonPressed && !plumaEntradaAbierta) {
          Serial.println("BUTTON_PRESSED");
          
          // ACTIVAR PLUMA DE ENTRADA (LOW = juntar cables a tierra)
          digitalWrite(PLUMA_ENTRADA_PIN, LOW);
          Serial.println("PLUMA ENTRADA ACTIVADA - Abriendo...");
          
          // Encender LED en verde
          digitalWrite(RED_PIN, HIGH);
          digitalWrite(GREEN_PIN, LOW);
          digitalWrite(BLUE_PIN, HIGH);
          
          plumaEntradaAbierta = true;
          tiempoAperturaEntrada = millis();
        }

        // Si el botón no está presionado y la pluma entrada está cerrada, LED siempre rojo
        if (!buttonPressed && !plumaEntradaAbierta && !plumaSalidaAbierta) {
          digitalWrite(RED_PIN, LOW);
          digitalWrite(GREEN_PIN, HIGH);
          digitalWrite(BLUE_PIN, HIGH);
          Serial.println("BUTTON_RELEASED");
        }
     }
   }
 
     // --- CONTROL DE TIEMPO DE LA PLUMA DE ENTRADA ---
  if (plumaEntradaAbierta) {
    // Cambiar LED a rojo después de DURACION_VERDE (pero pluma sigue abierta)
    if (millis() - tiempoAperturaEntrada >= DURACION_VERDE && 
        millis() - tiempoAperturaEntrada < DURACION_PLUMA) {
      // LED rojo pero pluma aún abierta (solo si pluma salida no está abierta)
      if (!plumaSalidaAbierta) {
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        digitalWrite(BLUE_PIN, HIGH);
      }
    }
    
    // Cerrar pluma entrada después de DURACION_PLUMA
    if (millis() - tiempoAperturaEntrada >= DURACION_PLUMA) {
      // DESACTIVAR PLUMA ENTRADA (HIGH = separar cables)
      digitalWrite(PLUMA_ENTRADA_PIN, HIGH);
      Serial.println("PLUMA ENTRADA DESACTIVADA - Cerrando...");
      
      plumaEntradaAbierta = false;
      
      // LED rojo solo si pluma salida tampoco está abierta
      if (!plumaSalidaAbierta) {
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        digitalWrite(BLUE_PIN, HIGH);
      }
    }
  }
  
  // --- CONTROL DE TIEMPO DE LA PLUMA DE SALIDA ---
  if (plumaSalidaAbierta) {
    // Cerrar pluma salida después de DURACION_PLUMA_SALIDA
    if (millis() - tiempoAperturaSalida >= DURACION_PLUMA_SALIDA) {
      // DESACTIVAR PLUMA SALIDA (HIGH = separar cables)
      digitalWrite(PLUMA_SALIDA_PIN, HIGH);
      Serial.println("PLUMA SALIDA DESACTIVADA - Cerrando...");
      
      plumaSalidaAbierta = false;
      
      // LED rojo solo si pluma entrada tampoco está abierta
      if (!plumaEntradaAbierta) {
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        digitalWrite(BLUE_PIN, HIGH);
      }
    }
  }
 
     // Guardar el estado actual para la próxima comparación
  lastButtonState = reading;

  // Pausa más larga para estabilidad
  delay(50);
}

// === FUNCIONES PARA COMANDOS SERIALES ===

// Función para procesar comandos recibidos por serial
void processSerialCommands() {
  // Leer datos disponibles del serial
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
  
  // Si recibimos un comando completo, procesarlo
  if (stringComplete) {
    inputString.trim(); // Remover espacios en blanco
    processCommand(inputString);
    
    // Limpiar para el próximo comando
    inputString = "";
    stringComplete = false;
  }
}

// Función para procesar comandos específicos
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

// Función para abrir la pluma de SALIDA
void openSalidaGate() {
  if (!plumaSalidaAbierta) {
    Serial.println("Abriendo pluma de SALIDA...");
    digitalWrite(PLUMA_SALIDA_PIN, LOW);  // Activar pluma salida
    plumaSalidaAbierta = true;
    tiempoAperturaSalida = millis();
    
    // LED azul para indicar pluma de salida abierta
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
    
    Serial.println("GATE_OPENED - Pluma salida activada");
  } else {
    Serial.println("La pluma de salida ya está abierta");
  }
}

// Función para cerrar la pluma de SALIDA
void closeSalidaGate() {
  if (plumaSalidaAbierta) {
    Serial.println("Cerrando pluma de SALIDA...");
    digitalWrite(PLUMA_SALIDA_PIN, HIGH);  // Desactivar pluma salida
    plumaSalidaAbierta = false;
    
    // LED rojo solo si pluma entrada tampoco está abierta
    if (!plumaEntradaAbierta) {
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, HIGH);
    }
    
    Serial.println("GATE_CLOSED - Pluma salida desactivada");
  } else {
    Serial.println("La pluma de salida ya está cerrada");
  }
}

// Función para abrir la pluma de ENTRADA por comando serial
void openEntradaGate() {
  if (!plumaEntradaAbierta) {
    Serial.println("Abriendo pluma de ENTRADA por comando serial...");
    digitalWrite(PLUMA_ENTRADA_PIN, LOW);  // Activar pluma entrada
    plumaEntradaAbierta = true;
    tiempoAperturaEntrada = millis();
    
    // LED verde para indicar pluma de entrada abierta
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, HIGH);
    
    Serial.println("ENTRY_GATE_OPENED - Pluma entrada activada por comando");
  } else {
    Serial.println("La pluma de entrada ya está abierta");
  }
}

// Función para cerrar la pluma de ENTRADA por comando serial
void closeEntradaGate() {
  if (plumaEntradaAbierta) {
    Serial.println("Cerrando pluma de ENTRADA por comando serial...");
    digitalWrite(PLUMA_ENTRADA_PIN, HIGH);  // Desactivar pluma entrada
    plumaEntradaAbierta = false;
    
    // LED rojo solo si pluma salida tampoco está abierta
    if (!plumaSalidaAbierta) {
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, HIGH);
    }
    
    Serial.println("ENTRY_GATE_CLOSED - Pluma entrada desactivada por comando");
  } else {
    Serial.println("La pluma de entrada ya está cerrada");
  }
}

// Función para enviar el estado actual del sistema
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

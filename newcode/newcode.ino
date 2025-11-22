//===================Incluyendo Librerias====================
//FC 212.09 Actual en Flash 18/11/25
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <HX711.h>
#include <Preferences.h>
#include <ESP32Servo.h>
//#include <PID_v1.h>
#include <HardwareSerial.h>
#include <Adafruit_HX711.h>
#include <math.h>

//Definicion de la pantalla LCD:
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Definicion del teclado 4x3
const uint8_t ROWS = 4;
const uint8_t COLS = 3;
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '.', '#' }
};
uint8_t colPins[COLS] = { 0, 2, 15 };
uint8_t rowPins[ROWS] = { 19, 17, 16, 4 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//Iniciando variables de SetPoint y Peso Final
String inputValue = "";  //referencia, setpoint
float pesoFinal = 0.0;

//Definicion de Pines de Motor Paso a Paso
#define DIR_PIN 14
#define STEP_PIN 12
#define ENABLE 10


//Definicion de Pines del Módulo HX711 (Celda de Carga)
#define DOUT 18
#define CLK 5
#define CAL_WEIGHT_GRAMS 680  // peso conocido para calibrar (g)

//Inicializacion de la Celda de Carga
HX711 scale;
Preferences preferences;
long sensor_Reading_Results;
float CALIBRATION_FACTOR = 211.74;  //Sacarloooooooooooooooooooooooooooooooo
bool show_Weighing_Results = false;
int weight_In_g;
float weight_In_oz;

//Inicializacion ServoMotor
#define SERVO_PIN 25
Servo servoMG996R;

//Parametros del Modulo LoRa
// ===== CONFIGURACIÓN =====
#define RX_LoRa 32
#define TX_LoRa 33
HardwareSerial lora(2);      // UART2 (Serial2)
String myAddress = "1";      // Direcciones propia de cada ESP32
String targetAddress = "2";  //
String loraBand = "915000000";
String loraNetworkID = "18";


// ===== VARIABLES =====
bool waitingAck = false;
unsigned long lastSendTime = 0;
const unsigned long ackTimeout = 2500;  // 2.5 segundos
String lastMessage = "";

//VARIABLES DEL PID
float Setpoint, Input, Output;
double Kp = 0.8, Ki = 0.3, Kd = 0.1;
//PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

//Variables de Apoyo
String pesoDeseadoStr = "";
bool setpointListo;
float weight_In_lb;



//Variables del StepMotor:
const int stepsPerRevolution = 200;




//Struct del Mensaje
struct DatosMensaje {
  float PesoDispensar = 0;
  bool Dispensando = false;
  bool Exitoso = false;
  int Rpm = 0;
  float PorcentajeUltrasonico = 0.0;
  int AumentoEqui = 0;
  bool datosValidos = false;  // Indica si el mensaje recibido contenía uno o más datos válidos
};

//Prototipado de Funciones Propias
void configuracionLCD();    //añadida
void configuracionCelda();  //añadida
void configuracionServo();  //añadida
void configuracionLoRa();   //añadida

void calibracionCelda();
void imprimirLCD(int col, int fila, String msj, bool clear);
void sendMessage(String msg);
void processIncoming(String data);
void sendAT(String command);
float leerTeclado(String mensaje);

float lecturaDePesoGr();
float GrtoLB(float Gr);
int mostrarMenu();
void configuracionMotor();
float SetPointValido();
bool Dispensado(float pesoObjetivo);



void moverMotor(int pasos, int velocidadMicrosegundos);
void ejecutarCicloMotor(int pasos, bool ultimos);
void Snr();
void ParametrosDeProceso(float PesoDispensar, bool Dispensando, bool Exitoso);
void EnviarRPM(int rpm);


//Variable de Control
bool sistema = true;
String CodigoDeError = "";
bool Exito = false;

//==========================SetUp=======================================//
void setup() {
  Serial.begin(115200);
  configuracionMotor();

  configuracionLCD();
  configuracionLoRa();


  //falta imprimir mensaje de peticion de peso

  configuracionCelda();
  configuracionServo();
  Serial.println("VERIF=Iniciando Comunicación con CoP");

  sendMessage("VERIF=Iniciando Comunicación con CoP");
  imprimirLCD(0, 1, "Iniciando\nComunicacion\nCon CoP", 1);

  imprimirLCD(0, 1, "INICIO\nPROGRAMA", 1);
}



//=========================Loop========================================//
void loop() {


  int opcion = mostrarMenu();

  if (opcion == 1) {  //Dispensado

    Setpoint = SetPointValido();

    String lbs = (Setpoint != 1) ? "lbs" : "lb";

    imprimirLCD(0, 1, "DISPENSANDO: " + String(Setpoint) + lbs, 1);

    delay(3000);


    bool prueba = Dispensado(Setpoint);
    digitalWrite(ENABLE, HIGH);



    if (prueba) {
      float pesoActualGr = lecturaDePesoGr();

      float pesoActualLb = GrtoLB(pesoActualGr);
      imprimirLCD(0, 0, "TERMINO BIEN\n" + String(pesoActualLb), 1);
      delay(5000);
    }

  } else if (opcion == 2) {  //Calibrado
    imprimirLCD(0, 0, "Iniciando\nCalibracion de\nCelda de Carga", true);
    delay(2000);
    calibracionCelda();
  } else if (opcion == 3) {
    imprimirLCD(0, 0, "Iniciando Pesaje", true);
    delay(2000);

    imprimirLCD(0, 0, "Coloque Peso en\nla Celda de Carga", 1);
    delay(5000);
    float pesoGr = lecturaDePesoGr();
    float pesoLb = GrtoLB(pesoGr);
    imprimirLCD(0, 0, "Peso:\nLibras: " + String(pesoLb) + "\nGramos: " + String(pesoGr), true);
    delay(5000);
  }

  Snr();
}



//FUNCIONES SECUNDARIAS

// ========================== CONFIGURACIONES ===========================================

void configuracionLoRa() {
  // RX=16, TX=17
  lora.begin(115200, SERIAL_8N1, RX_LoRa, TX_LoRa);
  delay(1500);
  sendAT("AT+BAND=" + loraBand);
  sendAT("AT+NETWORKID=" + loraNetworkID);
  sendAT("AT+ADDRESS=" + myAddress);
  Serial.println("🟢 ESP32 " + myAddress + " lista para comunicar.");
  imprimirLCD(0, 0, "ESP32 Lista:", true);
  imprimirLCD(0, 1, "Para Comunicar", false);
}





void configuracionCelda() {
  scale.begin(DOUT, CLK);
  preferences.begin("celda", false);
  if (preferences.isKey("factor")) {
    CALIBRATION_FACTOR = preferences.getFloat("factor", CALIBRATION_FACTOR);
    Serial.println("Factor de Calibración de Celda cargado desde Memoria 💾 : " + String(CALIBRATION_FACTOR));
    delay(1000);
    imprimirLCD(0, 0, "Factor Guardado de\nDe Calibracion:\n" + String(CALIBRATION_FACTOR), true);
    delay(5000);


  } else {
    Serial.println("Factor de Calibración de Celda cargado desde Código: " + String(CALIBRATION_FACTOR));
    scale.set_scale(CALIBRATION_FACTOR);
    imprimirLCD(0, 0, "Factor Guardado:", true);
    delay(5000);

    imprimirLCD(0, 1, String(CALIBRATION_FACTOR), false);
    delay(5000);
  }

  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();

  imprimirLCD(0, 0, "Celda de Carga\nConfigurada", true);
}

void configuracionServo() {
  servoMG996R.attach(SERVO_PIN, 500, 2400);  // Rango PWM típico para MG996R
  servoMG996R.write(90);                     // Posición neutral
  imprimirLCD(0, 0, "ServoMotor\nConfigurado", true);
  delay(1000);
}

void configuracionLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Configurada");
  delay(1000);
}

void configuracionMotor() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, HIGH);


  imprimirLCD(0, 0, "StepMotor\nConfigurado", true);
  delay(1000);
}

//=============================================== PROCESOS ==================================

//============ Celda

void calibracionCelda() {
  Serial.println("Iniciando Calibración de Celda de Carga...");
  if (scale.is_ready()) {
    Serial.println();
    Serial.println("Ingrese un Peso Conocido: ");
    imprimirLCD(0, 0, "Ingrese un Peso\nConocido en lbs", true);

    float PesoConocido = leerTeclado("Peso Ingresado: ") * 453.59237;

    Serial.println();
    Serial.println("Do not place any object or weight on the scale.");
    Serial.println("Please wait...");
    imprimirLCD(0, 0, "No coloque\nPeso en la\nCelda de Carga", true);

    for (int i = 5; i >= 0; i--) {
      imprimirLCD(0, 3, String(i), 0);
      delay(1000);
    }

    scale.set_scale();
    Serial.println();
    Serial.println("Set the scales...");
    Serial.println("Please wait...");
    delay(1000);
    scale.tare();
    imprimirLCD(0, 0, "Set the Scales", true);
    delay(1000);

    Serial.println();
    Serial.println("Please place an object whose weight is known on the scale.");
    Serial.flush();

    imprimirLCD(0, 0, "Coloque Peso en la\nCelda de Carga", true);
    for (int i = 5; i >= 0; i--) {
      imprimirLCD(9, 3, String(i), 0);
      delay(1000);
    }
    imprimirLCD(0, 0, "Realizando Lecturas", true);
    delay(1000);

    sensor_Reading_Results = scale.get_units(45);
    Serial.print("Sensor reading results : ");
    Serial.println(sensor_Reading_Results);
    delay(1000);

    CALIBRATION_FACTOR = sensor_Reading_Results / PesoConocido;
    preferences.putFloat("factor", CALIBRATION_FACTOR);

    scale.set_scale(CALIBRATION_FACTOR);

    imprimirLCD(0, 0, "Nuevo Factor:\n" + String(CALIBRATION_FACTOR), true);
    delay(5000);

  } else {
    CodigoDeError = "E: Calibracion";
    imprimirLCD(0, 0, CodigoDeError, true);
    Serial.println("🔴 " + CodigoDeError);
  }
}

//============ LORA

void sendAT(String command) {
  lora.println(command);
  delay(100);
}

void sendMessage(String msg) {
  String cmd = "AT+SEND=" + targetAddress + "," + String(msg.length()) + "," + msg;
  lora.println(cmd);
  lastMessage = msg;
  waitingAck = true;
  lastSendTime = millis();

  delay(300);
  if (lora.available()) {
    String resp = lora.readString();
    if (resp.indexOf("OK") >= 0) {
      Serial.println("📤 Enviado: \"" + msg + "\"");
    }
  }
}

//Editar para procesar en un formato especifica
void processIncoming(String data) {
  // Formato: +RCV=<addr>,<len>,<msg>,<RSSI>,<SNR>
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);
  int lastComma = data.lastIndexOf(',');

  String sender = data.substring(5, firstComma);
  String msg = data.substring(secondComma + 1, lastComma);
  msg.trim();

  if (msg == "ACK") {
    waitingAck = false;
    Serial.println("✅ Confirmación de entrega recibida.");
    return;
  }

  Serial.println("📩 Recibido de " + sender + ": \"" + msg + "\"");

  // Enviar ACK de respuesta solo al recibir mensaje válido
  String ack = "AT+SEND=" + sender + ",3,ACK";
  lora.println(ack);
}



//========= LCD
void imprimirLCD(int col, int fila, String msj, bool clear) {
  if (clear) {
    lcd.clear();
  }

  // Si el mensaje contiene saltos de línea '\n', se imprimen línea por línea
  int currentRow = fila;
  String linea = "";

  for (int i = 0; i < msj.length(); i++) {
    if (msj[i] == '\n') {
      lcd.setCursor(col, currentRow);
      lcd.print(linea);
      linea = "";
      currentRow++;
      if (currentRow > 3) break;  // evita escribir fuera del rango
    } else {
      linea += msj[i];
    }
  }

  // imprime la última línea si existe
  if (linea.length() > 0 && currentRow <= 3) {
    lcd.setCursor(col, currentRow);
    lcd.print(linea);
  }
}

//========= TECLADO
float leerTeclado(String mensaje) {
  String inputValue = "";
  char key;

  while (true) {  // esperar hasta que el usuario confirme
    key = keypad.getKey();

    if (key != NO_KEY) {
      if (key == '#') {                          // Confirmar entrada
        if (inputValue.length() == 0) continue;  // ignorar si está vacío

        float valor = inputValue.toFloat();
        Serial.print("Valor ingresado: ");
        Serial.println(valor);

        imprimirLCD(0, 3, "Confirmada: " + inputValue, true);
        delay(500);
        return valor;

      } else if (key == '*') {  // Borrar entrada
        inputValue = "";
        imprimirLCD(0, 3, mensaje, false);

      } else if ((key >= '0' && key <= '9') || key == '.') {  // agregar dígito o punto
        inputValue += key;
        imprimirLCD(0, 3, mensaje + inputValue, false);
      }
    }

    delay(100);
  }
}



//================Lectura de Peso
float lecturaDePesoGr() {
  float peso_g = scale.get_units(5);
  return (peso_g < 0) ? 0 : peso_g;
}
float GrtoLB(float Gr) {
  return Gr / 453.59237;
}

//===============Selladora

void sellarBolsaConServo() {
  Serial.println("Iniciando proceso de sellado...");
  imprimirLCD(0, 0, "Sellando...", true);

  // Giro derecha (0°)
  servoMG996R.write(180);
  Serial.println("Giro derecha");
  delay(7500);

  // Pausa (90°)
  servoMG996R.write(90);
  Serial.println("Pausa");
  delay(3200);

  // Giro izquierda (180°)
  servoMG996R.write(0);  //SUPUESTAMENTE IZQUIERDA
  Serial.println("Giro izquierda");
  delay(3000);

  // Volver a posición neutral
  servoMG996R.write(90);
  Serial.println("Proceso de sellado completado.");
  imprimirLCD(0, 0, "Bolsa Sellada", true);
  Exito = true;
}


int mostrarMenu() {
  while (true) {
    // Mostrar menú completo usando saltos de línea
    imprimirLCD(0, 0, "Seleccione opcion:\n1. Dispensar\n2. Calibrar\n3. Pesar", true);
    // Leer opción del teclado
    float opcion = leerTeclado("Seleccion: ");

    // Validar opción
    if (opcion == 1 || opcion == 2 || opcion == 3) {
      return (int)opcion;  // devuelve opción correcta
    } else {
      imprimirLCD(0, 0, "ERROR:\nOpcion no Valida", true);
      delay(1000);
      Serial.println("Opción inválida. Intente de nuevo.");
      delay(1500);  // espera antes de mostrar el menú de nuevo
    }
  }
}

float SetPointValido() {
  setpointListo = false;

  float validacion;
  while (!setpointListo) {
    imprimirLCD(0, 0, "Peso Deseado\nEn Libras: ", 1);
    validacion = leerTeclado("Peso: ");

    if (validacion >= 0.5 && validacion <= 2.0 && fmod(validacion, 0.5) == 0) {
      setpointListo = true;
      return validacion;

    } else {
      imprimirLCD(0, 0, "Peso invalido\nIntente nuevamente", 1);
      delay(1500);
    }
  }
}






void moverMotor(int pasos, int velocidadMicrosegundos) {
  for (int i = 0; i < pasos; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(velocidadMicrosegundos);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(velocidadMicrosegundos);
  }
}




void ejecutarCicloMotor(int pasos, bool ultimos) {
  digitalWrite(ENABLE, LOW);

  digitalWrite(DIR_PIN, HIGH);
  moverMotor(pasos, 1800);  // más rápido y suave
  delay(200);

  if (!ultimos) {
    digitalWrite(DIR_PIN, LOW);
    moverMotor(stepsPerRevolution / 4, 1800);
    delay(200);
  } else {
    digitalWrite(DIR_PIN, LOW);
    moverMotor(20, 1800);
    delay(200);
  }
}


bool Dispensado(float pesoObjetivo) {

  int control = 0;

  ParametrosDeProceso(pesoObjetivo, 1, 0);

  while (true) {

    float pesoActual = GrtoLB(lecturaDePesoGr());
    float error = pesoObjetivo - pesoActual;
    float errorRelativo = error / pesoObjetivo;


    // ---- Etapa final: muy cerca ----
    if (errorRelativo <= 0.3) {  // 2% del objetivo 0.178      //EnviarRPM(0); Actual 560 con 0.25
      delay(2500);
      sellarBolsaConServo();
      EnviarRPM(0);

      sendMessage("EXITOSO=" + String(true));

      // ParametrosDeProceso(pesoObjetivo, 0, 1);

      return true;
    }


    // ---- Motor rápido: falta mucho ----
    if (errorRelativo >= 0.50) {  // más del 50%
      if (control == 0) {
        EnviarRPM(210);
        control++;
      }
      ejecutarCicloMotor(210, 0);
      imprimirLCD(0, 0, "Etapa 1: Rápido", 1);
    }

    // ---- Motor medio: falta moderado ----
    else if (errorRelativo > 0.40) {  // entre 20% y 40%
      if (control == 1) {
        EnviarRPM(90);
        control++;
      }
      ejecutarCicloMotor(90, 1);
      imprimirLCD(0, 0, "Etapa 2: Medio", 1);
    }

    // ---- Motor fino: muy cerca ----
    else {
      if (control == 2) {
        EnviarRPM(70);
        control++;
      }
      //EnviarRPM(70);
      ejecutarCicloMotor(70, 1);  // movimientos MUY pequeños
      imprimirLCD(0, 0, "Etapa 3: Fina", 1);
    }

    // Control por error

    imprimirLCD(0, 0, "Peso: " + String(pesoActual), 1);
    delay(150);  // pequeño respiro para nueva lectura
  }
}

void Snr() {

  // sendMessage("SNR="+String(lora.packetSnr());

  sendMessage("SNR=" + String(40));
  delay(300);
}

void ParametrosDeProceso(float PesoDispensar, bool Dispensando, bool Exitoso) {

  // Crear el texto del mensaje en formato clave=valor
  String mensaje = "PESO=" + String(PesoDispensar) + ";DISPENSANDO=" + String(Dispensando ? 1 : 0) + ";EXITOSO=" + String(Exitoso ? 1 : 0) + ";";

  // Enviar el mensaje por LoRa

  // (Opcional) Mostrar lo enviado por serial
  Serial.print("Mensaje enviado: ");
  Serial.println(mensaje);

  sendMessage(mensaje);
}

void EnviarRPM(int pasos) {

  int rpm = (pasos / 200) / (1.8 / 60000);

  sendMessage("RPM=" + String(rpm));
}


// DatosMensaje ProcesarMensajeEntrante(String data) {

//   DatosMensaje datos;
//   datos.datosValidos = false;

//   // 1️⃣ Extraer mensaje del formato LoRa
//   int firstComma = data.indexOf(',');
//   int secondComma = data.indexOf(',', firstComma + 1);
//   int lastComma = data.lastIndexOf(',');

//   if (firstComma == -1 || secondComma == -1 || lastComma == -1) {
//     Serial.println("⚠️ Formato inválido");
//     return datos;
//   }

//   String sender = data.substring(5, firstComma);
//   String msg = data.substring(secondComma + 1, lastComma);
//   msg.trim();

//   // 2️⃣ Si es ACK, no procesar
//   if (msg == "ACK") {
//     Serial.println("ACK recibido");
//     return datos;
//   }

//   // 3️⃣ Enviar ACK de vuelta
//   String ack = "AT+SEND=" + sender + ",3,ACK";
//   lora.println(ack);

//   // 4️⃣ Procesar claves=valores
//   Serial.println("📩 Mensaje recibido:");

//   int start = 0;
//   datos.datosValidos = true;

//   while (true) {

//     int pos = msg.indexOf(';', start);
//     String segmento;

//     if (pos == -1) segmento = msg.substring(start);
//     else segmento = msg.substring(start, pos);

//     segmento.trim();
//     if (segmento.length() == 0) break;

//     int eqPos = segmento.indexOf('=');
//     if (eqPos != -1) {

//       String clave = segmento.substring(0, eqPos);
//       String valor = segmento.substring(eqPos + 1);
//       clave.trim();
//       valor.trim();

//       // 🖨️ Imprimir clave = valor
//       Serial.println(" - " + clave + " = " + valor);

//       // Guardar en el struct según la clave
//       if (clave == "PESO") datos.PesoDispensar = valor.toInt();
//       else if (clave == "DISPENSANDO") datos.Dispensando = valor.toInt() == 1;
//       else if (clave == "EXITOSO") datos.Exitoso = valor.toInt() == 1;
//       else if (clave == "RPM") datos.Rpm = valor.toInt();
//       else if (clave == "PORCENTAJE_ULTRA") datos.PorcentajeUltrasonico = valor.toFloat();
//       else if (clave == "AUMENTO_EQUI") datos.AumentoEqui = valor.toInt();
//     }

//     if (pos == -1) break;
//     start = pos + 1;
//   }

//   return datos;
// }


//HACE FALTA INTEGRAR EL ENVIO DE INFORMACION EN LAS DIFERENTES ETAPAS E IMPLEMENTAR
//LA INTEGRACION RECIBIR INFO
//RECIBIR:  , PESO CONOCIDO, OPCIONES (1,2).

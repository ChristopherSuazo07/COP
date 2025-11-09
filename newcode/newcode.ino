//===================Incluyendo Librerias====================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <HX711.h>
#include <Preferences.h>
#include <ESP32Servo.h>
#include <PID_v1.h>
#include <HardwareSerial.h>
#include <Adafruit_HX711.h>


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


//Definicion de Pines del Módulo HX711 (Celda de Carga)
#define DOUT 18
#define CLK 5
#define CAL_WEIGHT_GRAMS 455  // peso conocido para calibrar (g)


//Inicializacion de la Celda de Carga
HX711 scale;
Preferences preferences;
long sensor_Reading_Results;
float CALIBRATION_FACTOR = 2280.0f;  //Sacarloooooooooooooooooooooooooooooooo
bool show_Weighing_Results = false;
int weight_In_g;
float weight_In_oz;



//Inicializacion ServoMotor
#define SERVO_PIN 25
Servo servoMG996R;




//Parametros del Modulo LoRa
// ===== CONFIGURACIÓN =====
#define RX_LoRa 16
#define TX_LoRa 17
HardwareSerial lora(2);      // UART2 (Serial2)
String myAddress = "2";      // Direcciones propia de cada ESP32
String targetAddress = "1";  //
String loraBand = "915000000";
String loraNetworkID = "18";
// ===== VARIABLES =====
bool waitingAck = false;
unsigned long lastSendTime = 0;
const unsigned long ackTimeout = 2500;  // 2.5 segundos
String lastMessage = "";


//VARIABLES DEL PID
double Setpoint, Input, Output;
double Kp = 0.8, Ki = 0.3, Kd = 0.1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);



//Variables de Apoyo
String pesoDeseadoStr = "";
bool setpointListo = false;
float weight_In_lb;





//Prototipado de Funciones Propias
void configuracionLCD();     //añadida
void configuracionCelda();   //añadida
void configuracionServo();   //añadida
void configuracionesLoRa();  //añadida

void calibracionCelda();
void imprimirLCD(int col, int fila, String msj, bool clear);
void sendMessage(String msg);
void processIncoming(String data);
void sendMessage(String msg);
void sendAT(String command);
float leerTeclado();

float lecturaDePesoGr();
float GrtoLB(float Gr);


//Variable de Control
bool sistema = true;
String CodigoDeError = "";
bool Exito = false;

//==========================SetUp=======================================//
void setup() {
  Serial.begin(115200);
  configuracionLCD();
  configuracionLoRa();

  //falta imprimir mensaje de peticion de peso

  configuracionMotor();
  configuracionCelda();
  configuracionServo();
}


//=========================Loop========================================//
void loop() {
}

//FUNCIONES SECUNDARIAS

// ========================== CONFIGURACIONES ===========================================

void configuracionesLoRa() {
  // RX=16, TX=17

  if(lora.begin(115200, SERIAL_8N1, RX_LoRa, TX_LoRa)){
    delay(1500);
    sendAT("AT+BAND=" + loraBand);
    sendAT("AT+NETWORKID=" + loraNetworkID);
    sendAT("AT+ADDRESS=" + myAddress);
    Serial.println("🟢 ESP32 " + myAddress + " lista para comunicar.");
    imprimirLCD(0, 0, "ESP32 Lista:", true);
    imprimirLCD(0, 1, "Para Comunicar", false);
  } else{
    CodigoDeError = "E: Modulo LoRa";
    imprimirLCD(0, 0, CodigoDeError, true);
    Serial.println("🔴 CodigoDeError");


  }
}

void configuracionPID() {
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-1000, 1000);  // Rango de pasos por segundo buscar en datasheet
  myPID.SetSampleTime(100);            // 100 ms
  imprimirLCD(0, 0, "PID Establecido", true);
}


void configuracionCelda() {
  scale.begin(DOUT, CLK);
  preferences.begin("celda", false);

  if (scale.is_ready()) {

    if(preferences.iskey("factor")){
      CALIBRATION_FACTOR = preferences.getFloat("factor", CALIBRATION_FACTOR);
      Serial.println("Factor de Calibración de Celda cargado desde Memoria 💾 : " + String(CALIBRATION_FACTOR);
      imprimirLCD(0,0,"Factor Guardado:", true);
      imprimirLCD(0,1,String(CALIBRATION_FACTOR), false);
    } else{
      Serial.println("Factor de Calibración de Celda cargado desde Código: " + String(CALIBRATION_FACTOR);
      scale.set_scale(CALIBRATION_FACTOR);
      imprimirLCD(0,0,"Factor Guardado:", true);
      imprimirLCD(0,1,String(CALIBRATION_FACTOR), false);

    }
    scale.set_scale(CALIBRATION_FACTOR);
    imprimirLCD(0,0,"Celda Configurada", true);
  } else {
    CodigoDeError = "E: Modulo HX711";
    imprimirLCD(0, 0, CodigoDeError, true);
    Serial.println("🔴 " + CodigoDeError);
  }
}


//void imprimirLCD(int col, int fila, String msj, bool clear)


void configuracionServo() {
  servoMG996R.attach(SERVO_PIN, 500, 2400);  // Rango PWM típico para MG996R
  servoMG996R.write(90);                     // Posición neutral
  imprimirLCD(0, 0, "Servo Configurado", true);
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
  imprimirLCD(0, 0, "StepMotor Configurado", true);
}


//=============================================== PROCESOS ==================================


//============ Celda

void calibracionCelda() {
  Serial.println("Iniciando Calibración de Celda de Carga...");
  delay(1000);
  imprimirLCD(0, 0, "Iniciando Calibracion", true);
  imprimirLCD(0, 1, "De Celda de Carga", false);

  if (scale.is_ready()) {

    Serial.println();
    Serial.println("Ingrese un Peso Conocido: ");
    imprimirLCD(0, 0, "Ingrese un Peso", true);
    imprimirLCD(0, 1, "Conocido en gr", false);

    float PesoConocido = leerTeclado();

    Serial.println();
    Serial.println("Do not place any object or weight on the scale.");
    Serial.println("Please wait...");
    imprimirLCD(0, 0, "No coloque ningun Peso", true);
    imprimirLCD(0, 1, "en la Celda", false);

    scale.set_scale();
    Serial.println();
    Serial.println("Set the scales...");
    Serial.println("Please wait...");
    delay(1000);
    scale.tare();
    imprimirLCD(0, 0, "Set the Scales", true);


    Serial.println();
    Serial.println("Please place an object whose weight is known on the scale.");
    Serial.flush();

    imprimirLCD(0, 0, "Coloque un Peso", true);
    imprimirLCD(0, 1, "En La Celda", false);

    imprimirLCD(0, 0, "Realizando Lecturas", true);
    
    sensor_Reading_Results = scale.get_units(10);
    Serial.print("Sensor reading results : ");
    Serial.println(sensor_Reading_Results);
    delay(1000);
    

    CALIBRATION_FACTOR = sensor_Reading_Results / PesoConocido;
    preferences.putFloat("factor", CALIBRATION_FACTOR);

    scale.set_scale(CALIBRATION_FACTOR);

    imprimirLCD(0, 0, "Nuevo Factor: ", true);
    imprimirLCD(0, 1, String(CALIBRATION_FACTOR), false);

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
  } else {
    CodigoDeError = "E: Envio de Msj";
    imprimirLCD(0, 0, CodigoDeError, true);
    Serial.println("🔴 " + CodigoDeError);
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
  lcd.setCursor(col, fila);
  lcd.print(msj);
}



//========= TECLADO
float leerTeclado() {
  String inputValue = "";  // Variable local para almacenar la entrada del usuario
  char key;

  imprimirLCD(0, 0, "Ingrese valor:", 1);
  imprimirLCD(0, 1, "", 0);
  Serial.println("Ingrese un valor y presione '#' para confirmar:");

  while (true) {            // Esperar hasta que el usuario confirme con '#'
    key = keypad.getKey();  // Leer la tecla presionada

    if (key) {  // Si se presionó alguna tecla
      if (key == '#') {
        // Confirmar el número ingresado
        Serial.print("Valor ingresado: ");
        Serial.println(inputValue);
        imprimirLCD(0, 0, "Valor ingresado:", 1);
        imprimirLCD(0, 1, inputValue, 0);

        float valor = inputValue.toFloat();  // Convertir a número decimal
        inputValue = "";                     // Limpiar para próxima entrada

        return valor;  // ✅ Retorna el número confirmado
      } else if (key == '*') {
        // Si presiona '*', borrar todo el texto actual
        inputValue = "";
        Serial.println("Entrada borrada.");

        imprimirLCD(0, 0, "Ingresando:", 1);
        imprimirLCD(0, 1, "", 0);
      } else {
        // Agregar el carácter presionado al valor actual
        inputValue += key;
        Serial.print("Ingresando: ");
        Serial.println(inputValue);

        imprimirLCD(0, 0, "Ingresando:", 1);
        imprimirLCD(0, 1, inputValue, 0);
      }
    }

    delay(100);  // Pequeña pausa para evitar rebotes
  }
}


//================Lectura de Peso
float lecturaDePesoGr(){

  float peso_g = scale.get_units(5);

  return (peso_g < 0) ? 0 : peso_g;

}

float GrtoLB(float Gr){
  return Gr / 453.59237;
}



//===============Selladora

void sellarBolsaConServo() {
  Serial.println("Iniciando proceso de sellado...");
  imprimirLCD(0, 0, "Sellando...", true);

  // Giro derecha (0°)
  servoMG996R.write(180);
  Serial.println("Giro derecha");
  delay(3500);

  // Pausa (90°)
  servoMG996R.write(90);
  Serial.println("Pausa");
  delay(3200);

  // Giro izquierda (180°)
  servoMG996R.write(0); //SUPUESTAMENTE IZQUIERDA  
  Serial.println("Giro izquierda");
  delay(3000);

  // Volver a posición neutral
  servoMG996R.write(90);
  Serial.println("Proceso de sellado completado.");
  imprimirLCD(0, 0, "Bolsa Sellada", true);
  Exito = true;

}


//======================Control PID

void controlPID(float lectura) {
  Input = lectura; // Peso actual
  myPID.Compute();            // Calcula Output basado en Error
  int pasos = (int)Output;
  if(abs(pasos) > 2){ // ignorar pasos muy pequeños
    moverMotor(pasos);
  }

  // Revisar si peso alcanzado
  if(abs(Input - Setpoint) < 2){ // tolerancia 2 gramos
    imprimirLCD(0,0,"Peso alcanzado!",true);
    sellarBolsa();
    setpointListo = false; // listo para próxima bolsa
  }

  delay(50); // pequeño delay para estabilidad
}



//HACE FALTA INTEGRAR EL ENVIO DE INFORMACION EN LAS DIFERENTES ETAPAS E IMPLEMENTAR LA INTEGRACION RECIBIR INFO
//RECIBIR: VARAIBLES PID, PESO CONOCIDO, OPCIONES (1,2), PESO A DISPENSAR.




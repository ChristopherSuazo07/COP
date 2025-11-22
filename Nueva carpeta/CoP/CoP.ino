#include <HardwareSerial.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>

// ===== UART =====
HardwareSerial lora(2);  // UART2 (Serial2)

int maria = 0;
// ===== CONFIGURACIÓN =====
String myAddress = "2";      // 👈 Cambia a "2" en la otra ESP32
String targetAddress = "1";  // 👈 Cambia a "1" en la otra ESP32
String loraBand = "915000000";
String loraNetworkID = "18";

// ===== VARIABLES =====
bool waitingAck = false;
unsigned long lastSendTime = 0;
const unsigned long ackTimeout = 2500;  // 2.5 segundos
String lastMessage = "";

// ===== Variables de Config de Red =====
const char *WiFi_SSID = "Suazo";
const char *PASSWORD = "suazo0704";
unsigned long sendDataPrevMillis = 0;
bool WiFiStatus = false;

// ===== Variables de Firebase =====
const char *DB_URL = "https://copcomcon-default-rtdb.firebaseio.com/";
const char *API_WEB_KEY = "AIzaSyCitW41T9jLgBGisdPkaR9BxKFvPnkeKW0";
bool FBStatus = false;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOk = false;

// ===== Struct =====
struct Datos {
  float PesoDispensar;
  int Dispensando;
  int Exitoso;
  int Snr;
  int Rpm;
};

// ===== Zona horaria Nica =====
const long utcOffsetInSeconds = -6 * 3600;

// ===== Temporizador 24h =====
unsigned long proximaActualizacion = 0;
const unsigned long INTERVALO_24H = 24UL * 3600UL * 1000UL;  // 24 horas en ms


const int RSSI_MIN = -90;       // RSSI mínimo esperado (dBm)
const int RSSI_MAX = -70;       // RSSI máximo esperado (dBm)
const int NOISE_FLOOR = -120;   // Nivel de ruido de la banda (dBm)


// ===== Prototipos de funciones =====
bool isConnectionOK();
void FBConfig();
void verificarFirebase();
void actualizarConfiguracion();
String obtenerSemanaActual();
int obtenerSemanaISO();
String obtenerFechaHoy();
void configurarHora();
Datos ExtrayendoDatos(String msj);
String obtenerValor(String mensaje, String clave);
void processIncoming(String data);
void sendMessage(String msg);
void sendAT(String command);
void ActualizarconfiguracionFB(String fecha, String semana);
void registrarVenta();
void registrarVentaSemana();
void registrarVentaDiaria(Datos datos);
void registrarTodo(Datos datos);
void EnviarSnr();
void Sender(int Value, String path);
// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  lora.begin(115200, SERIAL_8N1, 32, 33);  // RX=32, TX=33

  delay(1500);
  sendAT("AT+BAND=" + loraBand);
  sendAT("AT+NETWORKID=" + loraNetworkID);
  sendAT("AT+ADDRESS=" + myAddress);

  Serial.println("🟢 ESP32 " + myAddress + " lista para comunicar.");
  Serial.println("Escribe un mensaje y presiona Enter para enviarlo:");

  FBConfig();
  delay(2000);   // ⬅ MUY IMPORTANTE

  configurarHora();

  actualizarConfiguracion();
}

// ===== LOOP =====
void loop() {
  // Leer mensaje desde el monitor serial


  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) sendMessage(input);
  }

  // Leer mensaje recibido desde LoRa
  if (lora.available()) {
    String response = lora.readStringUntil('\n');
    response.trim();

    if (response.startsWith("+RCV=")) processIncoming(response);
  }


  // if(maria <= 2){
  //   registrarVenta();
  //   registrarVentaSemana();

  // }
  // Actualización cada 24 horas
  if (millis() >= proximaActualizacion) actualizarConfiguracion();

  // maria++;
  delay(1000);
}

// ===== DEFINICIÓN DE FUNCIONES =====

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
    if (resp.indexOf("OK") >= 0) Serial.println("📤 Enviado: \"" + msg + "\"");
  }
}

void processIncoming(String data) {
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

  // Enviar ACK
  String ack = "AT+SEND=" + sender + ",3,ACK";
  lora.println(ack);


  // === NUEVO: registrar ventas por hora ===
  Datos datos = ExtrayendoDatos(msg);
  if (datos.Exitoso == 1) {
    verificarFirebase();

    registrarTodo(datos);

      // registrar con hora exacta y precio automático
  } else{
    Sender(datos.Rpm, "Parametros/RPM");

  }
}

void registrarTodo(Datos datos) {
    registrarVenta();
    delay(120);
    registrarVentaSemana();
    delay(120);
    registrarVentaDiaria(datos);
    delay(120);
    EnviarSnr();

}

void Sender(int Value, String path) {

  if (Firebase.ready() && signupOk && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.setInt(&fbdo, path, Value)) {

      Serial.println("Data sent successfully to Firebase.");
      Serial.println(IUV);
      FBStatus = true;
    } else {
      Serial.print("Failed to send data to Firebase.");
      FBStatus = false;
    }
  }

  delay(500);
}


void EnviarSnr(){
  float snr = float(rssi - NOISE_FLOOR);
  Sender(snr,"Parametros/SNR");

}

float obtenerSNRSimulado() {
  int rssi = random(RSSI_MIN, RSSI_MAX + 1);
  int fluctuacion = random(-2, 3);  // ±2 dBm de variación
  rssi += fluctuacion;

  // Calcula SNR estimada
  float snr = float(rssi - NOISE_FLOOR);
  return snr;
}


String obtenerValor(String mensaje, String clave) {
  String patron = clave + "=";
  int inicio = mensaje.indexOf(patron);
  if (inicio == -1) return "";
  inicio += patron.length();
  int fin = mensaje.indexOf(";", inicio);
  if (fin == -1) fin = mensaje.length();
  return mensaje.substring(inicio, fin);
}

Datos ExtrayendoDatos(String msj) {
  Datos datos;
  String vPeso = obtenerValor(msj, "PESO");
  String vDisp = obtenerValor(msj, "DISPENSANDO");
  String vExito = obtenerValor(msj, "EXITOSO");
  String vRPM = obtenerValor(msj,"RPM");

  datos.PesoDispensar = vPeso.length() > 0 ? vPeso.toFloat() : -1;
  datos.Dispensando = vDisp.length() > 0 ? vDisp.toInt() : -1;
  datos.Exitoso = vExito.length() > 0 ? vExito.toInt() : -1;
  datos.Rpm = vSNR.length() > 0 ? vSNR.toInt() : -1;

  return datos;
}

void FBConfig() {
  Serial.print("Configurando FireBase");
  WiFi.begin(WiFi_SSID, PASSWORD);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("Connected with IP: ");
  Serial.println(WiFi.localIP());
  WiFiStatus = true;

  config.api_key = API_WEB_KEY;
  config.database_url = DB_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK");
    signupOk = true;
    FBStatus = true;
  } else {
    Serial.println(config.signer.signupError.message.c_str());
    FBStatus = false;
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

bool isConnectionOK() {
  return WiFi.status() == WL_CONNECTED && Firebase.ready() && signupOk;
}

void configurarHora() {
  Serial.println("Sincronizando NTP...");
  configTime(utcOffsetInSeconds, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) Serial.println("❌ Error obteniendo hora NTP");
  else Serial.println("✔ Hora NTP sincronizada");
}

String obtenerFechaHoy() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "";

  char buffer[11];
  sprintf(buffer, "%04d-%02d-%02d",
          timeinfo.tm_year + 1900,
          timeinfo.tm_mon + 1,
          timeinfo.tm_mday);
  return String(buffer);
}

int obtenerSemanaISO() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char buffer[3];
  strftime(buffer, 3, "%V", &timeinfo);
  return atoi(buffer);
}

String obtenerSemanaActual() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  int anio = timeinfo.tm_year + 1900;
  int semana = obtenerSemanaISO();

  char buffer[20];
  sprintf(buffer, "semana_%04d-%02d", anio, semana);
  return String(buffer);
}

void actualizarConfiguracion() {
  Serial.println("\n---- ACTUALIZANDO FIREBASE ----");

  String fecha = obtenerFechaHoy();
  String semana = obtenerSemanaActual();

  Serial.println("fechaHoy = " + fecha);
  Serial.println("semanaActual = " + semana);

  ActualizarconfiguracionFB(fecha, semana);

  Serial.println("✔ Actualización enviada");

  proximaActualizacion = millis() + INTERVALO_24H;
}

void ActualizarconfiguracionFB(String fecha, String semana) {
  if (Firebase.ready() && signupOk && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {

    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setString(&fbdo, "/configuracion/fechaHoy", fecha))
      Serial.println("Fecha Actual: " + fecha);
    else
      Serial.println("Failed to send fecha Actual to firebase");

    if (Firebase.RTDB.setString(&fbdo, "/configuracion/semanaActual", semana))
      Serial.println("Semana Actual: " + semana);
    else
      Serial.println("Failed to send semanaActual");
  }
}




void registrarVenta() {
  if (!isConnectionOK()) return;

  // 1️⃣ Obtener fecha y hora
  String fecha = obtenerFechaHoy();  // Ej: "2025-11-20"

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int horaActual = timeinfo.tm_hour;

  // Limitar solo de 8 a 17
  if (horaActual < 0 || horaActual > 23) return;

  char horaBuffer[6];
  sprintf(horaBuffer, "%02d:00", horaActual);
  String horaStr = String(horaBuffer);  // Ej: "08:00"

  String pathBase = "/ventasPorHora/" + fecha;

  // 2️⃣ Inicializar horas si el día no existe
  // Usamos get para comprobar si el nodo existe
  if (!Firebase.RTDB.get(&fbdo, pathBase) || fbdo.dataType() != "json") {
    for (int h = 8; h <= 17; h++) {
      char buf[6];
      sprintf(buf, "%02d:00", h);
      String horaNodo = buf;
      Firebase.RTDB.setInt(&fbdo, pathBase + "/" + horaNodo, 0);
    }
  }

  // 3️Leer valor actual de la hora
  int valorActual = 0;
  if (Firebase.RTDB.getInt(&fbdo, pathBase + "/" + horaStr)) {
    valorActual = fbdo.intData();
  }

  // 4Incrementar en 1
  Firebase.RTDB.setInt(&fbdo, pathBase + "/" + horaStr, valorActual + 1);

  Serial.println(" Venta registrada: " + fecha + " " + horaStr + " = " + String(valorActual + 1));
}




void registrarVentaSemana() {
  if (!isConnectionOK()) return;

  // 1️⃣ Obtener semana actual y día de la semana
  String semana = obtenerSemanaActual();  // ej: "semana_2025-47"

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int wday = timeinfo.tm_wday;  // 0 = domingo, 1 = lunes, ..., 6 = sábado

  // Mapear número de día a string
  const char *diasSemana[7] = { "domingo", "lunes", "martes", "miercoles", "jueves", "viernes", "sabado" };
  String diaStr = String(diasSemana[wday]);

  String pathBase = "/ventasPorSemana/" + semana;

  // 2️⃣ Inicializar semana si no existe
  if (!Firebase.RTDB.get(&fbdo, pathBase) || fbdo.dataType() != "json") {
    for (int i = 0; i < 7; i++) {
      Firebase.RTDB.setInt(&fbdo, pathBase + "/" + diasSemana[i], 0);
    }
  }

  // 3️⃣ Leer valor actual del día
  int valorActual = 0;
  if (Firebase.RTDB.getInt(&fbdo, pathBase + "/" + diaStr)) {
    valorActual = fbdo.intData();
  }

  // 4️⃣ Incrementar en 1
  Firebase.RTDB.setInt(&fbdo, pathBase + "/" + diaStr, valorActual + 1);

  Serial.println("✅ Venta semanal registrada: " + semana + " " + diaStr + " = " + String(valorActual + 1));
}


void registrarVentaDiaria(Datos datos, float precioVenta) {
  if (!isConnectionOK()) return;

  // 1️⃣ Obtener fecha y hora actual
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  char fechaBuffer[11];
  sprintf(fechaBuffer, "%04d-%02d-%02d",
          timeinfo.tm_year + 1900,
          timeinfo.tm_mon + 1,
          timeinfo.tm_mday);
  String fecha = String(fechaBuffer);  // ej: "2025-11-20"

  char horaBuffer[9];
  sprintf(horaBuffer, "%02d:%02d:%02d",
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec);
  String hora = String(horaBuffer);  // ej: "08:15:32"

  // 2️⃣ Preparar el path en Firebase
  String pathBase = "/registrosDiarios/" + fecha + "/" + hora;

  // 3️⃣ Crear JSON con los datos
  FirebaseJson json;
  json.set("Cantidad", abs(datos.PesoDispensar));
  json.set("Exitoso", datos.Exitoso == 1 ? true : false);
  json.set("PrecioVenta", precioVenta);

  // 4️⃣ Guardar en Firebase
  if (Firebase.RTDB.setJSON(&fbdo, pathBase, &json)) {
    Serial.println("✅ Registro diario guardado: " + pathBase);
  } else {
    Serial.println("❌ Error al guardar registro diario: " + fbdo.errorReason());
  }
}



void registrarVentaDiaria(Datos datos) {
  if (!isConnectionOK()) return;

  // 1️⃣ Obtener fecha y hora actual
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  char fechaBuffer[11];
  sprintf(fechaBuffer, "%04d-%02d-%02d",
          timeinfo.tm_year + 1900,
          timeinfo.tm_mon + 1,
          timeinfo.tm_mday);
  String fecha = String(fechaBuffer);  // ej: "2025-11-20"

  char horaBuffer[9];
  sprintf(horaBuffer, "%02d:%02d:%02d",
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec);
  String hora = String(horaBuffer);  // ej: "08:15:32"

  // 2️⃣ Calcular precio según cantidad
  float cantidad = abs(datos.PesoDispensar);
  float precioVenta = (cantidad / 0.5) * 10;

  // Limitar máximo a 2 de cantidad → 40
  if (cantidad > 2.0) precioVenta = 40;

  // 3️⃣ Preparar el path en Firebase
  String pathBase = "/registrosDiarios/" + fecha + "/" + hora;

  // 4️⃣ Crear JSON con los datos
  FirebaseJson json;
  json.set("Cantidad", abs(cantidad));
  json.set("Exitoso", datos.Exitoso == 1 ? true : false);
  json.set("PrecioVenta", abs(precioVenta));

  // 5️⃣ Guardar en Firebase
  if (Firebase.RTDB.setJSON(&fbdo, pathBase, &json)) {
    Serial.println("✅ Registro diario guardado: " + pathBase + " | Precio: " + String(precioVenta));
  } else {
    Serial.println("❌ Error al guardar registro diario: " + fbdo.errorReason());
  }
}


void verificarFirebase() {
  if (!Firebase.ready()) {
    Serial.println("⚠ Reintentando conexión Firebase...");
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  }
}

#include <HardwareSerial.h>

HardwareSerial lora(2); // UART2 (Serial2)

// ===== CONFIGURACIÓN =====
String myAddress = "2";       // Direcciones propia de cada ESP32
String targetAddress = "1";   // 
String loraBand = "915000000";
String loraNetworkID = "18";

// ===== VARIABLES =====
bool waitingAck = false;
unsigned long lastSendTime = 0;
const unsigned long ackTimeout = 2500; // 2.5 segundos
String lastMessage = "";
#define RX_LoRa 32
#define TX_LoRa 33

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  lora.begin(115200, SERIAL_8N1, RX_LoRa, TX_LoRa); // RX=16, TX=17

  delay(1500);
  sendAT("AT+BAND=" + loraBand);
  sendAT("AT+NETWORKID=" + loraNetworkID);
  sendAT("AT+ADDRESS=" + myAddress);

  Serial.println("🟢 ESP32 " + myAddress + " lista para comunicar.");
  Serial.println("Escribe un mensaje y presiona Enter para enviarlo:");
}

// ===== LOOP =====
void loop() {
  // ✅ Leer mensaje desde el monitor serial (Arduino IDE)
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      sendMessage(input);
    }
  }

  // ✅ Leer mensaje recibido desde LoRa
  if (lora.available()) {
    String response = lora.readStringUntil('\n');
    response.trim();

    if (response.startsWith("+RCV=")) {
      processIncoming(response);
    }
  }

}

// ===== FUNCIONES =====

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

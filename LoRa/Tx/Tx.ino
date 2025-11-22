#include <HardwareSerial.h>

HardwareSerial lora(2); // UART2 (Serial2)

// ===== CONFIGURACIÓN =====
String myAddress = "2";       // 👈 Cambia a "2" en la otra ESP32
String targetAddress = "1";   // 👈 Cambia a "1" en la otra ESP32
String loraBand = "915000000";
String loraNetworkID = "18";

// ===== VARIABLES =====
bool waitingAck = false;
unsigned long lastSendTime = 0;
const unsigned long ackTimeout = 2500; // 2.5 segundos
String lastMessage = "";


float Snr();


struct DatosMensaje {
  int PesoDispensar = 0;
  bool Dispensando = false;
  bool Exitoso = false;
  int Rpm = 0;

  float PorcentajeUltrasonico = 0.0;

  int AumentoEqui = 0;

  bool datosValidos = false; // Para saber si hubo algo válido
};



// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  lora.begin(115200, SERIAL_8N1, 32, 33); // RX=16, TX=17

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



//FUNCIONES 

// float Snr() {
//   return LoRa.packetSnr();
// }

// void ParametrosDeProceso(int PesoDispensar, bool Dispensando, bool Exitoso, int Rpm) {
  
//   // Crear el texto del mensaje en formato clave=valor
//   String mensaje = "PESO=" + String(PesoDispensar) +
//                    ";DISPENSANDO=" + String(Dispensando ? 1 : 0) +
//                    ";EXITOSO=" + String(Exitoso ? 1 : 0) +
//                    ";RPM=" + String(Rpm);

//   // Enviar el mensaje por LoRa
//   lora.beginPacket();
//   lora.print(mensaje);
//   lora.endPacket();

//   // (Opcional) Mostrar lo enviado por serial
//   Serial.print("Mensaje enviado: ");
//   Serial.println(mensaje);
// }


// float calcularPorcentajeUltrasonico(float distanciaCm) {
//   const float minDist = 2.0;   // Distancia mínima del HC-SR04
//   const float maxDist = 10.0;  // 10 cm representan 0%

//   if (distanciaCm > maxDist) distanciaCm = maxDist;
//   if (distanciaCm < minDist) distanciaCm = minDist;

//   float porcentaje = (maxDist - distanciaCm) / (maxDist - minDist) * 100.0;

//   return porcentaje;
// }

// float leerUltrasonico() {
//   digitalWrite(TRIG_PIN, LOW);
//   delayMicroseconds(2);
//   digitalWrite(TRIG_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIG_PIN, LOW);

//   long duracion = pulseIn(ECHO_PIN, HIGH);
//   float distancia = duracion * 0.0343 / 2.0; // cm

//   return distancia;
// }

// // void EnviarPorcentajeUltrasonico() {
// //   float distancia = leerUltrasonico();
// //   float porcentaje = calcularPorcentajeUltrasonico(distancia);

// //   // Crear mensaje en formato clave=valor
// //   String mensaje = "PORCENTAJE_ULTRA=" + String(porcentaje, 1);

// //   // Enviar por LoRa
// //   LoRa.beginPacket();
// //   LoRa.print(mensaje);
// //   LoRa.endPacket();

// //   Serial.print("Distancia cm: ");
// //   Serial.print(distancia);
// //   Serial.print("  -> Porcentaje: ");
// //   Serial.println(porcentaje);
// // }






// DatosMensaje ProcesarMensaje(String mensaje) {
//   DatosMensaje datos; // la estructura donde guardaremos todo
//   datos.datosValidos = true;

//   mensaje.trim();
//   int start = 0;

//   while (true) {
//     int pos = mensaje.indexOf(';', start);
//     String segmento;

//     if (pos == -1) {
//       segmento = mensaje.substring(start);
//     } else {
//       segmento = mensaje.substring(start, pos);
//     }

//     segmento.trim();

//     int eqPos = segmento.indexOf('=');
//     if (eqPos != -1) {
//       String clave = segmento.substring(0, eqPos);
//       String valor = segmento.substring(eqPos + 1);

//       clave.trim();
//       valor.trim();

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





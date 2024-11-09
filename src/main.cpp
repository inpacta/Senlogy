#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Pino do LED interno
#define ledPin 2  // D4 no NodeMCU

// Configurações do DHT
#define DHTPIN 4     // D2 do NodeMCU
#define DHTTYPE DHT22   // DHT 22

// Pino do LDR
#define LDRPIN A0  // Pino que está conectado ao LDR

// Instância do DHT22
DHT dht(DHTPIN, DHTTYPE);

// Configurações do WiFi
const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA_DO_WIFI";

// // Configurações do MQTT Broker
const char* mqttServer = "endereco_do_broker";
const int mqttPort = 1883;
const char* mqttUser = "usuario"; // Opcional
const char* mqttPassword = "senha"; // Opcional

// Tópico MQTT e mensagem
const char* tempTopic = "SEU/TOPICO";
const char* humidityTopic = "SEU/TOPICO";
const char* ldrTopic = "SEU/TOPICO";


WiFiClient espClient;
PubSubClient client(espClient);

void connectToWiFi() {
  Serial.println();
  Serial.print("Conectando ao WiFi");
  
  // Conecta-se à rede Wi-Fi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado ao WiFi");
}

void connectToMQTT() {
  Serial.print("Conectando ao MQTT Broker");

  // Configura o servidor MQTT
  client.setServer(mqttServer, mqttPort);

  // Tenta conectar
  while (!client.connected()) {
    Serial.print(".");
    
    if (client.connect(WiFi.macAddress().c_str(), mqttUser, mqttPassword)) {
      Serial.println();
      Serial.println("Conectado ao MQTT Broker");
       digitalWrite(ledPin, HIGH);  // Desliga o LED interno quando conectado ao broker
    } else {
      Serial.print(".");
      delay(5000);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    connectToMQTT();
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Configura o pino do LED como saída
  pinMode(ledPin, OUTPUT);

  // Configura o pino do LDR como entrada
  pinMode(LDRPIN, INPUT);

  // Inicializa o DHT
  dht.begin();

  connectToWiFi();
  connectToMQTT();

  Serial.println("Iniciando a coleta dos dados...");
  // Local onde o valor do ldr é lido
  int ldrValue = analogRead(LDRPIN);

  // Coleta a temperatura e umidade
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Verifica a leitura dos dados de temperatura e umidade
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Imprime os valores coletados
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" °C");
  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Luminosidade: ");
  Serial.println(ldrValue);


  Serial.println("Dados coletados com sucesso!");
  Serial.println("Enviando dados...");
  // Publica os dados coletados nos canais respectivos
  client.publish(tempTopic, String(t).c_str());
  client.publish(humidityTopic, String(h).c_str());
  client.publish(ldrTopic, String(ldrValue).c_str());

  Serial.println("Dados enviados!");

  // Piscar o LED enquanto envia a mensagem
  digitalWrite(ledPin, LOW);
  delay(100);
  digitalWrite(ledPin, HIGH);

  // Desconectar do broker mqtt
  Serial.println("Desconectando do broker...");
  client.disconnect();
  Serial.println("Desconectado do broker!");

  // Entra em modo de suspensão profunda por 30 minutos
  Serial.println("Entrando em modo de suspensão profunda...");
  ESP.deepSleep(30 * 60e6);
}

void loop() {
  // Não é necessário neste caso, pois o ESP8266 está em modo de suspensão profunda
} 

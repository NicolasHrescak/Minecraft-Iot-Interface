// Código adaptado de: <https://www.arduinoecia.com.br/como-usar-o-wifimanager-com-esp32-e-esp8266/>
#if defined(ESP8266)
//Bibliotecas ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#else
//Bibliotecas ESP32
#include <WiFi.h>
#include <WebServer.h>
#endif

//Bibliotecas comuns
#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

const char* REDE_ESP_NOME="NOME_ULTRA_CONFIAVEL";
const char* REDE_ESP_SENHA="confiabrother";


// Código adaptado de: <https://community.hivemq.com/t/hivemq-using-esp32-and-nodered/1291>
// URL DO CLUSTER DO HIVEMQ CLOUD
const char* MQTT_SERVER="";
// USERNAME DO CLIENT DO HIVEMQ CLOUD
const char* MQTT_USERNAME="";
// SENHA DO CLIENT DO HIVEMQ CLOUD
const char* MQTT_PASSWORD="";
const int MQTT_PORT=8883;

WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

const int pin_display[7]={13,35,14,27,26,25,33};

const int pin_chuva=34;
const int chuva_const=1400;
bool chuva_tgl=false;

void setup() {
  Serial.begin(115200);

  pinMode(pin_chuva, INPUT);

  int i;
  for(i=0;i<7;i++)
    pinMode(pin_display[i], OUTPUT);

  WiFiManager wifiManager;

  wifiManager.setConfigPortalTimeout(240);

  //Cria um AP (Access Point) com: ("nome da rede", "senha da rede")
  if (!wifiManager.autoConnect(REDE_ESP_NOME, REDE_ESP_SENHA)) {
    Serial.println(F("Falha na conexao. Resetar e tentar novamente..."));
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  //Mensagem caso conexao Ok
  Serial.println(F("Conectado na rede Wifi."));
  Serial.print(F("Endereco IP: "));
  Serial.println(WiFi.localIP());

  espClient.setInsecure();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  int chuva=analogRead(pin_chuva);

  //Serial.println(chuva);

  if(chuva>=chuva_const&&!chuva_tgl) {
    Serial.println("Está chuvendo!");
    publishMessage("chuva", "10", true);
    chuva_tgl=true;
  } else if(chuva<chuva_const&&chuva_tgl)
    chuva_tgl=false;

  delay(100);
}

void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    String clientId = "ESP32Client-"; // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");

      client.subscribe("display");
      client.subscribe("foto");
      client.subscribe("lamp1");
      client.subscribe("lamp2");
      client.subscribe("luz");
      client.subscribe("chuva");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//=======================================
// This void is called every time we have a message from the broker

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagens = "";
  for (int i = 0; i < length; i++)
    mensagens+=(char)payload[i];
  if(!strcmp(topic, "reset")) {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    Serial.println("Configuracoes zeradas!");
    ESP.restart();
  } else if(!strcmp(topic, "display")) {
    exibirNoDisplay( isInteger(mensagens) ? atoi(mensagens.c_str()) : -1);
  } else
    Serial.println("Message arrived ["+String(topic)+"]"+mensagens);
  // check for other commands
  /* else if( strcmp(topic,command2_topic) == 0){
  if (mensagens.equals("1")) { } // do something else
  }
  */
}

//======================================= publising as string
void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
    Serial.println("Message publised ["+String(topic)+"]: "+payload);
}

void exibirNoDisplay(int numero) {
  if(numero<0||numero>9) {
    Serial.print(F("Display tentou exibir "));
    Serial.print(numero);
    Serial.println(F(", um numero invalido!"));

    return;
  }
  Serial.print(F("Display: "));
  Serial.println(numero);
  const int ON=LOW, OFF=HIGH;
  digitalWrite(pin_display[0], numero!=1&&numero!=4 ? ON : OFF);
  digitalWrite(pin_display[1], numero!=5&&numero!=6 ? ON : OFF);
  digitalWrite(pin_display[2], numero!=2 ? ON : OFF);
  digitalWrite(pin_display[3], numero!=1&&numero!=4&&numero!=7 ? ON : OFF);
  digitalWrite(pin_display[4], numero==0||numero==2||numero==6||numero==8 ? ON : OFF);
  digitalWrite(pin_display[5], numero!=1&&numero!=2&&numero!=3&&numero!=7 ? ON : OFF);
  digitalWrite(pin_display[6], numero!=0&&numero!=1&&numero!=7 ? ON : OFF);
}

bool isInteger(String str) {
  if(!str.length())
    return false;
  int i;
  for(i=0;i<str.length();i++)
    if(!isdigit(str[i]))
      return false;
  return true;
}
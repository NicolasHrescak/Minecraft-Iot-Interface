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
#include <Keypad.h>

const char* REDE_ESP_NOME="NOME_CONFIAVEL";
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

// Código adaptado de: <https://arduinogetstarted.com/tutorials/arduino-keypad>
#define ROW_NUM     4
#define COLUMN_NUM  3

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte pin_rows[ROW_NUM] = {13, 12, 14, 27};
byte pin_column[COLUMN_NUM] = {26, 25, 33};

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const int pin_foto=34;
int foto_limite=500;
bool foto_estado=false;
bool foto_infoatual=false;

const int pin_btn1=22;
const int pin_btn2=23;

bool lamp1=false;
bool lamp2=false;

int lamp1tgl=false;
int lamp2tgl=false;

const int pin_lamp1=2;
const int pin_lamp2=15;
const int pin_lamp3=4;

void setup() {
  Serial.begin(115200);

  pinMode(pin_foto, INPUT);
  pinMode(pin_btn1, INPUT_PULLUP);
  pinMode(pin_btn2, INPUT_PULLUP);
  pinMode(pin_lamp1, OUTPUT);
  pinMode(pin_lamp2, OUTPUT);
  pinMode(pin_lamp3, OUTPUT);

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

  int foto = analogRead(pin_foto);
  int inputlamp1 = !digitalRead(pin_btn1);
  int inputlamp2 = !digitalRead(pin_btn2);
  char key = keypad.getKey();

  //Serial.println(inputlamp1);
  //Serial.println(inputlamp2);

  if(inputlamp1&&!lamp1tgl) {
    lamp1tgl=true;
    lamp1=!lamp1;
    //Serial.print("lamp1:");
    //Serial.println(lamp1 ? "1" : "0");
    publishMessage("lamp1", lamp1 ? "1" : "0", true);
  } else if(!inputlamp1)
    lamp1tgl=false;
  if(inputlamp2&&!lamp2tgl) {
    lamp2tgl=true;
    lamp2=!lamp2;
    //Serial.print("lamp2:");
    //Serial.println(lamp2 ? "1" : "0");
    publishMessage("lamp2", lamp2 ? "1" : "0", true);
  } else if(!inputlamp2)
    lamp2tgl=false;
  if(key) {
    if(key>='0'&&key<='9') {
      publishMessage("display", String(key), true);
    }
  }
  if((foto_estado&&foto>foto_limite)||(!foto_estado&&foto<=foto_limite)) {
    foto_estado=!foto_estado;
    String str="definir,";
    str+=(foto_estado ? "1" : "0");
    publishMessage("foto", str, true);
  }
  if(foto_infoatual) {
    foto_infoatual=false;
    String str="info,intensidade atual do fotorresistor: ";
    str+=String(foto);
    publishMessage("foto", str, true);
  }

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
    exibirNoDisplay( isdigit(mensagens[0]) ? atoi(&mensagens[0]) : -1);
  } else if(!strcmp(topic, "foto")) {
    String tipo=mensagens.substring(0, mensagens.indexOf(","));
    mensagens=mensagens.substring(mensagens.indexOf(",")+1,mensagens.length());
    String arg=mensagens.substring(0, mensagens.indexOf(","));
    if(!strcmp(tipo.c_str(),"limite")&&isInteger(arg)) {
      foto_limite=atoi(arg.c_str());
    } else if(!strcmp(tipo.c_str(),"limite?")) {
      String str="info,limite do fotorresistor: ";
      char num[5];
      itoa(foto_limite,num,10);
      str+=num;
      publishMessage("foto",str,true);
    } else if(!strcmp(tipo.c_str(), "intensidade?")) {
      foto_infoatual=true;
    }
    Serial.println(tipo);
  } else if(!strcmp(topic, "lamp1")) {
    if(isdigit(mensagens[0])) {
      String str="Lamp1 ";
      str+=mensagens[0]-'0' ? "acendeu" : "apagou";
      digitalWrite(pin_lamp1, mensagens[0]-'0' ? HIGH : LOW);
    }
  } else if(!strcmp(topic, "lamp2")) {
    if(isdigit(mensagens[0])) {
      String str="Lamp2 ";
      str+=mensagens[0]-'0' ? "acendeu" : "apagou";
      Serial.println(str);
      digitalWrite(pin_lamp2, mensagens[0]-'0' ? HIGH : LOW);
    }
  } else if(!strcmp(topic, "luz")) {
    if(strcmp(mensagens.c_str(), "0")) {
      Serial.println("Lamp 3 acendeu");
      digitalWrite(pin_lamp3, LOW);
    } else {
      Serial.println("Lamp 3 apagou");
      digitalWrite(pin_lamp3, HIGH);
    }
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
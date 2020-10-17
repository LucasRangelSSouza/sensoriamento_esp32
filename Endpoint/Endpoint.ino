
/* Codigo fonte para o microcontrolador ESP32 Lora Wifi (Wifi Dev Kit V2) da marca Heltec
    Para desempenhar o papel de endpoint

    Lucas Rangel , Instituto Senai de Tecnologia em  Automaçao, Goiania - GO,  Brasil
    Editado em: 06/06/2020

    Mapa de funçoes:
    SETTERS
      void setSsidAp(String ssid) {
      void setSenhaAp(String senhaAp) {
      void setPrintSerial(String printSerial) {
      void setDisplaySempreLigado(String displaySempreLigado) {
      void setCliente(String  cliente) {
      void setEndpoint(String  endpoint) {
      void setGateway(String gateway) {
      void setSensoresHabilitados(String sensor, String state) {
      void setDataHora(int dia, int mes, int ano, int hora, int minuto, int segundo) {
      void setAmostragem(String tipeSensor, String valor) {
      void setProxAmostragem(String tipeSensor, String valor) {
      void setUltimoArquivoEnviado(String idUltimoArquivoEnviado) {
      void setUltimoArquivoSalvo(String idUltimoArquivoSalvo) {
      void setId(String id) {
      void setRestartPadrao(String estado) {
      void setModoWifi(String modo) {
    GETTERS
      String getSsidAp() {
      String getSenhaAp() {
      void getDisplaySempreLigado() {
      String getID() {
      String getCliente() {
      String getEndpoint() {
      String getGateway() {
      String getAmostragem(String tipeSensor) {
      String getSensorHabilitado(String sensor) {
      String getProxAmostragem(String tipeSensor) {
      String getUltimoArquivoEnviado() {
      String getUltimoArquivoSalvo() {
      String getRestartPadrao() {
      String getDataHora(String tipo = "normal") {
      void criaEstruturaInicial() {
    REPROGRAMAÇAO
      void checaConfiguracoesPendentes() {
      bool processaReprogramacao(String mensagemRecebida) {
    LORA
      void iniciaLora() {
      void desligaLora() {
      bool enviaMsgLora(String mensagem) {
      String recebeMsgLora(bool lerAteReceber = false ) {
    SENSORES
       String getCorrenteAC() {
       String getTensaoAC() {
       String getTemperatura() {
       String getUmidade() {
       String getCO2() {
       String getTVOC() {
       String getBateria() {
    VALIDAÇAO SENSORES
      bool sensorHabilitado(String sensor) {
      bool realizarLeitura(String sensor) {
    WIFI AP
      void configWifiManager() {
      void notFound(AsyncWebServerRequest *request) {
    WIFI AP
      void configWifiManager() {
      void notFound(AsyncWebServerRequest *request) {
    PONTO DE ACESSO
      void configWifiManager() {
      void notFound(AsyncWebServerRequest *request) {
    DISPLAY
      void IRAM_ATTR mostrarDisplay() {
      void showDisplay() {
    INICIALIZAÇAO
      void configInicialSensores() {
      void setup() {
    EXECUÇAO DA ROTINA
      void loop() {
    DADOS DOS SENSORES
      bool enviaDados() {
      void salvaDados(String sensor, String valor, String unMedida, String dataHora) {
      bool processaMsgRecebida(String mensagem, String tipoDeRespostaEsperado) {
*/

#include "EmonLib.h"            //Biblioteca para realizar Calculos de Corrente e Tensao
#include "Adafruit_CCS811.h"    //Biblioteca para comunicaçao com o sensor CSS811(Co2 e TVOC)
#include "DHT.h"                //Biblioteca para comunicaçao com o sensor DHT22 (Temperatura  e umidade)
#include <Wire.h>               //Biblioteca para comunicaçao I2C 
#include "RTClib.h"             //Biblioteca para comunicaçao com o modulo RTC (Real time clock) 
#include <Flash.h>              //Biblioteca para montar e utilizar o armazenamento flash em tempo de execulçao
#include <time.h>               //Biblioteca para realizar conversao de dateTime para unix datetime e vice e versa
#include <SPI.h>                //Biblioteca para comunicaçao SPI (Modulo lora e wifi)
#include <LoRa.h>               //Biblioteca para comunicaçao com o modulo Lora
#include <Adafruit_GFX.h>       //Biblioteca para comunicaçao com o Display oled
#include <Adafruit_SSD1306.h>   //Biblioteca para comunicaçao com o Display oled
#include <Arduino.h>            //Biblioteca com funçoes de proposito geral do arduino
#include "rom/gpio.h"           //Biblioteca para auxiliar no controle das GPIO`s de embarcados da expressif na Arduino IDE
#include <WiFi.h>               //Biblioteca para comunicaçao com o modulo  Wifi
#include <ESPAsyncWebServer.h>  //Biblioteca para criaçao e controle de um web server assincrono
#include <ArduinoJson.h>        //Biblioteca para serializaçao e deserializaçao Json
#include "esp_sleep.h"          //Biblioteca para controlar o sono profundo (deep sleep)


/*
   Pinagem do  modulo Lora
*/
#define SCK_LORA 5
#define MISO_LORA 19
#define MOSI_LORA 27
#define RESET_PIN_LORA 14
#define SS_PIN_LORA 18


/*
   definiçoes do  display Oled
*/
#define OLED_LINE1 0
#define OLED_LINE2 10
#define OLED_LINE3 20
#define OLED_LINE4 30
#define OLED_LINE5 40
#define OLED_LINE6 50
#define OLED_LINE7 60
#define OLED_ADDR 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define VOLT_CAL 288.9     // Variavel para calibraçao digital  do sensor de tensao ZMTP
#define CUR_CAL 2.59252     //Variavel para calibraçao digital  do sensor de corrente SCT

bool DEBUGS_PRINTS = true;  //Set como true para exibir um  log  de desempenho via serial
int pino_zmtp = 36;         //Pino
int pino_sct = 12;
int pino_dht = 13 ;
int pino_botao = 23;


DHT dht(pino_dht, DHT22);
EnergyMonitor emon1; //CRIA UMA INSTÂNCIA
Adafruit_CCS811 ccs;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 16);
TaskHandle_t task_mostrarDisplay;
volatile bool ligaDisplay = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile int sinalInterrupcao = 0;
int volatile opcaoDisplay = 0;

RTC_DS3231 rtc;
Flash flash;

boolean displayAlwaysOn = false;
AsyncWebServer server(80);
bool newData = false;




//--------- Variaveis globais
String nomeEndpoint;
String nomeGateway;
String nomeCliente;
String tensaoHabilidado;
String correnteHabilidado;
String temperaturaHabilidado;
String ummidadeHabilidado;
String co2Habilidado;
String tvocHabilidado;
String tensaoAmostragem;
String correnteAmostragem;
String temperaturaAmostragem;
String umidadeAmostragem;
String co2Amostragem;
String tvocAmostragem;
String ssid;
String senhaAp;
String loginMenu ;
String senhaMenu ;
String printSerial;
String displaySempreLigado;
String novaData;
String newHour;
bool hardReset = false;
String ultimaTensao = "0";
String ultimaCorrente = "0";
String ultimaTemperatura = "0";
String ultimaUmidade = "0";
String ultimaCo2 = "0";
String ultimaTvoc = "0";
String ultimaDataHora = "01/01/0001 00:00:00";
String modoAccessPoint = "auto";





/* ----------------------------------------GETTER E SETTERS----------------------------------------
   Este modulo contem Todas funçoes que manipulam as variaveis salvas permanentemente na flash do microcontrolador

*/

/*
  ---------------------------------SETTERS-----------------------------------------
*/
void setSsidAp(String ssid) {
  /*
    Esta funçao salva o ssid do ponto de acesso em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setSsidAp(" + ssid + ")");
  flash.salvarArquivo("/ssidAP.txt", ssid);
}

void setSenhaAp(String senhaAp) {
  /*
    Esta funçao salva a senha do ponto de acesso em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setSenhaAp(" + senhaAp + ")");
  flash.salvarArquivo("/senhaAP.txt", senhaAp);
}


void setPrintSerial(String printSerial) {
  /*
    Esta funçao salva na flash se deve printar todas as informaçoes de debug na serial
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setPrintSerial(" + printSerial + ")");
  if (printSerial == "true") {
    DEBUGS_PRINTS = true;
  } else {
    DEBUGS_PRINTS = false;
  }
}

void setDisplaySempreLigado(String displaySempreLigado) {
  /*
    Esta funçao salva se o display deve sempre estar ativado, em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setDisplaySempreLigado(" + displaySempreLigado + ")");
  if (displaySempreLigado == "true") {
    displayAlwaysOn = true;
    flash.salvarArquivo("/displaySempreLigado.txt", "true");
  } else {
    flash.salvarArquivo("/displaySempreLigado.txt", "false");
    displayAlwaysOn = false;
  }
}


void setCliente(String  cliente) {
  /*
    Esta funçao salva o nome do cliente em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setCliente(" + cliente + ")");
  flash.salvarArquivo("/cliente.txt", cliente);

}

void setEndpoint(String  endpoint) {
  /*
    Esta funçao salva o codigo/id do endpoint em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setEndpoint(" + endpoint + ")");
  flash.salvarArquivo("/endpoint.txt", endpoint);

}

void setGateway(String gateway) {
  /*
    Esta funçao salva o id/codigo do ponto do gateway receptor em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setGateway(" + gateway + ")");
  flash.salvarArquivo("/gateway.txt", gateway);

}


void setSensoresHabilitados(String sensor, String state) {
  /*
    Esta funçao salva o estado dos sensores (habilitado ou desabilitado) em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: SensoresHabilitados(" + sensor + "," + state + ")");

  if (sensor.equals("tensao")) {
    flash.salvarArquivo("/tensao-ativo.txt", state);
    return;
  }

  if (sensor.equals("corrente")) {
    flash.salvarArquivo("/corrente-ativo.txt", state);
    return;
  }

  if (sensor.equals("temperatura")) {
    flash.salvarArquivo("/temperatura-ativo.txt", state);
    return;
  }

  if (sensor.equals("umidade")) {
    flash.salvarArquivo("/umidade-ativo.txt", state);
    return;
  }

  if (sensor.equals("co2")) {
    flash.salvarArquivo("/co2-ativo.txt", state);
    return;
  }

  if (sensor.equals("tvoc")) {
    flash.salvarArquivo("/tvoc-ativo.txt", state);
    return;
  }


}

void setDataHora(int dia, int mes, int ano, int hora, int minuto, int segundo) {
  /*
    Esta funçao atualiza a data e a hora no modulo RTC
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setDataHora(" + String(dia) + "/" + String(mes) + "/" + String(ano) + " " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ")");
  rtc.adjust(DateTime(ano, mes, dia, hora, minuto, segundo)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
}


void setAmostragem(String tipeSensor, String valor) {
  /*
    Esta funçao salva a nova taxa de amostragem dos sensores em um arquivo txt na flash do microcontrolador
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setAmostragem(" + tipeSensor + "," + valor + ")");

  if (tipeSensor.equals("tensao")) {
    flash.salvarArquivo("/tensao-amostragem.txt", valor);
    flash.salvarArquivo("/tensao-proxLeitura.txt", "0");
  }
  if (tipeSensor.equals("corrente")) {
    flash.salvarArquivo("/corrente-amostragem.txt", valor);
    flash.salvarArquivo("/corrente-proxLeitura.txt", "0");
  }
  if (tipeSensor.equals("temperatura")) {

    flash.salvarArquivo("/temperatura-amostragem.txt", valor);
    flash.salvarArquivo("/temperatura-proxLeitura.txt", "0");
  }
  if (tipeSensor.equals("umidade")) {
    flash.salvarArquivo("/umidade-amostragem.txt", valor);
    flash.salvarArquivo("/umidade-proxLeitura.txt", "0");
  }
  if (tipeSensor.equals("co2")) {
    flash.salvarArquivo("/co2-amostragem.txt", valor);
    flash.salvarArquivo("/co2-proxLeitura.txt", "0");
  }
  if (tipeSensor.equals("tvoc")) {
    flash.salvarArquivo("/tvoc-amostragem.txt", valor);
    flash.salvarArquivo("/tvoc-proxLeitura.txt", "0");
  }

}

void setProxAmostragem(String tipeSensor, String valor) {
  /*
    Esta funçao salva o a data em unix da proxima leitura dos sensores em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setProxAmostragem(" + tipeSensor + "," + valor + ")");

  if (tipeSensor.equals("tensao")) {
    flash.salvarArquivo("/tensao-proxLeitura.txt", valor);

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Callback: setProxAmostragem(" + tipeSensor + "," + valor + ")");

    return;
  }
  if (tipeSensor.equals("corrente")) {
    flash.salvarArquivo("/corrente-proxLeitura.txt", valor);
    return;
  }
  if (tipeSensor.equals("temperatura")) {
    flash.salvarArquivo("/temperatura-proxLeitura.txt", valor);
    return;
  }
  if (tipeSensor.equals("umidade")) {
    flash.salvarArquivo("/umidade-proxLeitura.txt", valor);
    return;
  }
  if (tipeSensor.equals("co2")) {
    flash.salvarArquivo("/co2-proxLeitura.txt", valor);
    return;
  }
  if (tipeSensor.equals("tvoc")) {
    flash.salvarArquivo("/tvoc-proxLeitura.txt", valor);
    return;
  }

}

void setUltimoArquivoEnviado(String idUltimoArquivoEnviado) {
  /*
    Esta funçao salva o id do ultimo arquivo de leitura enviado via lora, em um arquivo txt na flash
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setUltimoArquivoEnviado(" + idUltimoArquivoEnviado + ")");
  flash.salvarArquivo("/ultimoArquivoEnviado.txt", idUltimoArquivoEnviado);

}

void setUltimoArquivoSalvo(String idUltimoArquivoSalvo) {

  /*
    Esta funçao salva o id do ultimo arquivo de leitura salvo localmente em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setUltimoArquivoSalvo(" + idUltimoArquivoSalvo + ")");
  flash.salvarArquivo("/ultimoArquivoSalvo.txt", idUltimoArquivoSalvo);

}

void setId(String id) {
  /*
    Esta funçao salva o numero do id do ultimo arquivo salvo localmente, em arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setId(" + id + ")");
  flash.salvarArquivo("/id.txt", id);

}

void setRestartPadrao(String estado) {
  /*
    Esta funçao salva se o motivo do microcontrolador reiniciar foi proposital ou houve um erro
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setRestartPadrao(" + estado + ")");
  flash.salvarArquivo("/restartPadrao.txt", estado);

}

void setModoWifi(String modo) {
  /*
    Esta funçao salva o modo de operaçao do wifi em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: setModoWifi(" + modo + ")");
  if (modo.equals("ligado")) {
    flash.salvarArquivo("/modoWifi.txt", "ligado");
    return;
  }
  if (modo.equals("auto")) {
    flash.salvarArquivo("/modoWifi.txt", "desligado");
    return;
  }
  if (modo.equals("auto")) {
    flash.salvarArquivo("/modoWifi.txt", "auto");
    return;
  }
  flash.salvarArquivo("/modoWifi.txt", "auto");
  return;

}



/*
  ---------------------------------GETTERS-----------------------------------------
*/

String getSsidAp() {
  /*
     Esta funçao retorna o ssid do ponto de acesso, salvo posteriormente em um arquivo txt na flash
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getSsidAp()");
  String ssidAp = flash.lerArquivo("/ssidAP.txt");
  return ssidAp;
}



String getSenhaAp() {
  /*
     Esta funçao retorna a senha do ponto de acesso, salvo posteriormente em um arquivo txt na flash
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getSenhaAp()");
  String senhaAp = flash.lerArquivo("/senhaAP.txt");
  return senhaAp;
}

void getDisplaySempreLigado() {
  /*
     Esta funçao retorna se o display deve ficar sempre ativo, salvo posteriormente em um arquivo txt na flash
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getDisplaySempreLigado()");
  String dsp =  flash.lerArquivo("/displaySempreLigado.txt");
  if (dsp == "true") {
    displayAlwaysOn = true;
  } else {
    displayAlwaysOn = false ;
  }
}
String getID() {
  /*
     Esta funçao retorna o id do ultimo arquivo salvo localmente, salvo posteriormente em um arquivo txt na flash
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getID()");

  long id = flash.lerArquivo("/id.txt").toInt();
  String proxId = String(id + 1);
  setId(proxId);
  return String(id);

}

String getCliente() {
  /*
     Esta funçao retorna o id/codigo do cliente, salvo posteriormente em um arquivo txt na flash
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getCliente()");
  String cliente = flash.lerArquivo("/cliente.txt");
  return cliente;
}

String getEndpoint() {
  /*
     Esta funçao retorna o id/codigo do endpoint, salvo posteriormente em um arquivo txt na flash
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getEndpoint()");
  String endpoint = flash.lerArquivo("/endpoint.txt");
  return endpoint;
}

String getGateway() {
  /*
     Esta funçao retorna o id/codigo do gateway receptor, salvo posteriormente em um arquivo txt na flash
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getGateway()");
  String gateway = flash.lerArquivo("/gateway.txt");
  return gateway;
}

String getAmostragem(String tipeSensor) {
  /*
    Está funçao retorna a taxa de amostragem do sensor passado como argumento
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getAmostragem(" + tipeSensor + ")");

  if (tipeSensor.equals("tensao")) {
    String amostragem = String(flash.lerArquivo("/tensao-amostragem.txt"));
    return amostragem;
  }
  if (tipeSensor.equals("corrente")) {
    String amostragem =  String(flash.lerArquivo("/corrente-amostragem.txt"));
    return amostragem;
  }
  if (tipeSensor.equals("temperatura")) {
    String amostragem =  String(flash.lerArquivo("/temperatura-amostragem.txt"));
    return amostragem;
  }
  if (tipeSensor.equals("umidade")) {
    String amostragem =  String(flash.lerArquivo("/umidade-amostragem.txt"));
    return amostragem;
  }
  if (tipeSensor.equals("co2")) {
    String amostragem =  String(flash.lerArquivo("/co2-amostragem.txt"));
    return amostragem;
  }
  if (tipeSensor.equals("tvoc")) {
    String amostragem =  String(flash.lerArquivo("/tvoc-amostragem.txt"));
    return amostragem;
  }
  return "0";
}

String getSensorHabilitado(String sensor) {
  /*
    Está funçao retorna se o sensor passado como argumento esta ativo ou nao
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getSensorHabilitado(" + sensor + ")");

  if (sensor.equals("tensao")) {
    String estadoSensor = flash.lerArquivo("/tensao-ativo.txt"); 

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Estado do Sensor: " + sensor + " é: "+estadoSensor);
 
    return estadoSensor;
  }

  if (sensor.equals("corrente")) {
    String estadoSensor = flash.lerArquivo("/corrente-ativo.txt"); 

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Estado do Sensor: " + sensor + " é: "+estadoSensor);
 
    return estadoSensor;
  }

  if (sensor.equals("temperatura")) {
    String estadoSensor = flash.lerArquivo("/temperatura-ativo.txt"); 

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Estado do Sensor: " + sensor + " é: "+estadoSensor);
 
    return estadoSensor;
  }

  if (sensor.equals("umidade")) {
    String estadoSensor = flash.lerArquivo("/umidade-ativo.txt"); 

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Estado do Sensor: " + sensor + " é: "+estadoSensor);
 
    return estadoSensor;
  }

  if (sensor.equals("co2")) {
    String estadoSensor = flash.lerArquivo("/co2-ativo.txt"); 

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Estado do Sensor: " + sensor + " é: "+estadoSensor);
 
    return estadoSensor;
  }

  if (sensor.equals("tvoc")) {
    String estadoSensor = flash.lerArquivo("/tvoc-ativo.txt"); 

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Estado do Sensor: " + sensor + " é: "+estadoSensor);
 
    return estadoSensor;
  }


}

String getProxAmostragem(String tipeSensor) {
  /*
    Está funçao retorna a taxa de amostragem do sensor passado como argumento
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getProxAmostragem(" + tipeSensor + ")");

  if (tipeSensor.equals("tensao")) {
    String proxAmostragem = flash.lerArquivo("/tensao-proxLeitura.txt");
    return proxAmostragem;
  }
  if (tipeSensor.equals("corrente")) {
    String proxAmostragem = flash.lerArquivo("/corrente-proxLeitura.txt");
    return proxAmostragem;
  }
  if (tipeSensor.equals("temperatura")) {
    String proxAmostragem = flash.lerArquivo("/temperatura-proxLeitura.txt");
    return proxAmostragem;
  }
  if (tipeSensor.equals("umidade")) {
    String proxAmostragem = flash.lerArquivo("/umidade-proxLeitura.txt");
    return proxAmostragem;
  }
  if (tipeSensor.equals("co2")) {
    String proxAmostragem = flash.lerArquivo("/co2-proxLeitura.txt");
    return proxAmostragem;
  }
  if (tipeSensor.equals("tvoc")) {
    String proxAmostragem = flash.lerArquivo("/tvoc-proxLeitura.txt");
    return proxAmostragem;
  }

}

String getUltimoArquivoEnviado() {
  /*
    Está funçao retorna o id/codigo do ultimo arquivo enviado via lora
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getUltimoArquivoEnviado()");
  String ultimoEnviado = flash.lerArquivo("/ultimoArquivoEnviado.txt");
  return ultimoEnviado;
}

String getUltimoArquivoSalvo() {
  /*
    Está funçao retorna o id/codigo do ultimo arquivo saalvo localmente
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getUltimoArquivoSalvo()");
  String ultimoSalvo = flash.lerArquivo("/ultimoArquivoSalvo.txt");
  return ultimoSalvo;
}

String getRestartPadrao() {
  /*
    Está funçao retorna o motivo do ultimo reinicio padrao
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getRestartPadrao()");
  String padrao = flash.lerArquivo("/restartPadrao.txt");
  return padrao;

}


String getDataHora(String tipo = "normal") {

  /*
     Retorna uma string com a data e a hora de acordo com o tipo passado como paramentro
     Se tipo for "normal" retorna uma string no formato "dd/mm/aaaa hh:mm:ss"
     Se tipo for "unix" retorna uma string com a data em unix (segundos contados desde 1 de janeiro de 1970 às 00:00:00 do Tempo Universal)
     O formato unix é amplamente utilizado no sketch pois nao e necessario realizar validaçoes constantes pertinentes a data e hora.
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getDataHora(" + tipo + ")");

  //Obtem os dados do RTC
  DateTime now = rtc.now();

  //Verifica se foi solicitado a hora no formato de string "dd/mm/aa hh:MM:ss"
  if (tipo.equals("normal")) {

    //cria uma string separada para cada dado a ser recebido
    String dia = "";
    String mes = "";
    String ano = "";
    String hora = "";
    String minuto = "";
    String segundo = "";

    //Verifica se o dia é menor que 10
    if (now.day() < 10) {

      //Se for menor que 10 adiciona um 0 a esquerda e salva na variavel
      dia += "0" + String(now.day(), DEC);
    } else {

      //Se for maior que 10 somente salva na variavel
      dia += String(now.day(), DEC);
    }

    //Verifica se o mes é menor que 10
    if (now.month() < 10) {

      //Se for menor que 10 adiciona um 0 a esquerda e salva na variavel
      mes += "0" + String(now.month(), DEC);
    } else {

      //Se for maior que 10 somente salva na variavel
      mes += String(now.month(), DEC);
    }

    //Verifica se o ano é menor que 10
    if (now.year() < 10) {

      //Se for menor que 10 adiciona um 0 a esquerda e salva na variavel
      ano += "0" + String(now.year(), DEC);
    } else {

      //Se for maior que 10 somente salva na variavel
      ano += String(now.year(), DEC);
    }

    //Verifica se a hora é menor que 10
    if (now.hour() < 10) {

      //Se for menor que 10 adiciona um 0 a esquerda e salva na variavel
      hora += "0" + String(now.hour(), DEC);
    } else {

      //Se for maior que 10 somente salva na variavel
      hora += String(now.hour(), DEC);
    }

    //Verifica se o minuto é menor que 10
    if (now.minute() < 10) {

      //Se for menor que 10 adiciona um 0 a esquerda e salva na variavel
      minuto += "0" + String(now.minute(), DEC);
    } else {

      //Se for maior que 10 somente salva na variavel
      minuto += String(now.minute(), DEC);
    }

    //Verifica se o segundo é menor que 10
    if (now.second() < 10) {

      //Se for menor que 10 adiciona um 0 a esquerda e salva na variavel
      segundo += "0" + String(now.second(), DEC);
    } else {

      //Se for maior que 10 somente salva na variavel
      segundo += String(now.second(), DEC);
    }

    //Cria a string formatada em "dd/mm/aa hh:MM:ss"
    String dataHora = dia + "-" + mes + "-" + ano + " " + hora + ":" + minuto + ":" + segundo;

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("DataHora: ");
    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(dataHora);

    //retorna a sting formatada em "dd/mm/aa hh:MM:ss"
    return dataHora;

  }

  //Se o formato da data solicitado for do tipo unix, segundo desde 1 de janeiro de 1970
  if (tipo == "unix") {

    //Cria uma strutura do tipo tm (time) com o nome t
    struct tm t;

    //Cria uma variavel do tipo time_t com o nome t_of_day
    time_t t_of_day;

    //Salva os dados do RTC em cadaa variavel da struct t
    t.tm_year = now.year();
    t.tm_mon = now.month();
    t.tm_mday = now.day();
    t.tm_hour = now.hour();
    t.tm_min = now.minute();
    t.tm_sec = now.second();

    //Informa que nao é horario de verao
    t.tm_isdst = 0;

    //Converte a strutura t em uma variavel do  tipo t_of_day
    t_of_day = mktime(&t);

    //Converte o tipo t_of_day em long e salva em unixDataTime
    long unixDatatime = (long) t_of_day;

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("UnixDataHora: ");
    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(unixDatatime);

    //retorna a string unixDataTime
    return String(unixDatatime);


  }

  //Em caso de errro fatal retorna 0, esse codigo so sera execultado se houver um defeito fisico com o rtc
  return "0";
}

void criaEstruturaInicial() {
  /*
    Está funçao verifica se exite a estrutura de arquivos necessarias para o funcionamento do endpoint
    se nao houver, cria
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: criaEstruturaInicial()");

  //obtem os dados se a estrutura foi criada
  String estruturaCriada = flash.lerArquivo("/estrutura.txt");

  //Se for retornado que a estrutura nao esta criada, cria a estrutura de arquivos com seus valores padroes
  if (!estruturaCriada.equals("EstruturaOK")) {

    //Dados do embarcado
    setSsidAp("Endpoint");
    setSenhaAp("123456789");
    setDisplaySempreLigado("false");
    setCliente("ClientePadrao");
    setEndpoint("00001");
    setGateway("00001");

    //Sensores habilitados
    setSensoresHabilitados("tensao", "true");
    setSensoresHabilitados("corrente", "true");
    setSensoresHabilitados("temperatura", "true");
    setSensoresHabilitados("umidade", "true");
    setSensoresHabilitados("co2", "true");
    setSensoresHabilitados("tvoc", "true");

    //Taxa de amostragem dos sensores
    setAmostragem("tensao", "900000");
    setAmostragem("corrente", "900000");
    setAmostragem("temperatura", "900000");
    setAmostragem("umidade", "900000");
    setAmostragem("co2", "900000");
    setAmostragem("tvoc", "900000");

    //Atualiza o id dos dados salvos localmente
    setId("0");
    setModoWifi("auto");

    //Salva a variavel estrutura.txt informando que a estrutura foi atualizada
    flash.salvarArquivo("/estrutura.txt", "EstruturaOK");


  }
}
/* ----------------------------------------REPROGRAMAÇAO----------------------------------------
   Este modulo modulo contem todas as funçoes que tratam a reprogramaçao OTA do endpoint
   ou seja, reprogramaçao em tempo de execuçao, tanto do wifi manager quanto reprogramaçao
   remota recebida do gateway

*/
void checaConfiguracoesPendentes() {
  /*
    Esta funçao verifica o status da variavel global newData se  for true, pega os dados das variaveis globais
    de cada dado e atualiza na flash do microcontrolador
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: checaConfiguracoesPendentes()");

  //Se newData estiver sinalizado como true
  if (newData) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("New data reconhecido");

    //Se a variavel global nomeEndpoint nao estiver em branco
    if (!nomeEndpoint.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setEndpoint(nomeEndpoint);

      //Deixo a variavel global em branco
      nomeEndpoint = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!nomeGateway.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setGateway(nomeGateway);

      //Deixo a variavel global em branco
      nomeGateway = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!correnteHabilidado.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setSensoresHabilitados("corrente", correnteHabilidado);

      //Deixo a variavel global em branco
      correnteHabilidado = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!tensaoHabilidado.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setAmostragem("tensao", tensaoHabilidado);

      //Deixo a variavel global em branco
      tensaoHabilidado = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!temperaturaHabilidado.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setSensoresHabilitados("temperatura", temperaturaHabilidado);

      //Deixo a variavel global em branco
      temperaturaHabilidado = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!ummidadeHabilidado.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setSensoresHabilitados("umidade", ummidadeHabilidado);

      //Deixo a variavel global em branco
      ummidadeHabilidado = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!co2Habilidado.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setSensoresHabilitados("co2", co2Habilidado);

      //Deixo a variavel global em branco
      co2Habilidado = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!tvocHabilidado.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setSensoresHabilitados("tvoc", tvocHabilidado);

      //Deixo a variavel global em branco
      tvocHabilidado = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!correnteAmostragem.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setAmostragem("corrente", correnteAmostragem);

      //Deixo a variavel global em branco
      correnteAmostragem = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!tensaoAmostragem.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setAmostragem("tensao", tensaoAmostragem);

      //Deixo a variavel global em branco
      tensaoAmostragem = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!temperaturaAmostragem.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setAmostragem("temperatura", temperaturaAmostragem);

      //Deixo a variavel global em branco
      temperaturaAmostragem = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!umidadeAmostragem.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setAmostragem("umidade", umidadeAmostragem);

      //Deixo a variavel global em branco
      umidadeAmostragem = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!co2Amostragem.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setAmostragem("co2", co2Amostragem);

      //Deixo a variavel global em branco
      co2Amostragem = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!tvocAmostragem.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setAmostragem("tvoc", tvocAmostragem);

      //Deixo a variavel global em branco
      tvocAmostragem = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!nomeCliente.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setCliente(nomeCliente);

      //Deixo a variavel global em branco
      nomeCliente = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!ssid.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setSsidAp(ssid);

      //Deixo a variavel global em branco
      ssid = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!senhaAp.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setSenhaAp(senhaAp);

      //Deixo a variavel global em branco
      senhaAp = "";
    }
    //Se a variavel global  nao estiver em branco
    if (!printSerial.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setPrintSerial(printSerial);

      //Deixo a variavel global em branco
      printSerial = "";
    }

    //Se a variavel global  nao estiver em branco
    if (!displaySempreLigado.equals("")) {

      //Atualizo o dado na flash do microcontrolados
      setDisplaySempreLigado(displaySempreLigado);

      //Deixo a variavel global em branco
      displaySempreLigado = "";
    }

    //Verifica se a data sera atualizada, porem nao a hora
    if ((!novaData.equals("")) && (newHour.equals(""))) {

      //Obtem a data e a hora atual
      String dataAtual = getDataHora();

      //Extrai o dia
      int dia = (String(novaData.charAt(8)) + String(novaData.charAt(9))).toInt();

      //Extrai o mes
      int mes = (String(novaData.charAt(5)) + String(novaData.charAt(6))).toInt();

      //Extrai o ano
      int ano = (String(novaData.charAt(0)) + String(novaData.charAt(1)) + String(novaData.charAt(2)) + String(novaData.charAt(3))).toInt();

      //Extrai a hora
      int hora = (String(dataAtual.charAt(0)) + String(dataAtual.charAt(1))).toInt();

      //Extrai o minuto
      int minuto = (String(dataAtual.charAt(3)) + String(dataAtual.charAt(4))).toInt();

      //Extrai o segundo
      int segundo = (String(dataAtual.charAt(6)) + String(dataAtual.charAt(7))).toInt();

      //Limpa a variavel global de data
      novaData = "";

      //Atualiza o rtc com a data e a hora passada
      setDataHora(dia, mes, ano, hora, minuto, segundo);
    }

    //Verifica se a hora sera atualizada, porem nao a data
    if ((!newHour.equals("")) && (novaData.equals(""))) {

      //Obtem a data e a hora atual
      String dataAtual = getDataHora();

      //Extrai o dia
      int dia = (String(dataAtual.charAt(0)) + String(dataAtual.charAt(1))).toInt();

      //Extrai o mes
      int mes = (String(dataAtual.charAt(3)) + String(dataAtual.charAt(4))).toInt();

      //Extrai o ano
      int ano = (String(dataAtual.charAt(6)) + String(dataAtual.charAt(7)) + String(dataAtual.charAt(8)) + String(dataAtual.charAt(9))).toInt();

      //Extrai a hora
      int hora = (String(newHour.charAt(0)) + String(newHour.charAt(1))).toInt();

      //Extrai o minuto
      int minuto = (String(newHour.charAt(3)) + String(newHour.charAt(4))).toInt();

      //Extrai o segundo
      int segundo = (String(newHour.charAt(6)) + String(newHour.charAt(7))).toInt();

      //Limpa a variavel global de hora
      newHour = "";

      //Atualiza o rtc com a data e a hora passada
      setDataHora(dia, mes, ano, hora, minuto, segundo);
    }

    //Verifica se a data e a hora sera atualizada
    if ((!novaData.equals("")) && (!novaData.equals(""))) {

      //Extrai o dia
      int dia = (String(novaData.charAt(8)) + String(novaData.charAt(9))).toInt();

      //Extrai o mes
      int mes = (String(novaData.charAt(5)) + String(novaData.charAt(6))).toInt();

      //Extrai o ano
      int ano = (String(novaData.charAt(0)) + String(novaData.charAt(1)) + String(novaData.charAt(2)) + String(novaData.charAt(3))).toInt();

      //Extrai a hora
      int hora = (String(newHour.charAt(0)) + String(newHour.charAt(1))).toInt();

      //Extrai o minuto
      int minuto = (String(newHour.charAt(3)) + String(newHour.charAt(4))).toInt();

      //Extrai o segundo
      int segundo = (String(newHour.charAt(6)) + String(newHour.charAt(7))).toInt();

      //Limpa as variaveis globais  de data e hora
      newHour = "";
      novaData = "";

      //Atualiza o rtc com a data e a hora passada
      setDataHora(dia, mes, ano, hora, minuto, segundo);
    }

    //Atualiza newData indicando que nao ha mais dados pendentes de atualizaçao
    newData = false;

  }

}



bool processaReprogramacao(String mensagemRecebida) {
  /*
    Essa funçao processa a reprogramaçao de uma mensagem recebisa, se a mensagem for uma reprogramaçao ela é enviada para esta funçao
    que fara a extraçao dos dados e atualiza-ra os dados

  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("Callback: processaReprogramacao(" + mensagemRecebida + ")");

  //Instancia o objeto para desserializar o json
  const size_t capacity = JSON_OBJECT_SIZE(14) + 290;
  DynamicJsonDocument doc(capacity);

  //Converte a string em um char array para poder ser desserializado pela biblioteca
  char json[mensagemRecebida.length() + 3];
  mensagemRecebida.toCharArray(json, mensagemRecebida.length() + 3);

  //Desserializa o Json
  deserializeJson(doc, json);

  //Extrai  os dados da mensagem e armazena em Strings
  String endpoint = doc["idAlvo"]; // "00001"
  String mensagem = doc["mensagem"]; // "reprogramacao"
  String tipoAlvo = doc["tipoAlvo"]; // "endpoint"
  String ssidAP = doc["ssidAP"]; // "00001"
  String senhaAP = doc["senhaAP"]; // "123456789"
  String novoNomeEndpoint = doc["novoIdEndpoint"]; // "00004"
  String gatewayReceptor = doc["gatewayReceptor"]; // "00001"
  String cliente = doc["cliente"]; // "ClientePadrao"
  String sensor = doc["sensor"]; // "co2"
  String habilitado = doc["habilitado"]; // "true"
  String taxaAmostragem = doc["taxaAmostragem"]; // "5"
  String dataHora = doc["dataHora"]; // "10/05/2020 16:06:25"
  String hardResetjson = doc["hardReset"]; // "true"

  //Se a mensagem for uma reprogramaçao E for destinada a um endpoint E for especificamente este endpoint, processamos a mensagem
  if ((mensagem.equals("reprogramacao")) && (tipoAlvo.equals("endpoint")) && (endpoint.equals((getEndpoint())))) {

    //Se a mensagem contem a chave ssidAP
    if (doc.containsKey("ssidAP")) {

      //Armazeno o novo dado na variavel global reservada a este dado
      ssid = ssidAP ;
    }
    //Se a mensagem contem a chave
    if (doc.containsKey("senhaAP")) {

      //Armazeno o novo dado na variavel global reservada a este dado
      senhaAp = senhaAP;
    }
    //Se a mensagem contem a chave
    if (doc.containsKey("novoIdEndpoint")) {

      //Armazeno o novo dado na variavel global reservada a este dado
      nomeEndpoint = novoNomeEndpoint ;
    }
    //Se a mensagem contem a chave
    if (doc.containsKey("gatewayReceptor")) {
      nomeGateway = gatewayReceptor;
    }
    //Se a mensagem contem a chave
    if (doc.containsKey("cliente")) {

      //Armazeno o novo dado na variavel global reservada a este dado
      nomeCliente = cliente;
    }
    //Se a mensagem contem a chave
    if (doc.containsKey("sensor")) {

      //verifica se a mensagem tem a chave taxaAmostragem (o que indica que a taxa de amostragem sera alterada)
      bool existeAmostragem = doc.containsKey("taxaAmostragem");

      //verifica se a mensagem tem a chave habilitado (o que indica que o sensor sera habilitado ou desabilitado)
      bool existeHabilitado = doc.containsKey("habilitado");

      //Verifica se existe uma das duas chaves "habilitado" ou "taxaAmostragem"
      if ((doc.containsKey("habilitado")) || (doc.containsKey("taxaAmostragem"))) {

        //Se o sensor pretendido for o sensor de tensao
        if (sensor.equals("tensao")) {

          //Se a chave habilitado existe na mensagem
          if (existeHabilitado) {

            //Salva o estado do sensor na variavel global destinada a este dado
            tensaoHabilidado = habilitado;
          }
          //Se a chave existeAmostragem existe
          if (existeAmostragem) {

            //Salva a taxa de amostragem do sensor na variavel global destinada a este dado
            tensaoAmostragem = taxaAmostragem;
          }
        }

        //Se o sensor pretendido for o sensor de corrente
        if (sensor.equals("corrente")) {

          //Se a chave habilitado existe na mensagem
          if (existeHabilitado) {

            //Salva o estado do sensor na variavel global destinada a este dado
            correnteHabilidado = habilitado;
          }
          //Se a chave existeAmostragem existe
          if (existeAmostragem) {

            //Salva a taxa de amostragem do sensor na variavel global destinada a este dado
            correnteAmostragem = taxaAmostragem;
          }
        }

        //Se o sensor pretendido for o sensor de temperatura
        if (sensor.equals("temperatura")) {

          //Se a chave habilitado existe na mensagem
          if (existeHabilitado) {

            //Salva o estado do sensor na variavel global destinada a este dado
            temperaturaHabilidado = habilitado;
          }

          //Se a chave existeAmostragem existe
          if (existeAmostragem) {

            //Salva a taxa de amostragem do sensor na variavel global destinada a este dado
            temperaturaAmostragem = taxaAmostragem;
          }
        }
        //Se o sensor pretendido for o sensor de umidade
        if (sensor.equals("umidade")) {

          //Se a chave habilitado existe na mensagem
          if (existeHabilitado) {

            //Salva o estado do sensor na variavel global destinada a este dado
            ummidadeHabilidado = habilitado;
          }

          //Se a chave existeAmostragem existe
          if (existeAmostragem) {

            //Salva a taxa de amostragem do sensor na variavel global destinada a este dado
            umidadeAmostragem = taxaAmostragem;
          }
        }
        //Se o sensor pretendido for o sensor de co2
        if (sensor.equals("co2")) {

          //Se a chave habilitado existe na mensagem
          if (existeHabilitado) {

            //Salva o estado do sensor na variavel global destinada a este dado
            co2Habilidado = habilitado;
          }
          //Se a chave existeAmostragem existe
          if (existeAmostragem) {

            //Salva a taxa de amostragem do sensor na variavel global destinada a este dado
            co2Amostragem = taxaAmostragem;
          }
        }
        //Se o sensor pretendido for o sensor de tvoc
        if (sensor.equals("tvoc")) {

          //Se a chave habilitado existe na mensagem
          if (existeHabilitado) {

            //Salva o estado do sensor na variavel global destinada a este dado
            tvocHabilidado = habilitado;
          }

          //Se a chave existeAmostragem existe
          if (existeAmostragem) {

            //Salva a taxa de amostragem do sensor na variavel global destinada a este dado
            tvocAmostragem = taxaAmostragem;
          }
        }
      }
    }


    //Se a mensagem contem a chave
    if (doc.containsKey("dataHora")) {

      //Armazeno o novo dado na variavel global reservada a este dado
      novaData = dataHora;
    }

    //Se a mensagem contem a chave
    if (doc.containsKey("hardReset")) {

      //se a mensagem de hard reset for true
      if (hardResetjson.equals("true")) {

        //sinaliza que no proximo loop sera realizado o hard reset
        hardReset = true;
      }
    }

    //Sinaliza que existe dados nas variaveis globais para serem atualizados
    newData = true;

    //Retorna true indicando o sucesso da operaçao
    return true;
  }

  //Retorna false indicando o NAO sucesso da operaçao
  return false;

}
/* ----------------------------------------LORA----------------------------------------
   Este modulo contem Todas funçoes responsaveis pela comunicaçao LoraWan, tanto envio quando recebimento de dados

*/


void iniciaLora() {
  /*
    Está funçao instancia o modulo lora e inicia o protocolo lora
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: iniciaLora()");

  //Indica a pinagem do modulo lora para a lib SPI
  SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);

  //Indica os pinos do modulo lora para a biblioteca Lora
  LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
  if (!LoRa.begin(915E6)) {
    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Falha ao iniciar o protocolo LoRa");

  } else {
    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Protocolo LoRa inicializado corretamente");

  }
}

void desligaLora() {
  /*
    Esta funçao desativa o protocolo lora, terminando o objeto SPI o qual o modulo lora foi instanciado
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: desligaLora()");

  //termia a instancia SPI o qual o modulo lora esta instanciado
  SPI.end();
}


bool enviaMsgLora(String mensagem) {
  /*
    Esta funçao envia a string passada como parametro via lora, para todos os dispositivos que estiverem trabalhando na mesma frequencia
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("Callback: enviaMsgLora(" + mensagem + ")");

  //inicia o protocolo lora
  iniciaLora();

  //Instancia um pacote para enviar via lora
  LoRa.beginPacket();

  //Insere a string no pacote a ser enviado
  LoRa.print(mensagem);

  //Envia o pacote via lora
  LoRa.endPacket();

  //Dasativa o protocolo lora para poupar bateria
  desligaLora();

  //Todo envio procedeu corretamente, retorno true indicando sucesso na operaçao
  return true;
}

String recebeMsgLora(bool lerAteReceber = false ) {
  /*
    Esta funçao realiza a leitura do protocolo lora, se o parametro lerAteReceber for definido como true
    Essa funçao entra-ra em um loop infinito e so retorna-ra quado receber alguma mensagem
    Caso nao seja definido, está funçao lera por até 10 segundos e retorna-ra a mensagem recebida
  */
  if (DEBUGS_PRINTS); Serial.println("Callback: recebeMsgLora(" + String(lerAteReceber) + ")");

  //Inicia protocolo Lora
  iniciaLora();

  if (lerAteReceber) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("Lendo lora até receber alguma coisa");

  } else {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("Lendo lora por até 10 segundos");
  }

  //variável long de tempo para controle do loop while
  long tempoInicialEsperarResposta = millis();

  Serial.print("TENTAANDO LER LORA");

  //O loop while ira operar até que se tenha passado 10000 millisegundos (10 segundos) do tempo salvo em tempoInicialEsperarResposta
  // Ou ira parar apenas quando receber uma mensagem se lerAteReceber estiver como true
  while ((millis() < tempoInicialEsperarResposta + 10000) || lerAteReceber ) {

    Serial.print(".");

    //Verifico o tamanho atual do pacote no buffer do lora
    int packetSize = LoRa.parsePacket();

    //Se o tamanho do pacote for maior que 0 foi recebido uma mensagem
    if (packetSize) {

      //Cria uma String que ira armazenar a mensagem
      String mensagem = "";

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS); Serial.print("Pacote recebido '");

      //Enquanto tiver algo no buffer do lora o ciclo while ira ser execultado
      while (LoRa.available ()) {

        //leio o char no buffer do lora e concateno na string mensagem
        mensagem += (char)LoRa.read();
      }

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS); Serial.print("Mensagem recebida: ");
      if (DEBUGS_PRINTS); Serial.println(mensagem);

      //Desativa o protocolo lora para poupar bateria
      desligaLora();

      //Retorna a mensagem recebida
      return mensagem;
    }
  }

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("Nenhuma mensagem recebida");

  //Desativa o protocolo lora para poupar bateria
  desligaLora();

  //Como se passaram 10 segundos e nenhuma mensagem foi recebida eu retorno a String "NULL"
  return "NULL";
}


/* ----------------------------------------------------SENSORES------------------------------------------------------------------
   Este modulo contem Todas funçoes que tratam a comunicaçao com os sensores e as validaçoes
   necessarias para realizar as leituras
*/

/* ----------------------------------------SENSOR SCT (CORRENTE)---------------------------------------------
   Este submodulo contem Todas funçoes que tratam a comunicaçao com o sensor de Corrente SCT
*/

String getCorrenteAC() {
  /*
    Esta funçao realiza da corrente no sensor SCT
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getCorrenteAC()");

  //Realizo a mediçao da corrente utilizando a lib emon1 e salvo na string corrente
  String corrente = String(emon1.calcIrms(1480));

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.print("Corrente AC: ");

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println(corrente);

  //Retorno a string contendo a corrente medida
  return corrente;

}


/* ----------------------------------------SENSOR ZMTP (TENSAO)---------------------------------------------
   Este submodulo contem Todas funçoes que tratam a comunicaçao com o sensor de tensao Zmtp
*/

String getTensaoAC() {
  /*
    Esta funçao realiza da tensaao no sensor ZMTP
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getTensaoAC()");

  //Realizo a mediçao da tensao utilizando a lib emon1
  emon1.calcVI(30, 500); //FUNÇÃO DE CÁLCULO (17 SEMICICLOS, TEMPO LIMITE PARA FAZER A MEDIÇÃO)

  //Ainda utilizando a emon1 calculo a tensao RMS por se tratar de corrente alternada
  //E salvo na string Tensao
  String tensao = String(emon1.Vrms); // VARIÁVEL RECEBE O VALOR DE TENSÃO RMS OBTIDO

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.print("Tensao AC: ");

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println(tensao);

  //Retorno a string contendo o valor da tensao
  return tensao;

}

/* ----------------------------------------SENSOR DHT22-----------------------------------------------
   Este submodulo contem Todas funçoes que tratam a comunicaçao com o sensor de temperatura e umidade dht22
*/

String getTemperatura() {
  /*
    Esta funçao realiza a leitura da temperatura no sensor DHT22
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getTemperatura()");

  //Lê a temperatura no sensor
  int Temp = dht.readTemperature();

  //Verifica se o valor que obtemos do sensor é um NAN(Not-A-Number)
  //Se nao for um numero sera retornado true
  if (isnan(Temp)) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Temperatura: 0");

    //Se nao for um numero eu retorno a string 0
    return "0";

    //Se for um numero
  } else {

    //Transforma a temperatura de inteiro para String
    String STemp = String(Temp);

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("Temperatura: ");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(STemp);

    //Retorna a string contendo a temperatura
    return STemp;
  }
}


String getUmidade() {
  /*
    Esta funçao realiza a leitura da umidade no sensor DHT22
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getUmidade()");

  //Lê a umidade no sensor
  int RH = dht.readHumidity();

  //Verifica se o valor que obtemos do sensor é um NAN(Not-A-Number)
  //Se nao for um numero sera retornado true
  if (isnan(RH)) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Umidade: 0");

    //Se nao for um numero eu retorno a string 0
    return "0";

    //Se for um numero
  } else {

    //Transforma a umidade de inteiro para String
    String SRH = String(RH);

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("Umidade: ");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(SRH);

    //Retorna a string contendo a temperatura
    return SRH;
  }
}

/* ----------------------------------------SENSOR CCS811 (CO2 e TVOC)---------------------------------------------
   Este submodulo contem Todas funçoes que tratam a comunicaçao com o sensor de CO2 e TVOC CCS811
*/
String getCO2() {
  /*
    Esta funçao realiza a leitura do CO2 no sensor CCS811
  */


  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getCO2()");

  //Se o endereço i2c do sensor estiver disponivel
  if (ccs.available()) {

    //lê os dados do sensor
    if (!ccs.readData()) {

      // Obtem o valor especifico do TVOC
      String co2 = String(ccs.geteCO2());

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.print("CO2: ");

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println(co2);

      //Retorno uma string contendo o valor obtido
      return co2;
    }
    //Se nao for possivel ler os dados no sensor
    else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("CCS NOT READ DATA CO2: 0");

      //retorno o valor 0
      return "0";
    }

    //Se o endereço i2c nao estiver disponivel
  } else {
    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(" CCS NOT AVALIBE CO2: 0");

    //retorno o valor 0
    return "0";
  }
}


String getTVOC() {
  /*
    Esta funçao realiza a leitura do TVOC no sensor CCS811
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: getTVOC()");

  //Se o endereço i2c do sensor estiver disponivel
  if (ccs.available()) {

    //lê os dados do sensor
    if (!ccs.readData()) {

      //Obtem o valor especifico do TVOC
      String TVOC = String(ccs.getTVOC());

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.print("TVOC: ");

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println(TVOC);

      //Retorna uma String com o valor do TVOC
      return TVOC;

    }

    //Se nao for possivel ler os dados no sensor
    else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("TVOC: 0");

      //retorno o valor 0
      return "0";
    }

    //Se o endereço i2c nao estiver disponivel
  } else {
    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("TVOC: 0");

    //retorno o valor 0
    return "0";
  }
}

String getBateria() {
  /*
    Está funçao retorna o percentual da bateria
  */
  String percentual = "100";

  //retorna o percentual da bateeria
  return percentual;
}


/* ----------------------------------------VALIDAÇAO SENSORES---------------------------------------------
   Este submodulo contem Todas funçoes que tratam a validaçao para realizar a leitura dos sensores
   conectados no endpoint
*/

bool sensorHabilitado(String sensor) {
  /*
    Esta funçao verifica se o sensor passado como parametro esta configurado para estar ativo neste endpoint
  */

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: sensorHabilitado(" + sensor + ")");


  //Verifica se o sensor passado como parametro é o sensor de tensao
  if (sensor.equals("tensao")) {

    //Le o arquivo txt o qual está salvo o estado do sensor
    String Variavel = getSensorHabilitado("tensao");

    //Se no arquivo estiver ecrito a string "true"
    if (Variavel == "true") {

      //Retorno true indicando que o sensor está ativo
      return true;

    } else {

      //Retorno false indicando que o sensor NAO está ativo
      return false;
    }
  }


  //Verifica se o sensor passado como parametro é o sensor de corrente
  if (sensor.equals("corrente")) {

    //Le o arquivo txt o qual está salvo o estado do sensor
    String Variavel = getSensorHabilitado("corrente");

    //Se no arquivo estiver ecrito a string "true"
    if (Variavel == "true") {

      //Retorno true indicando que o sensor está ativo
      return true;

    } else {

      //Retorno false indicando que o sensor NAO está ativo
      return false;
    }
  }


  //Verifica se o sensor passado como parametro é o sensor de temperatura
  if (sensor.equals("temperatura")) {

    //Le o arquivo txt o qual está salvo o estado do sensor
    String Variavel = getSensorHabilitado("temperatura");

    //Se no arquivo estiver ecrito a string "true"
    if (Variavel == "true") {

      //Retorno true indicando que o sensor está ativo
      return true;

    } else {

      //Retorno false indicando que o sensor NAO está ativo
      return false;
    }
  }


  //Verifica se o sensor passado como parametro é o sensor de umidade
  if (sensor.equals("umidade")) {

    //Le o arquivo txt o qual está salvo o estado do sensor
    String Variavel = getSensorHabilitado("umidade");

    //Se no arquivo estiver ecrito a string "true"
    if (Variavel == "true") {

      //Retorno true indicando que o sensor está ativo
      return true;

    } else {

      //Retorno false indicando que o sensor NAO está ativo
      return false;
    }
  }


  //Verifica se o sensor passado como parametro é o sensor de co2
  if (sensor.equals("co2")) {

    //Le o arquivo txt o qual está salvo o estado do sensor
    String Variavel = getSensorHabilitado("co2");

    //Se no arquivo estiver ecrito a string "true"
    if (Variavel == "true") {

      //Retorno true indicando que o sensor está ativo
      return true;

    } else {

      //Retorno false indicando que o sensor NAO está ativo
      return false;
    }
  }


  //Verifica se o sensor passado como parametro é o sensor de tvoc
  if (sensor.equals("tvoc")) {

    //Le o arquivo txt o qual está salvo o estado do sensor
    String Variavel = getSensorHabilitado("tvoc");

    //Se no arquivo estiver ecrito a string "true"
    if (Variavel == "true") {

      //Retorno true indicando que o sensor está ativo
      return true;

    } else {

      //Retorno false indicando que o sensor NAO está ativo
      return false;
    }
  }

  //Retorno false indicando que o sensor NAO está ativo
  return false;
}


bool realizarLeitura(String sensor) {
  /*
    Esta funçaoo valida se está na hora de fazer a leitura do sensor passado como parametro, a leitura só é liberada para ser realizada
    Se passar pela seguinte validaçao:

                                               (O sensor esta habilitado)                                  - So vamos ler o sensor se ele estiver Habilitado
                                                           E
                            (a data e a hora atual É MAIOR OU IGUAL a data salva para a proxima leitura )  - Significa que a esta na hora de fazer a leitura ou já passou da hora
                                                           OU
    ((data e hora atual SOMADO com a taxa de amostragem) É MENOR que a data e a hora da proxima leitura)   - Significa que a data e a hora da proxima leitura está no futuro,
                                                                                                                         o que nao faz sentido, vamos ler o sensor

    Se o resultado obtido pela expressao acima for true o sensor é liberado para realizar a leitura, caso contrario nao é liberado

  */


  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: realizaleitura(" + sensor + ")");


  //Verifica se o sensor solicitado é o sensor de tensao
  if (sensor.equals("tensao")) {

    //Obtem a data e hora atual em formato unix
    long dataHoraAtual = getDataHora("unix").toInt();

    //Obtem a data e hora da proxima leitura salvo posteriormente
    long proxAmostragem = getProxAmostragem("tensao").toInt();

    //Obtem a taxa de amostragem do sensor
    long taxaAmostragem = getAmostragem("tensao").toInt();

    //Realiza a validaçao se o sensor pode ser lido (a explicaçao da expressao é a expressao no cabeçalho da funçao)
    if (sensorHabilitado(sensor) && ((dataHoraAtual >= proxAmostragem) || ((dataHoraAtual + taxaAmostragem) < proxAmostragem))) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("true");

      //Uma vez a validaçao sendo ok retorno true indicando que posso ler o sensor
      return true ;
    } else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("false");

      //Uma vez a validaçao NAO sendo ok retorno false indicando que nao posso ler o sensor
      return false;
    }

  }



  //Verifica se o sensor solicitado é o sensor de corrente
  if (sensor.equals("corrente")) {

    //Obtem a data e hora atual em formato unix
    long dataHoraAtual = getDataHora("unix").toInt();

    //Obtem a data e hora da proxima leitura salvo posteriormente
    long proxAmostragem = getProxAmostragem("corrente").toInt();

    //Obtem a taxa de amostragem do sensor
    long taxaAmostragem = getAmostragem("corrente").toInt();


    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("Data prevista prox amostragem: ");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(proxAmostragem);


    //Realiza a validaçao se o sensor pode ser lido (a explicaçao da expressao é a expressao no cabeçalho da funçao)
    if (sensorHabilitado(sensor) && ((dataHoraAtual >= proxAmostragem) || ((dataHoraAtual + taxaAmostragem) < proxAmostragem))) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("true");

      //Uma vez a validaçao sendo ok retorno true indicando que posso ler o sensor
      return true ;
    } else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("false");

      //Uma vez a validaçao NAO sendo ok retorno false indicando que nao posso ler o sensor
      return false;
    }

  }



  //Verifica se o sensor solicitado é o sensor de temperatura
  if (sensor.equals("temperatura")) {

    //Obtem a data e hora atual em formato unix
    long dataHoraAtual = getDataHora("unix").toInt();

    //Obtem a data e hora da proxima leitura salvo posteriormente
    long proxAmostragem = getProxAmostragem("temperatura").toInt();


    //Obtem a taxa de amostragem do sensor
    long taxaAmostragem = getAmostragem("temperatura").toInt();

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("Data prevista prox amostragem: ");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(proxAmostragem);


    //Realiza a validaçao se o sensor pode ser lido (a explicaçao da expressao é a expressao no cabeçalho da funçao)
    if (sensorHabilitado(sensor) && ((dataHoraAtual >= proxAmostragem) || ((dataHoraAtual + taxaAmostragem) < proxAmostragem))) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("true");

      //Uma vez a validaçao sendo ok retorno true indicando que posso ler o sensor
      return true ;
    } else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("false");

      //Uma vez a validaçao NAO sendo ok retorno false indicando que nao posso ler o sensor
      return false;
    }

  }



  //Verifica se o sensor solicitado é o sensor de umidade
  if (sensor.equals("umidade")) {

    //Obtem a data e hora atual em formato unix
    long dataHoraAtual = getDataHora("unix").toInt();

    //Obtem a data e hora da proxima leitura salvo posteriormente
    long proxAmostragem = getProxAmostragem("umidade").toInt();

    //Obtem a taxa de amostragem do sensor
    long taxaAmostragem = getAmostragem("umidade").toInt();

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("Data prevista prox amostragem: ");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(proxAmostragem);


    //Realiza a validaçao se o sensor pode ser lido (a explicaçao da expressao é a expressao no cabeçalho da funçao)
    if (sensorHabilitado(sensor) && ((dataHoraAtual >= proxAmostragem) || ((dataHoraAtual + taxaAmostragem) < proxAmostragem))) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("true");

      //Uma vez a validaçao sendo ok retorno true indicando que posso ler o sensor
      return true ;
    } else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("false");

      //Uma vez a validaçao NAO sendo ok retorno false indicando que nao posso ler o sensor
      return false;
    }

  }



  //Verifica se o sensor solicitado é o sensor de co2
  if (sensor.equals("co2")) {

    //Obtem a data e hora atual em formato unix
    long dataHoraAtual = getDataHora("unix").toInt();

    //Obtem a data e hora da proxima leitura salvo posteriormente
    long proxAmostragem = getProxAmostragem("co2").toInt();

    //Obtem a taxa de amostragem do sensor
    long taxaAmostragem = getAmostragem("co2").toInt();

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("Data prevista prox amostragem: ");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(proxAmostragem);


    //Realiza a validaçao se o sensor pode ser lido (a explicaçao da expressao é a expressao no cabeçalho da funçao)
    if (sensorHabilitado(sensor) && ((dataHoraAtual >= proxAmostragem) || ((dataHoraAtual + taxaAmostragem) < proxAmostragem))) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("true");

      //Uma vez a validaçao sendo ok retorno true indicando que posso ler o sensor
      return true ;
    } else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("false");

      //Uma vez a validaçao NAO sendo ok retorno false indicando que nao posso ler o sensor
      return false;
    }

  }



  //Verifica se o sensor solicitado é o sensor de tvoc
  if (sensor.equals("tvoc")) {

    //Obtem a data e hora atual em formato unix
    long dataHoraAtual = getDataHora("unix").toInt();

    //Obtem a data e hora da proxima leitura salvo posteriormente
    long proxAmostragem = getProxAmostragem("tvoc").toInt();

    //Obtem a taxa de amostragem do sensor
    long taxaAmostragem = getAmostragem("tvoc").toInt();

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.print("Data prevista prox amostragem: ");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println(proxAmostragem);


    //Realiza a validaçao se o sensor pode ser lido (a explicaçao da expressao é a expressao no cabeçalho da funçao)
    if (sensorHabilitado(sensor) && ((dataHoraAtual >= proxAmostragem) || ((dataHoraAtual + taxaAmostragem) < proxAmostragem))) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("true");

      //Uma vez a validaçao sendo ok retorno true indicando que posso ler o sensor
      return true ;
    } else {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("false");

      //Uma vez a validaçao NAO sendo ok retorno false indicando que nao posso ler o sensor
      return false;
    }

  }
  if (DEBUGS_PRINTS); Serial.println("Nenhuma opcao plausivel");

  //Nenhuma opçao valida foi encontrada, representa um erro, retorna false para nao ler o sensor
  return false;
}

/* ----------------------------------------WIFI AP------------------------------------------------------------------------------------
   Este modulo contem todas funçoes e variaveis que lidam com o ponto de acesso wifi gerado
   para reconfigurar localmente o endpoint

*/



/* ----------------------------------------PAGINAS HTML----------------------------------------
   Este submodulo contem as variaveis contendo o codigo fonte das paginas HTML do wifi manager
   por razoes de espaço é altamente recomendavel a nao utilizaçao de apis como bootstrap
   tambem é desaconselhavel utilizar css em um arquivo separado, caso deseje trabalhar a aparencia
   das paginas recomendo que utilize ccs in line
   A comunicaçao deve ser estritamente htttp, utilizar outro protocolo como https ira trazer
   resultado inesperado nos valores das variaveis recuperadas

*/

/*Variavel que guarda a pagina html  index, ou seja é a pagina principal que é enviada quando se entra no web service
  ,a variavel é do tipo PROGMEM, ou seja está variavel global nao ocupa a memoria RAM do microcontrolador e sim a FLASH
  nao é possivel documentar dentro da area R"rawliteral() pois comentarios nao sao aceitos
*/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Enpoint Config</title>
    <meta charset="utf-8">
  </head>
  <body>
    <table align='center' width='400' border='0'>
        <form action="/get">
        <tr>
          <td align='center'>
          <hr/>
            <h3>Dados do Dispositivo:</h3>
          </td>
        </tr>
        <tr>
          <td  align='left'>
            
              
              Nome do enpoint: <input type="text" name="nomeEndpoint"></br></br>
              Gateway receptor: <input type="text" name="nomeGateway"></br></br>
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Cliente: &nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; <input type="text" name="cliente"></br></br>
              Data: <input type="date" name="novaData"> &nbsp;&nbsp;&nbsp;&nbsp; Hora <input type="time" step="2" name="newHour" /></br></br>
              <hr/>
              <h3 align='center'>Sensores habilitados:</h3>
               &nbsp;&nbsp;&nbsp;&nbsp;<b>Corrente AC</b></br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="correnteAcHabilitado" value="true"> Habilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="correnteAcHabilitado" value="false"> Desabilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Amostragem:<input type="number" name="correnteAmostragem" size="5"> S</br></br>

               &nbsp;&nbsp;&nbsp;&nbsp;<b>Tensao AC</b></br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="tensaoAcHabilitado" value="true"> Habilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="tensaoAcHabilitado" value="false"> Desabilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Amostragem:<input type="number" name="tensaoAmostragem" size="5"> S</br></br>
              

               &nbsp;&nbsp;&nbsp;&nbsp;<b>Temperatura</b></br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="temperaturaHabilitado" value="true"> Habilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="temperaturaHabilitado" value="false"> Desabilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Amostragem:<input type="number" name="temperaturaAmostragem" size="5"> S</br></br>
              

               &nbsp;&nbsp;&nbsp;&nbsp;<b>Umidade</b></br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="umidadeHabilitado" value="true"> Habilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="umidadeHabilitado" value="false"> Desabilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Amostragem:<input type="number" name="umidadeAmostragem" size="5"> S</br></br>
              

               &nbsp;&nbsp;&nbsp;&nbsp;<b>CO2</b></br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="co2Habilitado" value="true"> Habilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="co2Habilitado" value="false"> Desabilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Amostragem:<input type="number" name="co2Amostragem" size="5"> S</br></br>
              

               &nbsp;&nbsp;&nbsp;&nbsp;<b>TVOC</b></br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="tvocHabilitado" value="true"> Habilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="tvocHabilitado" value="false"> Desabilitado </br>
               &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Amostragem:<input type="number" name="tvocAmostragem" size="5"> S</br></br>
              
              <hr/>
              <h3 align='center' >Rede Access Point</h3>
              &nbsp;&nbsp;&nbsp;&nbsp;SSID: <input type="text" name="ssidAp"></br></br>
              &nbsp;&nbsp;&nbsp;&nbsp;Senha: <input type="text" name="senhaAp"></br></br>
              
                            
            </td> 
          </tr>
          <tr>
            <td align='center'>
              <hr/>
              <h3>Opçoes de debug</h3>
            </td>
          </tr>
          
          <tr>
            <td align='left'>
              <b>Print via Serial</b></br>
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="printSerial" value="true"> Habilitado </br>
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="printSerial" value="false"> Desabilitado </br></br>
              
              <b>Display sempre ativado</b></br>
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="displaySempreLigado" value="true"> Habilitado </br>
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type="radio"  name="displaySempreLigado" value="false"> Desabilitado </br></br>
               
              
              
            
            </td>
          </tr>
        <tr>
          <td align='center'>
            <input type="submit" value="Salvar">
          </td>
        </tr>
        </form><br>

        <form action="/hardReset">
         <tr>
          <td align='center'>
            <hr/>
            <input type="submit" value="Hard Reset">
            <div align='center' >Isto irá proceder com um reset de frabrica do embarcado, e todos os valores serao setados com os valores padroes</div>
          </td>
        </tr>
        </form><br>
        
        
      </table>
 </body></html>)rawliteral";

/*Variavel que guarda a pagina html  de confirmaçao geral,a variavel é do tipo PROGMEM
  ou seja está variavel global nao ocupa a memoria RAM do microcontrolador e sim a FLASH
  nao é possivel documentar dentro da area R"rawliteral() pois comentarios nao sao aceitos
*/
const char confirm_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Enpoint Config</title>
    <meta charset="utf-8">
  </head>
  <body>
    <table align='center' width='400' border='0'>
          <tr>
          <td align='center'>
            Todos os dados foram salvos com Sucesso<br><a href="/">Retornar para configuraçoes</a>
          </td>
        </tr>
          </table>
 </body></html>)rawliteral";


/*Variavel que guarda a pagina html  de confirmaçao do hard reset,a variavel é do tipo PROGMEM
  ou seja está variavel global nao ocupa a memoria RAM do microcontrolador e sim a FLASH
  nao é possivel documentar dentro da area R"rawliteral() pois comentarios nao sao aceitos
*/
const char reset_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Enpoint Config</title>
    <meta charset="utf-8">
  </head>
  <body>
    <table align='center' width='400' border='0'>
          <tr>
          <td align='center'>
            <div align='center'>O hard reset pode demorar até 20 segundos para proceder completamente<br>
            Por Favor aguarde...</div>
          </td>
        </tr>
          </table>
 </body></html>)rawliteral";

/* ----------------------------------------PONTO DE ACESSO----------------------------------------
  Este submodulo contem Todas funçoes que geram e colocam online o wifi manager gerado
  atravez do poonto de acesso do modulo wifi do microcontrolador
*/

void configWifiManager() {
  /*
    Essa funçao trata da criaçao e da comunicaçao do ponto de acesso wifi
    e do servidor web e suas requisiçoes
  */

  //Obtem o SSID e a SENHA do ponto de acesso wifi
  String ssidString = getSsidAp();
  String senhaString = getSenhaAp();

  //Converte o Object String em char array, pois a lib Wifi aceita parametros apenas em array de caracteres
  char ssid2[ssidString.length() + 3] = {};
  char password2[senhaString.length() + 3] = {};
  ssidString.toCharArray(ssid2, ssidString.length() + 3);
  senhaString.toCharArray(password2, senhaString.length() + 3);

  //Gera o ponto de acesso wifi passando o ssid e a senha do ponto de acesso
  //Observe que a senha obrigatoriamente deve ter mais que 9 caracteres
  WiFi.softAP(ssid2, password2);

  //Obtem o IP do web service gereado pelo ponto de acesso (sempre será 192.168.4.1)
  IPAddress IP = WiFi.softAPIP();

  //Define o arquivo HTML que sera carregado quando "/" (raiz) for requisitado, que no caso será o index_html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {

    //Envia index_html
    request->send_P(200, "text/html", index_html);
  });

  //Define as açoes tomadas quando qualquer requisiçao "/get" for solicitado
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {


    //Se a requisiçao tiver o parametro nomeEndpoint recuperamos o valor enviado
    if (request->hasParam("nomeEndpoint")) {

      //Recupera o valor salvo no parametro nomeEndpoint enviado na requisiçao
      nomeEndpoint = request->getParam("nomeEndpoint")->value();
    }


    //Se a requisiçao tiver o parametro nomeGateway recuperamos o valor enviado
    if (request->hasParam("nomeGateway")) {

      //Recupera o valor salvo no parametro nomeGateway enviado na requisiçao
      nomeGateway = request->getParam("nomeGateway")->value();
    }


    //Se a requisiçao tiver o parametro correnteAcHabilitado recuperamos o valor enviado
    if (request->hasParam("correnteAcHabilitado")) {

      //Recupera o valor salvo no parametro correnteAcHabilitado enviado na requisiçao
      correnteHabilidado = request->getParam("correnteAcHabilitado")->value();
    }


    //Se a requisiçao tiver o parametro tensaoAcHabilitado recuperamos o valor enviado
    if (request->hasParam("tensaoAcHabilitado")) {

      //Recupera o valor salvo no parametro tensaoAcHabilitado enviado na requisiçao
      tensaoHabilidado = request->getParam("tensaoAcHabilitado")->value();
    }


    //Se a requisiçao tiver o parametro temperaturaHabilitado recuperamos o valor enviado
    if (request->hasParam("temperaturaHabilitado")) {

      //Recupera o valor salvo no parametro temperaturaHabilitado enviado na requisiçao
      temperaturaHabilidado = request->getParam("temperaturaHabilitado")->value();

    }


    //Se a requisiçao tiver o parametro umidadeHabilitado recuperamos o valor enviado
    if (request->hasParam("umidadeHabilitado")) {

      //Recupera o valor salvo no parametro umidadeHabilitado enviado na requisiçao
      ummidadeHabilidado = request->getParam("umidadeHabilitado")->value();

    }


    //Se a requisiçao tiver o parametro co2Habilitado recuperamos o valor enviado
    if (request->hasParam("co2Habilitado")) {

      //Recupera o valor salvo no parametro co2Habilitado enviado na requisiçao
      co2Habilidado = request->getParam("co2Habilitado")->value();
    }


    //Se a requisiçao tiver o parametro tvocHabilitado recuperamos o valor enviado
    if (request->hasParam("tvocHabilitado")) {

      //Recupera o valor salvo no parametro tvocHabilitado enviado na requisiçao
      tvocHabilidado = request->getParam("tvocHabilitado")->value();
    }



    //Se a requisiçao tiver o parametro correnteAmostragem recuperamos o valor enviado
    if (request->hasParam("correnteAmostragem")) {

      //Recupera o valor salvo no parametro correnteAmostragem enviado na requisiçao
      correnteAmostragem = String(request->getParam("correnteAmostragem")->value());

    }


    //Se a requisiçao tiver o parametro tensaoAmostragem recuperamos o valor enviado
    if (request->hasParam("tensaoAmostragem")) {

      //Recupera o valor salvo no parametro tensaoAmostragem enviado na requisiçao
      tensaoAmostragem = String(request->getParam("tensaoAmostragem")->value());
    }


    //Se a requisiçao tiver o parametro temperaturaAmostragem recuperamos o valor enviado
    if (request->hasParam("temperaturaAmostragem")) {

      //Recupera o valor salvo no parametro temperaturaAmostragem enviado na requisiçao
      temperaturaAmostragem = String(request->getParam("temperaturaAmostragem")->value());
    }


    //Se a requisiçao tiver o parametro umidadeAmostragem recuperamos o valor enviado
    if (request->hasParam("umidadeAmostragem")) {

      //Recupera o valor salvo no parametro umidadeAmostragem enviado na requisiçao
      umidadeAmostragem = String(request->getParam("umidadeAmostragem")->value());
    }


    //Se a requisiçao tiver o parametro co2Amostragem recuperamos o valor enviado
    if (request->hasParam("co2Amostragem")) {

      //Recupera o valor salvo no parametro co2Amostragem enviado na requisiçao
      co2Amostragem = String(request->getParam("co2Amostragem")->value());
    }


    //Se a requisiçao tiver o parametro tvocAmostragem recuperamos o valor enviado
    if (request->hasParam("tvocAmostragem")) {

      //Recupera o valor salvo no parametro tvocAmostragem enviado na requisiçao
      tvocAmostragem = String(request->getParam("tvocAmostragem")->value());
    }



    //Se a requisiçao tiver o parametro cliente recuperamos o valor enviado
    if (request->hasParam("cliente")) {

      //Recupera o valor salvo no parametro cliente enviado na requisiçao
      nomeCliente = String(request->getParam("cliente")->value());
    }


    //Se a requisiçao tiver o parametro ssidAp recuperamos o valor enviado
    if (request->hasParam("ssidAp")) {

      //Recupera o valor salvo no parametro ssidAp enviado na requisiçao
      ssid = String(request->getParam("ssidAp")->value());
    }


    //Se a requisiçao tiver o parametro senhaAp recuperamos o valor enviado
    if (request->hasParam("senhaAp")) {

      //Recupera o valor salvo no parametro senhaAp enviado na requisiçao
      senhaAp = String(request->getParam("senhaAp")->value());
    }


    //Se a requisiçao tiver o parametro loginMenu recuperamos o valor enviado
    if (request->hasParam("loginMenu")) {

      //Recupera o valor salvo no parametro loginMenu enviado na requisiçao
      loginMenu = String(request->getParam("loginMenu")->value());
    }


    //Se a requisiçao tiver o parametro senhaMenu recuperamos o valor enviado
    if (request->hasParam("senhaMenu")) {

      //Recupera o valor salvo no parametro senhaMenu enviado na requisiçao
      senhaMenu = String(request->getParam("senhaMenu")->value());
    }


    //Se a requisiçao tiver o parametro printSerial recuperamos o valor enviado
    if (request->hasParam("printSerial")) {

      //Recupera o valor salvo no parametro printSerial enviado na requisiçao
      printSerial = String(request->getParam("printSerial")->value());
    }


    //Se a requisiçao tiver o parametro displaySempreLigado recuperamos o valor enviado
    if (request->hasParam("displaySempreLigado")) {

      //Recupera o valor salvo no parametro displaySempreLigado enviado na requisiçao
      displaySempreLigado = String(request->getParam("displaySempreLigado")->value());
    }


    //Se a requisiçao tiver o parametro novaData recuperamos o valor enviado
    if (request->hasParam("novaData")) {

      //Recupera o valor salvo no parametro novaData enviado na requisiçao
      novaData = String(request->getParam("novaData")->value());
    }


    //Se a requisiçao tiver o parametro newHour recuperamos o valor enviado
    if (request->hasParam("newHour")) {

      //Recupera o valor salvo no parametro newHour enviado na requisiçao
      newHour = String(request->getParam("newHour")->value());
    }

    //Indica que novos dados para reprogramaçao estao disponiveis, a funçao responsavel
    //ira checar e atualizar as novas informaçoes posteriormente
    newData = true;

    //Apos verificar todos os parametros e recuperar seus valores, enviamos a pagina de confirmaçao, informando ao usuario
    //que as informaçoes foram salvos
    request->send(200, "text/html", confirm_html);
  });

  //Define as açoes tomadas quando a requisiçao "/hardReset" for solicitada (o microcontrolador ira apagar todos os arquivos internos)
  // Logo apos irá reiniciar e no inicio criara os arquivos com valores padroes
  server.on("/hardReset", HTTP_GET, [] (AsyncWebServerRequest * request) {

    //Sinaliza que um hard reset foi sinalizado a funçao responsavel
    //ira checar e resetar posteriormente
    hardReset = true;

    //Apos identificar a sinalizaçao do hard reset, enviamos a pagina de confirmaçao, informando ao usuario
    //que o hard reset ira ser execultado em breve
    request->send(200, "text/html", reset_html);
  });

  //Quando o usuario modificar a url e tentar acessar uma pagina que nao existe, ou fizer uma requisiçao invalida
  //A funçao notFound será acionada
  server.onNotFound(notFound);

  //Todas configuraçoes do servidor web foram realizadas, iniciando o servidor web
  server.begin();


}

void notFound(AsyncWebServerRequest * request) {
  /*
    Essa funçao apenas envia "Not found" para o servidor web caso seja solicitado
  */
  request->send(404, "text/plain", "Not found");
}

/* ----------------------------------------DISPLAY----------------------------------------
   Este modulo contem Todas funçoes que manipulam O display integrado e seus eventos

*/


void IRAM_ATTR mostrarDisplay() {
  /*
    Esta funçao é a funçao de interrupçao do push button, ela apenas reconhece que o push button
    foi pressionado e desativa qualquer interrupçao para nao causar efeito bounce
    e sinaliza atravez da variavel global sinalInterrupcao que o push button foi pressionado
  */

  //Desativa as interrupçoes para evitar efeito bounce
  noInterrupts();

  //Indica que iremos entrar em uma area critica do codico e fecha o semaforo para a variavel volatil sinalInterrupcao
  //Dessa forma nenhuma outra funçao pode utilizar essa variavel, uma vez que o semaforo esta fechado
  portENTER_CRITICAL_ISR(&mux);

  //Sinaliza para que possamos tratar posteriormente o click no push button
  sinalInterrupcao = 1;

  //Indica que iremos sair da area critica do codico e abre o semaforo para a variavel volatil sinalInterrupcao
  //Dessa forma qualquer outra funçao pode utilizar essa variavel, uma vez que o semaforo agora esta aberto
  portEXIT_CRITICAL_ISR(&mux);
}


void showDisplay() {
  /*
    Esta funçao trata da alternancia das telas do display quando é pressionado o pushbutton
    A variavel que contem o numero da tela atual é opcaoDisplay
    Temos 4 telas:
    opcaoDisplay = 0 - Tela desligada
    opcaoDisplay = 1 - Valores das leituras dos sensores em tempo real
    opcaoDisplay = 2 - Informaçoes de conexao do ponto de acesso
    opcaoDisplay = 3 - Informaçoes do endpoint (id/ codigo, cliente, gateway receptor)
    opcaoDisplay = 4 - Mostra quais sensores estao ativados e a taxa de amostragem de cada um


    Mapeamento do display 128x64
     ________________________________
    |* Ponto(X=1,Y=1)                |1px
    |                                | |
    |         Ponto(X=64,Y=32)       | |
    |                *               |Eixo Y
    |                                | |
    |              Ponto(X=128,Y=64) | |
    |_______________________________*|64px
    1px --------Eixo X---------128px


    Esta funçao tambem trata de iniciar e desabilitar o wifi manager caso esteja no modo automatico
  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: showDisplay()");


  //Imprime mensagem para debug na serial informando a opçao do display atual
  if (DEBUGS_PRINTS); Serial.print("OPCAO DISPLAY");
  if (DEBUGS_PRINTS); Serial.println(opcaoDisplay);

  //Indica o inicio de uma seçao critica de acesso a uma variavel volatil
  //(que pode ser acessada por duas tarefas ao mesmo tempo) e que em razao disso é
  //controlada por um semaforo
  portENTER_CRITICAL_ISR(&mux); // início da seção crítica

  //verifica se a funçao de interrupçao foi acionada (se sim sinalInterrupcao sera = 1 )
  if (sinalInterrupcao == 1) {

    //reseta o valor de sinalInterrupcao para 0
    sinalInterrupcao = 0;

    //O push button foi pressionado, entao incrementamos o valor de opcaoDisplay indicando que devemos passar para a proxima tela
    opcaoDisplay += 1;

    //Se opcaoDisplay for maior que 4, retonamos para 0, pois so temos 4 telas
    if (opcaoDisplay > 4) {
      opcaoDisplay = 0;
    }
  }

  //Habilita novamente as interrupçoes que foram desabilitadas na funçao de interrupçao
  interrupts();

  //Indica o fim de uma seçao critica de acesso a uma variavel volatil, aqui abrimos o semaforo
  portEXIT_CRITICAL_ISR(&mux);

  //Verifica se a opçao do display for diferente de 0 (0 é a opçao desligado)
  if (opcaoDisplay != 0) {

    //inicia o display embarcado
    display.ssd1306_command(SSD1306_DISPLAYON);

    //ligaDisplay recebe true indicando que o display esta habilitado
    ligaDisplay = true;

  }

  //Verifica se a opçao do display for igual a 0 (0 é a opçao desligado)
  if ((opcaoDisplay == 0)) {

    //Verifica a opçao display sempre ativado esta habilitada
    if (displayAlwaysOn) {

      //Liga o display embarcado. Mesmo a opcaoDisplay sendo 0, o embarcado está configurado para nunca desligar o display
      display.ssd1306_command(SSD1306_DISPLAYON);

      //Altero a opcaoDisplay para 1 (Valores dos sensores em tempo real)
      opcaoDisplay = 1;

    } else {

      //Se a configuraçao de display sempre ativado estiver desabilitado, desliga o display embarcado pois a opcaoDisplay atual é a 0 (display desligado)
      display.ssd1306_command(SSD1306_DISPLAYOFF);

      //ligaDisplay recebe false indicando que o display esta desabilitado
      ligaDisplay = false;
    }
  }

  //Verifica se o display esta ativado no momento
  if (ligaDisplay || displayAlwaysOn) {

    //Verifica se a opcaoDisplay é igual a 1 (Valores sensores em tempo real)
    if (opcaoDisplay == 1) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS); Serial.println("Opcao 1 ");

      //Pega o valor da ultima tensao lida, salva na variavel global ultimaTensao
      String tensao = ultimaTensao;

      //Desenha um retangulo preto na linha 2 do display de x=1 ate x = 55
      display.fillRect (1, OLED_LINE2, 55, OLED_LINE3 - 12, BLACK);

      //Posiciona o cursor na posiçao 5 da linha 2 do display
      display.setCursor(5, OLED_LINE2);

      //Printa o valor da tensao
      display.println(tensao + " V");


      //Atualiza no display as alteraçoes realizadas
      display.display();


      //Pega o valor da ultima corrente lida, salva na variavel global ultimaCoorrente
      String corrente = ultimaCorrente;

      //Desenha um retangulo preto na linha 2 do display de x=55 ate x = 130
      display.fillRect (55, OLED_LINE2, 130, OLED_LINE3 - 12, BLACK);

      //Posiciona o cursor na posiçao 70 da linha 2 do display
      display.setCursor(70, OLED_LINE2);

      //Printa o valor da corrente
      display.println(corrente + " A");


      //Atualiza no display as alteraçoes realizadas
      display.display();

      //Pega o valor da ultima temperatura lida, salva na variavel global ultimaTemperatura
      String temperatura = ultimaTemperatura;

      //Desenha um retangulo preto na linha 3 do display de x=1 ate x = 55
      display.fillRect (1, OLED_LINE3, 55, OLED_LINE4 - 23, BLACK);

      //Posiciona o cursor na posiçao 5 da linha 3 do display
      display.setCursor(5, OLED_LINE3);

      //Printa o valor da temperatura
      display.println(temperatura + " C");


      //Atualiza no display as alteraçoes realizadas
      display.display();

      //Pega o valor da ultima umidade lida, salva na variavel global ultimaUmidade
      String umidade = ultimaUmidade;

      //Desenha um retangulo preto na linha 3 do display de x=55 ate x = 130
      display.fillRect (55, OLED_LINE3, 130, OLED_LINE4 - 23, BLACK);

      //Posiciona o cursor na posiçao 70 da linha 3 do display
      display.setCursor(70, OLED_LINE3);

      //Printa o valor da umidade
      display.println(umidade + " RH");


      //Atualiza no display as alteraçoes realizadas
      display.display();

      //Pega o valor da ultima co2 lida, salva na variavel global ultimaCo2
      String co2 =  ultimaCo2;

      //Desenha um retangulo preto na linha 4 do display de x=1 ate x = 55
      display.fillRect (1, OLED_LINE4, 55, OLED_LINE5 - 33, BLACK);

      //Posiciona o cursor na posiçao 5 da linha 4 do display
      display.setCursor(5, OLED_LINE4);

      //Printa o valor do co2
      display.println(co2 + " PPM");

      //Atualiza no display as alteraçoes realizadas
      display.display();

      //Pega o valor da ultima tvoc lida, salva na variavel global ultimaTvoc
      String tvoc = ultimaTvoc;

      //Desenha um retangulo preto na linha 4 do display de x=55 ate x = 130
      display.fillRect (55, OLED_LINE4, 130, OLED_LINE5 - 33, BLACK);

      //Posiciona o cursor na posiçao 70 da linha 4 do display
      display.setCursor(70, OLED_LINE4);

      //Printa o valor do tvoc
      display.println(tvoc + " TVOC");

      //Atualiza no display as alteraçoes realizadas
      display.display();

      //Desenha um retangulo preto na linha 5 do display de x=1 ate x = 130
      display.fillRect (1, OLED_LINE5, 130, OLED_LINE6 - 40, BLACK);

      //Posiciona o cursor na posiçao 5 da linha 5 do display
      display.setCursor(5, OLED_LINE5);

      //Printa o valor da ultima data e hora que foi lida no RTC
      display.println(ultimaDataHora);

      //Atualiza no display as alteraçoes realizadas
      display.display();
    }

    //Verifica se a opcaoDisplay é diferente de 2 (Informaçoes do ponto de acesso)
    if (opcaoDisplay != 2) {

      //Se o modo de funcionamento do ponto de acesso estiver em auto (o ponto de acesso so ativa na tela de informaçoes sobre o ponto de acesso)
      if (modoAccessPoint.equals("auto")) {

        //O ponto de acesso é desativado pois nao estamos na opcaoDisplay = 2 (Informaçoes do ponto de acesso)
        WiFi.softAPdisconnect (true);
      }
    }

    //Verifica se a opcaoDisplay é igual a 2 (Informaçoes do ponto de acesso)
    if (opcaoDisplay == 2) {

      //Se o modo de funcionamento do ponto de acesso estiver em auto (o ponto de acesso so ativa na tela de informaçoes sobre o ponto de acesso)
      if (modoAccessPoint.equals("auto")) {

        //O ponto de acesso é ativado pois estamos na opcaoDisplay = 2 (Informaçoes do ponto de acesso)
        configWifiManager();
      }

      //Obtenho o login e a senha do ponto de acesso
      String ssid  = getSsidAp();
      String senha = getSenhaAp();

      //Desenha um retangulo (Parametros: X inicial / Y inicial/ X final / Y Final / Cor do retangulo)
      display.fillRect (1, 1, 128, 64, BLACK);

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE2);

      //Escreve a string no display )
      display.println("Ponto de Acesso Wifi");

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE4);

      //Escreve a string no display )
      display.println("SSID: " + ssid);

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE5);


      //Escreve a string no display )
      display.println("Senha: " + senha);

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(15, OLED_LINE6);

      //Escreve a string no display )
      display.println("IP: 192.168.4.1");


      //Atualiza no display as alteraçoes realizadas
      display.display();


    }

    //Verifica se a opcaoDisplay é igual a 3 (Dados do endpoint)
    if (opcaoDisplay == 3) {

      //Obtenho os dados do endpoint que irei apresentar ID, Gateway vinculado e o Cliente
      String id = getEndpoint();
      String gateway = getGateway();
      String cliente = getCliente();

      //Desenha um retangulo (Parametros: X inicial / Y inicial/ X final / Y Final / Cor do retangulo)
      display.fillRect (1, 1, 128, 64, BLACK);

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(20, OLED_LINE2);

      //Escreve a string no display )
      display.println("ENDPOINT");

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE3);

      //Escreve a string no display )
      display.println("ID: " + id);

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE4);

      //Escreve a string no display )
      display.println("Gateway: " + gateway);

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(30, OLED_LINE5);

      //Escreve a string no display )
      display.println("CLIENTE: ");

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE6);

      //Escreve a string no display )
      display.println(cliente);

      //Atualiza no display as alteraçoes realizadas
      display.display();

    }

    //Verifica se a opcaoDisplay é igual a 4 (Informaçoes sobre os sensores ativados e a taxa de amostragem)
    if (opcaoDisplay == 4) {

      //Inicia todos os sensores como desativados, se estiverem ativados as variaveis serao atualizadas para ON
      String tensaoAtivo = "OFF" ;
      String correnteAtivo = "OFF" ;
      String tempAtivo = "OFF" ;
      String co2Ativo = "OFF" ;

      //Verifica se o sensor de tensao está configurado para ler dados
      if (getSensorHabilitado("tensao").equals("true")) {

        //Atualiza a variavel para ON
        tensaoAtivo = "ON";
      }

      //Verifica se o sensor de corrente está configurado para ler dados
      if (getSensorHabilitado("corrente").equals("true")) {

        //Atualiza a variavel para ON
        correnteAtivo = "ON";
      }

      //Verifica se o sensor de temperatura e umidade está configurado para ler dados
      if (getSensorHabilitado("temperatura").equals("true")) {

        //Atualiza a variavel para ON
        tempAtivo = "ON";
      }

      //Verifica se o sensor de co2/tvoc está configurado para ler dados
      if (getSensorHabilitado("co2").equals("true")) {

        //Atualiza a variavel para ON
        co2Ativo = "ON";
      }

      //obtem a taxa de amostragem da tensao e converte o valor de segundos para minutos
      String tensaoAmos = String((getAmostragem("tensao").toInt() / 60));

      //obtem a taxa de amostragem da corrente e converte o valor de segundos para minutos
      String correnteAmos = String((getAmostragem("corrente").toInt() / 60));

      //obtem a taxa de amostragem da temperatura e converte o valor de segundos para minutos
      String tempAmos = String((getAmostragem("temperatura").toInt() / 60));

      //obtem a taxa de amostragem do co2 e converte o valor de segundos para minutos
      String co2Amos = String((getAmostragem("co2").toInt() / 60));

      //Desenha um retangulo (Parametros: X inicial / Y inicial/ X final / Y Final / Cor do retangulo)
      display.fillRect (1, 1, 128, 64, BLACK);

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(30, OLED_LINE2);

      //Escreve a string no display
      display.println("SENSORES");

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE3);

      //Escreve a string no display
      display.println("Tensao -" + tensaoAtivo + " - " + tensaoAmos + "M");

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE4);

      //Escreve a string no display
      display.println("Corrente - " + correnteAtivo + " - " + correnteAmos + "M");

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE5);

      //Escreve a string no display
      display.println("Temp/Um - " + tempAtivo + " - " + tempAmos + "M");

      //Posiciona o cursor para escrita (Parametros: Posiçao X inicial / Posiçao Y inicial)
      display.setCursor(5, OLED_LINE6);

      //Escreve a string no display
      display.println("Co2/Tvoc -" + co2Ativo + " - " + co2Amos + "M");

      //Atualiza no display as alteraçoes realizadas
      display.display();

    }

  }

}
/* ----------------------------------------INICIALIZAÇAO----------------------------------------
   Este modulo contem Todas funçoes que sao executadas quando o endpoin é iniciado
   Este é o periodo em que é mostrado o splash no display do microcontrolador
*/

void configInicialSensores() {
  /*
      Esta funçao inicializa todos os modulo externos para que possam ser utilizados no decorrer
      da execuçao do sketch.

      Desenha o splash logo e a mensagem de aguarde no display do embarcado (independe de configuraçoes de ativaçao do display)
      Habilita os pinos dos sensores analogicos
      Testa os endereçoes dos sensores e modulos i2c
      Realiza 10 leituras iniciais de cada sensor (nao sao salvas ou enviadas) para calibrar os sensores

  */
  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Callback: configInicialSensores()");

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Inicializando display OLED");

  //Inicia a pinagem I2C do display embarcado nos pinos 4,15 do esp32
  Wire.begin(4, 15);

  //Tenta inicializar o display embarcado chamando a funçao da biblioteca proprietaria do display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {

    //Imprime mensagem para debug na serial se o display nao inicializar
    if (DEBUGS_PRINTS);  Serial.println("Falha ao inicializar display OLED");


  } else {
    //Imprime mensagem para debug na serial se o display inicializar corretamente
    if (DEBUGS_PRINTS);  Serial.println("Display OLED inicializacao com  sucesso");

    //Verifica se e o micro controlador reiniciou por tempo (o micro controlador reinicia a cada 1 hora de uso)
    if (getRestartPadrao().equals("true")) {

      //Se reiniciou por  tempo ele nao mostra o splash screen
      setRestartPadrao("false");
    } else {

      //Se nao reiniciou por tempo, e sim por outro motivo, mostra o splash screen

      //limpa o display embarcado
      display.clearDisplay();

      //configura a fonte para o tmanho 4
      display.setTextSize(4);

      //Configura a cor do texto branca
      display.setTextColor(WHITE);

      //Desenha um retangulo 128x64 (todo o display) na cor preta
      display.fillRect (1, 1, 128, 64, BLACK);

      //coloca o cursor na posiçao 30 da segunda linha do display
      display.setCursor(30, OLED_LINE2);

      //Escreve a string EC+
      display.println("EC+");

      //configura a fonte para tamanho 1
      display.setTextSize(1);

      //coloca o cursor na posiçao x = 5 e y =43 do eixo cartesiano do display
      display.setCursor(5, 43);

      //Ecreve a string no display
      display.println("Iniciando Endpoint");

      //coloca o cursor na posiçao x = 30 e y =53 do eixo cartesiano do display
      display.setCursor(30, 53);

      //Ecreve a string no display
      display.println("Aguarde...");

      //inicia a visualizaçao no display (todas as configuraçoes feitas anteriormente so serao visualizadas aqui)
      //Atualiza no display as alteraçoes realizadas
      display.display();

      //reseta o arquivo que quarda os dados de reinicializaçao
      setRestartPadrao("false");

    }

  }

  //Se o  sensor de tensao estiver habilitado
  if (sensorHabilitado("tensao")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Iniciando Sensor de tensao");

    //Instancia o sensor zmtp de tensao
    //PASSA PARA A FUNÇÃO OS PARÂMETROS (PINO ANALÓGIO / VALOR DE CALIBRAÇÃO / MUDANÇA DE FASE)
    emon1.voltage(pino_zmtp, VOLT_CAL, 1.7);

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Sensor de tensao iniciado com sucesso");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Aferindo mediçao dos sensores");

    //Se o sensor tensao estiver habilitado afere a mediçao 3 vezes consecultivas
    for (int i = 0; i <= 3; i++) {
      getTensaoAC();
    }

  }

  //Se o  sensor de corrente estiver habilitado
  if (sensorHabilitado("corrente")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Iniciando Sensor de corrente");

    //Instancia o sensor sct de corrente
    //PASSA PARA A FUNÇÃO OS PARÂMETROS (PINO ANALÓGIO / VALOR DE CALIBRAÇÃO)
    emon1.current(pino_sct, CUR_CAL);

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Sensor de corrente iniciado com sucesso");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Aferindo mediçao dos sensores");

    //afere a mediçao do sensor de corrente 3 vezes consecultivas
    for (int i = 0; i <= 3; i++) {
      getCorrenteAC();
    }

  }

  //Se o  sensor de temperatura/umidade estiver habilitado
  if (sensorHabilitado("temperatura")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Iniciando Sensor DHT22 para umidade e temperatura");

    //Inicia o sensor dht22 que é I2C
    dht.begin();

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Sensor DHT22 iniciado com sucesso");

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Aferindo mediçao dos sensores");

    //Afere os sensores de temperatura e umidade 3 vezes consecultivas
    for (int i = 0; i <= 3; i++) {
      getTemperatura();
      getUmidade();
    }

  }

  //Se o  sensor de co2/tvoc estiver habilitado
  if (sensorHabilitado("co2")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("Iniciando Sensor CCS811 para CO2e TVOC");

    //Se o sensor ccs811 iniciar com sucesso
    if (ccs.begin()) {

      //habilita o sensor no modo 4 (realiza 4 leituras por segundo)
      ccs.setDriveMode(4);

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("Sensor de CO2 e TVOC iniciado com sucesso");

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS);  Serial.println("Aferindo mediçao dos sensores");

      //afere a leitura do CO2 e TVOC 3 vezes
      for (int i = 0; i <= 3; i++) {
        getCO2();
        getTVOC();
      }

    } else {
      //Imprime mensagem para debug na serial caso o sensor ccs811 nao iniciar com sucesso
      if (DEBUGS_PRINTS);  Serial.println("Sensor de CO2 e TVOC NAO iniciado");
    }
  }


  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Iniciando Real time Clock");

  //verifica se o modulo RTC (i2c) iniciou com sucesso
  if (rtc.begin()) {

    //Imprime mensagem para debug na serial caso o rtc seja iniciado com sucesso
    if (DEBUGS_PRINTS);  Serial.println("RTC iniciado com sucesso");

  } else {
    //Imprime mensagem para debug na serial caso o RTC nao seja iniciado
    if (DEBUGS_PRINTS);  Serial.println("RTC NAO iniciado");
  }

  //Limpa o display embarcado
  display.clearDisplay();

  //Atualiza a vizualizaçao do display
  //Atualiza no display as alteraçoes realizadas
  display.display();

  //Se o display estiver configurado para permanecer sempre ativado
  if (displayAlwaysOn) {

    //A tela principal do display sera a tela 1 que dispoe em tempo real as medidas lidas pelos sensores
    opcaoDisplay = 1;

  } else {

    //Desliga o display embarcado
    display.ssd1306_command(SSD1306_DISPLAYOFF);
  }
}



void setup() {

  /*
     Funçao setup (ou main) padrao do arduino
     Essa funçao é a primeira funçao a ser execultada
     É imperativo que seja void, NAO adicione comandos de retorno nesta funçao

  */
  //Inicializa a comunicaçao serial em 115200 bits/segundo
  Serial.begin(115200);

  //Cria um delay de 2 segundos para esperar a inicalizaçao da serial
  delay(2000);

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Iniciando debug");

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Iniciando Flash");
  flash.inicializaFlash();

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("Armazenamento Flash iniciado");

  //Chama a funçao criaEstruturaInicial() que é responsavel por realizar a checagem dos arquivos
  criaEstruturaInicial();

  //Verifica se o endpoint está programado para permanecer com o display sempre ativado
  getDisplaySempreLigado();

  //Realiza a configuraçao inicial dos sensores (habilita e realiza testes)
  configInicialSensores();

  //Inicia o pino do sensor de Tensao como entrada
  pinMode(pino_zmtp, INPUT);

  //Inicia o pino do push button como entrada com um resistor de pull up para evitar estado flutuante
  pinMode(pino_botao, INPUT_PULLUP);

  //Limpa o display do embarcado
  display.clearDisplay();

  //Cria uma interrupçao para o push button e vincula a interrupçao a funçao mostrarDisplay
  attachInterrupt(digitalPinToInterrupt(pino_botao), mostrarDisplay, CHANGE);

  //Se o ponto de acesso wifi foi configurado para ficar ativado durante toda a operaçao ele é ativado aqui
  if (modoAccessPoint.equals("ativado")) {
    configWifiManager();
  }

}

/* ----------------------------------------EXECUÇAO DA ROTINA----------------------------------------
   Este modulo contem apenas a funçao loop responsavel pera execuçao da rotina ciclica do microcontrolador

   Verifica se está no momento de realizar a leitura dos sensores
   Envia dados pendentes
   Verifica reconfiguraçoes pendentes
   Verifica se existe hardReset pendente
   Verifica se está no momento do reinicio padrao
   Entra no deepSleep
*/


void loop() {
  if (DEBUGS_PRINTS); Serial.println("Calback: loop()");

  //verifica se posso realizar a leitura da tensao neste instante
  if (realizarLeitura("tensao")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n loop - Tensao AC: ");

    //realiza a leitura da data e hora atual no formato unix(millissegundos desde 01/01/1970 00:00:00 ) e salva em uma variavel do tipo long
    long dataHoraAtualTensao = (long)getDataHora("unix").toInt();

    //Realiza a leitura do sensor ZMTP
    String tensao = getTensaoAC();

    //realiza a leitura da data e hora em formato datetime dd/mm/aaaa hh:mm:ss
    String dataHoraTensao = getDataHora();

    //Atualiza a variavel globlal ultimaDataHora para que seja mostrada no display
    ultimaDataHora = dataHoraTensao;

    //Chama a funçao salvaDados para salvar os dados recolhidos do sensor anteriormente
    salvaDados("tensao", tensao, "V (Volts)", dataHoraTensao);

    //Atualiza a variavel globlal ultimaTensao para que seja mostrada no display
    ultimaTensao = tensao;

    //Obtem a taxa de leitura da tensao e salva na variavel amostragemTensao, para posteriormente calcular a data e hora da proxima leitura da tensao
    long amostragemTensao = (long)getAmostragem("tensao").toInt();

    //Calcula a data e a hora da proxima leitura da Tensao somando a data e Hora Atual (dataHoraAtualTensao) com a taxa de amstragem da Tensao (amostragemTensao)
    String proxLeituraTensao = String(dataHoraAtualTensao + amostragemTensao);

    //Salva a data e a hora da leitura da proxima tensao
    setProxAmostragem("tensao", proxLeituraTensao);


    //Chama a função para atualizar o display
    showDisplay();
  }

  //verifica se posso realizar a leitura da corrente neste instante
  if (realizarLeitura("corrente")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n loop - Corrente AC: ");

    //realiza a leitura da data e hora atual no formato unix(millissegundos desde 01/01/1970 00:00:00 ) e salva em uma variavel do tipo long
    long dataHoraAtualCorrente = getDataHora("unix").toInt();

    //Realiza a leitura do sensor sct
    String corrente = getCorrenteAC();

    //realiza a leitura da data e hora em formato datetime dd/mm/aaaa hh:mm:ss
    String dataHoraCorrente = getDataHora();

    //Atualiza a variavel globlal ultimaDataHora para que seja mostrada no display
    ultimaDataHora = dataHoraCorrente;

    //Chama a funçao salvaDados para salvar os dados recolhidos do sensor anteriormente
    salvaDados("corrente", corrente, "A (Amperes)", dataHoraCorrente);

    //Atualiza a variavel globlal ultimaCorrente para que seja mostrada no display
    ultimaCorrente = corrente;

    //Obtem a taxa de leitura da corrente e salva na variavel amostragemCorrente, para posteriormente calcular a data e hora da proxima leitura da corrente
    long amostragemCorrente = getAmostragem("corrente").toInt();

    //Calcula a data e a hora da proxima leitura da corrente somando a data e Hora Atual (dataHoraAtualCorrente) com a taxa de amstragem da corrente (amostragemCorrente)
    String proxLeituraCorrente = String(dataHoraAtualCorrente + amostragemCorrente);

    //Salva a data e a hora da leitura da proxima corrente
    setProxAmostragem("corrente", proxLeituraCorrente);


    //Chama a função para atualizar o display
    showDisplay();
  }

  //verifica se posso realizar a leitura da umidade neste instante
  if (realizarLeitura("umidade")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n loop - Umidade: ");

    //realiza a leitura da data e hora atual no formato unix(millissegundos desde 01/01/1970 00:00:00 ) e salva em uma variavel do tipo long
    long dataHoraAtualUmidade = getDataHora("unix").toInt();

    //Realiza a leitura do sensor DHT22
    String umidade = getUmidade();

    //realiza a leitura da data e hora em formato datetime dd/mm/aaaa hh:mm:ss
    String dataHoraUmidade = getDataHora();

    //Atualiza a variavel globlal ultimaDataHora para que seja mostrada no display
    ultimaDataHora = dataHoraUmidade;


    //Chama a funçao salvaDados para salvar os dados recolhidos do sensor anteriormente
    salvaDados("umidade", umidade, "RH (Umidade Relativa)", dataHoraUmidade);

    //Atualiza a variavel globlal ultimaUmidade para que seja mostrada no display
    ultimaUmidade = umidade;

    //Obtem a taxa de leitura da Umidade e salva na variavel amostragemUmidade, para posteriormente calcular a data e hora da proxima leitura da Umidade
    long amostragemUmidade = getAmostragem("umidade").toInt();

    //Calcula a data e a hora da proxima leitura da Umidade somando a data e Hora Atual (dataHoraAtualUmidade) com a taxa de amstragem da Umidade (amostragemUmidade)
    String proxLeituraUmidade = String(dataHoraAtualUmidade + amostragemUmidade);

    //Salva a data e a hora da leitura da proxima umidade
    setProxAmostragem("umidade", proxLeituraUmidade);


    //Chama a função para atualizar o display
    showDisplay();

  }

  //verifica se posso realizar a leitura do co2 neste instante
  if (realizarLeitura("co2")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n loop - CO2: ");

    //realiza a leitura da data e hora atual no formato unix(millissegundos desde 01/01/1970 00:00:00 ) e salva em uma variavel do tipo long
    long dataHoraAtualCO2 = getDataHora("unix").toInt();

    //Realiza a leitura do sensor CCS811
    String co2 = getCO2();

    //realiza a leitura da data e hora em formato datetime dd/mm/aaaa hh:mm:ss
    String dataHoraCo2 = getDataHora();

    //Atualiza a variavel globlal ultimaDataHora para que seja mostrada no display
    ultimaDataHora = dataHoraCo2;

    //Chama a funçao salvaDados para salvar os dados recolhidos do sensor anteriormente
    salvaDados("co2", co2, "PPM (Particulas por milhão)", dataHoraCo2);

    //Atualiza a variavel globlal ultimaCo2 para que seja mostrada no display
    ultimaCo2 = co2;

    //Obtem a taxa de leitura do CO2  e salva na variavel amostragemCO2 , para posteriormente calcular a data e hora da proxima leitura do CO2
    long amostragemCO2 = getAmostragem("co2").toInt();

    //Calcula a data e a hora da proxima leitura da CO2 somando a data e Hora Atual (dataHoraAtualCO2) com a taxa de amstragem da CO2 (amostragemCO2)
    String proxLeituraCO2 = String(dataHoraAtualCO2 + amostragemCO2);

    //  Salva a data e a hora da leitura da proxima Co2
    setProxAmostragem("co2", proxLeituraCO2);

    //Chama a função para atualizar o display
    showDisplay();
  }

  //verifica se posso realizar a leitura do TVOC neste instante
  if (realizarLeitura("tvoc")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n loop - TVOC: ");

    //realiza a leitura da data e hora atual no formato unix(millissegundos desde 01/01/1970 00:00:00 ) e salva em uma variavel do tipo long
    long dataHoraAtualTVOC = getDataHora("unix").toInt();

    //Realiza a leitura do sensor CCS811
    String tvoc = getTVOC();

    //realiza a leitura da data e hora em formato datetime dd/mm/aaaa hh:mm:ss
    String dataHoraTvoc = getDataHora();

    //Atualiza a variavel globlal ultimaDataHora para que seja mostrada no display
    ultimaDataHora = dataHoraTvoc;

    //Chama a funçao salvaDados para salvar os dados recolhidos do sensor anteriormente
    salvaDados("tvoc", tvoc, "TVOC (Total de compostos organicos  volateis)", dataHoraTvoc);

    //Atualiza a variavel globlal ultimaTvoc para que seja mostrada no display
    ultimaTvoc = tvoc;

    //Obtem a taxa de leitura da TVOC  e salva na variavel amostragemTVOC, para posteriormente calcular a data e hora da proxima leitura da TVOC
    long amostragemTVOC = getAmostragem("tvoc").toInt();

    //Calcula a data e a hora da TVOC proxima leitura da TVOC somando a data e Hora Atual (dataHoraAtualTVOC) com a taxa de amstragem da TVOC (amostragemTVOC)
    String proxLeituraTVOC = String(dataHoraAtualTVOC + amostragemTVOC);

    //Salva a data e a hora da leitura da proxima tvoc
    setProxAmostragem("tvoc", proxLeituraTVOC);


    //Chama a função para atualizar o display
    showDisplay();

  }

  //verifica se posso realizar a leitura da temperatura neste instante
  if (realizarLeitura("temperatura")) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n loop - Temperatura: ");

    //realiza a leitura da data e hora atual no formato unix(millissegundos desde 01/01/1970 00:00:00 ) e salva em uma variavel do tipo long
    long dataHoraAtualTemperatura = getDataHora("unix").toInt();

    //Realiza a leitura do sensor DHT22
    String temperatura = getTemperatura();

    //realiza a leitura da data e hora em formato datetime dd/mm/aaaa hh:mm:ss
    String dataHoraTemperatura = getDataHora();

    //Atualiza a variavel globlal ultimaDataHora para que seja mostrada no display
    ultimaDataHora = dataHoraTemperatura;

    //Chama a funçao salvaDados para salvar os dados recolhidos do sensor anteriormente
    salvaDados("temperatura", temperatura, "C (Celcius)", dataHoraTemperatura);

    //Atualiza a variavel globlal ultimaTemperatura para que seja mostrada no display
    ultimaTemperatura = temperatura;

    //Obtem a taxa de leitura da Temperatura e salva na variavel amostragemTemperatura, para posteriormente calcular a data e hora da proxima leitura da Temperatura
    long amostragemTemperatura = getAmostragem("temperatura").toInt();

    //Calcula a data e a hora da proxima leitura da Temperatura somando a data e Hora Atual (dataHoraAtualTemperatura) com a taxa de amstragem da Temperatura(amostragemTemperatura)
    String proxLeituraTemperatura = String(dataHoraAtualTemperatura + amostragemTemperatura);

    //Salva a data e a hora da leitura da proxima temperatura
    setProxAmostragem("temperatura", proxLeituraTemperatura);


    //Chama a função para atualizar o display
    showDisplay();

  }

  //Variavel long que armazena o tempo em millissegundos do inicio do loop abaixo
  long tempoInicialEnviarDados = millis();

  //O loop abaixo executara por ate 2 minutos ou houver um break
  while (millis() < tempoInicialEnviarDados + 120000) {

    //Chama a função para atualizar o display
    showDisplay();

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n loop - Permanece no loop de envio dos dados: ");

    //Verifica se os dados foram enviados com sucesso, se sim ele continua o envio até esgotar os 2 minutos, ou nao ter mais dados na fila de envio
    if (!enviaDados()) {

      //Imprime mensagem para debug na serial
      if (DEBUGS_PRINTS); Serial.println("\n loop - Houve algum erro no envio dos dados ");

      //Se houve algum erro no envio dos dados nao tentarei enviar novamente, irei abortar o envio
      break;

    }

    //fim do loop de envio de dados
  }

  //Desativa o protocolo lora para poupar energia, uma vez que o protoclo lora nao é mais necessario por hora
  desligaLora();

  //Chama a função para atualizar o display
  showDisplay();

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("\n loop - Checando reprogramaçoes pendentes ");

  //Checa se existe reprogramaçoes, se houver as realiza
  checaConfiguracoesPendentes();

  //Chama a função para atualizar o display
  showDisplay();

  //Verifica se existe um hard reset pendente
  if (hardReset == true) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("\n loop - HARD RESET EM ANDAMENTO AGUARDE");

    //Chama a funçao que apaga todos os arquivos na flash
    flash.resetarFlash("/", 0);

    //Reinicia o microcontrolador, quando for iniciado ele recriará todos os arquivos com os valores default
    ESP.restart();
  }



  //Verifica se o microcontrolador está a mais de 1 hora ativo
  if ( millis() >= 3600000) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS);  Serial.println("\n loop - REINICIANDO ESP (REINICIO PADRAO A CADA 1 HORA)");

    //Salva um arquivo informando que foi um reestart padrao e nao é necessario mostrar o splash logo na proxima inicializaçao
    setRestartPadrao("true");

    //Reinicia o microcontrolador
    ESP.restart();
  }

  //Obtem a data e hora unix de todos os sensores
  long proxLeituras[6] = {getProxAmostragem("tensao").toInt(),
                          getProxAmostragem("corrente").toInt(),
                          getProxAmostragem("umidade").toInt(),
                          getProxAmostragem("co2").toInt(),
                          getProxAmostragem("tvoc").toInt(),
                          getProxAmostragem("temperatura").toInt()
                         };

  //variavel que guarda a proxima leitura mais proxima, necessario para calcular o tempo em que o microcontrolador entrara em deep sleep
  long menorProxLeitura = proxLeituras[0];

  //Seleciona a menor proxima leitura
  for ( int i = 0; i < 6; i ++) {
    // Aqui voce faze o seguinte
    if ((proxLeituras[i] > 0 ) && (proxLeituras[i] < menorProxLeitura)) {
      menorProxLeitura = proxLeituras[i];
    }
  }

  //Com base na hora atual e na proxima leitura, calcula o tempo que o esp podera dormir
  long dataHoraAtual = getDataHora("unix").toInt();
  long tempoParaDormirEmSegundos = menorProxLeitura - dataHoraAtual;
  long tempoParaDormirMicrossegundos = tempoParaDormirEmSegundos * 1000000;

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS);  Serial.println("\n INICIANDO O SONO PROFUNDO POR " + String(tempoParaDormirEmSegundos) + " SEGUNDOS)");

  //delay(tempoParaDormirEmSegundos * 1000);

  /*
    //Configura o tempo do deep sleep
    esp_sleep_enable_timer_wakeup(tempoParaDormirMicrossegundos);

    //configura detalhes de hardware do deep sleep
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

    //Inicia de fato o deep sleep
    esp_deep_sleep_start();
  */
  //Informa que o restart é padrao e nao ocasionado por um erro
  //setRestartPadrao("true");

  //Reinicia o microcontrolador apos sair do sono profundo
  //ESP.restart();

}


/* ----------------------------------------DADOS DOS SENSORES ----------------------------------------
   Este modulo contém Todas funções que manipulam, salvam na memória interna,
   validam os dados recebidos e enviam os dados lidos pelos sensores via lora

   Funções:

   bool enviaDados()
   void salvaDados(String sensor, String valor, String unMedida, String dataHora)
   bool processaMsgRecebida(String mensagem, String tipoDeRespostaEsperado)

*/

bool enviaDados() {
  /*
     Essa função é a função principal responsável pela comunicação lora com o gateway e
     as suas respectivas temporizações

     1º Verifica se existe dado pendente de envio no flash
     2º Serializa os dados em um .json
     3º Aguarda o gateway solicitar o envio dos dados
     4º Envia os dados
     5º Recebe confirmação de recebimento ou um json com reprogramações
     6º Envia a confirmação de recebimento de reprogramação

     Retorna um booleano:
      True se conseguiu enviar os dados
      False se não conseguiu enviar os dados
  */

  if (DEBUGS_PRINTS); Serial.println("Calback: enviaDados()");


  // Le o id do último arquivo que realmente foi enviado para o gateway sem erros, e converte o id para int e salva na variável ultimoEnviado
  int ultimoEnviado = getUltimoArquivoEnviado().toInt();

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("\n enviaDados - arquivo /ultimoArquivoEnviado.txt lido com sucesso:\t VALUE: " + (String(ultimoEnviado)));

  //Por razoes do tamanho do armazenamento do flash do microcontrolador, foi limitado que só poderão ser gravados 1001 dados,
  //então se o ultimo enviado + 1 for igual a 1001, último enviado recebe o valor de -1, para que dessa forma seja realizado
  // a tentativa de envio do arquivo de número 0
  if (ultimoEnviado + 1 == 1001) {
    ultimoEnviado = -1;
  }

  //Com base no id do arquivo que foi lido posteriormente gera uma String com o nome do arquivo completo
  String proxEnviar = String(ultimoEnviado + 1);
  String nome = "/arq-Pendente-" + proxEnviar + ".txt";

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("\n enviaDados - próximo arquivo a ser lido " + nome);

  //converte o  Objeto String contendo o nome para um char array, algumas funções do Arduino operam apenas com char array
  char arq[nome.length() + 3];
  nome.toCharArray(arq, nome.length() + 3);

  //Verifica se o arquivo com o nome que obtemos existe e salva o resultado da verificação na variável bool prontoParaEnviar
  //só irá proceder para o envio, se o arquivo existir
  bool prontoParaEnviar = flash.arquivoExiste(arq);

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("\n enviaDados - verificando se o arquivo " + nome + " existe");

  if (prontoParaEnviar) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n enviaDados - O arquivo " + nome + " existe, proceder para o envio");

    //lendo o arquivo o qual obtemos o nome anteriormente e verificamos se o mesmo realmente existe, e salva os dados na variável arquivo
    String arquivo = flash.lerArquivo(arq);

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Dados presentes no arquivo: " + arquivo);

    //Os dados estão salvos em um JSON, nas próximas linhas vou deserializar o JSON para obter os dados salvos nele

    //calcula qual o tamanho do Object Json
    const size_t capacidade = JSON_OBJECT_SIZE(9) + 200;

    //Cria uma instancia do objeto JSON informando seu tamanho
    DynamicJsonDocument documento(capacidade);

    //Realiza o cast da variável com os dados do JSON de Object String para char array e salva na variável arqJson
    int tamanhoArquivo = arquivo.length() + 3;
    char arqJson[tamanhoArquivo];
    arquivo.toCharArray(arqJson, tamanhoArquivo);

    //Deserializa o json contido na variável arqJson e salvo na variável documento que é um Object DynamicJsonDocument
    deserializeJson(documento, arqJson);

    //Obtém os valores salvos no Json e salva em variáveis Object String
    String sensor = documento["sensor"];
    String valor =  documento["valor"];
    String  un_medida = documento["unidadeMedida"];
    String dataHora = documento["dataHora"];


    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Entrando no loop por até 2 min ou até chegar à mensagem do gateway pedindo os dados");

    //variável long de tempo para controle do loop while
    long tempoInicialEsperarReceberPedido = millis();

    //O loop while irá operar até que se tenha passado 30000 milissegundos (30 segundos) do tempo salvo em tempoInicialEsperarReceberPedido
    // Ou que haja um break interno que force o loop a parar
    while (millis() < tempoInicialEsperarReceberPedido + 30000) {

      //Chama a função para atualizar o display
      showDisplay();

      //chama a função para receber uma mensagem via lora
      String mensagemRecebida = recebeMsgLora();

      //String com o tipo de mensagem que eu espero receber do gateway
      String mensagemEsperada = "ENVIAR-DADOS";

      //verifico se a mensagem que eu recebi é a mensagem que eu esperava ter  recebido, neste caso o gateway pedindo o envio de dados
      if (processaMsgRecebida(mensagemRecebida, mensagemEsperada)) {

        //Imprime mensagem para debug na serial
        if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Mensagem, pedindo o envio de dados recebida, gerando o JSON");

        //Gera um Object String contendo o Json com os dados completos de leitura
        String json = "{\"id\":\"" + getID() + "\",\"mensagem\":\"DADOS\",\"cliente\":\"" + getCliente() + "\",\"endpoint\":\"" + getEndpoint() + "\",\"gateway\":\"" + getGateway() + "\",\"sensor\":\"" + sensor + "\",\"data_hora\":\"" + dataHora + "\",\"valor\":" + valor + ",\"un_medida\":\"" + un_medida + "\",\"taxa_amostragem\":" + getAmostragem(sensor) + ",\"bateria\":\"" + getBateria() + "\"}" ;

        //Imprime mensagem para debug na serial
        if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Json Gerado com sucesso \valor: " + json);

        //Imprime mensagem para debug na serial
        if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Entrando no loop por até 1 min para enviar os dados para o Gateway, ou receber uma confirmação de recebimento do gateway");

        //variável long de tempo para controle do loop while
        long tempoInicialEnvioDados = millis();

        //O loop while irá operar até que se tenha passado 60000 milissegundos (60 segundos) do tempo salvo em tempoInicialEnvioDados
        // Ou que haja um break interno que force o loop a parar
        while (millis() < tempoInicialEnvioDados + 60000) {

          //Chama a função para atualizar o display
          showDisplay();

          //Imprime mensagem para debug na serial
          if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Enviando mensagem");

          //Chama a função que envia os dados via Lora
          enviaMsgLora(json);

          //Imprime mensagem para debug na serial
          if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Mensagem enviada");

          //Chama a função que recebe os dados via Lora e salva os dados recebidos na String mensagemRecebida
          String mensagemRecebida = recebeMsgLora();

          //Cria os Object String com as mensagens que podem ser recebidas do gateway
          String primeiraMensagemEsperada = "REPROGRAMACAO";
          String segundaMensagemEsperada = "DADOS-RECEBIDOS-COM SUCESSO";

          //Salva o id/código do gateway que este endpoint está configurado a enviar em uma String (Será necessário para enviar uma resposta ao gateway posteriormente)
          String nomeGateway = getGateway();

          //Salva o id/código do endpoint em uma String (será necessário para enviar uma resposta ao gateway posteriormente)
          String nomeEndpoint = getEndpoint();

          //Verifica se a mensagem que foi recebida é uma mensagem de reprogramação
          if (processaMsgRecebida(mensagemRecebida, primeiraMensagemEsperada)) {

            //Chama a função para atualizar o display
            showDisplay();

            //Imprime mensagem para debug na serial
            if (DEBUGS_PRINTS); Serial.println("\n recebi uma mensagem de reprogramação \n iniciando processamento da reprogramação");

            //chama a função que irá obter os dados de reprogramação do Json recebido e procede com a reprogramação
            //Observe que a reprogramação não é instantânea, os dados são salvos, mas só passam a ser validos após o microcontrolador reiniciar
            //e ele irá reiniciar apenas no fim do ciclo da função loop
            processaReprogramacao(mensagemRecebida);

            //Gera um JSON informando que a mensagem de reprogramação foi recebida com sucesso
            String msgReprogramacaoOK = "{\"gateway\":\"" + nomeGateway + "\",\"endpoint\":\"" + nomeEndpoint + "\",\"mensagem\":\"REPROGRAMACAO-RECEBIDA-COM-SUCESSO\"}";

            //Imprime mensagem para debug na serial
            if (DEBUGS_PRINTS); Serial.println("\n enviaDados - Entrando no loop por até 1 min para enviar  a confirmação de reprogramação e obter uma resposta valida do gateway");

            //variável long de tempo para controle do loop while
            long tempoInicialEnviaRespReprog = millis();

            //O loop while irá operar até que se tenha passado 60000 milissegundos (60 segundos) do tempo salvo em tempoInicialEnviaRespReprog
            // Ou que haja um break interno que force o loop a parar
            while (millis() < tempoInicialEnviaRespReprog + 60000) {

              //Chama a função para atualizar o display
              showDisplay();

              //Imprime mensagem para debug na serial
              if (DEBUGS_PRINTS); Serial.println("\n enviaDados – Enviando uma mensagem que a reprogramação foi feita com sucesso");

              //Chama função para envio da mensagem de resposta via Lora
              enviaMsgLora(msgReprogramacaoOK);

              //Imprime mensagem para debug na serial
              if (DEBUGS_PRINTS); Serial.println("enviaDados - Mensagem enviada aguardando resposta");

              //Recebe uma mensagem via lora e salva na String msgRecebida
              String msgRecebida = recebeMsgLora();

              //Verifica se a mensagem que foi recebida é uma mensagem de confirmação de recebimento
              if (processaMsgRecebida(msgRecebida, segundaMensagemEsperada)) {

                //Imprime mensagem para debug na serial
                if (DEBUGS_PRINTS); Serial.println("enviaDados - Mensagem de confirmação recebida, enviando uma mensagem de confirmação de entrega recebida, desta vez não esperarei por resposta");

                //cria um delay de 1 segundo (OBS: é completamente desaconselhável utilizar o delay, pois ele cria uma pausa em toda a CPU,
                //o vTaskDelay, pausa a execução apenas do código, porem a CPU continua ativa), caso esteja com o wifi manager ativo pode ocorrer inconsistências e erros fatais caso use o delay)
                const TickType_t xDelay = 1000;
                vTaskDelay (xDelay);

                //Chama a função para atualizar o display
                showDisplay();

                //Gera um JSON com uma mensagem confirmação de entrega recebida para ser enviada ao gateway
                String confirmacaoDeEntrega = "{\"gateway\":\"" + nomeGateway + "\",\"endpoint\":\"" + nomeEndpoint + "\",\"mensagem\":\"CONFIRMACAO-DE-ENTREGA-RECEBIDA\"}";

                //Chama função que envia a mensagem via Lora
                enviaMsgLora(confirmacaoDeEntrega);

                //Imprime mensagem para debug na serial
                if (DEBUGS_PRINTS); Serial.println("enviaDados - Mensagem enviada com sucesso");

                //Uma vez que o envio do arquivo foi bem sucedido, vamos deleta-lo da memória interna do microcontrolador
                flash.deletarArquivo(arq);

                //Salva que o valor do último arquivo enviado na memória interna do microcontrolador
                setUltimoArquivoEnviado( proxEnviar);

                //Chama a função para atualizar o display
                showDisplay();

                //Uma vez que o envio ocorreu normalmente e a reprogramação foi realizada com sucesso, retorno True indicando sucesso na operação
                return true;
              }
            }
            //Chama a função para atualizar o display
            showDisplay();

            //Este break é acionado caso o endpoint não consiga receber uma resposta do gateway após receber uma reprogramação, neste caso abortamos toda a operação
            // este break irá quebrar o ciclo principal de envio de dados
            break;

          }


          //Verifica se a mensagem que foi recebida é uma mensagem de confirmação de recebimento
          if (processaMsgRecebida(mensagemRecebida, segundaMensagemEsperada)) {

            //Imprime mensagem para debug na serial
            if (DEBUGS_PRINTS); Serial.println("enviaDados - Mensagem de confirmação recebida, enviando uma mensagem de confirmação de entrega recebida, desta vez não esperarei por resposta");

            //cria um delay de 1 segundo (OBS: é completamente desaconselhável utilizar o delay, pois ele cria uma pausa em toda a CPU,
            //o vTaskDelay, pausa a execução apenas do código, porem a CPU continua ativa), caso esteja com o wifi manager ativo pode ocorrer inconsistências e erros fatais caso use o delay)
            const TickType_t xDelay = 1000;
            vTaskDelay (xDelay);

            //Chama a função para atualizar o display
            showDisplay();

            //Gera um JSON com uma mensagem confirmação de entrega recebida para ser enviada ao gateway
            String confirmacaoDeEntrega = "{\"gateway\":\"" + nomeGateway + "\",\"endpoint\":\"" + nomeEndpoint + "\",\"mensagem\":\"CONFIRMACAO-DE-ENTREGA-RECEBIDA\"}";

            //Chama função que envia a mensagem via Lora
            enviaMsgLora(confirmacaoDeEntrega);

            //Imprime mensagem para debug na serial
            if (DEBUGS_PRINTS); Serial.println("enviaDados - Mensagem enviada com sucesso");

            //Uma vez que o envio do arquivo foi bem sucedido, vamos deleta-lo da memória interna do microcontrolador
            flash.deletarArquivo(arq);

            //Salva que o valor do último arquivo enviado na memória interna do microcontrolador
            setUltimoArquivoEnviado( proxEnviar);

            //Chama a função para atualizar o display
            showDisplay();

            //Uma vez que o envio ocorreu normalmente retorno true indicando sucesso na operação
            return true;
          }

          //Fim do while Envio de Dados
        }

        //Fim do if que verifica se eu recebi uma mensagem do gateway pedindo para enviar os dados
      }

      //Fim do While em que o endpoint espera o gateway pedir para que envie os dados
    }

    //Fim do if em que verifico se o arquivo existe ou não para poder ser enviado
  } else {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("enviaDados - o arquivo não existe, vou abortar");
  }

  //Chama a função para atualizar o display
  showDisplay();

  //Se chegou até aqui algo ocorreu de errado no envio dos dados, retorno false para indicar que não obtive sucesso em enviar os dados
  return false;
}


void salvaDados(String sensor, String valor, String unMedida, String dataHora) {
  /*
     Essa função recebe como parâmetro os dados obtidos através da leitura dos sensores e salva em um arquivo no flash para um envio posterior
     O json não é formado com todos os dados neste momento, os  únicos dados que são salvos são:
     String sensor (o sensor),
     String valor (o valor obtido pelo sensor),
     String unMedida (a unidade de medida obtida pelo sensor),
     String dataHora  (a data e hora da leitura)
  */
  if (DEBUGS_PRINTS); Serial.println("Call-back: salvaDados (" + sensor + "," + valor + "," + unMedida + "," + dataHora + ")");

  //Gero uma String contendo o Json com os dados de leitura que eu recebi
  String dados = "{\"sensor\":\"" + sensor + "\",\"valor\":\"" + valor + "\",\"dataHora\":\"" + dataHora + "\",\"unidadeMedida\":\"" + unMedida + "\"}";

  //Converto o Object String em um char array e salvo na variável dat
  char dat[dados.length() + 3];
  dados.toCharArray(dat, dados.length() + 3);

  //Obtenho o id do último arquivo salvo
  int ultimoSalvo = getUltimoArquivoSalvo().toInt();

  //Por razoes de espaço só são salvos 1001 registros na memória interna do microcontrolador
  //Por esta razão quando o último arquivo salvo + 1 for 1001 eu altero o valor do último arquivo salvo para -1
  // Desta forma o próximo arquivo é o arquivo com o id nº 0
  if (ultimoSalvo + 1 == 1001) {
    ultimoSalvo = -1;
  }

  //Incremento o valor de ultimoSalvo para obter o id do próximo arquivo a ser salvo
  String proxSalvo = (String( (ultimoSalvo + 1)));

  //Através do id obtido anteriormente eu obtenho o nome do próximo arquivo a ser salvo
  String nome = "/arq-Pendente-" + proxSalvo + ".txt";

  //Realiza o cast da variável nome de Object Sting para um char array e salva na variável arq
  char arq[nome.length() + 3];
  nome.toCharArray(arq, nome.length() + 3);

  //Imprime mensagem para debug na serial
  if (DEBUGS_PRINTS); Serial.println("salvaDados - Nome do próximo arquivo a ser salvo: " + nome);

  //Verifica se o arquivo já existe e salva o resultado inverso na variável prontoParaSalvar
  //Se o arquivo já existe, não está pronto para salvar, pois significa que já existe um dado com o mesmo id para ser enviado na fila de 1001 posições e não podemos sobrescreve-lo
  //Se o arquivo não existe, está pronto para salvar, a posição na fila de envio está livre
  bool prontoParaSalvar = !flash.arquivoExiste(arq);

  //Verifica se está pronto para salvar
  if (prontoParaSalvar) {

    //Salva no arquivo de nome armazenado no char array arq os dados presentes no char array dat
    flash.salvarArquivo(arq, dat);

    //já que foi salvo sem problemas, vamos incrementar o valor de ultimoSalvo e transforma-lo em uma String ultSalvo
    String ultSalvo = String(ultimoSalvo + 1);

    //Realiza o cast de ultSalvo de Object String para char array e salva na variável ult
    char ult[ultSalvo.length() + 3];
    ultSalvo.toCharArray(ult, ultSalvo.length() + 3);

    //salva o valor de ult no arquivo /ultimoArquivoSalvo.txt
    setUltimoArquivoSalvo( proxSalvo);

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("salvaDados - arquivo salvo com sucesso ");

  } else {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("salvaDados - o arquivo não pode ser salvo, pois a fila de envio está cheia " + nome);
  }


}

bool processaMsgRecebida(String mensagem, String tipoDeRespostaEsperado) {
  /*
     Verifica se a mensagem recebida é:
      Um json valido.
      Destinada a este endpoint.
      É do gateway a qual esse endpoint está configurado para trabalhar.
     É a mensagem que eu espero no momento.
  */
  if (DEBUGS_PRINTS); Serial.println("Call-back: processaMsgRecebida(" + mensagem + " , " + tipoDeRespostaEsperado + ")");

  //Instancia um objeto do DynamicJsonDocument
  const size_t capacity = JSON_OBJECT_SIZE(9) + 200;
  DynamicJsonDocument doc(capacity);

  //Realiza o  cast de Object String para char array da variável mensagem e salva na variável json
  int tamanhoMensagem = mensagem.length() + 3;
  char json[tamanhoMensagem];
  mensagem.toCharArray(json, tamanhoMensagem);

  //Realiza a desserializaçao de String para Json
  deserializeJson(doc, json);

  //Obtém o valor da chave "gateway" presente no json da mensagem
  String gateway = doc["gateway"];

  //Obtém o valor da chave "mensagem" presente no json da mensagem
  String mensagemTipo = doc["mensagem"];

  //obtém o id/código do gateway em que o endpoint está configurado para enviar os dados
  String nomeGateway = getGateway();

  //obtém o id/código do endpoint em que o endpoint está configurado para enviar os dados
  String nomeEndpoint = getEndpoint();

  //Variavel String para armazenar o nome do endpoint a qual a mensagem foi destinada
  String endpoint = "";

  //Verifica se o tipo da mensagem recebida é uma reprogramação
  if (mensagemTipo.equals("REPROGRAMACAO")) {

    //Se for uma mensagem de reprogramação não existira a chave "endpoint" no lugar terá a chave "idAlvo"
    //Obtém o valor da chave "idAlvo"
    endpoint = doc["idAlvo"].as<String>();

  } else {

    //Obtém o valor da chave "endpoint" caso não seja uma mensagem de reprogramação
    endpoint = doc["endpoint"].as<String>();

  }

  // verifica se:
  //1º o código/id do gateway é igual ao código/id do gateway recebido no JSON
  //2º o código/id do endpoint é igual ao código/id do endpoint recebido no JSON
  //3º a String na chave "mensagem" do JSON é igual a String na variável tipoDeRespostaEsperado recebida como parâmetro
  if ((gateway.equals(nomeGateway)) && (endpoint.equals(nomeEndpoint)) && (mensagemTipo.equals(tipoDeRespostaEsperado))) {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("processaMsgRecebida - Mensagem correta, para esse endpoint e do gateway deste endpoint");

    //Se a mensagem for a mensagem o qual eu espero, eu retorno true, indicando que é a mensagem que eu espero
    return true;


  } else {

    //Imprime mensagem para debug na serial
    if (DEBUGS_PRINTS); Serial.println("processaMsgRecebida - Está mensagem não foi destinada a este gateway, ou o endpoint não está cadastrado, ou a mensagem não é a desejada");

    //Se a mensagem não for a mensagem a qual eu espero, eu retorno false, indicando que não é a mensagem que eu espero
    return false;
  }
}

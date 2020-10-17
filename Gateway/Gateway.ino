#define FS_NO_GLOBALS
#include <SPI.h>
#include <LoRa.h>
#include <MicroSD.h>
#include <Flash.h> 
#include <PubSubClient.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "Arduino.h"


#define SCK_LORA 5
#define MISO_LORA 19
#define MOSI_LORA 27
#define RESET_PIN_LORA 14
#define SS_PIN_LORA 18
#define MAX_ENDPOINTS 10

WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);
Flash flash;
MicroSD sdCard;

bool loraON = false;
bool sdON = false;

//--------- Variaveis temporarias
bool newData =false; 
String nomeGateway;
String ssidAp;
String senhaAp;
String wifiHabilitado;
String ssidWifi;
String senhaWifi;
String gsmHabilitado;
String operadoraGSM;
String servidorMQTT;
String portaMQTT;
String loginMQTT;
String senhaMQTT;
bool conexaoDisponivel = false;
String mensagemCallbackMQTT;
String IdEndpoints[MAX_ENDPOINTS];
int quantidadeEndpoints;
String topicoMqttEnvioDados;
String topicoMqttEventos;
bool hardReset = false;

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
            
              
              Nome do Gateway: <input type="text" name="nomeGateway"></br></br>
        
        <hr/>
              <h3 align='center' >Rede Access Point</h3>
              &nbsp;&nbsp;&nbsp;&nbsp;SSID: <input type="text" name="ssidAp"></br></br>
              &nbsp;&nbsp;&nbsp;&nbsp;Senha: <input type="text" name="senhaAp"></br></br>
        <hr/>
              <h3 align='center' >Rede Wifi</h3>
        <div align='center'><input type="radio"  name="wifiHabilitado" value="true"> Habilitado <input type="radio"  name="wifiHabilitado" value="false"> Desabilitado </div></br>
              &nbsp;&nbsp;&nbsp;&nbsp;SSID: <input type="text" name="ssidWifi"></br></br>
              &nbsp;&nbsp;&nbsp;&nbsp;Senha: <input type="text" name="senhaWifi"></br></br>
              <hr/>
              <h3 align='center' >Rede GSM</h3>
        <div align='center'><input type="radio"  name="gsmHabilitado" value="true"> Habilitado <input type="radio"  name="gsmHabilitado" value="false"> Desabilitado </div></br>
              &nbsp;&nbsp;&nbsp;&nbsp;Operadora: &nbsp;&nbsp;&nbsp;
        <select  name="operadora">
        <option>Oi</option>
        <option>Vivo</option>
        <option>Tim</option>
        <option>Claro</option>
        </select> <br><br>
        <hr/>
              <h3 align='center' >Servidor MQTT</h3>
        
              &nbsp;&nbsp;&nbsp;&nbsp;Servidor: <input type="text" name="servidorMQTT"> </br></br>
              &nbsp;&nbsp;&nbsp;&nbsp;Porta: <input type="text" name="portaMQTT" size="5"></br></br>
              &nbsp;&nbsp;&nbsp;&nbsp;Login: <input type="text" name="loginMQTT"></br></br>
        &nbsp;&nbsp;&nbsp;&nbsp;Senha: <input type="text" name="senhaMQTT"></br></br>
        <tr>
          <td align='center'>
            <input type="submit" value="Salvar">
      </form>
      <hr/>
      <form action="/cadastroEndpoints">
        <div align="center">
        <input type="submit" value="Cadastro de endpoints"></br></br>
        </div>
       </form>
          </td>
        </tr>
        <br>

        
        
      </table>
 </body></html>)rawliteral";

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
            <div align='center'>%s<br>
            Por Favor aguarde...</div>
          </td>
        </tr>
          </table>
 </body></html>)rawliteral";

const char cadastroEndpoints_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
  <head>
    <title>Cadastro de Endpoints</title>
    <meta charset="utf-8">
  </head>
  <body>
    <table align='center' width='400' border='0'>
        
        <tr>
          <td align='center'>
          <hr/>
     <form action="/inserirRemoverEndpoint">
            <h3>Dados do Dispositivo:</h3>
      ID endpoint: <input type="text" name="idEdp" required ></br></br>
          
      <input type="submit" name="add" value="Adicionar">
      <input type="submit" name="del" value="Remover"></br></br>
      </form>
      </td>
        </tr>
        <tr>
          <td align='center'>
      <hr/>
      <h3>IDs cadastrados</h3>)rawliteral";

const char cadastroEndpoints2_html[] PROGMEM = R"rawliteral(</td>
        </tr>
      </table>
 </body></html>)rawliteral";
     


const char edp_salvo_sucesso_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Sucesso</title>
    <meta charset="utf-8">
  </head>
  <body>
    <table align='center' width='400' border='0'>
          <tr>
          <td align='center'>
            <div align='center'>Endpoint cadastrado com sucesso \n reinicie o gateway para proceder com a captura<br>
            <a href="/cadastroEndpoints">Retornar</a>.</div>
          </td>
        </tr>
          </table>
 </body></html>)rawliteral";

 
const char edp_salvo_falha_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Falha</title>
    <meta charset="utf-8">
  </head>
  <body>
    <table align='center' width='400' border='0'>
          <tr>
          <td align='center'>
            <div align='center'>Este ID de endpoint já se encontra cadastrado<br>
            <a href="/cadastroEndpoints">Retornar</a>.</div>
          </td>
        </tr>
          </table>
 </body></html>)rawliteral";

 
const char edp_deletado_sucesso_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Sucesso</title>
    <meta charset="utf-8">
  </head>
  <body>
    <table align='center' width='400' border='0'>
          <tr>
          <td align='center'>
            <div align='center'>O endpoint foi removido com sucesso \n reinicie o gateway para interromper a captura de dados <br>
            <a href="/cadastroEndpoints">Retornar</a>.</div>
          </td>
        </tr>
          </table>
 </body></html>)rawliteral";


void configWifiManager(){
  Serial.println("Calback configWifiManager() ");
  String ssidString = flash.lerArquivo("/ssidAp.txt");
  String senhaString = flash.lerArquivo("/senhaAp.txt");
  
  char ssid2[ssidString.length()+3]={};
  char password2[senhaString.length()+3]={};
  
  ssidString.toCharArray(ssid2,ssidString.length()+3);
  senhaString.toCharArray(password2,senhaString.length()+3);
  
  Serial.print("SSID ");
  Serial.println(ssid2);
  Serial.print("Senha ");
  Serial.println(password2);
  WiFi.softAP(ssid2,password2);
  IPAddress IP = WiFi.softAPIP();

  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
     if(request->hasParam("nomeGateway")){
      nomeGateway = request->getParam("nomeGateway")->value();
    }
     
    if(request->hasParam("ssidAp")){
      ssidAp = request->getParam("ssidAp")->value();
    }
    
    if(request->hasParam("senhaAp")){
      senhaAp = request->getParam("senhaAp")->value(); 
    }
    
    if(request->hasParam("wifiHabilitado")){
      wifiHabilitado = request->getParam("wifiHabilitado")->value();
    }
    
    if(request->hasParam("ssidWifi")){
      ssidWifi = request->getParam("ssidWifi")->value();
      
    }

    if(request->hasParam("senhaWifi")){
      senhaWifi = request->getParam("senhaWifi")->value();
      
    }
    
    if(request->hasParam("gsmHabilitado")){
      gsmHabilitado = request->getParam("gsmHabilitado")->value();
    }

    if(request->hasParam("operadora")){
      operadoraGSM = request->getParam("operadora")->value();
    }

    
    if(request->hasParam("servidorMQTT")){
      servidorMQTT = String(request->getParam("servidorMQTT")->value()); 
      
    }
    
    if(request->hasParam("loginMQTT")){
      loginMQTT = String(request->getParam("loginMQTT")->value()); 
    }
    
    if(request->hasParam("senhaMQTT")){
      senhaMQTT = String(request->getParam("senhaMQTT")->value()); 
    }
     
     newData = true;
         
    request->send(200, "text/html", "Dados Salvos com Sucesso<br><a href=\"/\">Retornar para Home Page</a>");
  });

  server.on("/cadastroEndpoints", HTTP_GET, [](AsyncWebServerRequest *request){
    String todosIds= flash.lerArquivo("/endpointsParaLeitura.txt");
    todosIds.replace(";","<br/>");
    char buff[todosIds.length()+3];
    todosIds.toCharArray(buff,todosIds.length()+3);
    
   // char Idsendpoits[] = "00001</br>00002";
    int tamanho = sizeof(cadastroEndpoints_html)+sizeof(cadastroEndpoints2_html)+sizeof(buff);
    char pagina[tamanho];
    sprintf(pagina,"%s %s %s",cadastroEndpoints_html,buff,cadastroEndpoints2_html);
    
    request->send_P(200, "text/html", pagina);
  });

  

  server.on("/inserirRemoverEndpoint", HTTP_GET, [](AsyncWebServerRequest *request){
    
    if(request->hasParam("add")){
      
      String addEdp = String(request->getParam("idEdp")->value());
      bool retorno = addEndpoint(addEdp);
      if(retorno){
         request->send(200, "text/html", edp_salvo_sucesso_html);
         
        }else{
         request->send(200, "text/html", edp_salvo_falha_html);
        } 
    }
    
    if(request->hasParam("del")){
      String removeIdEdp = String(request->getParam("idEdp")->value());
      bool retorno = removeEndpoint(removeIdEdp); 
        if(retorno){
         request->send(200, "text/html", edp_deletado_sucesso_html);
        }
      
    }
    
    
  });

  
  server.onNotFound(notFound);
  server.begin();


  }

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void criaEstruturaInicial(){
  String estruturaCriada = flash.lerArquivo("/estrutura.txt");
  if(!estruturaCriada.equals("EstruturaOK")){
      
    flash.salvarArquivo("/ssidAp.txt","Gateway");
    flash.salvarArquivo("/senhaAp.txt","123456789");
    flash.salvarArquivo("/endpointsParaLeitura.txt","0");
    flash.salvarArquivo("/nomeGateway.txt","00001");
    flash.salvarArquivo("/wifiHabilitado.txt","true");
    flash.salvarArquivo("/gsmHabilitado.txt","false");
    flash.salvarArquivo("/operadoraGSM.txt","vivo");
    
    flash.salvarArquivo("/ssidWifi.txt","helton");
    flash.salvarArquivo("/senhaWifi.txt","15041998");
    
    flash.salvarArquivo("/servidorMQTT.txt","167.114.153.23");
    flash.salvarArquivo("/portaMQTT.txt","1883");
    flash.salvarArquivo("/loginMQTT.txt","");
    flash.salvarArquivo("/senhaMQTT.txt","");
    
    flash.salvarArquivo("/estrutura.txt","EstruturaOK");
    
  }
}
bool addEndpoint(String idEndpoint){
  String allData = flash.lerArquivo("/endpointsParaLeitura.txt");
    int i=0;
    int k=0;
    String newData;
    String mensagem = "";
    for(i=0;i<=(allData.length()+3);i++){
      char aux = allData[i];

      if(aux != ';'){
        mensagem += String(aux);
        }else{
          if(mensagem.equals(idEndpoint)){
            return false;
            }
            mensagem="";
            k++;
          }     
      }
      String quantidade = String(k);
      allData+=idEndpoint+";";
      flash.salvarArquivo("/endpointsParaLeitura.txt",allData);
      flash.salvarArquivo("/quantidadeParaLeitura.txt",quantidade);
      return true;
}

bool removeEndpoint(String idEndpoint){

    String allData = flash.lerArquivo("/endpointsParaLeitura.txt");
    int i=0;
    int k=0;
    String newData;
    String mensagem = "";
    for(i=0;i<=(allData.length()+3);i++){
      char aux = allData[i];

      if(aux != ';'){
        mensagem += String(aux);
        }else{
          if(!mensagem.equals(idEndpoint)){
            newData+=mensagem+";";
            }
            mensagem="";
            k++;
          }     
      }
      String quantidade = String(k);
      flash.salvarArquivo("/endpointsParaLeitura.txt",newData);
      flash.salvarArquivo("/quantidadeParaLeitura.txt",quantidade);
      return true;
  }


void getEndpoints(){
  Serial.println("Lendo arquivo com ids");
  String allData = flash.lerArquivo("/endpointsParaLeitura.txt");
  Serial.println("Arquivo lido");
    int k =0;
    int i=0;
    String newData;
    String mensagem = "";
    Serial.println("Entrando no FOR");
    for(i=0;i<=(allData.length()+3);i++){
      char aux = allData[i];
      Serial.print("Ciclo for: ");
      Serial.print(i);
      Serial.print("    Char:  ");
      Serial.println(aux);

      if(aux == '\0'){
        Serial.println("É um fim de string ");
        return;
        }
        
      if(aux != ';'){
        Serial.println("Nao encontrei ; ainda ");
        mensagem += String(aux);
        }else{
          Serial.println("Encontrei ; ");
          Serial.print("ID: ");
          Serial.println(mensagem);
         IdEndpoints[k]=mensagem;
         Serial.print("Salvo no vetor");
         mensagem="";
         k++;
         quantidadeEndpoints = k;
         Serial.print("QTD de endpoints: ");
         Serial.println(quantidadeEndpoints);
         }
      }  
  }


String getNomeGateway() {
    Serial.println("Calback getNomeGateway() ");
    String nome = flash.lerArquivo("/nomeGateway.txt");
    return nome;
  }
void setNomeGateway(String nomeGateway) {
     Serial.println("Calback setNomeGateway() ");
    flash.salvarArquivo("/nomeGateway.txt", nomeGateway);
  }

String getWifiHabilitado() { 
   Serial.println("Calback getWifiHabilitado() ");
    return flash.lerArquivo("/wifiHabilitado.txt");
  }
void setWifiHabilitado(String wifiHabilitado) {
  Serial.println("Calback setWifiHabilitado() ");
    flash.salvarArquivo("/wifiHabilitado.txt", wifiHabilitado);
}

String getGsmHabilitado() { 
   Serial.println("Calback getGsmHabilitado() ");
    return flash.lerArquivo("/gsmHabilitado.txt");
  }
void setGsmHabilitado(String gsmHabilitado) {
  Serial.println("Calback setGsmHabilitado() ");
    flash.salvarArquivo("/gsmHabilitado.txt", gsmHabilitado);
  }
String getOperadoraGSM() { 
   Serial.println("Calback getOperadoraGSM() ");
    return flash.lerArquivo("/operadoraGSM.txt");
  }
void setOperadoraGSM(String operadoraGSM) { 
   Serial.println("Calback setOperadoraGSM() ");
    flash.salvarArquivo("/operadoraGSM.txt", operadoraGSM);
  }
const char* getSsidAp() { 
   Serial.println("Calback getSsidAp() ");
     String dado =  flash.lerArquivo("/ssidAp.txt");
    char charArray[dado.length()+5];
     dado.toCharArray(charArray,dado.length()+3);
     Serial.print("String ");
     Serial.println(dado);
     Serial.print("Char Array ");
     Serial.println(charArray);
     return charArray;
  }
void setSsidAp(String ssidAp) { 
   Serial.println("Calback setSsidAp() ");
    flash.salvarArquivo("/ssidAp.txt", ssidAp);
  }
const char* getSenhaAp() { 
   Serial.println("Calback getSenhaAp() ");
     String dado =  flash.lerArquivo("/senhaAp.txt");
    char charArray[dado.length()+5];
     dado.toCharArray(charArray,dado.length()+3);
     Serial.print("String ");
     Serial.println(dado);
     Serial.print("Char Array ");
     Serial.println(charArray);
     return charArray;
     return charArray;
  }
void setSenhaAp(String senhaAp) { 
   Serial.println("Calback setSenhaAp() ");
    flash.salvarArquivo("/senhaAp.txt", senhaAp);
  }
   
const char* getSsidWifi() { 
   Serial.println("Calback getSsidWifi() ");
     String dado =  flash.lerArquivo("/ssidWifi.txt");
    char charArray[dado.length()+5];
     dado.toCharArray(charArray,dado.length()+3);
     return charArray;
  }
void setSsidWifi(String ssidWifi) {
  Serial.println("Calback setSsidWifi() ");
    flash.salvarArquivo("/ssidWifi.txt", ssidWifi);
  }
const char* getSenhaWifi() { 
   Serial.println("Calback getSenhaWifi() ");
     String dado =  flash.lerArquivo("/senhaWifi.txt");
    char charArray[dado.length()+5];
     dado.toCharArray(charArray,dado.length()+3);
     return charArray;
  }
void setSenhaWifi(String senhaWifi) { 
   Serial.println("Calback setSenhaWifi() ");
    flash.salvarArquivo("/senhaWifi.txt", senhaWifi);
  }
   
const char* getServidorMQTT() { 
   Serial.println("Calback getServidorMQTT() ");
     String dado =  flash.lerArquivo("/servidorMQTT.txt");
      char charArray[dado.length()+5];
     dado.toCharArray(charArray,dado.length()+3);
     return charArray;
  }
void setServidorMQTT(String servidorMQTT) { 
   Serial.println("Calback setServidorMQTT() ");
    flash.salvarArquivo("/servidorMQTT.txt", servidorMQTT);
  }
int getPortaMQTT() {
  Serial.println("Calback getPortaMQTT() ");
     int dado =  flash.lerArquivo("/portaMQTT.txt").toInt();
     return dado;
  }
void setPortaMQTT(String portaMQTT) { 
   Serial.println("Calback setPortaMQTT() ");
    flash.salvarArquivo("/portaMQTT.txt", portaMQTT);
  }
const char* getLoginMQTT() { 
   Serial.println("Calback getLoginMQTT() ");
     String dado =  flash.lerArquivo("/loginMQTT.txt");
    char charArray[dado.length()+5];
     dado.toCharArray(charArray,dado.length()+3);
     return charArray;
  }
void setLoginMQTT(String loginMQTT) { 
   Serial.println("Calback setLoginMQTT() ");
    flash.salvarArquivo("/loginMQTT.txt", loginMQTT);
  }
const char* getSenhaMQTT() { 
   Serial.println("Calback getSenhaMQTT() ");
     String dado =  flash.lerArquivo("/senhaMQTT.txt");
    char charArray[dado.length()+5];
     dado.toCharArray(charArray,dado.length()+3);
     return charArray;
  }
void setSenhaMQTT(String senhaMQTT) { 
   Serial.println("Calback setSenhaMQTT() ");
    flash.salvarArquivo("/senhaMQTT.txt", senhaMQTT);
  }

String getModoEnvio(){ 
  Serial.println("Calback: getModoEnvio()");
  if(getWifiHabilitado()=="true"){
    return "WIFI";
  }
  if(getGsmHabilitado()=="true"){
    return "GSM";
  }
  return "NONE";
  }
  

bool iniciaLora(){ 
  Serial.println("Calback: iniciaLora()");
  SPI.end();
  sdCard.finalizaSD();
  
   pinMode(21,OUTPUT);
   digitalWrite(21,LOW);
   digitalWrite(21,LOW);

   Serial.println("Iniciando protocolo LoraWan");
   SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
     if(!LoRa.begin(915E6)){
       Serial.println("Falha ao iniciar o protocolo LoRa");
       bool loraON = false;
       bool sdON = false;
       return false;
     }else{
       Serial.println("Protocolo LoRa inicializado corretamente");
       bool loraON = true;
        bool sdON = false;
       return true;
     }
       
  }




bool iniciaSD(){ 
  Serial.println("Calback: iniciaSD()");
   SPI.end();

   bool statusSd = sdCard.iniciaSD();
   return statusSd;
  }

bool conectarWifi(){ 

  Serial.println("Calback: conectarWifi()");
  int tentativasDeConexao = 0;
  String ssidString = flash.lerArquivo("/ssidWifi.txt");
  String senhaString = flash.lerArquivo("/senhaWifi.txt");
  
  char ssid2[ssidString.length()+3]={};
  char password2[senhaString.length()+3]={};
  
  ssidString.toCharArray(ssid2,ssidString.length()+3);
  senhaString.toCharArray(password2,senhaString.length()+3);
  
  Serial.print("SSID ");
  Serial.println(ssid2);
  Serial.print("Senha ");
  Serial.println(password2);  
    
    WiFi.begin(ssid2, password2);
    while ((WiFi.status() != WL_CONNECTED)) {
        tentativasDeConexao++;
        if(tentativasDeConexao>5){
            break;
          }
        delay(500);
    }
  if(WiFi.status() == WL_CONNECTED){
      WiFi.setAutoReconnect(false);
      conexaoDisponivel = true;
      Serial.println("Conectado com sucesso ao WIFI");
      return true;
    }else{
      Serial.println("Nao foi possivel conectar ao WIFI");
      conexaoDisponivel = false;
      return false;
      }
      Serial.println("Nao foi possivel conectar ao WIFI");
      conexaoDisponivel = false;
      return false;
  }
  

bool enviarDado(String mensagem){ 
  Serial.println("Calback: enviarDado()");

  if(getModoEnvio()=="WIFI"){
    if(WiFi.status() == WL_CONNECTED){
        Serial.println("Conexao com WIFI é estavel Enviando ao MQTT");
        return enviarMQTT(mensagem,"test");  
      }else{
        Serial.println("Conexao com WIFI NAO é estavel, tentando reconectar");
        if(conectarWifi()){
          Serial.println("Enviando ao MQTT");
          return enviarMQTT(mensagem,"test");
        } else{
          return false;
          }
        }
    
  }else{
      if(getModoEnvio()=="GSM"){
         //auto impendancia sd card pins    
         Serial.println("GO-ARDUINO-GSM");
        // espera o arduino responder
        return true;
        }
    }
  
  }
bool enviaPendentes(){
  if(iniciaSD()){
    Serial.println("Enviando dados salvos no SD");
    String allData = sdCard.lerArquivo("data.txt");
    int i=0;

    String mensagem = "";
    for(i=0;i<=(allData.length()+3);i++){
      char aux = allData[i];

      if(aux != '\n'){
        mensagem += String(aux);
        }else{
          Serial.print("Dado salvo: ");
          Serial.println(mensagem);
          enviarDado(mensagem);
          mensagem="";
          }
      
      
      }
    
     flash.salvarArquivo("/enviosPendentes.txt","false");
     sdCard.deletarArquivo("data.txt");
  
  }
  return true;
  }
  
bool enviarMQTT(String mensagem, String topico){  
  Serial.println("Calback: enviarMQTT()");
  /*
  String servidor = flash.lerArquivo("/servidorMQTT.txt");
  String porta = flash.lerArquivo("/portaMQTT.txt");
  String login = flash.lerArquivo("/loginMQTT.txt");
  String senha = flash.lerArquivo("/senhaMQTT.txt");
  
  
  char server[servidor.length()+3]={};
  char logon[login.length()+3]={};
  char pass[senha.length()+3]={};
  
  servidor.toCharArray(server,servidor.length()+3);
  int port = porta.toInt();
  login.toCharArray(logon,login.length()+3);
  senha.toCharArray(pass,senha.length()+3);

  Serial.print("Servidor MQTT");
  Serial.println(server);
  Serial.print("Porta MQTT");
  Serial.println(port);
  Serial.print("Usuario MQTT");
  Serial.println(logon);
  Serial.print("Senha MQTT");
  Serial.println(pass);

  
  int tentativasDeConexao =0;
  char msg[mensagem.length()+3];
  mensagem.toCharArray(msg,mensagem.length()+3);
    
  long inicio = millis();
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Conexao com wifi estavel, conectando ao Broker MQTT");
      client.setServer(server, port);
      Serial.println("Server e porta configurados, tentando cliente.conected");
               
               if(!client.connected()){
                while (!client.connected()) {
                    Serial.println("Conectado ao Broker, autenticando usuario ");
                    String clientId = "Gateway";
                    clientId += String(random(0xffff), HEX);
                    if (client.connect(clientId.c_str(),logon,pass)) {
                       Serial.println("Usuario autenticado");
                    } else {
                       delay(500);
                      
                      }
                }
                             
               
                
       }
       
       char topic[topico.length()+3];
       topico.toCharArray(topic,topico.length()+3);
       Serial.println("MQTT ja configurado, enviando dados");
       client.publish(topic, msg,true);
       //client.disconnect();
       return true;         
  
            
  }
  Serial.println("Nao foi possivel efetuar a publicaçao no servidor MQTT");         
  return false;

  */
  return true;
  }

bool receberMQTT(){  
  Serial.println("Calback: receberMQTT()");
  String servidor = flash.lerArquivo("/servidorMQTT.txt");
  String porta = flash.lerArquivo("/portaMQTT.txt");
  String login = flash.lerArquivo("/loginMQTT.txt");
  String senha = flash.lerArquivo("/senhaMQTT.txt");
  
  
  char server[servidor.length()+3]={};
  char logon[login.length()+3]={};
  char pass[senha.length()+3]={};
  
  servidor.toCharArray(server,servidor.length()+3);
  int port = porta.toInt();
  login.toCharArray(logon,login.length()+3);
  senha.toCharArray(pass,senha.length()+3);

  Serial.print("Servidor MQTT");
  Serial.println(server);
  Serial.print("Porta MQTT");
  Serial.println(port);
  Serial.print("Usuario MQTT");
  Serial.println(logon);
  Serial.print("Senha MQTT");
  Serial.println(pass);
  
  int tentativasDeConexao =0;
  
    
  long inicio = millis();
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Conexao com wifi estavel, conectando ao Broker MQTT");
      client.setServer(server, port);
      client.setCallback(callback);
      Serial.println("Server e porta configurados, tentando cliente.conected");
               
               if(!client.connected()){
                while (!client.connected()) {
                    Serial.println("Conectado ao Broker, autenticando usuario ");
                    String clientId = "Gateway";
                    clientId += String(random(0xffff), HEX);
                    if (client.connect(clientId.c_str(),logon,pass)) {
                       Serial.println("Usuario autenticado");
                    } else {
                       delay(500);
                      
                      }
                }
                             
               
                
       }
       Serial.println("MQTT ja configurado, recebendo dados");
       String topico = "gatewayId-"+flash.lerArquivo("/nomeGateway.txt");
       char top[topico.length()+3];
       topico.toCharArray(top,topico.length()+3);
       
       client.subscribe(top);

       long init = millis();
       while(init+5000>millis()){
       client.loop();
       
       }
        if(!mensagemCallbackMQTT.equals("MENSAGEM DE REPROGRAMAÇAO RECEBIDA COM SUCESSO")){
          processaMsgMqtt(mensagemCallbackMQTT);
          enviarMQTT("MENSAGEM DE REPROGRAMAÇAO RECEBIDA COM SUCESSO",topico);
        }else{
          Serial.println("Nenhuma MSG Nova recebida");
          }
    
       return true;                     
  }
  Serial.println("Nao foi possivel receber do servidor MQTT");         
  return false;
  }

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message Recebida no topico: ");
  Serial.println(topic);
  String mensagem ="";
  char aux;
  for (int i = 0; i < length; i++) {
    aux=(char)payload[i];
    mensagem+=String(aux);
  }
  mensagemCallbackMQTT=mensagem;
}

bool processaMsgMqtt(String mensagem){
  Serial.print("######################Processando MSG MQTT#########################");
  Serial.println(mensagem);
  const size_t capacity = 2*JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(21) + 520;
  DynamicJsonDocument doc(capacity);
  char json[mensagem.length()+3];
  mensagem.toCharArray(json,mensagem.length()+3);
  deserializeJson(doc, json);
  
  String mensagemJson = doc["mensagem"]; // "reprogramaçao"
  String tipoAlvoJson  = doc["tipoAlvo"]; // "gateway"
  String idAlvoJson  = doc["idAlvo"]; // "00001"
  String nomeGateway = flash.lerArquivo("/nomeGateway.txt");

  if((mensagemJson.equals("reprogramacao"))){

      if((tipoAlvoJson.equals("gateway"))){
         
         if((idAlvoJson.equals(nomeGateway))){
            reprogramacaoGateway(mensagem);
            return true;
          }else{
            //essa mesagem de reprogramacao nao é para esse gateway
            return false;
            }
          
        }
        
      if((tipoAlvoJson.equals("enpoint"))){
        int i = 0;
        for(i=0;i<quantidadeEndpoints;i++){
             if(idAlvoJson.equals(IdEndpoints[i])){
                String nomeArquivo = "/atualizacaoEndpoint-"+idAlvoJson+".txt";
                char nome[nomeArquivo.length()+3];
                nomeArquivo.toCharArray(nome,nomeArquivo.length()+3);

                char msg[mensagem.length()+3];
                mensagem.toCharArray(msg,mensagem.length()+3);
                
                flash.salvarArquivo(nome,msg);
                return true;
              }
            }      
        }
    }

   
  }

void reprogramacaoGateway(String mensagem){
  const size_t capacity = 2*JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(21) + 520;
  DynamicJsonDocument doc(capacity);
  char json[mensagem.length()+3];
  mensagem.toCharArray(json,mensagem.length()+3);
  deserializeJson(doc, json);

  String novoNomeGatewayJson = doc["novoIdGateway"];
  String ssidAPJson = doc["ssidAP"]; // "Gateway"
  String senhaAPJson = doc["senhaAP"]; // "123456789"
  String ssidWifiJson = doc["ssidWifi"]; // "Helton"
  String senhaWifiJson = doc["senhaWifi"]; // "15041998"
  String modoEnvioJson = doc["modoEnvio"]; // "WIFI"
  String mqttServerJson = doc["mqttServer"]; // "192.168.000.000"
  String mqttPortJson = doc["mqttPort"]; // "1880"
  String mqttUserJson = doc["mqttUser"]; // "admin"
  String mqttPassJson = doc["mqttPass"]; // "Senha@2020"
  String mqttTopicoDadosJson = doc["mqttTopicoDados"]; // "test"
  String mqttTopicoEventosJson = doc["mqttTopicoEventos"]; // "default"
  String gsmOperadoraJson = doc["gsmOperadora"]; // "vivo"
  String hardResetJson = doc["hardReset"]; // "true"

  if(doc.containsKey("addEndpoints")){
    int quantidadeAdd = doc["quantEndpoints"];
    int i = 0;
    for(i=0;i<quantidadeAdd;i++){
        String edpToAdd = doc["addEndpoints"][i];
        addEndpoint(edpToAdd);
      }
  
  }

  if(doc.containsKey("removeEndpoint")){
    int quantidadeRem = doc["quantEndpoints"];
    int i = 0;
    for(i=0;i<quantidadeRem;i++){
        String edpToRem = doc["removeEndpoint"][i];
        removeEndpoint(edpToRem);
      }
  
  }

  
  if(doc.containsKey("novoIdGateway")){
    nomeGateway = novoNomeGatewayJson;
  }
  
  
  if(doc.containsKey("ssidAP")){
      ssidAp = ssidAPJson;
  }
  
  
  if(doc.containsKey("senhaAP")){
      senhaAp = senhaAPJson;
  }
  
  
  if(doc.containsKey("ssidWifi")){
      ssidWifi = ssidWifiJson;
  }
  
  
  if(doc.containsKey("senhaWifi")){
      senhaWifi = senhaWifiJson;
  }
  
  
  if(doc.containsKey("modoEnvio")){
      if(modoEnvioJson.equals("WIFI")){
        wifiHabilitado = "true";
        gsmHabilitado = "false";
      }
      
      if(modoEnvioJson.equals("GSM")){
        gsmHabilitado = "true";
        wifiHabilitado = "false";
      }
  }
  
  
  if(doc.containsKey("mqttServer")){
      servidorMQTT = mqttServerJson;
  }
  


  if(doc.containsKey("mqttPort")){
    portaMQTT  = mqttPortJson;
  }
  
  
  if(doc.containsKey("mqttUser")){
      loginMQTT = mqttUserJson;
  }
  
  
  if(doc.containsKey("mqttPass")){
      senhaMQTT = mqttPassJson;
  }
  
  
  if(doc.containsKey("mqttTopicoDados")){
     topicoMqttEnvioDados = mqttTopicoDadosJson;
  }
  
  
  if(doc.containsKey("mqttTopicoEventos")){
     topicoMqttEventos = mqttTopicoEventosJson;
  }
  
  
  if(doc.containsKey("gsmOperadora")){
      operadoraGSM = gsmOperadoraJson;
  }
  
  
  if(doc.containsKey("hardReset")){
      if(hardResetJson.equals("true")){
        hardReset = true;
      }
  }
     
     newData = true;
  
  }




  
void processaDado(String mensagem){ 
         
      Serial.println(mensagem);
      bool envio = enviarDado(mensagem);
      
      if(envio){ //se conseguir enviar para mqtt nao salva no SD
          Serial.print("Envio: ");
          Serial.print(envio);
          Serial.println(" Envio efetuado com sucesso:");
        
          String pendente = flash.lerArquivo("/enviosPendentes.txt");
          if(pendente.equals("true")){
            Serial.println(" Existe dados nao enviados salvos no SD:");
            enviaPendentes();
            iniciaLora();
            }
       }else{
          Serial.print("Envio: ");
          Serial.print(envio);
          Serial.println("  Nao foi possivel enviar o pacote agora:");
          if(iniciaSD()){
            sdCard.salvarArquivo("data.txt",mensagem);  
            flash.salvarArquivo("/enviosPendentes.txt","true");
            }
            iniciaLora();
        }
  }


void attDados(){ 
   
  if(newData){
      Serial.println("Calback attDados() ");
      Serial.println("New data reconhecido");
      if(!nomeGateway.equals("")){
        setNomeGateway(nomeGateway);
        nomeGateway="";
      }
      if(!wifiHabilitado.equals("")){
        setWifiHabilitado(wifiHabilitado);
        wifiHabilitado="";
      }
      if(!gsmHabilitado.equals("")){
        setGsmHabilitado(gsmHabilitado);
        gsmHabilitado="";
      }
      if(!operadoraGSM.equals("")){
        setOperadoraGSM(operadoraGSM);
        operadoraGSM="";
      }
      if(!ssidAp.equals("")){
        setSsidAp(ssidAp);
        ssidAp="";
      }
      if(!senhaAp.equals("")){
        setSenhaAp(senhaAp);
        senhaAp="";
        
      }
      if(!ssidWifi.equals("")){
        setSsidWifi(ssidWifi);
        ssidWifi="";
      }
      if(!senhaWifi.equals("")){
        setSenhaWifi(senhaWifi);
        senhaWifi="";
        conectarWifi();
      }
      if(!servidorMQTT.equals("")){
        setServidorMQTT(servidorMQTT);
        servidorMQTT="";
      }
      
      if(!portaMQTT.equals("")){
        setPortaMQTT(portaMQTT);
        portaMQTT="";
      }
      if(!loginMQTT.equals("")){
        setLoginMQTT(loginMQTT);
        loginMQTT="";
      }
      if(!senhaMQTT.equals("")){
        setSenhaMQTT(senhaMQTT);
        senhaMQTT="";
      }
      newData= false;
    }
    return;
  
  }

void setup() {
   Serial.begin(115200);
   pinMode(21,OUTPUT);
   digitalWrite(21,LOW);
   digitalWrite(21,LOW);
   Serial.println("Iniciando");
   flash.inicializaFlash();
   criaEstruturaInicial();
   if(getModoEnvio()=="WIFI"){
      conectarWifi();
    }
   configWifiManager();
   getEndpoints();
   iniciaLora();
   
}
bool processaMsgLora(String mensagem, String idEndpoint, String tipoDeRespostaEsperado){
  Serial.println("processaMsgLora("+mensagem+", "+idEndpoint+", "+ tipoDeRespostaEsperado+")");
  const size_t capacity = JSON_OBJECT_SIZE(9) + 200;
  DynamicJsonDocument doc(capacity);
  int tamanhoMensagem = mensagem.length()+3;
  char json[tamanhoMensagem];
  mensagem.toCharArray(json,tamanhoMensagem);
  
  // Deserialize the JSON document
  deserializeJson(doc, json);

        const char* gat = doc["gateway"];
        String gateway = String(gat);
        String nomeGateway = flash.lerArquivo("/nomeGateway.txt");

        const char* endp = doc["endpoint"];
        String endpoint = String(endp);

        const char* msg = doc["mensagem"];
        String mensagemTipo = String(msg);
        
        Serial.println("Gateway: "+nomeGateway);
        Serial.println("Endpoint: "+idEndpoint);
        Serial.println("Mensagem Recebida: "+mensagem);
        Serial.println("O que eu espero da mensagem: "+tipoDeRespostaEsperado);
        
        
        if((gateway.equals(nomeGateway))&&(endpoint.equals(idEndpoint))&&(mensagemTipo.equals(tipoDeRespostaEsperado))){
          Serial.println("Mensagem correta");
          return true;
     
          }else{
            Serial.println("Está mensagem nao foi destinada a este gateway, ou o endpoint nao esta cadastrado, ou a mensagem nao é a desejada");
            return false;
            }
  }

bool enviaMsgLora(String mensagem){
  LoRa.beginPacket();
  LoRa.print(mensagem);
  LoRa.endPacket();
  return true;
  }

String recebeMsgLora(){
  Serial.println("Esperando receber mensagem por até 5 segundos");

  long tempoInicialEsperarResposta = millis();
  while(millis()<tempoInicialEsperarResposta+5000){
      int packetSize = LoRa.parsePacket();
          if(packetSize) {
            String mensagem ="";
            Serial.print("Pacote recebido '");
            while(LoRa.available ()) {
               mensagem += (char)LoRa.read();
           }
            Serial.println("Uma mensagem Foi recebida:");
            Serial.println("Mensagem: "+mensagem);           
            return mensagem;
          }
     }
      Serial.println("Nao recebi nada via lora");
      return "NULL";
  }


 
bool temReprogramacao(String idEndpoint){
  String nomeArquivo = "/atualizacaoEndpoint-"+idEndpoint+".txt";
  char nomeArq[nomeArquivo.length()+3];
  nomeArquivo.toCharArray(nomeArq,nomeArquivo.length()+3);
  String reprogramacao = flash.lerArquivo(nomeArq);
  

  if(reprogramacao.equals("0")){
      Serial.println("NAO existe reprogramacao para este endpoint");
      return false;
    }else{
      Serial.println("Existe reprogramacao para este endpoint");
      return true;
      }
  
  return true;
  }

bool enviaReprogramacao(String idEndpoint){
    Serial.println("Callback: enviaReprogramacao("+idEndpoint+")");

    Serial.println("Gerando a mensagem que sera enviada com dados de reprogramaçao");
    String nomeArquivo = "/atualizacaoEndpoint-"+idEndpoint+".txt";
    char nomeArq[nomeArquivo.length()+3];
    nomeArquivo.toCharArray(nomeArq,nomeArquivo.length()+3);
    String mensagemReprogramacao = flash.lerArquivo(nomeArq);
    
    Serial.println("MSG: "+mensagemReprogramacao);
    
    enviaMsgLora(mensagemReprogramacao);
    Serial.println("Mensagem Lora enviada, esperando  ate 5 segundos por uma resposta");
    long tempoInicialRespostaReprogramacao = millis();
    while(millis()<tempoInicialRespostaReprogramacao+5000){
      String msg = recebeMsgLora();
      if(!msg.equals("NULL")){
        Serial.println("Mensagem Util recebida");
        String tipoDeRespostaEsperado = "REPROGRAMACAO-RECEBIDA-COM-SUCESSO"; 
        if(processaMsgLora(msg,idEndpoint,tipoDeRespostaEsperado)){
            flash.salvarArquivo(nomeArq,"0");
            return true;
         }
      }
    }

   return false;
}
  
bool lerEndpoint(String idEndpoint){
    Serial.println("Callback: lerEndpoint("+idEndpoint+")");

    Serial.println("Gerando a mensagem que sera enviada pedindo os dados");
        
    String nomeGateway = flash.lerArquivo("/nomeGateway.txt");
    String mensagemPedindoDados = "{\"gateway\":\""+nomeGateway+"\",\"endpoint\":\""+idEndpoint+"\",\"mensagem\":\"ENVIAR-DADOS\"}";
    Serial.println("Mensagem: "+mensagemPedindoDados);
    
    enviaMsgLora(mensagemPedindoDados);
    
    Serial.println("A mensagem pedindo os dados foi enviada, esperando até 5 segundos por uma resposta");
    String msgRecebida = recebeMsgLora();
    if(!msgRecebida.equals("NULL")){
      Serial.println("Recebi uma mensagem util\n Checando se eu recebi um DADO do endpoint que eu desejo e se é pra esse Gateway");

      String tipoDeRespostaEsperado = "DADOS"; 
      if(processaMsgLora(msgRecebida,idEndpoint,tipoDeRespostaEsperado)){
          processaDado(msgRecebida);
          Serial.println("Verificando se existe reprogramaçao para esse endpoint");
          if(temReprogramacao(idEndpoint)){
             Serial.println("Existe reprogramaçao para esse endpoint, tentando enviar por até 1 minuto");
              long tempoInicialReprog = millis();
              while(millis()<tempoInicialReprog+60000){
                if(enviaReprogramacao(idEndpoint)){
                    break;
                  }
                }
             Serial.println("Mensagem processada e confirmaçao de reprogramaçao recebida");
             
            }else{
              Serial.println("NAO existe reprogramaçao pendente para esse endpoint");
             }

            
              
              String mensagemConfirmacao = "{\"gateway\":\""+nomeGateway+"\",\"endpoint\":\""+idEndpoint+"\",\"mensagem\":\"DADOS-RECEBIDOS-COM SUCESSO\"}";
              long tempoInicialEnvioConfirmacao = millis();
              
              while(millis()<tempoInicialEnvioConfirmacao+30000){
                enviaMsgLora(mensagemConfirmacao);
                String msgRecebida = recebeMsgLora();
                if(!msgRecebida.equals("NULL")){
                    String tipoDeRespostaEsperado = "CONFIRMACAO-DE-ENTREGA-RECEBIDA"; 
                    if(processaMsgLora(msgRecebida,idEndpoint,tipoDeRespostaEsperado)){

                        return true;
                  }

               
              }
            }
          Serial.println("Dados recebidos porem o endpoint nao informou se recebeu uma confirmaçao de entrega");
          return true;
       }
      
        
  }
  Serial.println("Nao obtive nenhuma resposta...");
  return false;     
}

void processarEndpoint(String idEndpoint){
  Serial.println("Callback: processarEndpoint("+idEndpoint+")");
  long tempoInicialProcessamento = millis();

  // tenta ler endpoint por ate 1 minuto
  Serial.print("Tentando ler o endpoint "+idEndpoint+" por até 1 minuto");
  while(millis()<tempoInicialProcessamento+60000){
      if(lerEndpoint(idEndpoint)){
          Serial.println("Endpoint lido com sucesso");
          break;
        }
  }
}

void loop() {    

      int i=0;
      for(i=0;i<quantidadeEndpoints;i++){

        Serial.print("Processar dados do Endpoint nº: ");
        Serial.print(IdEndpoints[i]);
        processarEndpoint(IdEndpoints[i]);

        attDados();   
        //receberMQTT();
      
      /*
     if(millis()>=(900000)){
        ESP.restart();
        }
    */
     
      }
}

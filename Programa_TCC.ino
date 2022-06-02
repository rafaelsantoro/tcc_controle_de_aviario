//Programa para utilização em ESP8266 para controle de variáveis em pequenos aviários//
//Elaborado sob licença GPL v3.0//
//Os interessados podem utilizar o software para qualquer finalidade,//
//mudar o software de acordo com suas necessidades, compartilhar o software//
//com seus amigos e vizinhos, entre outros e compartilhar as mudanças que você fez.//
//Peço a gentileza, que cite o criador desse código quando fizer uso deste e mantenha o link//
//do projeto original no GitHub: https://github.com/rafaelsantoro/tcc_controle_de_aviario nos comentários//
//do seu projeto. No mais faça bom uso e tenha um ótimo dia.//
//Este código teve por objetivo permitir o funcionamento de um protótipo elaborado como trabalho de//
//conclusão de curso para o curso de Engenharia de Biossistemas pelo acadêmico Rafael R. Santoro//
//no campus do IFSP-Avaré, Brasil em 2022.//

//Inicializando Wi-Fi
#include <ESP8266WiFi.h>                  //Biblioteca para controle do WI-FI
#define WLAN_SSID  "wwwwwwww"             //Variável com o SSID da rede
#define WLAN_PASS  "xxxxxxxx"             //Variável com a senha de rede
#define AP_SSID    "yyyyyyyy"          //Constante com o nome de SSID para acesso via Access Point (direto no aparelho)
#define AP_PASS    "zzzzzzzz"             //Constante com a senha para acesso via Access Point (direto no aparelho)

//Inicializando OTA                   
#include <ESP8266mDNS.h>              //Biblioteca para publicação de IP via DNS
#include <WiFiUdp.h>                  //Biblioteca para protocolo UDP, auxilia no DNS
#include <ArduinoOTA.h>               //Biblioteca para programação do microcontrolador via WI-FI (Over the air)

//Inicialização para formulário e WEB Server
#include <WiFiClient.h>               //Biblioteca auxiliar de conexão WI-FI para o serviço Adafruit
#include <ESP8266WebServer.h>         //Biblioteca com o servidor WEB para o microcontrolador
ESP8266WebServer server(80);          //Cria o objeto server do servidor web na porta 80

//Inicializando io.adafruit.com (Protocolo MQTT)
#include "Adafruit_MQTT.h"                                  //Biblioteca necessária para comunicação com o serviço Adafruit
#include "Adafruit_MQTT_Client.h"                           //Biblioteca necessária para comunicação com o serviço Adafruit
#define AIO_SERVER      "io.adafruit.com"                   //Constante com o endereço DNS do servidor da Adafruit
#define AIO_SERVERPORT  1883                                //Porta de comunicação com a Adafruit
#define AIO_USERNAME  "xxxxxxxxxxxxx"                       //Usuário do serviço Adafruit
#define AIO_KEY  "aio_yyyyyyyyyyyyyyyyyyyyyyyyyyyy"         //Chave para publicação do serviço Adafruit
WiFiClient client;                                          //Cria objeto client do tipo WifiClient para uso com Adafruit                         
//Parâmetros a serem utilizados na conexão com a Adafruit
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);                      

  //Configurando os feeds do MQTT
  Adafruit_MQTT_Publish modo = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/modo");                    //Publicação para Modo Automático 
  Adafruit_MQTT_Publish atuador1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/atuador1");            //Publicação para atuador1 
  Adafruit_MQTT_Publish atuador2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/atuador2");            //Publicação para atuador2 
  Adafruit_MQTT_Publish atuador3 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/atuador3");            //Publicação para atuador3 
  Adafruit_MQTT_Publish atuador4 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/atuador4");            //Publicação para atuador4 
  Adafruit_MQTT_Publish temperatura = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");      //Publicação da Temperatura
  Adafruit_MQTT_Publish umidade = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/umidade");              //Publicação da Umidade
  Adafruit_MQTT_Publish luminosidade = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/luminosidade");    //Publicação da Luminosidade
  Adafruit_MQTT_Publish amonia = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/amonia");                //Publicação do estado de NH3
    
//Inicializando ATUADORES
#define ATUADOR_1 13     //Constante de conexão da porta para o atuador1 (Relé)
#define ATUADOR_2 12     //Constante de conexão da porta para o atuador2 (Relé)
#define ATUADOR_3 14     //Constante de conexão da porta para o atuador3 (Relé)
#define ATUADOR_4 15     //Constante de conexão da porta para o atuador4 (Relé)

//Inicializando DHT
#include <SimpleDHT.h>        //Inclusão da biblioteca SimpleDHT, para leitura do sensor DHT22
#define DHTPORT D3            //Constante de conexão da porta para o sensor DHT22
SimpleDHT22 DHT22(DHTPORT);   //Criação do objeto DHT22 na porta DHTPORT para leitura

//Inicializando LDR
#define LDRPORT A0            //Constante de conexão da porta analógica para o sensor LDR

//Inicializando MQ135
#define MQ135PORT 16          //Constante de conexão da porta para o sensor MQ135

//Inicializando o BUZZER
#define BUZZER 2              //Constante de conexão da porta para o buzzer, apito.

//Inicializando o RTC (DS1307)
#include <Wire.h>             //Biblioteca auxiliar de funcionamento para o DS1307
#include <RTC.h>              //Biblioteca para leitura e escrita dos dados do DS1307
static DS1307 RTC;            //Criação do objeto de trabalho com o DS1307

//Inicializando o SPIFFS (Leitura e gravação de arquivos)
#include <FS.h>               //Biblioteca para trabalho com arquivos no SPIFFS (Armazenamento não volátil)

//Variáveis de trabalho
bool dht_erro = false;                    //Variável para tratamento de erro na leitura do sensor DHT22
bool modo_teste = false;                  //Variável para sinalizar que o sistema está em modo de teste
bool calibracao = false;				  //Variável para verificar se modo de calibração está ativo
float leitura_temperatura = 0;            //Variável para armazenar o valor de temperatura lido
float leitura_umidade = 0;                //Variável para armazenar o valor de umidade relativa lido
float leitura_luz = 0;                    //Variável para armazenar o valor de luminosidade lido
String leitura_gas = "";                  //Variável para armazenar texto resultado da leitura do sensor de gás MQ135
String leitura_gas_anterior = "";		  //Variável para mudança no estado da Amônia
int dht_leit_ant = 60;                    //VAriável para controle do intervalo de leituras do DHT22
String buf = "";                          //Variável para armazenar dados à serem lidos por SPIFFS
static unsigned long ult_tempo = 0;       //Variável auxiliar na rotina de controle de tempo com a classe millis();
int tempo_passado_s = 0;                  //Variável de controle de tempo determinado
bool atu1 = false;                        //Variável para armazenar o estado do atuador 1
bool atu2 = false;                        //Variável para armazenar o estado do atuador 2
bool atu3 = false;                        //Variável para armazenar o estado do atuador 3
bool atu4 = false;                        //Variável para armazenar o estado do atuador 4
bool automatico = true;                   //Variável que define o modo de operação dos atuadores para manual ou automático
bool buzina = false;                      //Variável para habilitar e desabilitar o buzzer

//Função para reinício da placa remotamente
void(* reinicia) (void) = 0;              //Função de acionamento da porta 0 que provoca o reinício do microcontrolador

//Função para criação do arquivo "dados.txt"
void cria_arquivo(void){                              //Cria função
  File wFile;                                         //Variável para o arquivo
  if(SPIFFS.exists("/dados.txt")){                    //Verifica se o arquivo existe
    Serial.println("Arquivo já existe!");             //Informa que já existe
  }
  else {                                              //Caso contrário cria um novo arquivo
    Serial.println("Criando arquivo ...");            //Informa via serial
    wFile = SPIFFS.open("/dados.txt","w+");           //Abre arquivo para escrita

    if(!wFile) {                                      //Caso ocorra um erro
      Serial.println("Erro ao criar arquivo.");       //Informa via serial
    }
    else {                                            //Caso não ocorra nenhum erro
      Serial.println("Arquivo criado com sucesso.");  //Informa via serial
    }
  }
  wFile.close();                                      //Fecha o arquivo
}

//Função para deletar o arquivo "dados.txt"
void deleta_dados(void) {                                           //Cria a função
  if(SPIFFS.remove("/dados.txt")) {                                 //Deleta o arquivo
    Serial.println("Arquivo deletado.");                            //Informa via serial
  }
  else {                                                            //Caso ocorra erro
    Serial.println("Erro ao remover o arquivo.");                   //Informa via serial
  }
}

//Função que anexa uma string ao final do arquivo "/dados.txt"
void anexa_dados(String msg) {                                      //Cria função que recebe string
  File rFile = SPIFFS.open("/dados.txt","a+");                      //Abre o arquivo para adição (append) 
  if(!rFile){                                                       //Caso ocorra erro
    Serial.println("Erro ao escrever no final do arquivo.");        //Informa via serial
  } else {                                                          //Caso não ocorra erro
    rFile.println(msg);                                             //Acrescenta na linha
  }
  rFile.close();                                                    //Fecha arquivo
}

//Função de controle dos atuadores
void Atuador(int atu) {                                       //Cria função atuador
  switch (atu) {                                              //Função de tratamento para número do atuador solicitado
    case 1:                                                   //Caso atuador 1
    digitalWrite(ATUADOR_1,!digitalRead(ATUADOR_1));          //Inverte o sinal da porta do Atuador 1, entre HIGH e LOW
    if (digitalRead(ATUADOR_1) == HIGH) { Serial.println("Atuador 1 - Ligado"); anexa_dados("\nAtuador 1 - Ligado;\n"); } 
    else { Serial.println("Atuador 1 - Desligado"); anexa_dados("\nAtuador 1 - Desligado;\n"); } 
    break;                                                    //Sai da função switch
    case 2:                                                   //Idem para atuador 2
    digitalWrite(ATUADOR_2,!digitalRead(ATUADOR_2)); 
    if (digitalRead(ATUADOR_2) == HIGH) { Serial.println("Atuador 2 - Ligado"); anexa_dados("\nAtuador 2 - Ligado;\n"); } 
    else { Serial.println("Atuador 2 - Desligado"); anexa_dados("\nAtuador 2 - Desligado;\n"); }          
    break;
    case 3:                                                   //Idem para atuador 3
    digitalWrite(ATUADOR_3,!digitalRead(ATUADOR_3));
    if (digitalRead(ATUADOR_3) == HIGH) { Serial.println("Atuador 3 - Ligado"); anexa_dados("\nAtuador 3 - Ligado;\n"); } 
    else { Serial.println("Atuador 3 - Desligado"); anexa_dados("\nAtuador 3 - Desligado;\n"); }           
    break;
    case 4:                                                   //Idem para atuador 4
    digitalWrite(ATUADOR_4,!digitalRead(ATUADOR_4));
    if (digitalRead(ATUADOR_4) == HIGH) { Serial.println("Atuador 4 - Ligado"); anexa_dados("\nAtuador 4 - Ligado;\n"); }  
    else { Serial.println("Atuador 4 - Desligado"); anexa_dados("\nAtuador 4 - Desligado;\n"); } 
    break;
  }
} 

//Função para limitar o acionamento de algum atuador para menos de 3 minutos
void tempo_atuador(int atuador, int tempo){ 						//Cria a função para que determinado atuador possa ser temporizado em menos de 3 minutos
  if (atuador == 1 && tempo_passado_s >= tempo && atu1 == true) { Atuador (atuador); }	//Verifica se o atuador selecionado está ativado e se já alcançou o tempo determinado, desligando-o
  if (atuador == 2 && tempo_passado_s >= tempo && atu2 == true) { Atuador (atuador); }	//Idem para o atuador 2
  if (atuador == 3 && tempo_passado_s >= tempo && atu3 == true) { Atuador (atuador); }	//Idem para o atuador 3
  if (atuador == 4 && tempo_passado_s >= tempo && atu4 == true) { Atuador (atuador); }	//Idem para o atuador 4
}                                                            		//Final da função monitora_atuador

//Função para atualizar variáveis de atuadores
void verif_atuadores () {                                     //Cria a função de verificar atuadores
  if (digitalRead(ATUADOR_1) == HIGH) { atu1 = true; }        //Caso a porta do atuador 1 esteja acionada, em HIGH, atualiza a variável atu1
  else { atu1 = false; }                                      //Caso contrário atualiza a variável para false
  if (digitalRead(ATUADOR_2) == HIGH) { atu2 = true; }        //Idem para atuador 2
  else { atu2 = false; }
  if (digitalRead(ATUADOR_3) == HIGH) { atu3 = true; }        //Idem para atuador 3
  else { atu3 = false; }
  if (digitalRead(ATUADOR_4) == HIGH) { atu4 = true; }        //Idem para atuador 4
  else { atu4 = false; }
}

//Função para leitura de temperatura e umidade
void Temp_Umid() {                                                                            //Cria a função de leitura de Temp. e Umidade
  dht_leit_ant = RTC.getMinutes();                                                            //Atualiza variável para leitura a cada 1 minuto
  float temperatura = 0;                                                                      //Variável local para armazenar temperatura
  float umidade = 0;                                                                          //Variável local para armazenar umidade
  int err = SimpleDHTErrSuccess;                                                              //Variável com estado de leitura do DHT22
  if ((err = DHT22.read2(&temperatura, &umidade, NULL)) != SimpleDHTErrSuccess) {             //Caso tenha algum erro na leitura
    Serial.print("Falha ao ler sensor DHT22, err="); Serial.print(SimpleDHTErrCode(err));     //Envia mensagem e código de erro via porta serial
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err));                             //Envia duração do erro na porta serial
    Buzzer(3, 150);                                                                           //Em caso de erro no DHT22 um sinal sonoro de 3 bips longos é acionado
    dht_erro = true;                                                                          //Atualiza a variável de erro no DHT para true
    leitura_temperatura = 0;                                                                  //Zera a variável global para temperatura
    leitura_umidade = 0;                                                                      //Zera a variável global para umidade
  }                                                                                           //Fim do tratamento de erro
  else {                                                                                      //Caso a leitura seja bem sucedida
    dht_erro = false;                                                                         //Atualiza a variável de erro no DHT para false
    leitura_temperatura = temperatura - 3;                                                    //Atualiza variável global de temperatura
    leitura_umidade = umidade;                                                                //Atualiza variável global de umidade
  }                                                                                           //Final do tratamento das leituras
}  

//Função para leitura da luminosidade
void Luz () {                                                   //Cria a função de leitura
  float vLDR = (3.3 / 1024) * analogRead(LDRPORT);              //Retorna o valor de voltagem para vLDR, da porta LDRPORT
  float ResistLDR = ((10000 * 3.3) / vLDR)-10000;               //Retorna o valor de resistância para ResistLDR utilizando cálculo de divisão de tensão
  float luminosidade = 2242931 * pow(ResistLDR,-1.3011);        //Calcula o valor em Lux de luminosidade que incide sobre o LDR 5528
  if (luminosidade >= 100) { luminosidade = 100; }              //Levando em conta o limite do sensor, se o valor em LUX for igual ou maior que 100, será considerado como 100 LUX
  else if (luminosidade < 0.5) { luminosidade = 0; }            //Levando em conta o limite do sensor, se o valor em LUX for menor que 1, será considerado como ZERO
  leitura_luz = luminosidade;                                   //Atualiza variável global de luminosidade
}

//Função para calibragem da porta digital do sensor MQ-135
//Para realizar a calibragem da porta digital do sensor MQ-135 usa-se momentâneamente a porta analógica, devendo-se desconectar
//o LDR da porta A0 e ligar a porta de dados analógicos do MQ-135 em seu lugar, em seguida deve-se posicionar o resitor
//do sensor MQ-135 até que o sinal desejado fique com a concentração desejada. O sensor neste momento deve estar exposto
//a amônia na concentração informada na porta analógica, podendo-se então fazer a comparação.
void Calib_Gas () {                                                                                 //Criação da função de calibração
  float vLDR = (3.3 / 1024) * analogRead(LDRPORT);                                                  //Retorna o valor de voltagem para o sensor de gás na porta A0 do LDR
  float ResistLDR = ((22000 * 3.3) / vLDR)-22000;                                                   //Retorna o valor de resistância para a porta A0
  float amonia = 2242931 * pow(ResistLDR,-1.3011);                                                  //Calcula o valor em ppm de amonia que incide sobre o MQ-135
  if (amonia >= 300) { amonia = 300; }                                                              //Levando em conta o limite do sensor, se o valor de Amônia for igual ou maior que 300, será considerado como 300 ppm
  else if (amonia < 10) { amonia = 10; }                                                            //Levando em conta o limite do sensor, se o valor em Amônia for menor que 10, será considerado como 10 ppm ou AUSENTE
  Gas ();                                                                                           //Faz a leitura do sensor de gás
  Serial.println("AMÔNIA DIGITAL: " + leitura_gas + " // AMÔNIA ANALÓGICO: " + String(amonia));     //Envia dados para comparação via porta serial 
}

//Função para acionamento do Buzzer
void Buzzer (int vez,int tempo) {         //Cria a função de acionamento do buzzer, apito.
  if (buzina == true) {                   //Executa se a buzina estiver habilitada
    for (int i = 0; i < vez; i++) {       //Laço que faz tocar pelo número de vezes solicitado
      digitalWrite(BUZZER, LOW);          //Porta do buzzer em estado de nível baixo
      delay(tempo);                       //Para o microcontrolador pelo tempo solicitado
      digitalWrite(BUZZER, HIGH);         //Muda o estado da porta para alto
      delay(tempo);                       //Para o microcontrolador pelo tempo solicitado, mantendo o apito
    }
  }
}

//Função para toque de alarme de presença de amônia
void Alarme () {                        //Cria uma função de alarme
  Buzzer (10, 200);                     //Ativa o buzzer por 10 vez e 200 ms de intervalo
}

//Função para leitura do sensor MQ-135
void Gas () {                           //Cria função para leitura de gás no sensor MQ 135
  int gas = digitalRead(MQ135PORT);     //Lê porta digital do sensor
  String saida = "";                    //Zera a string de saida da função
  if (gas ==1) {saida = "Ausente";}     //Caso o sensor retorne 1 não há presença de gás
  else {saida = "Presente";}            //Caso contrário altera variável para Presente
  leitura_gas = saida;                  //Atualiza variável global de leitura de gás
}

//Função para leitura do dia e hora
String Data_Hora() {                      //Criação da função
  String saida = "";                      //Variável de saída
  switch (RTC.getWeek())                  //Seleção para o dia da semana
  {
    case 1:                               //Caso retorne 1
      saida = "DOM-";  break;             //Variável saída será domingo
    case 2:                               //Caso retorne 2
      saida = "SEG-";  break;             //Variável saída será segunda
    case 3:                               //e assim sucessivamente
      saida = "TER-";  break;
    case 4:
      saida = "QUA-";  break;
    case 5:
      saida = "QUI-";  break;
    case 6:
      saida = "SEX-";  break;
    case 7:
      saida = "SAB-";  break;
  }

  saida = saida + RTC.getHours() + ":" + RTC.getMinutes() + ":" + RTC.getSeconds() + "-"; //Adiciona o tempo na variável de saída
  saida = saida + RTC.getDay() + "/" + RTC.getMonth() + "/" + RTC.getYear();              //Adiciona a data na variável de saída
  return saida;                                                                           //Retorna string com dia, data e hora.
}

//Função para a atualização do dia, data e hora no DS 1307
void Atua_Data_Hora (String tempo) {                  //Cria a função de atualização
                                                      //Uma variável para cada campo, como descrito abaixo, a partir de substring
  int semana = (tempo.substring(0,2)).toInt();        //01-DOM, 02-SEG, 03-TER, 04-QUA, 05-QUI, 06-SEX, 07-SAB
  int hora = (tempo.substring(2,4)).toInt();          //01 ATÉ 23
  int minuto = (tempo.substring(4,6)).toInt();        //01 ATÉ 59
  int segundo = (tempo.substring(6,8)).toInt();       //00 ATÉ 59
  int dia = (tempo.substring(8,10)).toInt();          //01 ATÉ 31
  int mes = (tempo.substring(10,12)).toInt();         //01 ATÉ 12
  int ano = (tempo.substring(12,14)).toInt();         //00 ATÉ 99 (2000-2099)

  RTC.setWeek(semana);                                //Armazena o dia da semana no DS 1307
  RTC.setTime(hora,minuto,segundo);                   //Armazena a hora minuto e segundo
  RTC.setDate(dia,mes,ano);                           //Armazena o dia mês e ano
}

//Função para teste dos diferentes dispositivos da placa
void Modo_Teste () {                                                          //Cria a função de teste

  Serial.println("----------------------------------------------------");     //Delimitador na saída serial do início do ciclo
  Serial.println(Data_Hora());                                                //Envia na porta serial o tempo atual no DS 1307
  Serial.println("UMIDADE: " + String(leitura_umidade) + " %");               //Envia a umidade na porta serial
  delay(1000);                                                                //Aguarda 1 segundo
  Serial.println("AMÔNIA: " + leitura_gas);                                   //Envia a leitura de gás
  delay(1000);                                                                //Aguarda 1 segundo
  Serial.println("LUMINOSIDADE: " + String(leitura_luz) + " Lux");            //Envia a leitura de lumnosidade
  delay(1000);                                                                //Aguarda 1 segundo
  Serial.println("TEMPERATURA: " + String(leitura_temperatura) + " *C");      //Envia a leitura de temperatura
  delay(1000);                                                                //Aguarda 1 segundo
  Buzzer(1,50);              //Toca apito para testar o buzzer
  Atuador(1);                //Alterna sinal do atuador 1
  delay(3000);               //Aguarda 3 segundos
  Atuador(2);                //Alterna sinal do atuador 2
  delay(3000);               //Aguarda 3 segundos
  Atuador(3);                //Alterna sinal do atuador 3
  delay(3000);               //Aguarda 3 segundos
  Atuador(4);                //Alterna sinal do atuador 4
  delay(3000);               //Aguarda 3 segundos
}

//Função de conexão MQTT
void MQTT_connect() {                                     //Cria a função de conexão MQTT
  int8_t ret;                                             //Variável auxiliar para erros de conexão
  if (mqtt.connected()) {                                 //Se já estiver conectado sai da função
    return;                                               //Sai da função
  }
  Serial.print("Conectando MQTT... ");                    //Envia mensagem na porta serial de conexão MQTT
  uint8_t retries = 10;                                   //Variável auxiliar para contar tentativas de conexão
  while ((ret = mqtt.connect()) != 0) {                   //Executa enquanto não houver conexão MQTT
       Serial.println(mqtt.connectErrorString(ret));      //Se ocorrer erro, envia erro via serial
       Serial.println("Reconectando em 3s...");           //Envia mensagem de reconexão na porta serial
       mqtt.disconnect();                                 //Desconecta o MQTT
       for (int i = 1; i < 30; i++) {                     //Executa um loop para que o microcontrolador não fique parado muito tempo
       delay(100);                                        //Aguarda 100 milisegundos
       }
       retries--;                                         //Subtrai 1 da variável auxiliar de contagem de conexões
       if (retries == 0) {                                //Executa se não houver mais tentativas
         break;                                           //Encerra o loop while
       }       
  }
  yield();
}

//Função para controle de tempos  
void tempo (int intervalo) {                              //Cria a função tempo
  if ((millis() - ult_tempo) >= 1000) {                   //Se a variável de sistema millis completar 1 segundo passado     
    tempo_passado_s ++;                                   //Acresce 1 a variável tempo_passado_s
    ult_tempo = millis();                                 //Atualiza a variável ult_tempo
  }
  if (tempo_passado_s >= intervalo) { tempo_passado_s = 0; }     //Se tempo_passado_s for maior que intervalo atualiza
}

//Função de transmissão MQTT
void Transmite_MQTT () {                                                        //Cria a função de transmissão

    verif_atuadores ();                                                         //Verifica o estado dos atuadores para enviar o estado atualizado
    
    int i_amonia = 0;                                                           //Variável inteira local para status do gás
    if (leitura_gas == "Ausente") { i_amonia = 0; } else { i_amonia = 1; }      //Trata leitura afim  de transformar string em inteiro
  
    if (dht_erro == false) {                                                    //Transmite a temperatura e umidade somente se a leitura não tiver retornado erro   
      if (! temperatura.publish(leitura_temperatura)) {                         //Se ocorrer erro na transmissão da temperatura
        Serial.println("Falha na publicação da temperatura"); }                 //Informa erro via serial
      else {                                                                    //Executa caso não tenha erro
       Serial.println("Temperatura publicada! (" + String(leitura_temperatura, 2) + " *C)"); }    //Informa via serial que publicou com sucesso
      if (! umidade.publish(leitura_umidade)) {                                                   //Se ocorrer erro na transmissão da umidade
        Serial.println("Falha na publicação da umidade"); }                                       //Informa erro via serial
      else {                                                                                      //Executa caso não tenha erro
        Serial.println("Umidade publicada! (" + String(leitura_umidade, 2) + " %)"); }            //Informa via serial que publicou com sucesso
    }                               
    if (! luminosidade.publish(leitura_luz)) {                                                    //Se ocorrer erro na transmissão da luminosidade
      Serial.println("Falha na publicação da luminosidade"); }                                    //Informa erro via serial
    else {                                                                                        //Executa caso não tenha erro
      Serial.println("Luminosidade publicada! (" + String(leitura_luz, 2) + " Lux)"); }           //Informa via serial que publicou com sucesso
    if (! amonia.publish(i_amonia)) {                                                             //Se ocorrer erro na transmissão da Amônia
      Serial.println("Falha na publicação de NH3"); }                                             //Informa erro via serial
    else {                                                                                        //Executa caso não tenha erro
      if ( i_amonia == 0 ) { Serial.println("NH3 publicado! (Ausente)"); }                        //Informa via serial que publicou com sucesso (Amônia Ausente)
      else { Serial.println("NH3 publicado! (Presente)"); }                                       //Informa via serial que publicou com sucesso (Amônia Presente)
    }    
    
    int operacao = 0;                                                                             //Cria variável local para converter modo
    if (automatico == true) { operacao = 1; } else { operacao = 0; }                              //Faz a conversão de booleano para inteiro
    if (! modo.publish(operacao)) {                                                               //Se ocorrer erro na transmissão do modo de operação
      Serial.println("Falha na publicação do modo de operação"); }                                //Informa erro via serial
    else {                                                                                        //Executa caso não tenha erro
      if ( operacao == 0 ) { Serial.println("Modo de operação publicado! (Manual)"); }            //Informa via serial que publicou com sucesso (Manual)
      else { Serial.println("Modo de operação publicado! (Automático)"); } }                      //Informa via serial que publicou com sucesso (Automático)

    int conv_atu1 = 0;                                                                            //Cria variável local para converter booleano para inteiro
    if (atu1 == true) { conv_atu1 = 1; } else { conv_atu1 = 0; }                                  //Faz a conversão de booleano para inteiro
    if (! atuador1.publish(conv_atu1)) {                                                          //Se ocorrer erro na transmissão do Atuador
      Serial.println("Falha na publicação do Atuador #1"); }                                      //Informa erro via serial
    else {                                                                                        //Executa caso não tenha erro
      if (conv_atu1 == 1) {Serial.println("Atuador #1 publicado! (Ligado)"); }                    //Informa via serial que publicou com sucesso (Ligado)
      else { Serial.println("Atuador #1 publicado! (Desligado)"); } }                             //Informa via serial que publicou com sucesso (Desligado)

    int conv_atu2 = 0;                                                                            //Idem para Atuador #1
    if (atu2 == true) { conv_atu2 = 1; } else { conv_atu2 = 0; }                                  
    if (! atuador2.publish(conv_atu2)) {                                                          
      Serial.println("Falha na publicação do Atuador #2"); }                                      
    else {                                                                                        
      if (conv_atu2 == 1) {Serial.println("Atuador #2 publicado! (Ligado)"); }                    
      else { Serial.println("Atuador #2 publicado! (Desligado)"); } }        

    int conv_atu3 = 0;                                                                            //Idem para Atuador #1
    if (atu3 == true) { conv_atu3 = 1; } else { conv_atu3 = 0; }                                  
    if (! atuador3.publish(conv_atu3)) {                                                          
      Serial.println("Falha na publicação do Atuador #3"); }                                      
    else {                                                                                        
      if (conv_atu3 == 1) {Serial.println("Atuador #3 publicado! (Ligado)"); }                    
      else { Serial.println("Atuador #3 publicado! (Desligado)"); } } 

    int conv_atu4 = 0;                                                                            //Idem para Atuador #1
    if (atu4 == true) { conv_atu4 = 1; } else { conv_atu4 = 0; }                                  
    if (! atuador4.publish(conv_atu4)) {                                                          
      Serial.println("Falha na publicação do Atuador #4"); }                                      
    else {                                                                                        
      if (conv_atu4 == 1) {Serial.println("Atuador #4 publicado! (Ligado)"); }                    
      else { Serial.println("Atuador #4 publicado! (Desligado)"); } } 
}

//Função para formatar o SPIFFS
void formata(void){                         //Cria função
  Serial.print("Formatando SPIFFS...");     //Informa formatação via serial
  SPIFFS.format();                          //Apaga o conteúdo do SPIFFS
  Serial.println(" OK");                    //Informa finalização
}

//Função que lê arquivo "/dados.txt"
void le_dados_html(void) {                            //Cria função
  buf = "";                                           //Zera variável de buffer
  File rFile = SPIFFS.open("/dados.txt","r");         //Faz a abertura do arquivo
  Serial.println("Lendo arquivo...");                 //Informa a leitura do arquivo
  server.sendContent ("<p>Início dos dados.</p><p>");
  while(rFile.available()) {                          //Enquanto não acabar a leitura do arquivo
    buf = rFile.readString();
    buf += "<br>";
    server.sendContent (buf);
    buf = "";
    
    
//    server.sendContent (rFile.readString());
//    server.sendContent ("<br>");
  }
  server.sendContent ("</p><p>Fim dos dados.</p>");
  rFile.close();                                      //Fecha arquivo
}

//Função que inicia sistema de arquivos SPIFFS
void abre_FS(void){                                               //Cria função
  if(!SPIFFS.begin()){                                            //Tenta abrir o sistema de arquivos SPIFFS
    Serial.println("Erro ao abrir o sistema de arquivos.");       //Informa que houve um erro
  } else {                                                        //Caso não ocorra erro
    Serial.println("Sistema de arquivos aberto com sucesso.");    //Informa via serial
  }
}

//Função que fecha sistema de arquivos SPIFFS
void fecha_FS(void){                              //Cria função
  SPIFFS.end();                                   //Fecha sistema de arquivos
}

//Função que envia dados pela porta serial para ser lido no monitor serial
void enviaSerial(void){                                     //Cria função
  File rFile = SPIFFS.open("/dados.txt","r");               //Faz a abertura do arquivo
  Serial.println("***Lendo arquivo...***");                 //Informa a leitura do início do arquivo
  while(rFile.available()) {                                //Enquanto não acabar a leitura do arquivo
  Serial.println(rFile.readString());                       //Envia a linha via serial
  }
  rFile.close();                                            //Fecha arquivo
  Serial.println("***Leitura Finalizada!***");              //Informa final da leitura do arquivo
}

//Função que armazena as leituras no arquivo "dados.txt" 
void gravaLEITURAS(void){                                   //Cria a função   
      //Envia uma linha ao final do arquivo de dados.txt
      anexa_dados(Data_Hora() + ";" + String(leitura_temperatura) + ";" + String(leitura_umidade) + ";" + String(leitura_luz) + ";" + leitura_gas + ";"); 
}

//Função para acionamento de atuador por amônia
void trata_amonia(int atuador){                             //Cria a função
  if ( leitura_gas_anterior != leitura_gas ) {              //Mudança no estado da Amônia
    Atuador(atuador);                                       //Aciona atuador
    leitura_gas_anterior = leitura_gas;                     //Atualiza a variável
  }
  if (leitura_gas == "Presente") { Alarme(); }              //Soa alarme caso ainda haja amônia
}

//Função para acionamento de atuador por luz
void trata_luz(int atuador, float lux, int hora_inic, int hora_term, int hora_in_apa, int hora_ter_apa){     //Cria a função
  bool trata_luz = false;									  		//Variável local para estado do relé de luz
  if (atuador == 1) {trata_luz = atu1; }					  		//Verifica se está utilizando o atuador 1
  if (atuador == 2) {trata_luz = atu2; }					  		//Verifica se está utilizando o atuador 2
  if (atuador == 3) {trata_luz = atu3; }					  		//Verifica se está utilizando o atuador 3
  if (atuador == 4) {trata_luz = atu4; }    				  		//Verifica se está utilizando o atuador 4
  if (RTC.getHours() >= hora_inic && RTC.getHours() < hora_term) {  //No intervalo de tempo selecionado mantém a luz acesa
        if (trata_luz == false) { Atuador(atuador); }
  }    
  else if ((RTC.getHours() >= hora_in_apa && RTC.getHours() > hora_ter_apa) || (RTC.getHours() <= hora_in_apa && RTC.getHours() < hora_ter_apa)) {      //No intervalo de tempo selecionado mantém a luz apagada
        if (trata_luz == true) { Atuador(atuador); }
  }
  else {
    if (leitura_luz < lux) {                                  //Alterna o atuador se a leitura de luz for inferior e se atuador estiver desligado
      if (trata_luz == false) { Atuador(atuador); }           //Alterna o estado do atuador
    }    
    if (leitura_luz >= lux) {                                 //Alterna o atuador se a leitura de luz for superior e se atuador estiver ligado
      if (trata_luz == true) { Atuador(atuador); }            //Alterna o estado do atuador
    } 
  }      
}

//Função para acionamento de atuador por temperatura quente
void trata_temp(int atuador, float temp){ 		//Cria a função
  bool trata_temp = false;						//Variável local para estado do relé de temperatura
  if (atuador == 1) {trata_temp = atu1; }		//Verifica se está utilizando o atuador 1
  if (atuador == 2) {trata_temp = atu2; }		//Verifica se está utilizando o atuador 2
  if (atuador == 3) {trata_temp = atu3; }		//Verifica se está utilizando o atuador 3
  if (atuador == 4) {trata_temp = atu4; }		//Verifica se está utilizando o atuador 4
    if (leitura_temperatura > temp && trata_temp == false) { 				//Alterna o atuador se leitura maior que temperatura desejada e atuador estiver desligado
      Atuador(atuador);														//Alterna o estado do atuador
    }
    if (leitura_temperatura < temp && trata_temp == true) {					//Alterna o atuador se leitura menor que temperatura desejada e atuador estiver ligado
      Atuador(atuador);														//Alterna o estado do atuador
    }
}

//Função para acionamento de atuador por temperatura fria
void trata_temp_fria(int atuador, float temp_fria){     //Cria a função
  bool trata_temp_fria = false;            		 		//Variável local para estado do relé de temperatura
  if (atuador == 1) {trata_temp_fria = atu1; }   		//Verifica se está utilizando o atuador 1
  if (atuador == 2) {trata_temp_fria = atu2; }   		//Verifica se está utilizando o atuador 2
  if (atuador == 3) {trata_temp_fria = atu3; }   		//Verifica se está utilizando o atuador 3
  if (atuador == 4) {trata_temp_fria = atu4; }   		//Verifica se está utilizando o atuador 4
    if (leitura_temperatura < temp_fria && trata_temp_fria == false) {        //Alterna o atuador se leitura maior que temperatura desejada e atuador estiver desligado
      Atuador(atuador);                           //Alterna o estado do atuador
    }
    if (leitura_temperatura >= temp_fria && trata_temp_fria == true) {         //Alterna o atuador se leitura menor que temperatura desejada e atuador estiver ligado
      Atuador(atuador);                           //Alterna o estado do atuador
    }
}

//Função para acionamento de atuador por umidade alta
void trata_umid(int atuador, float umid){ 		//Cria a função
  bool trata_umid = false;						//Variável local para estado do relé de umidade
  if (atuador == 1) {trata_umid = atu1; }		//Verifica se está utilizando o atuador 1
  if (atuador == 2) {trata_umid = atu2; }		//Verifica se está utilizando o atuador 2
  if (atuador == 3) {trata_umid = atu3; }		//Verifica se está utilizando o atuador 3
  if (atuador == 4) {trata_umid = atu4; }		//Verifica se está utilizando o atuador 4
    if (leitura_umidade > umid && trata_umid == false) {					//Alterna o atuador se leitura maior que umidade desejada e atuador estiver desligado
      Atuador(atuador);														//Alterna o estado do atuador
    }
    if (leitura_umidade <= umid && trata_umid == true) {					//Alterna o atuador se leitura menor que umidade desejada e atuador estiver ligado
      Atuador(atuador);														//Alterna o estado do atuador
    }
}

//Função para tratar entradas na porta serial
String trata_serial (String texto) {              //Cria função
    if ((texto.substring(0,5)) == "tempo") {      //Caso o texto enviado se inicie por tempo
    Atua_Data_Hora(texto.substring(5,19));        //Envia a string para a função de atualizar o tempo em DS 1307
    Serial.println(Data_Hora());                  //Informa nova hora via serial
    }     
    else if (texto == "reinicia") {               //Caso o texto seja reinicia
    reinicia();                                   //Reinicia o microcontrolador
    }

    else if (texto == "modoteste") {              //Caso o texto seja modoteste
    modo_teste = !modo_teste;                     //Muda a variável afim de ligar o modo teste, ou desligá-lo
    }
    else if (texto == "calibracao") {             //Caso o texto seja calibracao 
    calibracao = !calibracao;					  //Muda a variável afim de ligar o modo de calibração ou desligá-lo
    }

    else if (texto == "enviaserial") {            //Caso o texto seja enviaserial
      enviaSerial();                              //Envia dados do arquivo dados.txt via porta serial
    }
    
    else if (texto == "atuador1") {               //Caso o texto seja atuador1
      Atuador(1);                                 //Alterna o Atuador 1
    }
    else if (texto == "atuador2") {               //Idem para atuador 2
      Atuador(2);
    }
    else if (texto == "atuador3") {               //Idem para atuador 3
      Atuador(3);
    }
    else if (texto == "atuador4") {               //Idem para atuador 4
      Atuador(4);
    }

    
}

//Variáveis globais de construção HTML
//Variável do início dos documentos html
String htmlinicio = "<!DOCTYPE HTML><meta charset ='utf-8'/><meta name='viewport' content='width=device-width, initial-scale=1'><html>";
//Variável do final dos documentos html
String htmlfim = "</html>";

//Função HTTP
//Cria Função
void pagina_http () {

String operacao = "";     //Variável para conversão do formato do modo de operação de booleana para string
if (automatico == true) { operacao = "Automático"; } else { operacao = "Manual"; } //Conversão do formato

String modo_buzina = "";     //Variável para conversão do formato do modo de buzina de booleana para string
if (buzina == true) { modo_buzina = "Ligada"; } else { modo_buzina = "Desligada"; } //Conversão do formato
  
//HTML da pagina principal
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}"); //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Situa&ccedil;&atilde;o Atual - MICROCONTROLADOR</h2>Data e hora do sistema: " + Data_Hora());
         server.sendContent ("<p>Temperatura atual: " + String(leitura_temperatura) + "&#176;C<br>");     //Anexa no HTML a temperatura
         server.sendContent ("Umidade relativa atual: " + String(leitura_umidade) + "%<br>");             //Anexa no HTML a umidade
         server.sendContent ("Luminosidade: " + String(leitura_luz) + " Lux<br>");                        //Anexa no HTML a luminosidade
         server.sendContent ("Presen&ccedil;a de Am&ocirc;nia (NH3): " + leitura_gas + "<br>");           //Anexa no HTML a leitura de amonia
         server.sendContent ("Modo de operação: " + operacao + "<br>");                                   //Anexa no HTML o modo de operação 
         server.sendContent ("Buzzer (buzina): " + modo_buzina + "<br>");                                 //Anexa no HTML o modo de operação         
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b3\" value=\"Atualizar\" onclick=\"location.href='/'\"></p>");                //Anexa botão Atualizar
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b4\" value=\"Ver DADOS\" onclick=\"location.href='/dados'\"></p>");           //Anexa botão Ver DADOS  
         server.sendContent ("<h3>Atuadores</h3>");  
         server.sendContent ("<input class='button' type=\"button\" name=\"b9\" value=\"Alternar atuador #1\" onclick=\"location.href='/atuador1'\">&emsp;");             //Anexa botão para alternar atuador 1 
         server.sendContent ("Estado: ");
         if (atu1 == true) { server.sendContent ("Ligado<br>"); } else { server.sendContent ("Desligado<br>"); }
         server.sendContent ("<input class='button' type=\"button\" name=\"b10\" value=\"Alternar atuador #2\" onclick=\"location.href='/atuador2'\">&emsp;");            //Anexa botão para alternar atuador 2 
         server.sendContent ("Estado: ");
         if (atu2 == true) { server.sendContent ("Ligado<br>"); } else { server.sendContent ("Desligado<br>"); }
         server.sendContent ("<input class='button' type=\"button\" name=\"b11\" value=\"Alternar atuador #3\" onclick=\"location.href='/atuador3'\">&emsp;");            //Anexa botão para alternar atuador 3 
         server.sendContent ("Estado: ");
         if (atu3 == true) { server.sendContent ("Ligado<br>"); } else { server.sendContent ("Desligado<br>"); }
         server.sendContent ("<input class='button' type=\"button\" name=\"b12\" value=\"Alternar atuador #4\" onclick=\"location.href='/atuador4'\">&emsp;");            //Anexa botão para alternar atuador 4 
         server.sendContent ("Estado: ");
         if (atu4 == true) { server.sendContent ("Ligado<br>"); } else { server.sendContent ("Desligado<br>"); }
         server.sendContent ("<h3>Avan&ccedil;ado</h3>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b13\" value=\"Alternar Modo\" onclick=\"location.href='/modo'\"></p>");					  //Anexa botão que permite mudar o modo para manual
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b14\" value=\"Ligar/Desligar Buzina\" onclick=\"location.href='/buzina'\"></p>"); 		  //Anexa botão que permite desligar o buzzer
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b5\" value=\"Deletar DADOS\" onclick=\"location.href='/delete'\"></p>"); 					  //Anexa botão que permite deletar dados
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b6\" value=\"Formatar SPIFFS\" onclick=\"location.href='/format'\"></p>"); 				  //Anexa botão para formatar SPIFFS
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b7\" value=\"Enviar por Serial\" onclick=\"location.href='/enviadados'\"></p>");            //Anexa botão Enviar por Serial 
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b9\" value=\"Reiniciar\" onclick=\"location.href='/reinicia'\"></p>");					  //Anexa botão reiniciar microcontrolador 
         server.sendContent (htmlfim);                      
}

//Função para exibir a página HTML de dados
void pagina_dados () {		//Cria a função
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Dados</h2>");         //Envia o cabeçalho para a página
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página
         server.sendContent ("<a download=\"dados.html\" href=\"data:text/html;charset=UTF-8," + htmlinicio);                            				//Início da tag de download
            File rFile = SPIFFS.open("/dados.txt","r");         //Faz a leitura do arquivo para o link de download
            while(rFile.available()) {
              String linha = rFile.readStringUntil('\n');
              server.sendContent (linha + "</br>");				//Insere os dados dentro do código HTML para gerar o arquivo dados.txt
            }
            rFile.close();
         server.sendContent (htmlfim + "\">Download dos DADOS</a><br>");     //Final da tag de donwload
         le_dados_html();
         server.sendContent ("</p>");    //Finaliza parágrafo
         server.sendContent (htmlfim);   //Encerra o html
} 


//Função para exibir HTML que formata SPIFFS
void pagina_format () {		//Cria a função
formata();                                                //Formata o SPIFFS
cria_arquivo(); 										  //Cria o arquivo de dados
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h3>SPIFFS formatado! Arquivo recriado!</h3>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html
}

//Função para exibir HTML de deletar o arquivo de dados
void pagina_delete () {      //Cria a função       
deleta_dados();                                           //Deleta "/dados.txt" do SPIFFS
cria_arquivo();                                           //Cria arquivo "/dados.txt" caso ele não exista
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h3>Arquivo Deletado!</h3>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html
}

//Função para exibir HTML com solicitação de envio pela serial
void pagina_enviadados () {  
enviaSerial();                                                 
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Dados enviados!</h2>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html
}

//Função para exibir HTML com alternância do atuador 1
void pagina_atuador1 () {         
Atuador(1);
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Atuador 1 alternado!</h2>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html                                          
}

//Função para exibir HTML com alternância do atuador 2
void pagina_atuador2 () { 
Atuador(2);
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Atuador 2 alternado!</h2>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html                                          
}

//Função para exibir HTML com alternância do atuador 3
void pagina_atuador3 () { 
Atuador(3);
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Atuador 3 alternado!</h2>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html                                          
}

//Função para exibir HTML com alternância do atuador 4
void pagina_atuador4 () { 
Atuador(4);
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Atuador 4 alternado!</h2>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html                                          
}

//Função para alternar modo entre manual e automático
void pagina_alternar_modo () { 
         automatico = !automatico;      
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Modo alternado!</h2>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html                                  
}

//Função para alternar funcionamento buzzer
void pagina_alternar_buzzer () { 
         buzina = !buzina;      
         server.sendContent (htmlinicio); 
         server.sendContent ("<style>body { color: #000000; font-family: Verdana, Helvetica, sans-serif; font-size: 17px;}");  //Linhas para adição de código CSS à página 
         server.sendContent ("h2 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 25px; }");
         server.sendContent ("h3 { color: #008CBA; text-shadow: 2px 2px 4px #bbbbbb; font-size: 20px; }");
         server.sendContent (".button { font-family: Verdana, Helvetica, sans-serif; font-size: 15px; transition-duration: 0.2s; background-color: white; color: black; border: 2px solid #008CBA; padding: 8px 8px;"); 
         server.sendContent ("text-align: center; text-decoration: none; display: inline-block; margin: 2px 1px; cursor: pointer; border-radius: 12px;"); 
         server.sendContent ("box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19); }");
         server.sendContent (".button:hover { background-color: #008CBA; color: white; }</style>");
         server.sendContent ("<h2>Buzina alternada!</h2>");
         server.sendContent ("<p><input class='button' type=\"button\" name=\"b2\" value=\"Voltar\" onclick=\"location.href='/'\"></p>");               //Insere o botão voltar na página  
         server.sendContent (htmlfim);   //Encerra o html                                  
}

//Função para que reinicia o microcontrolador
void pagina_reinicia () { 
reinicia();                                          
}

//Função OTA - Over the Air
void OTA() {		//Cria a função

  //Define porta padrão de utilização
  ArduinoOTA.setPort(8266);

  //Define o nome padrão do microcontrolador para o IDE
  ArduinoOTA.setHostname("MINI");

  //Inicia o serviço de OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) { //Se gravará sistema de arquivos ou programação
      type = "sketch";
    } else { 
      type = "filesystem";
    }

	//Envia pela serial o início da atualização
    Serial.println("Começando atualização via OTA " + type);
  });
  //Executa quando termina a atualização
  ArduinoOTA.onEnd([]() {
    Serial.println("\nFim");		//Informa via serial o fim da atualização
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {		//Executa durante atualização
    Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));			//Informa via serial durante a atualização
  });
  ArduinoOTA.onError([](ota_error_t error) {	//Caso haja erro na atualização
    Serial.printf("Erro[%u]: ", error);			//Informa o número do erro
    if (error == OTA_AUTH_ERROR) {				//Caso seja erro de autenticação
      Serial.println("Auth Falhou");
    } else if (error == OTA_BEGIN_ERROR) {		//Caso erro no início
      Serial.println("Begin Falhou");
    } else if (error == OTA_CONNECT_ERROR) {	//Caso erro na conexão
      Serial.println("Connect Falhou");
    } else if (error == OTA_RECEIVE_ERROR) {	//Caso erro de recepção
      Serial.println("Receive Falhou");
    } else if (error == OTA_END_ERROR) {		//Caso erro no fim
      Serial.println("End Falhou");
    }
  });

}

//Função para realizar a conexão com a rede de Wi-Fi local
void conecta_WIFI () {
    if (WiFi.status() != WL_CONNECTED) {                //Se não estiver conectado a rede Wi-Fi executa
        int vezes = 0;                                  //Variável para permitir sair do loop while após algum tempo
        WiFi.begin(WLAN_SSID, WLAN_PASS);               //Tenta realizar a conexão  
        Serial.print("Tentando conexão Wi-Fi ...");     //Informa via serial a tentativa de conexão
        while (WiFi.status() != WL_CONNECTED) {         //Executa enquanto aguarda a conexão
          delay(10);                                    //Faz um delay de 10 milisegundos
          vezes++;                                      //Incrementa a variável de controle
          if (vezes >= 1500) {                          //Executa se o tempo ultrapassar 15 segundos na tentativa de conexão
            Serial.print("Sem sucesso.");               //Informa via serial que não houve sucesso na tentativa
            break;                                      //Fecha o loop while
          }                     
        }
    }
        Serial.println();                               //Adiciona linha em branco na saída serial
        if (WiFi.status() == WL_CONNECTED) {            //Executa caso consiga efetuar a conexão Wi-Fi com êxito
          Serial.println();                             //Pula uma linha na saída serial
          Serial.print("Endereço IP da rede ");         //Adiciona texto na saída serial
          Serial.print(WLAN_SSID);                      //Informa o nome da rede a qual conseguiu se conectar via serial
          Serial.print(" : ");                          //Adiciona texto na saída serial
          Serial.println(WiFi.localIP());               //Adiciona o endereço IP local na saída serial
        }  
}


//Laço de configuração
void setup() {

  //Configurando porta serial
  Serial.begin(115200);  	      //Inicia serial na velocidade 115200 bps
  delay(500);				      //Executa uma pausa de 500 ms no controlador

  //Abrindo sistema de arquivos e verificando ou criando arquivo "dados.txt"
  abre_FS();
  cria_arquivo();
  
  //Configurando pinos de E/S
  pinMode(DHTPORT, INPUT_PULLUP);			//Pino do DHT22 como entrada digital e resistor interno
  pinMode(MQ135PORT, INPUT);		//Pino do MQ135 como entrada digital
  pinMode(ATUADOR_1, OUTPUT);		//Pino para atuador 1 como saída digital
  pinMode(ATUADOR_2, OUTPUT);		//Pino para atuador 2 como saída digital
  pinMode(ATUADOR_3, OUTPUT);		//Pino para atuador 3 como saída digital
  pinMode(ATUADOR_4, OUTPUT);		//Pino para atuador 4 como saída digital
  pinMode(LDRPORT, INPUT);			//Pino do LDR como entrada analógica
  pinMode(BUZZER, OUTPUT);			//Pino do Alarme como saída digital
  digitalWrite(BUZZER, HIGH);		//Desliga sinal do buzzer se estiver ativo
  Buzzer(3,50);						      //Emite som para avisar que chegou neste ponto
    
  //Configurando RTC
  RTC.begin();						      //Inicializa o RTC
  RTC.setHourMode(CLOCK_H24);			  //Configura o RTC para modo de horas 24
  
  //Configurando Wi-Fi 
  WiFi.mode(WIFI_AP_STA);			  //Configurando o WI-FI para modo Estação e Ponto de Acesso Simultâneos
  //Setando ips quando necessário
  //IPAddress local_IP(10,0,0,99);
  //IPAddress gateway(10,0,0,1);
  //IPAddress subnet(255,255,255,0);
  //WiFi.softAPConfig(local_IP, gateway, subnet);
  conecta_WIFI();
  
  WiFi.softAP(AP_SSID, AP_PASS);								  //Inicializa o Wi-Fi no modo de ponto de acesso
  Serial.println();
  Serial.print("Endereço IP da rede ");
  Serial.print(AP_SSID);
  Serial.print(" : ");
  Serial.print(WiFi.softAPIP());
  Serial.println();
  Serial.print("Senha da rede ");
  Serial.print(AP_SSID);										  //Informa a senha de rede para Ponto de Acesso
  Serial.print(" : ");
  Serial.print(AP_PASS);
  uint8_t macAddr[6];											  //Cria variável com MAC Address do microcontrolador
  WiFi.softAPmacAddress(macAddr);								  //Captura endereço de MAC
  Serial.println();
  //Envia via serial o endereço de MAC 
  Serial.printf("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  
  //Inicializando OTA
  OTA ();						      //Inicializa rotinas para envio via OTA
  ArduinoOTA.begin();				  //Inicializa o serviço de envio via OTA

  //Inicializa servidor WEB
  server.on("/", pagina_http);						      //Encaminha para página principal se requisição for igual a "/"
  server.on("/dados", pagina_dados);				      //Encaminha para página de dados se requisição for igual a "/dados"
  server.on("/format", pagina_format);			     	  //Encaminha para página de formatação se requisição for igual a "/format"
  server.on("/delete", pagina_delete);					  //Encaminha para página de deleção se requisição for igual a "/delete"
  server.on("/enviadados", pagina_enviadados);			  //Encaminha para página envio de dados via serial se requisição for igual a "/enviadados"
  server.on("/atuador1", pagina_atuador1);		    	  //Encaminha para página Atuador 1 se requisição for igual a "/atuador1"
  server.on("/atuador2", pagina_atuador2);			      //Encaminha para página Atuador 2 se requisição for igual a "/atuador2"
  server.on("/atuador3", pagina_atuador3);			      //Encaminha para página Atuador 3 se requisição for igual a "/atuador3"
  server.on("/atuador4", pagina_atuador4);			      //Encaminha para página Atuador 4 se requisição for igual a "/atuador4"
  server.on("/modo", pagina_alternar_modo);               //Encaminha para página Alternar Modo se requisição for igual a "/modo"
  server.on("/buzina", pagina_alternar_buzzer);           //Encaminha para página Alternar Buzina se requisição for igual a "/buzina"
  server.on("/reinicia", pagina_reinicia);			      //Reinicia o microcontrolador se requisição for igual a "/reinicia"
  server.begin();									      //Inicializa o servidor HTTP

  //Desligando atuadores quando o dispositivo é ligado, assegurando desligá-las mesmo que haja memória de porta
  digitalWrite(ATUADOR_1,LOW);			//Desliga atuador 1
  digitalWrite(ATUADOR_2,LOW);			//Desliga atuador 2
  digitalWrite(ATUADOR_3,LOW);			//Desliga atuador 3
  digitalWrite(ATUADOR_4,LOW);			//Desliga atuador 4
  
  Temp_Umid();								          //Faz leitura e atualiza variável de temperatura e umidade
  Luz();									          //Faz leitura e atualiza variável de luminosidade
  Gas();									          //Faz leitura e atualiza variável de presença de gás
  leitura_gas_anterior = leitura_gas;		//Atualiza variável de leitura de gás
}

//Laço de repetição
void loop() {
  
  //Escutando para respostas no protocolo HTTP
  server.handleClient();

  //Escutando para OTA
  ArduinoOTA.handle();
  
  //Aciona rotina de atualização dos controles de tempo definindo intervalo de leitura para 180 segundos
  tempo(180);

  //Aciona rotina para atualização do estado dos atuadores
  verif_atuadores();
  
  //Se estiver em modo teste, executa apenas a rotina de teste
  if (modo_teste == true) { Modo_Teste(); }
  //Se estiver em modo de calibração, executa apenas a rotina de calibração
  else if (calibracao == true) { Calib_Gas(); }
  //Rotinas de trabalho padrão do sistema
  else {
    if (tempo_passado_s == 0) {			  //Executa se o tempo passado for 0 no intervalo da função tempo
    conecta_WIFI();                       //Realiza uma tentativa de conexão Wi-Fi local caso ainda não esteja conectado
    tempo_passado_s++;					  //Incrementa variável de controle do tempo passado
    Temp_Umid();						  //Faz leitura e atualiza variável de temperatura e umidade
    Luz();								  //Faz leitura e atualiza variável de luminosidade
    Gas();								  //Faz leitura e atualiza variável de presença de gás
    Buzzer(1,15);						  //Emite pequeno sinal sonoro para saber que as leituras estão sendo realizadas normalmente
    gravaLEITURAS();					  //Grava as leituras realizadas na memória
    
      if (automatico == true) {           //Executa apenas se o modo automático estiver ligado
      trata_amonia(1);					  //Verifica a presença de gás e realiza o tratamento
      trata_luz(2, 25, 18, 20, 20, 6);	  //Verifica o nível de luz e realiza o tratamento
      trata_temp_fria(3, 24);			  //Verifica a temperatura e realiza o tratamento
      trata_temp(4, 26);  			      //Verifica a temperatura e realiza o tratamento
      }

    if (WiFi.status() == WL_CONNECTED) {  
    MQTT_connect();                   //Conecta ao serviço de MQTT
    Transmite_MQTT();                 //Executa a rotina de envio e recepção de informações do MQTT
    }
    
    server.close();                   //Fecha o servidor HTTP
    server.begin();					  //Reinicia o servidor HTTP	
    }
    tempo_atuador(3, 60);	//Limita o tempo de acionamento da resistência para 1 minuto, para não superaquecer
    tempo_atuador(4, 45);	//Limita o tempo de acionamento do ventilador para 45 segundos, para não remover todo o ar quente
  }
 
  Serial.setTimeout(50);							//Define tempo de escuta da porta serial para entrada
  if (Serial.available() > 0) {						//Se houver algo na entrada da porta serial
    trata_serial(Serial.readStringUntil('\n'));		//Excuta rotina de tratamento da porta serial com a informação de entrada
  }    

  yield();

}	//Final do loop

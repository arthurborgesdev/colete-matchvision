// COLETE DISPLAY RYD (MATCH VISION) GOIÂNIA-GO  V2
// ---------------------------------------------------
// Início: 24/08/2016 
//----------------------------------------------------
// 7 segmentos de X (10 inicialmente) LEDs, cada. 
// QTd de LEDs da Fita inteira = 7*X. (70)
//----------------------------------------------------
// Intervalo total: 0 ~ (7*X) - 1
// Segmento A:  (0*X) ~ (1*X) - 1
// Segmento B:  (1*X) ~ (2*X) - 1
// Segmento C:  (2*X) ~ (3*X) - 1
// Segmento D:  (3*X) ~ (4*X) - 1
// Segmento E:  (4*X) ~ (5*X) - 1
// Segmento F:  (5*X) ~ (6*X) - 1
// Segmento G:  (6*X) ~ (7*X) - 1
//----------------------------------------------------
//                 C  
//        _ _ _ _ _ _ _ _ _ _    
//     29|      <---         |19
//       |                   |
//   |   |                   |  ^
// D |   |                   |  |
//   |   |                   |  | B
//   V   |                   |  |
//       |                   |   
//       |         A         |
//       |CONTR.  --->       |
//     39|_ _ _ _ _ _ _ _ _ _|                    
// (5V)40|0(5V)             9|69
//       |                   |  ^
//   |   |                   |  | 
//  E|   |                   |  | G
//   |   |                   |  |
//   V   |                   |  |
//       |                   |
//       |         F         |
//       |        --->       |
//     49|_ _ _ _ _ _ _ _ _ _|59
//                            
//
//  CONTR. = Shield (ESP8266 + AMS1117 + MT3608 + Capacitor 1000uF + Resistor 470 Ohm) + 2x Pilhas AA (2400mAH 1,2V - 5.76 Wh) 
//
//  Consumo de corrente por Barramento (5V):
//
//  Barramento superior: (A + B + C + D):
//  4 * X LEDS ==> 4*X*20mA = 80*X mA
//
//  Barramento inferior: (E + F + G):
//  3 * X LEDS ==> 3*X*20mA = 60*X mA
//
//  Total: (A + B + C + D + E + F +G):
//  7 * X LEDS ==> 7*X*20mA = 140*X mA
//-----------------------------------------------------------------


/* RGB web server with ESP8266-01
* only 2 GPIOs available: 0 and 2
* but RX and TX can also be used as: 3 and 1
* we use 0=red 2=green 3=blue
* analogWrite with values received from web page
*
* web server with captive portal works but better use fixed domain: http://rgb
* web page returns POST request with 3 RGB parameters
* web page inspired by https://github.com/dimsumlabs/nodemcu-httpd
* Serial Monitor for debugging but interferes with Blue channel GPIO3=RX
* switch off Serial for full RGB
*/

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

//************************************************************* variaveis de eeprom
// Locais dos itens da configuração
#define VERSION_START  500
#define CONFIG_START   6

// ID de Configuração 
#define CONFIG_VERSION "1a"

// Estrutura de configuração da EEPROM
struct ledStripStatus
{
  int r;
  int g;
  int b;
  int veloc;
  int brilho;
} ledStatus;
//*************************************************************
const char *ssid = "CEI";
 const char *password = "#CEI-ufg16%";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

#define PIN 2 // mudar pino de acordo com o ESP8266
#define SEGMENTOS 7

int leds_segmento = 7; // alterar esse parâmetro
int sel_velocidade = 1; // 3 seleções são possíveis: 1 => 900 ms; 2 => 800 ms; 3 => 700 ms; 
//int velocidade = 700; // número em milisegundos para alternar os números do visor

//------------TIMERS for Delay ----------------------
unsigned long previousMillis = 0;
const long velocidade = 700 // número em milisegundos para alternar os números do visor - TIMER
//---------------------------------------------------

int qtdLEDs = SEGMENTOS*leds_segmento;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(qtdLEDs, PIN, NEO_GRB + NEO_KHZ800);

uint8_t tom_vermelho = 800;
uint8_t tom_verde = 200;
uint8_t tom_azul = 0;
uint32_t cor = strip.Color(tom_vermelho, tom_verde, tom_azul);
uint32_t red = strip.Color(1023, 0, 0);////////////1023
uint32_t green = strip.Color(0, 1023, 0);///////////1023
uint32_t blue = strip.Color(0, 0, 1023);///////////1023

int brilho = 128;        // Valor de 0 a 1023 que controla o brilho da fita inteira

int rand_number = 0;

String webpage = ""
                 "<!DOCTYPE html><html><head><title>RGB control</title><meta name='mobile-web-app-capable' content='yes' />"
"                  <meta name='viewport' content='width=device-width' />"
" Brilho <input type='range' id= 'inputSliderR' min='0' max='100' value='0'     step='5' oninput='showValueR(this.value)' />"
" <span id='outputTextR'>0</span>%<p></p>"
"    <input type='radio' name='veloc' onclick='velocInput(900)'/>900ms"
"   <input type='radio' name='veloc' onclick='velocInput(800)'/>800ms"
"    <input type='radio' name='veloc' onclick='velocInput(700)'/>700ms"
"    <br></br><span id='Velocidade'>900</span>ms<p></p>"
" <span id='R'>0</span> ; <span id='G'>0</span> ; <span id='B'>0</span> ;<p></p>"
"</head><body style='margin: 0px; padding: 0px;'>"
"                  <canvas id='colorspace'></canvas> "
"</body>"
"                  <script type='text/javascript'>"
"var R=800;"
"var G=200;"
"var B=0;"
"var Velocidade=900;"
"var brilho=128;"
"(function () {"
"            var canvas = document.getElementById('colorspace');"
"            var ctx = canvas.getContext('2d');"
"            function drawCanvas() {"
"     var colours = ctx.createLinearGradient(0, 0, window.innerWidth, 0);"
"     for(var i=0; i <= 360; i+=10) {"
"       colours.addColorStop(i/360, 'hsl(' + i + ', 100%, 50%)');"
"     }"
"     ctx.fillStyle = colours;"
"     ctx.fillRect(0, 0, window.innerWidth, window.innerHeight);"
"     var luminance = ctx.createLinearGradient(0, 0, 0, ctx.canvas.height);"
"     luminance.addColorStop(0, '#ffffff');"
"     luminance.addColorStop(0.05, '#ffffff');"
"     luminance.addColorStop(0.5, 'rgba(0,0,0,0)');"
"     luminance.addColorStop(0.95, '#000000');"
"     luminance.addColorStop(1, '#000000');"
"     ctx.fillStyle = luminance;"
"     ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);"
"               }"
"            var eventLocked = false;"
"     function handleEvent(clientX, clientY) {"
"       if(eventLocked) {"
"         return;"
"       }"
"       function colourCorrect(v) {"
"         return Math.round(1023-(v*v)/64);"
"         }"
"       var data = ctx.getImageData(clientX, clientY, 1, 1).data;"
"        R = colourCorrect(data[0]);"
"        G = colourCorrect(data[1]);"
"        B = colourCorrect(data[2]);"
"        var params = ["
"        'r=' + colourCorrect(data[0]),"
"        'g=' + colourCorrect(data[1]),"
"        'b=' + colourCorrect(data[2]),"
"        'v=' + Velocidade,"
"        'br='+ brilho"
"        ].join('&');"
"        showRGB(colourCorrect(data[0]),colourCorrect(data[1]),colourCorrect(data[2]));"
"        var req = new XMLHttpRequest();"
"        req.open('POST', '?' + params, true);"
"        req.send();"
"       eventLocked = true;"
"       req.onreadystatechange = function() {"
"         if(req.readyState == 4) {"
"           eventLocked = false;"
"         }"
"       }"
"           }"
"                   canvas.addEventListener('click', function(event) {"
"         handleEvent(event.clientX, event.clientY, true);"
"         }, false);"
"                   canvas.addEventListener('touchmove', function(event){"
"         handleEvent(event.touches[0].clientX, event.touches[0].clientY);"
"         }, false);"
"                  function resizeCanvas() {"
"           canvas.width = window.innerWidth;"
"           canvas.height = window.innerHeight;"
"           drawCanvas();"
"         }"
"                   window.addEventListener('resize', resizeCanvas, false);"
"                   resizeCanvas();"
"                   drawCanvas();"
"                   document.ontouchmove = function(e) {e.preventDefault()};"
"})();"
"function showValueR(newValue)"
"               {"           
"           document.getElementById('outputTextR').innerHTML=newValue;"
"           brilho = newValue;"
"           var params = ["
"           'r=' + R,"
"           'g=' + G,"
"           'b=' + B,"
"           'v=' + Velocidade,"
"           'br='+ newValue"
"           ].join('&');"
"           var req = new XMLHttpRequest();"
"           req.open('POST', '?' + params, true);"
"           req.send();"
"                }"
"function velocInput(newValue)"
"                {"         
"           document.getElementById('Velocidade').innerHTML=newValue;"
"           Velocidade = newValue;"
"           var params = ["
"           'r=' + R,"
"           'g=' + G,"
"           'b=' + B,"
"           'v=' + newValue,"
"           'br='+ brilho"
"           ].join('&');"
"           var req = new XMLHttpRequest();"
"           req.open('POST', '?' + params, true);"
"           req.send();"
"                }" 
"function showRGB(vR,vG,vB)"
"       {"
"           document.getElementById('R').innerHTML=vR;"
"           document.getElementById('G').innerHTML=vG;"
"           document.getElementById('B').innerHTML=vB;"
"       }"
"                  </script></html>";

/*String webpage = ""
                  "<!DOCTYPE html><html><head><title>RGB control</title><meta name='mobile-web-app-capable' content='yes' />"
                  "<meta name='viewport' content='width=device-width' /></head><body style='margin: 0px; padding: 0px;'>"
                  "<canvas id='colorspace'></canvas></body>"
                  "<script type='text/javascript'>"
                  "(function () {"
                  " var canvas = document.getElementById('colorspace');"
                  " var ctx = canvas.getContext('2d');"
                  " function drawCanvas() {"
                  " var colours = ctx.createLinearGradient(0, 0, window.innerWidth, 0);"
                  " for(var i=0; i <= 360; i+=10) {"
                  " colours.addColorStop(i/360, 'hsl(' + i + ', 100%, 50%)');"
                  " }"
                  " ctx.fillStyle = colours;"
                  " ctx.fillRect(0, 0, window.innerWidth, window.innerHeight);"
                  " var luminance = ctx.createLinearGradient(0, 0, 0, ctx.canvas.height);"
                  " luminance.addColorStop(0, '#ffffff');"
                  " luminance.addColorStop(0.05, '#ffffff');"
                  " luminance.addColorStop(0.5, 'rgba(0,0,0,0)');"
                  " luminance.addColorStop(0.95, '#000000');"
                  " luminance.addColorStop(1, '#000000');"
                  " ctx.fillStyle = luminance;"
                  " ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);"
                  " }"
                  " var eventLocked = false;"
                  " function handleEvent(clientX, clientY) {"
                  " if(eventLocked) {"
                  " return;"
                  " }"
                  " function colourCorrect(v) {"
                  " return Math.round((v*v)/64);"
                  " }"
                  " var data = ctx.getImageData(clientX, clientY, 1, 1).data;"
                  "var params = ["
                  "'r=' + colourCorrect(data[0]),"
                  "'g=' + colourCorrect(data[1]),"
                  "'b=' + colourCorrect(data[2])"
                  "].join('&');"
                  " var req = new XMLHttpRequest();"
                  " req.open('POST', '?' + params, true);"
                  " req.send();"
                  " eventLocked = true;"
                  " req.onreadystatechange = function() {"
                  " if(req.readyState == 4) {"
                  " eventLocked = false;"
                  " }"
                  " }"
                  " }"
                  " canvas.addEventListener('click', function(event) {"
                  " handleEvent(event.clientX, event.clientY, true);"
                  " }, false);"
                  " canvas.addEventListener('touchmove', function(event){"
                  " handleEvent(event.touches[0].clientX, event.touches[0].clientY);"
                  "}, false);"
                  " function resizeCanvas() {"
                  " canvas.width = window.innerWidth;"
                  " canvas.height = window.innerHeight;"
                  " drawCanvas();"
                  " }"
                  " window.addEventListener('resize', resizeCanvas, false);"
                  " resizeCanvas();"
                  " drawCanvas();"
                  " document.ontouchmove = function(e) {e.preventDefault()};"
                  " })();"
                  "</script></html>"; */

//////////////////////////////////////////////////////////////////////////////////////////////////

//--------------EEPROM----------------------------------------
void saveStatus(int r, int g, int b, int veloc, int brilho){

      ledStatus.r = r;
      ledStatus.g = g;
      ledStatus.b = b;
      ledStatus.veloc = veloc;
      ledStatus.brilho = brilho;
      
      for (unsigned int t = 0; t < sizeof(ledStatus); t++) {
      EEPROM.write(CONFIG_START + t, *((char*)&ledStatus + t));
    }
    EEPROM.commit();
  }

void loadStatus(){
      // Carregando a estrutura status do LED
      for (unsigned int t = 0; t < sizeof(ledStatus); t++)
        *((char*)&ledStatus + t) = EEPROM.read(CONFIG_START + t);
}
//-----------EEPROM------------------------------------------


void handleRoot() {
  //Atualiza tempo atual.  SALVA O STATUS APOS 5 MINUTOS SEM ATUALIZAÇÃO/////////////CRIAR UM FLAG DE ATUALIZADO/////////////////(Criar variavel de tempo)
  
// Serial.println("handle root..");
  String red = webServer.arg(0); // read RGB arguments
  String green = webServer.arg(1);
  String blue = webServer.arg(2);
  String veloc = webServer.arg(3);
  String brilh = webServer.arg(4);
  
  //tom_vermelho = map(red.toInt(),0,255,0,1023);////////ENVIAR PARA SERVIDOR ULTIMO ESTADO SALVO
  //tom_verde = map(green.toInt(),0,255,0,1023);/////////SALVAR ULTIMOS ESTADOS DA FITA DE LED
  //tom_azul = map(blue.toInt(),0,255,0,1023);///////////FAZER INTERRUPÇÃO QUANDO RECEBE DADOS DO SERVIDOR
  
  tom_vermelho = red.toInt();
  tom_verde = green.toInt();
  tom_azul = blue.toInt();
  cor = strip.Color(tom_vermelho, tom_verde, tom_azul);
   //Serial.println(comand); 
   Serial.print("RAW: ");  
   Serial.print(tom_vermelho); 
   Serial.print(" "); 
   Serial.print(tom_verde); 
   Serial.print(" "); 
   Serial.println(tom_azul); 
   Serial.print("R: "); 
   Serial.print(1023-tom_vermelho); 
   Serial.print(" G: ");
   Serial.print(1023-tom_verde); 
   Serial.print(" B: ");  
   Serial.print(1023-tom_azul); 
   Serial.print(" Veloc: ");
   Serial.print(veloc); 
   Serial.print(" brilho: ");  
   Serial.println(brilh); 
   
  webServer.send(200, "text/html", webpage);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  /*WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);*/
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
// if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);
  webServer.on("/", handleRoot);
  webServer.begin();

  strip.begin();
  strip.setBrightness(brilho);  
  
  testRGB();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  //timer
  unsigned long currentMillis = millis();
  dnsServer.processNextRequest();
  webServer.handleClient();

  limpa();
  rand_number = random(10);
  strip.setBrightness(brilho);////////////////ALTERAR DEPOIS PARA WEB
  switch(rand_number){
    case 0: ZERO(); break;
    case 1: UM(); break;
    case 2: DOIS(); break;
    case 3: TRES(); break;
    case 4: QUATRO(); break;
    case 5: CINCO(); break;
    case 6: SEIS(); break;
    case 7: SETE(); break;
    case 8: OITO(); break;
    case 9: NOVE(); break;
  }
  //def_velocidade();
  //strip.show();
  //delay(velocidade);
  // "delay" with timers
  if(currentMillis - previousMillis >= velocidade) {
    previousMillis = currentMillis;
    strip.show();
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void testRGB() { // fade in and out of Red, Green, Blue
  /*
  analogWrite(0, 1023); // R off
  analogWrite(2, 1023); // G off
  analogWrite(3, 1023); // B off

  fade(0); // R
  fade(2); // G
  fade(3); // B
  */
  acende(red);
  delay(1000);
  limpa();
  acende(green);
  delay(1000);
  limpa();
  acende(blue);
  delay(1000);
  limpa();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void fade(int pin) {

  for (int u = 0; u < 1024; u++) {
    analogWrite(pin, 1023 – u);
    delay(1);
  }
  for (int u = 0; u < 1024; u++) {
    analogWrite(pin, u);
    delay(1);
  }
}
*/

void limpa(){
  for (int i = 0*leds_segmento; i < 7*leds_segmento; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
  }

void acende(uint32_t cor){
  for (int i = 0*leds_segmento; i < 7*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
  strip.show();
  }

//DECLARAÇÃO SEGMENTOS-----------------------------------------------------------------

void SEGMENTO_A() {
  for (int i = 0*leds_segmento; i < 1*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
}

void SEGMENTO_B() {
  for (int i = 1*leds_segmento; i < 2*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
}

void SEGMENTO_C() {
  for (int i = 2*leds_segmento; i < 3*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
}

void SEGMENTO_D() {
  for (int i = 3*leds_segmento; i < 4*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
}

void SEGMENTO_E() {
  for (int i = 4*leds_segmento; i < 5*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
}

void SEGMENTO_F() {
  for (int i = 5*leds_segmento; i < 6*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
}

void SEGMENTO_G() {
  for (int i = 6*leds_segmento; i < 7*leds_segmento; i++) {
    strip.setPixelColor(i, cor);
  }
}
// FIM DECLARAÇÃO SEGMENTOS-------------------------------------------------------------

// DECLARAÇÃO DOS NÚMEROS (0 ~ 9) ------------------------------------------------------

void ZERO() {
  SEGMENTO_B();
  SEGMENTO_C();
  SEGMENTO_D();
  SEGMENTO_E();
  SEGMENTO_F();
  SEGMENTO_G();
  //strip.show();
}

void UM() {
  SEGMENTO_B();
  SEGMENTO_G();
  //strip.show();
}

void DOIS() {
  SEGMENTO_A();
  SEGMENTO_B();
  SEGMENTO_C();
  SEGMENTO_E();
  SEGMENTO_F();
  //strip.show();
}

void TRES() {
  SEGMENTO_A();
  SEGMENTO_B();
  SEGMENTO_C();
  SEGMENTO_F();
  SEGMENTO_G();
  //strip.show();
}

void QUATRO() {
  SEGMENTO_A();
  SEGMENTO_B();
  SEGMENTO_D();
  SEGMENTO_G();
  //strip.show();
}

void CINCO() {
  SEGMENTO_A();
  SEGMENTO_C();
  SEGMENTO_D();
  SEGMENTO_F();
  SEGMENTO_G();
  //strip.show();
}

void SEIS() {
  SEGMENTO_A();
  SEGMENTO_C();
  SEGMENTO_D();
  SEGMENTO_E();
  SEGMENTO_F();
  SEGMENTO_G();
  //strip.show();
}

void SETE() {
  SEGMENTO_B();
  SEGMENTO_C();
  SEGMENTO_G();
  //strip.show();
}

void OITO() {
  SEGMENTO_A();
  SEGMENTO_B();
  SEGMENTO_C();
  SEGMENTO_D();
  SEGMENTO_E();
  SEGMENTO_F();
  SEGMENTO_G();
  //strip.show();
}

void NOVE() {
  SEGMENTO_A();
  SEGMENTO_B();
  SEGMENTO_C();
  SEGMENTO_D();
  SEGMENTO_F();
  SEGMENTO_G();
  //strip.show();
}

// FIM DECLARAÇÃO DOS NÚMEROS (0 ~ 9) ------------------------------------------------------

void getData(String message, char separator){
  
  int strIndex[] = {0, -1};
  int maxIndex = message.length()-1;
  int index = 0;
  //variaveis: tom_vermelho; tom_verde; tom_azul; brilho; velocidade;
  //int aX = angleX, aZ=angleZ, rl, fb, light, posZ, camDisp;
  for(int i=0; i<=maxIndex; i++){
    if(message.charAt(i)==separator){
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = i;
      switch(index){
        case 0:
          tom_vermelho = message.substring(strIndex[0], strIndex[1]).toInt();
          //checkSum+=aX*11;
          break;
        case 1:
          tom_verde = message.substring(strIndex[0], strIndex[1]).toInt();
          //checkSum+=aZ*7;
          break;
        case 2:
          tom_azul = message.substring(strIndex[0], strIndex[1]).toInt();
          //checkSum+=rl*3;
          break;
        case 3:
          brilho = message.substring(strIndex[0], strIndex[1]).toInt();
          //checkSum+=fb;
          break;
        case 4:
          velocidade = message.substring(strIndex[0], strIndex[1]).toInt();
          //checkSum+=posZ*2;
          break;
      }
      index++;      
    }
  }
}

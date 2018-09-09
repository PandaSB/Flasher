#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <Adafruit_NeoPixel.h>

std::unique_ptr<ESP8266WebServer> server;


#define DEFAULT_DELAY 75

#define HEX2DEC(H) (toupper(H) >= 'A' ? (toupper(H)-'A'+10) : (H - '0'))
#define ASC2DEC(A) (A - '0')

#define PIN 2
#define NB_LED 10


unsigned long led_time;
unsigned long led_delay = DEFAULT_DELAY ;
char szCommand[32] = {0};
char CommandReceived = false ; 
char data[3000];


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NB_LED, PIN, NEO_RGB + NEO_KHZ800);

uint32_t GlobalColor = strip.Color(255, 255, 0) ; 
uint32_t GlobalColorSave = GlobalColor ; 

uint32_t GlobalType = 0 ; 
uint32_t GlobalLoop = 0 ;

void ColorOn (uint32_t c)
{
  if ((GlobalLoop == 0 ) || (GlobalColorSave != c))
  {
     for(uint16_t i=0; i<strip.numPixels(); i++) 
     {
        strip.setPixelColor(i, c);
     }
     strip.show();     
     GlobalLoop ++ ; 
     GlobalColorSave = c ; 
  }
}

void ColorSwipe(uint32_t c)
{
  if ( GlobalLoop >=  (strip.numPixels() +1)) GlobalLoop = 0 ;  
  if ( GlobalLoop ==0 ) 
  {
    for(uint16_t i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  else 
  {
    strip.setPixelColor(GlobalLoop -1, c);
  }
  strip.show();
  GlobalLoop ++ ;
}

void ColorGyro(uint32_t c) 
{
  if ( GlobalLoop >= strip.numPixels()) GlobalLoop = 0 ;  
    for(uint16_t j=0; j<strip.numPixels(); j++) 
    {
      if (( j == GlobalLoop )) {
        strip.setPixelColor(j, c);
      } else if ( j == (( GlobalLoop +  (strip.numPixels()/2) )% strip.numPixels())) {
        strip.setPixelColor(j, c);
      }
      else  strip.setPixelColor(j, strip.Color(0, 0, 0));
    }
  strip.show();
  GlobalLoop ++ ;
}

void ColorFlash(uint32_t c) 
{
  if ( GlobalLoop >= strip.numPixels()) GlobalLoop = 0 ;
  for(uint16_t j=0; j<strip.numPixels(); j++) 
  {
     if ((GlobalLoop == 1) || (GlobalLoop == 3) || (GlobalLoop == 5))
     {
       strip.setPixelColor(j, c );
     }
     else
     {
       strip.setPixelColor(j,strip.Color(0,0,0));
     }
  }
  strip.show();
  GlobalLoop ++ ;
}


void handleRoot() {
  Serial.println (server->uri());
  for (uint8_t i = 0; i < server->args(); i++) {
    Serial.print ( server->argName(i) + ": " + server->arg(i) + "\n");
    if ( server->argName(i) ==  "cmd" ) {
      strcpy (szCommand , server->arg(i).c_str());
      CommandReceived = true ; 
    }
  }
  strcpy (data,"");
  strncat(data,\
  "<!DOCTYPE html>\
<html>\
<body>\
<center>\
Pick a Color<br>\
<canvas id=\"myColorPicket\" width=\"340\" height=\"150\" style=\"border:1px solid #d3d3d3;\">\
Your browser does not support the HTML5 canvas tag.</canvas>\
<br>\
<div><input type=\"text\" id = \"mycolor\"></input></div>\
<br>\
<button onclick=\"myAction('t00')\">Off</button>\
<button onclick=\"myAction('t01')\">On</button>\
<button onclick=\"myAction('t02')\">Swipe</button>\
<button onclick=\"myAction('t03')\">Gyro</button>\
<button onclick=\"myAction('t04')\">Flash</button>\
<br><br>\
<div>HEX  : <input type=\"text\" id = \"myhex\" ></input></div>\
<div>RGB  : <input type=\"text\" id = \"myrgb\" ></input></div>\
</center>\
<script>\
var c = document.getElementById(\"myColorPicket\");\
var color = document.getElementById(\"mycolor\");\
var hex = document.getElementById(\"myhex\");\
var rgb = document.getElementById(\"myrgb\");\
var ctx = c.getContext(\"2d\");\
var size = 40 ;\
var offset = 4 ;\
var step = size + (offset *2) ;\
var colors = [\"#000000\",\"#FFFFFF\",\"#FF0000\",\"#FFFF00\",\"#FF00FF\",\"#00FF00\",\"#00FFFF\",\
              \"#0000FF\"];\
hex.setAttribute('value','none');\
rgb.setAttribute('value','none');\
x = step ;\
y = 20 ;\
for (i=0;i<colors.length; i++){\
    ctx.fillStyle = colors[i];\
    ctx.fillRect(x+offset, y+offset, size , size);\
    ctx.rect(x+offset, y+offset, size , size);\
    ctx.stroke();\
    x = x + step ;\ 
    if ( x > (step*5) )\ 
    {\
        x = step ; \
        y = y + step;\
    } \
}\
\
function myAction(s) {\
      location.replace(\"/?cmd=\"+s);\
}\
\
function decimalToHex(d) {\
  var hex = Number(d).toString(16);\
  hex = \"000000\".substr(0, 6 - hex.length) + hex;\ 
  return hex;\
}\
c.addEventListener(\"click\", function(event){\
    var x = event.pageX - this.offsetLeft;\
    var y = event.pageY - this.offsetTop;\
    var img_data = ctx.getImageData(x, y, 1, 1).data;\
    var R = img_data[0];\
    var G = img_data[1];\
    var B = img_data[2];\
    var rgb_value = R + ',' + G + ',' + B;\
    var dColor = B + 256 * G + 65536 * R;\
    rgb.setAttribute('value',rgb_value);\
    hex.setAttribute('value','#' + decimalToHex(dColor));\
    color.style.backgroundColor = '#' + decimalToHex(dColor);\
    myAction('c' + decimalToHex(dColor));\
});\
c.addEventListener(\"mousemove\", function(event){\
    var x = event.pageX - this.offsetLeft;\
    var y = event.pageY - this.offsetTop;\
    var img_data = ctx.getImageData(x, y, 1, 1).data;\
    var R = img_data[0];\
    var G = img_data[1];\
    var B = img_data[2];\
    var rgb_value = R + ',' + G + ',' + B;\
    var dColor = B + 256 * G + 65536 * R;\
    color.style.backgroundColor = '#' + decimalToHex(dColor);\
});\
</script>\
</body>\
</html>",3000);
  server->send(200, "text/html", data);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i = 0; i < server->args(); i++) {
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", message);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
    
  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

    
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  strip.begin();  
  strip.show();

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.reset(new ESP8266WebServer(WiFi.localIP(), 80));

  server->on("/", handleRoot);
  server->onNotFound(handleNotFound);
  server->begin();
  Serial.println("HTTP server started");
    
  led_time = millis() + led_delay ;
}

void loop() {
  if (millis() > led_time ) {
    led_time = millis() + led_delay ; 
  
    switch (GlobalType) 
    {
      case 0 : ColorOn ( strip.Color(0,0,0)); break ; 
      case 1 : ColorOn ( GlobalColor); break ; 
      case 2 : ColorSwipe (GlobalColor); break ; 
      case 3 : ColorGyro (GlobalColor); break ;
      case 4 : ColorFlash(GlobalColor); break ;  
      default : break; 
    }
  }
    
  if ((Serial.available()) && (CommandReceived == false)) 
  {
    char car = Serial.read() ;
    int pos = strlen (szCommand);
    if ( pos < 31 )
    {
      szCommand[pos] = car ;
      szCommand[pos+1] = 0 ;
    }
    if ((car ==  0x0A) || ( car == 0x0D) || (car == 0x20)) CommandReceived = true ; 
  }


  if (CommandReceived  == true ) 
  {
    Serial.print   ("Command: ");
    Serial.println (szCommand);

    switch (toupper(szCommand[0])) 
    {
      case 'C' :
        {
          unsigned char R,G,B;
          R = HEX2DEC(szCommand[1]) * 0x10 + HEX2DEC(szCommand[2]); 
          G = HEX2DEC(szCommand[3]) * 0x10 + HEX2DEC(szCommand[4]);
          B = HEX2DEC(szCommand[5]) * 0x10 + HEX2DEC(szCommand[6]); 
          GlobalColor = strip.Color(R, G, B); 
          Serial.print   ("  Set color : ");
          Serial.print   (R);Serial.print(",");
          Serial.print   (G);Serial.print(",");
          Serial.println (B);
        } 
        break;
      case 'T' :
        {
          GlobalType = ASC2DEC(szCommand[1]) * 10 + ASC2DEC(szCommand[2]);
          GlobalLoop = 0 ; 
          Serial.print   ("  Set type : ");
          Serial.println (led_delay);  

        }
        break ;
      case 'S' : 
        {
          led_delay = ASC2DEC(szCommand[1]) * 1000 + ASC2DEC(szCommand[2]) * 100 + \
                      ASC2DEC(szCommand[3]) * 10 + ASC2DEC(szCommand[4]);;
           Serial.print   ("  Set speed : ");
           Serial.println (led_delay);  
        }
      default : 
        break;
    }
    szCommand[0] = 0 ; 
    CommandReceived = false ; 
  }  


  server->handleClient();

}

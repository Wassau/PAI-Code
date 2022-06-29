/*


  this example will show
  1. how to use and ESP 32 for reading pins
  2. building a web page for a client (web browser, smartphone, smartTV) to connect to
  3. sending data from the ESP to the client to update JUST changed data
  4. sending data from the web page (like a slider or button press) to the ESP to tell the ESP to do something

  If you are not familiar with HTML, CSS page styling, and javascript, be patient, these code platforms are
  not intuitive and syntax is very inconsitent between platforms

  I know of 4 ways to update a web page
  1. send the whole page--very slow updates, causes ugly page redraws and is what you see in most examples
  2. send XML data to the web page that will update just the changed data--fast updates but older method
  3. JSON strings which are similar to XML but newer method
  4. web sockets very very fast updates, but not sure all the library support is available for ESP's

  I use XML here...

  compile options
  1. esp32 dev module
  2. upload speed 921600
  3. cpu speed 240 mhz
  flash speed 80 mhz
  flash mode qio
  flash size 4mb
  partition scheme default


  NOTE if your ESP fails to program press the BOOT button during programm when the IDE is "looking for the ESP"


*/

#include <WiFi.h>       // standard library
#include <WebServer.h>  // standard library
#include "indexAJAX.h"   // .h file that stores your html page code
#include "funcionesMotores.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server
//#define USE_INTRANET

AsyncWebServer server(80);
// replace this with your homes intranet connect parameters
#define LOCAL_SSID "FamiliaArcila"
#define LOCAL_PASS "51710096"

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Heliodon"
#define AP_PASS "16051984"

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
const char* PARAM_INPUT_3 ="value"; //opcional con el id se puede

// variables to store measure data and sensor states
int BitsPOT = 0, BitsA1 = 0;
float VoltsPOT = 0, VoltsA1 = 0;
int FanSpeed = 0;
bool LED0 = false, SomeOutput = false, isOnNORTH = false, isOnSOUTH = false, isOnWEST = false, isOnEAST = false;
uint32_t SensorUpdate = 0;
int FanRPM = 0;

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// just some buffer holder for char operations
char buf[32];

// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

// gotta create a server
//WebServer server(80);

///////////////////////////////////////////
////////////////// SETUP //////////////////
///////////////////////////////////////////

void setup() {
  // standard stuff here
  Serial.begin(115200);
  
  setupMotors();

  // maybe disable watch dog timer 1 if needed
  disableCore1WDT();

  // just an update to progress
  Serial.println("starting server");

  // if you have this #define USE_INTRANET,  you will connect to your home intranet, again makes debugging easier
#ifdef USE_INTRANET
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  Actual_IP = WiFi.localIP();
#endif

  // if you don't have #define USE_INTRANET, here's where you will creat and access point
  // an intranet with no internet connection. But Clients can connect to your intranet and see
  // the web page you are about to serve up
#ifndef USE_INTRANET
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: "); Serial.println(Actual_IP);
#endif


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGE_MAIN );
  });
  
    server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    String inputMessage3;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    switch(inputMessage1.toInt()){
      case 33:
        ProcessButton_NORTH();
        break;
      case 25:
        ProcessButton_SOUTH();  
        break;
      case 26:
        ProcessButton_EAST();
        break;
      case 32:
        ProcessButton_WEST();
        break;          
    }
    
    Serial.print("ID: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  

  // these calls will handle data coming back from your web page
  // this one is a page request, upon ESP getting / string the web page will be sent

  // finally begin the server
  server.begin();

}


///////////////////////////////////////////
////////////////// LOOP ///////////////////
///////////////////////////////////////////

void loop() {


}


void ProcessButton_NORTH() {

  //


  myStepper.step(1);
  Serial.println("estas en elProcess_North");
  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or let the XML manage
  // sending feeback

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status
  // like the illumination status will be send in the main XML page update
  // code
// request->send(200, "text/plain", "OK"); //Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feed back immediataly
  // note you must have reading code in the java script
  /*
    if (LED0) {
    server.send(200, "text/plain", "1"); //Send web page
    }
    else {
    server.send(200, "text/plain", "0"); //Send web page
    }
  */

}


void ProcessButton_SOUTH() {

  //


  myStepper.step(-1);
   Serial.println("estas en elProcess_South");
  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or let the XML manage
  // sending feeback

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status
  // like the illumination status will be send in the main XML page update
  // code
  // request->send(200, "text/plain", "OK"); //Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feed back immediataly
  // note you must have reading code in the java script
  /*
    if (LED0) {
    server.send(200, "text/plain", "1"); //Send web page
    }
    else {
    server.send(200, "text/plain", "0"); //Send web page
    }
  */

}


void ProcessButton_WEST() {

  //
    for (int i = 0; i <= 10; i++) {
      pwmMovForward();
  }
  
   Serial.println("estas en elProcess_West");
  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or let the XML manage
  // sending feeback

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status
  // like the illumination status will be send in the main XML page update
  // code
 //request->send(200, "text/plain", "OK"); //Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feed back immediataly
  // note you must have reading code in the java script
  /*
    if (LED0) {
    server.send(200, "text/plain", "1"); //Send web page
    }
    else {
    server.send(200, "text/plain", "0"); //Send web page
    }
  */

}

void ProcessButton_EAST() {

  //
  for (int i = 0; i <= 10; i++) {
    pwmMovBackwards();
  }
   Serial.println("estas en elProcess_East");

  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or let the XML manage
  // sending feeback

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status
  // like the illumination status will be send in the main XML page update
  // code
  //request->send(200, "text/plain", "OK");//Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feed back immediataly
  // note you must have reading code in the java script
  /*
    if (LED0) {
    server.send(200, "text/plain", "1"); //Send web page
    }
    else {
    server.send(200, "text/plain", "0"); //Send web page
    }
  */

}



// code to send the main web page
// PAGE_MAIN is a large char defined in SuperMon.h

// code to send the main web page
// I avoid string data types at all cost hence all the char mainipulation code
// I think I got this code from the wifi example


// end of code

// This #include statement was automatically added by the Particle IDE.
#include <WebServer.h>

// This #include statement was automatically added by the Particle IDE.
#include <MDNS.h>

/*
    Rotilio.cc firmware base
    carlo@techmakers.io
    http://techmakers.io

    © 2016 Techmakers srl

    
*/


#define FIRMWARE_CLASS      "ROTILIO SEED FIRMWARE"
#define FIRMWARE_VERSION    0.22

//STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

SYSTEM_MODE(SEMI_AUTOMATIC);

#define AUTO_CONNECT_TO_PARTICLE true ;

bool connect_to_particle = AUTO_CONNECT_TO_PARTICLE ;


#include <Particle_BaroSensor.h>

#define UICONFIGARRAYSIZE 12
#define UICONFIGVERSION 4

String uiConfig[UICONFIGARRAYSIZE] = {
    // page title
    "[{'t':'head','text':'General purpose application v 1.0'}]",
    // sensors
    "[{'n':'temperature','l':'Temperature 1'},{'n':'exttemp','l':'Temperature 2'}]", // no 't' means, default:text, no 'l' means use 'n' as label
    "[{'n':'humidity','l':'Humidity'},{'n':'pressure','l':'Pressure'}]",    
    "[{'n':'photoresistor','l':'Light'},{'n':'trimmer','l':'Trimmer'}]",
    //"[{'n':'button1','t':'led','l':'Button 1'},{'n':'button2','t':'led','l':'Button 2'}]", 
    "[{'n':'button1','t':'led','l':'Button 1'}]",
    "[{'n':'dimmer','l':'Dimmer'},{'n':'setdimmer','step':5, 'min':0,'max':100,'l':'Set dimmmer','t':'slider'}]", 
    // actions
    
    "[{'n':'relais','t':'led','l':'Relais status'},{'n':'switch','l':'Switch','t':'switch-readonly'}]",
    
    "[{'t':'button','l':'Manual relais on','m':'setrelais:on'},{'t':'button','l':'Manual relais off','m':'setrelais:off'}]",  // Relais on or off, normally open, button label for open: Warm up, button label for close: Off
    "[{'t':'button','l':'Relais on for 1 second','m':'setrelais:1000'}]",    // Relais pulse on click, for 100 msec, button label: Open door
    "[{'t':'button','l':'2 Beeps','m':'setalarm:2'}]",
    "[{'t':'button','l':'3 Beeps','m':'setalarm:3'}]",
    "[{'t':'button','l':'D7 on','m':'d7:on'},{'t':'button','l':'D7 off','m':'d7:off'},{'t':'button','l':'Reset','m':'reset:now'}]"
};

#define LIGHTUPIFDARK        1
#define LIGHTDOWNIFDARK      2

#define LIGHTMODE            LIGHTDOWNIFDARK

#define RELAIS_IS_MONO       0

#define DEBUG_INTERVAL_SECS 60
String debugMsg = "" ;

#define Si7020_ADDRESS    0x40
#define SAMPLE_NUMBER        1

#define BUTTON_1            D2
#define BUTTON_2            D6
#define SWITCH              D5
#define BUZZER              A0
#define RELAIS_SET          D4
#define RELAIS_RESET        D3
#define RELAIS_FDB          A3
#define PHOTORESISTOR       A1
#define TRIMMER             A2

#define TEMP_ADJUST_OFFSET  -0.7

#define LOOP_DELAY      1000.0

#define DIMMER_ON       1
#define DIMMER_OFF      0
#define DIMMER_FREQ     20000

// variable Inizialization
double temperature = 99999 ; 
double extTemperature = 99999 ;
int humidity = 9999 ;
double pressure = -1 ;
double photoresistor = -1 ;
int trimmer = -1 ;
int button1 = false ;
int button2 = false ;
int switch0 = false ; 
int relais = false ;
int alarmBeeper = 0 ;
int dimmer = 0 ;
int deltaLight = 0 ;

int lastRSSI ;

String lastSSID ;

int rssiDeltaEvent = 5 ;

String status = "{}" ; 
String status_template = "{\"dimmer\":<dimmer>,\"temperature\":<temperature>,\"exttemp\":<exttemp>,\"humidity\":<humidity>,\"pressure\":<pressure>,\"photoresistor\":<photoresistor>,\"trimmer\":<trimmer>,\"deltalight\":<deltalight>,\"button1\":<button1>,\"button2\":<button2>,\"switch\":<switch>,\"relais\":<relais>,\"alarm\":<alarm>}" ;


String STATUS_0 = "" ;
String STATUS_0_template = "<ssid>,<rssi>,<temperature>,<exttemp>,<humidity>,<pressure>,<photoresistor>,<trimmer>,<button1>,<button2>,<switch>,<relais>,<alarm>,<dimmer>" ;



// stats
double relaisOnMinutesInLastDay = 0 ;
double relaisOnAverageMinutesPerDay = 0 ;
double relaisOnAveragePerDayCounter = 0;
int lastDay = 0 ;

double temperatureMin = 9999 ;
double temperatureMax = -9999 ;
double extTempMin = 9999 ;
double extTempMax = 9999 ;
int humidityMin = 9999 ;
int humidityMax = -9999 ;
double pressureMin = 9999 ;
double pressureMax = -9999 ;
int lightMin = 9999 ;
int lightMax = -9999 ;

String stats = "{}" ;
String stats_template = "{\"Temperature Min\":<temperatureMin>,\"Temperature Max\":<temperatureMax>,\"Ext Temperature Min\":<extTemperatureMin>,\"Ext Temperature Max\":<extTemperatureMax>,\"Humidity Min\":\"<humidityMin>\",\"Humidity Max\":\"<humidityMax>\",\"Pressure Min\":<pressureMin>,\"Pressure Max\":<pressureMax>,\"Light Min\":<lightMin>,\"Light Max\":<lightMax>,\"RelaisOn minutes today\":<relaisOnMinutesInLastDay>,\"RelaisOn minutes a day\":<relaisOnAverageMinutesPerDay>}" ;



// counter for debug function
int l=0;
int i=0;

long lastMillis = 0 ;
double timePassed = 0.0 ;

char myIpString[24];
String TCPBuffer = "" ;

//TCPServer server = TCPServer(80);
//TCPClient client;

#define PREFIX ""
#define NAMELEN 32
#define VALUELEN 32
WebServer webserver(PREFIX, 80);

void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    server.httpFail();
    return;
  }

  //server.httpSuccess(false, "application/json");
  server.httpSuccess("application/json");
  
  if (type == WebServer::HEAD)
    return;
    
    
    #if PLATFORM_ID == 10 // electron
        CellularSignal sig = Cellular.RSSI();
        debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(i) + "," + sig.qual + "," + sig.rssi + "," + myIpString ;
        // dont send debug, we published a variable so the cloud can know the situation
        // Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
    #else
        debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(i) + "," +WiFi.SSID() + "," + WiFi.RSSI() + "," + myIpString  ;
    #endif

  server.print("{\"result\":\"" + debugMsg + "\"}") ;
}

void jsonCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    server.httpFail();
    return;
  }

  //server.httpSuccess(false, "application/json");
  server.httpSuccess("application/json");
  
  if (type == WebServer::HEAD)
    return;

  server.print("{\"result\":" + status + "}") ;
}

void messageCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::GET)
  {
    server.httpFail();
    return;
  }

  //server.httpSuccess(false, "application/json");
  server.httpSuccess("application/json");
  
  if (type == WebServer::HEAD)
    return;
    
  char name[NAMELEN];
  char value[VALUELEN];
  while (server.readPOSTparam(name, NAMELEN, value, VALUELEN)){
      int i = message(value) ;
      server.print("{\"result\":" + String(i) + "}") ;
  }

}

MDNS mdns;
bool mdnsSuccess;
bool mdnsBeginned ;

void setup(){
    
    BaroSensor.begin();
      
    Particle.variable("status", status) ;
    Particle.variable("stats",stats) ;
    Particle.variable("d", debugMsg) ;
    
    Particle.variable("STATUS_0", STATUS_0) ;
    
    Particle.function("message", message);      // see workMessage function     
    
    pinMode(BUZZER, OUTPUT);  
    pinMode(RELAIS_SET, OUTPUT);  
    pinMode(RELAIS_RESET, OUTPUT);     
    
    pinMode(PHOTORESISTOR, INPUT) ;
    pinMode(TRIMMER, INPUT) ;
    
    pinMode(RELAIS_FDB,INPUT_PULLUP);  
    #if DIMMER_ON
        pinMode(BUTTON_1, OUTPUT);
        pinMode(BUTTON_2, OUTPUT);  
    #else 
        pinMode(BUTTON_1, INPUT_PULLUP);
        pinMode(BUTTON_2, INPUT_PULLUP);  
    #endif
     
    pinMode(SWITCH, INPUT_PULLUP); 
    
    digitalWrite(BUZZER, LOW);
    digitalWrite(RELAIS_SET, LOW);
    digitalWrite(RELAIS_RESET, LOW);  
    
    pinMode(D7, OUTPUT); 
    
    setRelaisOff();

    Serial.begin(115200);
    Wire.begin();              // si connette col bus i2c
    
    lastMillis = millis() ;
    
    
    if (WiFi.hasCredentials()) WiFi.connect();
    
      /* start the webserver */
    if (WiFi.ready()){
        IPAddress myIp = WiFi.localIP();
        sprintf(myIpString, "%d.%d.%d.%d", myIp[0], myIp[1], myIp[2], myIp[3]);
    
        /* setup our default command that will be run when the user accesses
        * the root page on the server */
        webserver.setDefaultCommand(&defaultCmd);
    
        /* run the same command if you try to load /index.html, a common
        * default page name */
        webserver.addCommand("status", &jsonCmd);
        webserver.addCommand("message", &messageCmd);
        
        webserver.begin();
    } 
    
    sendDebug(0) ;

}

void loop(){
    
    if (WiFi.ready()){
        IPAddress myIp = WiFi.localIP();
        sprintf(myIpString, "%d.%d.%d.%d", myIp[0], myIp[1], myIp[2], myIp[3]);
        
        char buff[64];
        int len = 64;

        /* process incoming connections one at a time forever */
        webserver.processConnection(buff, &len);
    }
     
    if (WiFi.ready() && !mdnsSuccess){
        mdnsSuccess = mdns.setHostname(System.deviceID());
    } 
    
    if (WiFi.ready() && mdnsSuccess && !mdnsBeginned){
        mdnsBeginned = mdns.begin();
    } 
    
    if (WiFi.ready() && mdnsSuccess && mdnsBeginned){
        mdns.processQueries();
    }
    
    if (connect_to_particle && !Particle.connected()) Particle.connect();
    
    while (Serial.available()) {
        char c = Serial.read() ;
        if (c == 13) {
            Serial.print(c);
            message(TCPBuffer);
            TCPBuffer = ""; 
        } else {
            if (c != 10) TCPBuffer += c ;
            Serial.print(c);
        }
    }

    long newMillis = millis() ;
    timePassed = newMillis - lastMillis ;
    lastMillis = newMillis ;

    i++ ;
    if (i > DEBUG_INTERVAL_SECS){
        i = 0;
        l++ ;
        sendDebug(l) ;
    }
    
    temperature = temperatureRead() + TEMP_ADJUST_OFFSET;
    pressure = BaroSensor.getPressure(OSR_8192) ;
    extTemperature = BaroSensor.getTemperature() + TEMP_ADJUST_OFFSET;
    humidity = humidityRead() ;
    photoresistor = analogRead(PHOTORESISTOR) ;
    trimmer = analogRead(TRIMMER) ;
    deltaLight = photoresistor - trimmer ;
    
    #if DIMMER_ON
        //analogWrite(D2,map(dimmer,0,100,0,255),DIMMER_FREQ);
        analogWrite(D2,dimmer,DIMMER_FREQ);
    #else
        int actualButton1 = digitalRead(BUTTON_1) ;
        if (actualButton1 == LOW){
            sound(1000,250) ;
            button1 = !button1 ;
            sendVariableChanged("button1",button1 ? "1" : "0");
        }
        int actualButton2 = digitalRead(BUTTON_2) ;
        if (actualButton2 == LOW){
            sound(1000,250) ;
            button2 = !button2 ;
            sendVariableChanged("button2",button2 ? "1" : "0");
        }
    #endif

    relais = digitalRead(RELAIS_FDB) ;
    
    switch0 = !digitalRead(SWITCH) ;
    
    status = String(status_template) ; // copying ;


    status.replace("<temperature>",String(temperature));
    status.replace("<exttemp>",String(extTemperature)) ;
    status.replace("<humidity>",String(humidity));
    status.replace("<pressure>",String(pressure));
    status.replace("<photoresistor>",String(photoresistor));
    status.replace("<trimmer>",String(trimmer));
    status.replace("<deltalight>",String(deltaLight));
    status.replace("<switch>",String(switch0));
    status.replace("<relais>",String(relais));
    status.replace("<button1>",String(button1));
    status.replace("<button2>",String(button2));
    status.replace("<alarm>",String(alarmBeeper));
    status.replace("<dimmer>",String(dimmer));
    
    
    stats = String(stats_template);
    
    if (lastDay != Time.day()){
        lastDay = Time.day() ;
        relaisOnAveragePerDayCounter++ ;
        relaisOnMinutesInLastDay = 0 ;
        
        extTempMin = 9999 ;
        extTempMax = -9999 ;
        temperatureMin = 9999 ;
        temperatureMax = -9999 ;
        humidityMin = 9999 ;
        humidityMax = -9999 ;
        pressureMin = 9999 ;
        pressureMax = -9999 ;
        lightMin = 9999 ;
        lightMax = -9999 ;
    }
    
   
    
    if (extTempMin > extTemperature) extTempMin = extTemperature ;
    if (extTempMax < extTemperature) extTempMax = extTemperature ;
    if (temperatureMin > temperature) temperatureMin = temperature ;
    if (temperatureMax < temperature) temperatureMax = temperature ;
    if (humidityMin > humidity) humidityMin = humidity ;
    if (humidityMax < humidity) humidityMax = humidity ;
    if (pressureMin > pressure) pressureMin = pressure ;
    if (pressureMax < pressure) pressureMax = pressure ;
    if (lightMin > photoresistor) lightMin = photoresistor ;
    if (lightMax < photoresistor) lightMax = photoresistor ;
    
    if (relais == 1){
        relaisOnMinutesInLastDay = relaisOnMinutesInLastDay + double(timePassed/1000.0/60.0) ;
    }
    
    relaisOnAverageMinutesPerDay = (relaisOnAverageMinutesPerDay * relaisOnAveragePerDayCounter + relaisOnMinutesInLastDay) / (relaisOnAveragePerDayCounter + 1.0);
    
    stats.replace("<temperatureMin>",String(temperatureMin));
    stats.replace("<temperatureMax>",String(temperatureMax));
    stats.replace("<extTemperatureMin>",String(extTempMin));
    stats.replace("<extTemperatureMax>",String(extTempMax));
    stats.replace("<humidityMin>",String(humidityMin));
    stats.replace("<humidityMax>",String(humidityMax));
    stats.replace("<pressureMin>",String(pressureMin));
    stats.replace("<pressureMax>",String(pressureMax));
    stats.replace("<lightMin>",String(lightMin));
    stats.replace("<lightMax>",String(lightMax));
    stats.replace("<relaisOnMinutesInLastDay>",String(relaisOnMinutesInLastDay));
    stats.replace("<relaisOnAverageMinutesPerDay>",String(round(relaisOnAverageMinutesPerDay*10)/10));

    if (switch0){
        if (photoresistor > trimmer){
            // is DARK
            if (LIGHTMODE == LIGHTUPIFDARK) setRelaisOn();
            if (LIGHTMODE == LIGHTDOWNIFDARK) setRelaisOff();
        } else {
            // is NOT DARK
            if (LIGHTMODE == LIGHTUPIFDARK) setRelaisOff();
            if (LIGHTMODE == LIGHTDOWNIFDARK) setRelaisOn();
        }
    }    
    
    STATUS_0 = String(STATUS_0_template) ; // copying ;
    
    int delta ;
    
    #if PLATFORM_ID == 10 // electron
        CellularSignal sig = Cellular.RSSI();
        STATUS_0.replace("<ssid>",String(sig.qual));
        STATUS_0.replace("<rssi>",String(sig.rssi));
        
        int rssi = sig.rssi ;
        delta = lastRSSI - rssi ;
        if (delta < 0) delta = delta * -1 ;
        if (delta > rssiDeltaEvent){
            lastRSSI = rssi ;
            sendVariableChanged("rssi", String(rssi)) ;
        }
        
    #else
    
        STATUS_0.replace("<ssid>",String(WiFi.SSID()));
        
        STATUS_0.replace("<rssi>",String(WiFi.RSSI()));
        
        int rssi = WiFi.RSSI() ;
        delta = lastRSSI - rssi ;
        if (delta < 0) delta = delta * -1 ;
        if (delta > rssiDeltaEvent){
            lastRSSI = rssi ;
            sendVariableChanged("rssi", String(rssi)) ;
        }
        
        if (!lastSSID.equals(String(WiFi.SSID()))){
            lastSSID = WiFi.SSID() ;
            sendVariableChanged("ssid", String(lastSSID)) ;
        }
    #endif
    
    
    STATUS_0.replace("<temperature>",String(temperature));
    STATUS_0.replace("<exttemp>",String(extTemperature)) ;
    STATUS_0.replace("<humidity>",String(humidity));
    STATUS_0.replace("<pressure>",String(pressure));
    STATUS_0.replace("<photoresistor>",String(photoresistor));
    STATUS_0.replace("<trimmer>",String(trimmer));
    STATUS_0.replace("<switch>",String(switch0));
    STATUS_0.replace("<relais>",String(relais));
    STATUS_0.replace("<button1>",String(button1));
    STATUS_0.replace("<button2>",String(button2));
    STATUS_0.replace("<alarm>",String(alarmBeeper));
   
    //delay(LOOP_DELAY); 

}

int message(String message){
    // ex: setrelais:on
    // ex: setrelais:off
    // ex: setalarm:10
    int colonPos = message.indexOf(":") ;
    if (colonPos < 0) {
        Serial.println("{\"error\":\"Wrong command format please use <command>:<argument> format.\"}") ;
        return -1 ;                 // syntax error not found
    }
    String command = message.substring(0,colonPos) ;
    String argument = message.substring(colonPos+1) ;

    
    if (command.equals("d")){
        Serial.println("{\"debugMsg\":\""+debugMsg+"\"}");
        return 1;
    } else if (command.equals("s")){
        Serial.println("{\"result\":"+status+"}");
        return 1;
    } else if (command.equals("setdimmer")){
        return setdimmer(argument) ;
    } else if (command.equals("setrelais")){
        return setrelais(argument) ;
    } else if (command.equals("setalarm")){
        return setalarm(argument) ;
    } else if (command.equals("d7")){
        return setD7(argument) ;
    } else if (command.equalsIgnoreCase("getuiconfig")){
        return sendUIConfig();
    } else if (command.equalsIgnoreCase("reset")){
        Serial.println("{\"result\":\"System reset !!!\"}") ;
        System.reset();
        return 0;
    } else {
        Serial.println("{\"error\":\"Wrong command '"+ command +  "'\"}") ;
        return -2 ;                         // command not found
    }
}

int setD7(String argument){
    if (argument.equals("on")){
        digitalWrite(D7,HIGH) ;
        return 1;
    } else {
        digitalWrite(D7,LOW) ;
        return 0;
    }
}

void sound(int cicles,int period){ // emette il suono .. 
    for(int n=0;n<cicles;n++) {
        digitalWrite(BUZZER, HIGH);
        delayMicroseconds(period);
        digitalWrite(BUZZER, LOW);
        delayMicroseconds(period);           
    }
}

double temperatureRead(void){
    int accumulo=0;
    for(byte n=0;n<SAMPLE_NUMBER;n++)
          {
            Wire.beginTransmission(Si7020_ADDRESS);   // "start" i2c
            Wire.write(0xF3);                           // comando lettura temperatura
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(Si7020_ADDRESS, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    double res = ((175.72*(accumulo/SAMPLE_NUMBER))/65536)-46.85;   // calcola la media e converte la lettura grezza in temperatura (gradi centigradi)
    return round(res*10)/10 ;
}

double humidityRead(void){
    int accumulo=0;
    for(byte n=0;n<SAMPLE_NUMBER;n++)
          {
            Wire.beginTransmission(Si7020_ADDRESS);   // "start" i2c
            Wire.write(0xF5);                           // comando lettura umidita'
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(Si7020_ADDRESS, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    
    return ((125*(accumulo/SAMPLE_NUMBER))/65536)-6;   // calcola la media e converte la lettura grezza in umidita' (% relativa)     
}

int setalarm(String command){
    alarmBeeper = command.toInt();
    for (int i=0;i<alarmBeeper;i++){
        sound(1000,250);
        delay(1000) ;
    }
    return alarmBeeper ;
}

int setdimmer(String command){
    dimmer = command.toInt() ;
    return dimmer ;
}

void setRelaisOn(){
    digitalWrite(RELAIS_SET, HIGH);
    
    if (RELAIS_IS_MONO) return ;
    
    delay(100) ;
    digitalWrite(RELAIS_SET, LOW);
}

void setRelaisOff(){
    if (RELAIS_IS_MONO){
        digitalWrite(RELAIS_SET, LOW);
    } else {
        digitalWrite(RELAIS_RESET, HIGH);
        delay(100) ;
        digitalWrite(RELAIS_RESET, LOW);
    }
}

int setrelais(String command){
    int cmd = LOW ;
    if (command.equalsIgnoreCase("on"))  {
        cmd = HIGH ;
        setRelaisOn() ;
    } else if (command.equalsIgnoreCase("off")){
        setRelaisOff() ;
    } else {
        cmd = command.toInt() ;
        setRelaisOn() ;
        delay(cmd) ;
        setRelaisOff() ;
    }
    return cmd ;
}

int sendUIConfig(){
    for (int iii=0;iii<UICONFIGARRAYSIZE;iii++){
        String row = "{'id':" + String(iii) + ",'c':" + uiConfig[iii] + "}" ;
        if (Particle.connected()) Particle.publish("uiConfig",row,60,PRIVATE) ;
        delay(500) ; // needed to avoid message drops by Particle cloud
    }
    return UICONFIGVERSION ;
}

void sendDebug(int count){
    #if PLATFORM_ID == 10 // electron
        CellularSignal sig = Cellular.RSSI();
        debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(count) + "," + sig.qual + "," + sig.rssi + "," + myIpString ;
        // dont send debug, we published a variable so the cloud can know the situation
        // Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
    #else
        debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(count) + "," +WiFi.SSID() + "," + WiFi.RSSI() + "," + myIpString  ;
        if (Particle.connected()) Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
    #endif
    //Serial.println(debugMsg) ;
}

void sendVariableChanged(String name, String value){
    if (Particle.connected()) Particle.publish("variableChanged",name+":"+value,60,PRIVATE) ;
}

void setup_the_fundulating_conbobulator(){
   setRelaisOff();
}

// The STARTUP call is placed outside of any other function
// What goes inside is any valid code that can be executed. Here, we use a function call.
// Using a single function is preferable to having several `STARTUP()` calls.
STARTUP( setup_the_fundulating_conbobulator() );
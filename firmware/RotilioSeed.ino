/*
    Rotilio.cc firmware base
    carlo@techmakers.io
    http://techmakers.io

    © 2016 Techmakers srl

    
*/

#define FIRMWARE_CLASS      "ROTILIO SEED FIRMWARE"
#define FIRMWARE_VERSION    0.15

//STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));


#include "Particle_BaroSensor/Particle_BaroSensor.h"

#define UICONFIGARRAYSIZE 13
#define UICONFIGVERSION 3

String uiConfig[UICONFIGARRAYSIZE] = {
    // page title
    "[{'t':'head','text':'General purpose application v 1.0'}]",
    // sensors
    "[{'n':'temperature','l':'Temperature 1'},{'n':'exttemp','l':'Temperature 2'}]", // no 't' means, default:text, no 'l' means use 'n' as label
    "[{'n':'humidity','l':'Humidity'},{'n':'pressure','l':'Pressure'}]",    
    "[{'n':'photoresistor','l':'Light'},{'n':'trimmer','l':'Trimmer'}]",
    //"[{'n':'button1','t':'led','l':'Button 1'},{'n':'button2','t':'led','l':'Button 2'}]", 
    "[{'n':'button1','t':'led','l':'Button 1'}]", 
    // actions
    
    "[{'n':'relais','t':'led','l':'Relais status'},{'n':'switch','l':'Switch','t':'switch-readonly'}]",
    
    "[{'t':'button','l':'Manual relais on','m':'setrelais:on'},{'t':'button','l':'Manual relais off','m':'setrelais:off'}]",  // Relais on or off, normally open, button label for open: Warm up, button label for close: Off
    "[{'t':'button','l':'Relais on for 1 second','m':'setrelais:1000'}]",    // Relais pulse on click, for 100 msec, button label: Open door
    "[{'t':'button','l':'2 Beeps','m':'setalarm:2'}]",
    "[{'t':'button','l':'3 Beeps','m':'setalarm:3'}]",
    "[{'t':'button','l':'D7 on','m':'d7:on'},{'t':'button','l':'D7 off','m':'d7:off'},{'t':'button','l':'Reset','m':'reset:now'}]",
    "[{'t':'barchart','n':'barchart','names':['temperature','exttemp']}]",
    "[{'t':'piechart','n':'piechart','segments':[{'n':'temperature','c':'#F7464A','h':'#FF5A5E','l':'Temp1'},{'n':'exttemp','c':'#46BFBD','h':'#5AD3D1','l':'Temp 2'}]}]"
};


#define RELAIS_IS_MONO       0

#define DEBUG_INTERVAL_SECS 30
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

#define TEMP_ADJUST_OFFSET  -0.7

#define LOOP_DELAY      1000.0


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
int alarm = 0 ;

String status = "{}" ; 
String status_template = "{\"temperature\":<temperature>,\"exttemp\":<exttemp>,\"humidity\":<humidity>,\"pressure\":<pressure>,\"photoresistor\":<photoresistor>,\"trimmer\":<trimmer>,\"button1\":<button1>,\"button2\":<button2>,\"switch\":<switch>,\"relais\":<relais>,\"alarm\":<alarm>}" ;


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

void setup(){
    
    sendDebug(0) ;
    
    BaroSensor.begin();
      
    Particle.variable("status", status) ;
    Particle.variable("stats",stats) ;
    Particle.variable("d", debugMsg) ;
    
    Particle.function("message", message);      // see workMessage function     
    
    pinMode(BUZZER, OUTPUT);  
    pinMode(RELAIS_SET, OUTPUT);  
    pinMode(RELAIS_RESET, OUTPUT);     
    
    pinMode(RELAIS_FDB,INPUT_PULLUP);  
    pinMode(BUTTON_1, INPUT_PULLUP);     
    pinMode(BUTTON_2, INPUT_PULLUP);   
    pinMode(SWITCH, INPUT_PULLUP); 
    
    digitalWrite(BUZZER, LOW);
    digitalWrite(RELAIS_SET, LOW);
    digitalWrite(RELAIS_RESET, LOW);  
    
    pinMode(D7, OUTPUT); 
    
    setRelaisOff();

    Serial.begin(115200);
    Wire.begin();              // si connette col bus i2c
    
}

void loop(){
    
    if (alarm > 0){
        sound(1000,250);
        alarm-- ;
    }
    
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
    photoresistor = analogRead(A1) ;
    trimmer = analogRead(A2) ;
    
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


    relais = digitalRead(RELAIS_FDB) ;
    
    switch0 = !digitalRead(SWITCH) ;
    
    status = String(status_template) ; // copying ;


    status.replace("<temperature>",String(temperature));
    status.replace("<exttemp>",String(extTemperature)) ;
    status.replace("<humidity>",String(humidity));
    status.replace("<pressure>",String(pressure));
    status.replace("<photoresistor>",String(photoresistor));
    status.replace("<trimmer>",String(trimmer));
    status.replace("<switch>",String(switch0));
    status.replace("<relais>",String(relais));
    status.replace("<button1>",String(button1));
    status.replace("<button2>",String(button2));
    status.replace("<alarm>",String(alarm));
    
    
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
        relaisOnMinutesInLastDay = relaisOnMinutesInLastDay + double(LOOP_DELAY/1000.0/60.0) ;
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
    
    delay(LOOP_DELAY); 
}

int message(String message){
    // ex: setrelais:on
    // ex: setrelais:off
    // ex: setalarm:10
    int colonPos = message.indexOf(":") ;
    if (colonPos < 0) return -1 ;                 // syntax error not found
    String command = message.substring(0,colonPos) ;
    String argument = message.substring(colonPos+1) ;
    
    if (command.equals("setrelais")){
        return setrelais(argument) ;
    } else if (command.equals("setalarm")){
        return setalarm(argument) ;
    } else if (command.equals("d7")){
        return setD7(argument) ;
    } else if (command.equalsIgnoreCase("getuiconfig")){
        return sendUIConfig();
    } else if (command.equalsIgnoreCase("reset")){
        System.reset();
        return 0;
    } else {
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
    alarm = command.toInt();
    return alarm ;
}

void setRelaisOn(bool force){

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
        setRelaisOn(true) ;
    } else if (command.equalsIgnoreCase("off")){
        setRelaisOff() ;
    } else {
        cmd = command.toInt() ;
        setRelaisOn(true) ;
        delay(cmd) ;
        setRelaisOff() ;
    }
    return cmd ;
}

int sendUIConfig(){
    for (int iii=0;iii<UICONFIGARRAYSIZE;iii++){
        String row = "{'id':" + String(iii) + ",'c':" + uiConfig[iii] + "}" ;
        Particle.publish("uiConfig",row,60,PRIVATE) ;
        delay(500) ; // needed to avoid message drops by Particle cloud
    }
    return UICONFIGVERSION ;
}

void sendDebug(int count){
    #if PLATFORM_ID == 10 // electron
        CellularSignal sig = Cellular.RSSI();
        debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(count) + "," + sig.qual + "," + sig.rssi ;
        // dont send debug, we published a variable so the cloud can know the situation
        // Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
    #else
        debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(count) + "," +WiFi.SSID() + "," + WiFi.RSSI() ;
        Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
    #endif
}

void sendVariableChanged(String name, String value){
    Particle.publish("variableChanged",name+":"+value,60,PRIVATE) ;
}

void setup_the_fundulating_conbobulator(){
   setRelaisOff();
}

// The STARTUP call is placed outside of any other function
// What goes inside is any valid code that can be executed. Here, we use a function call.
// Using a single function is preferable to having several `STARTUP()` calls.
STARTUP( setup_the_fundulating_conbobulator() );


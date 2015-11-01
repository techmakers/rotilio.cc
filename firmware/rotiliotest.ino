/*
    Rotilio.cc firmware
    carlo@techmakers.io

*/

#define FIRMWARE_VERSION    0.19

#define ARRAYSIZE 8

#define DEBUG_INTERVAL_SECONDS 30

#define INDIRIZZO_Si7020    0x40
#define NUMERO_LETTURE      5

#define PULSANTE_1          D2
#define PULSANTE_2          D6
#define SWITCH              D5
#define BUZZER              A0
#define RELAIS_SET          D4
#define RELAIS_RESET        D3
#define RELAIS_FDB          A3

#define PREMUTO             LOW
#define NON_PREMUTO         HIGH


#define tempDeltaEvent      0.1
#define humiDeltaEvent      1
#define photoDeltaEvent     200
#define trimmerDeltaEvent   100

double temperature = 99999 ; 
double humidity = 9999 ;
double pressure = -1 ;
int photoresistor = -1 ;
int trimmer = -1 ;
int button1 = false ;
int button2 = false ;
int switch0 = false ; 
int relais = false ;
int alarm = 0 ;
int actualTimeRangeIndex = -1 ;


// storage for last published values 
double lastTemp = -999999 ;  
double lastHumi = -999999 ;
double lastPressure = -99999 ;
int lastPhotoRes = -999999 ;
int lastTrimmer = -999999 ;
int lastTimeRangeIndex = -1 ;

// counter for debug function
int l=0;
int i=0;

/*

T = temp
H = humi
B = baro
var inputConditions  = "Th" ;  // Temperature above setpoint
var actual = "ThHhBh" ;  // T,H,B all above (h)
var relais = setup in actual ;
*/
int timeZone = 1 ;

/*

SET POINTS

T = TEMP            es: T=22.1  = setpoint 22.1 °C 
H = HUMI            es: H=80    = setpoint 80% 
P = PRESSIONE       es: P=1800  = setpoint 1800 mbar
R = TRIMMER         es: R=3400  = setpoint 3400 units (adc, from 0 to 4095)
L = LIGHT           es: L=1200  = setpoint 1200 units (adc, from 0 to 4095)
S = SWITCH          es: S=1     = setpoint switch on (1=on, 0=off)
A = P1              es: A=1     = setpoint button1 == true
B = P2              es: B=0     = setpoint button2 == false


*/

String timeRanges[ARRAYSIZE] = { 
    //"00:00-23:59|MTWTFSS|JFMAMJJASOND", // alwais on, alwais off is : 00:00-00:00|mtwtfss|jfmamjjasond
    "0:06:00-07:30|MTWTFss|JFmamjjasOND|T=22", // working days morning
    "1:07:31-12:59|MTWTFss|JFmamjjasOND|T=20",
    "2:13:00-14:00|MTWTFSS|JFmamjjasOND|T=22", // lunch time every day
    "3:14:01-18:59|MTWTFSS|JFmamjjasOND|T=20",
    "4:19:00-22:00|MTWTFSS|JFmamjjasOND|T=22", // dinner time every day
    "5:22:00-05:59|MTWTFss|JFmamjjasOND|T=20", // sleeping time working days
    "6:00:00-08:59|mtwtfSS|JFmamjjasOND|T=28", // sleeping time weekend
    "7:09:00-10:00|mtwtfSS|JFmamjjasOND|T=28", // weekend morning
};

bool isInTimeRange(unsigned long now, String tRange){
    
    bool yesItIs = false ;
   
    /*
    example "4:01:16-01:44|MTWTFSS|JFMAMJJASOND|T=22.1"
             01234567890123456789012345678901234567890
                       1         2         3         4
                       
    means: from 6.50 to 8.30 of Monday, twensday,Wendsday,Thursday,Friday on Jan,Feb,Oct,Nov,Dec
    */
    
    // check for weekday
    
    int dayOfWeek = Time.weekday(now) ;

    yesItIs =  tRange.charAt(13+dayOfWeek) < 97 ; // Uppercase char in this position means the day of the week is active in the time range
    
    if (!yesItIs) return false ;
    
    
    // check fom month
    int month = Time.month(now) ;
    
    yesItIs = tRange.charAt(21+month) < 97 ;
    
    if (!yesItIs) return false ;
    
    
    // check for hour and minute
    int hourFrom = tRange.substring(2,4).toInt();
    int minuteFrom =  tRange.substring(5,7).toInt();
    int hourTo = tRange.substring(8,10).toInt() ;
    int minuteTo = tRange.substring(11,13).toInt() ;
    
    int hourMinuteFrom = hourFrom * 60 + minuteFrom ;
    int hourMinuteTo = hourTo * 60 + minuteTo ;
    
    int hourNow = Time.hour(now) + timeZone ;
    int minuteNow = Time.minute(now) ;
    
    int hourMinuteNow = minuteNow + hourNow * 60 ;
    
    if (hourMinuteFrom > hourMinuteTo){ 
        //23:00-08:59 @ 23:34
        
        int midNight = 23*60+60 ;
        int delta = midNight - hourMinuteFrom ;
        hourMinuteFrom = 0;
        hourMinuteTo = hourMinuteTo+delta;
        hourMinuteNow = hourMinuteNow + delta ;
        if (hourMinuteNow > midNight) {
            hourMinuteNow = hourMinuteNow - midNight ;
        }
    }
    
    yesItIs = (hourMinuteFrom <= hourMinuteNow) && (hourMinuteNow <= hourMinuteTo) ;
    
    return yesItIs ;
}

int getTimeRangeForNow(){
    unsigned long now = Time.now() ;
    int res = -1 ;
    for (int ii=0;ii<ARRAYSIZE;ii++){
        if (isInTimeRange(now,timeRanges[ii])){
            res = ii ;
        }
    }
    return res ;
}


void workTimeRange(){
    if (actualTimeRangeIndex < 0 ) return ; // no time range active
    String actualTimeRange = timeRanges[actualTimeRangeIndex] ;
    char c = actualTimeRange.charAt(35) ;
    //Particle.publish("c",String(c),60,PRIVATE) ;
    switch (c)
    {
        
        case 'T': { // temp
            float TempSetPoint = actualTimeRange.substring(37).toFloat() ;
            //Particle.publish("Tempsetpoint",String(TempSetPoint),60,PRIVATE) ;
            if (temperature < TempSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'H': { // temp
            int HumiSetPoint = actualTimeRange.substring(37).toInt() ;
            if (humidity < HumiSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'P': { // temp
            int PressuerSetPoint = actualTimeRange.substring(37).toInt() ;
            if (pressure < PressuerSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
/*

        case 'R': // trimmer
        // statements
            break;
        case 'L': // light
        // statements
            break;
        case 'S': // Switch
        // statements
            break;
        case 'A': // buntton 1
        // statements
            break;
        case 'B':  // buntton 2
        // statements
            break;
*/
        default: {
            Particle.publish("timerangeerror",actualTimeRange,60,PRIVATE) ;
        }
    }
}
// accessToken = 397c2c0c884ee6ed7b0c33acc25ff890fa59cf6b
// deviceId = 30001c000647343232363230

// curl "https://api.particle.io/v1/devices/30001c000647343232363230/temperature?access_token=397c2c0c884ee6ed7b0c33acc25ff890fa59cf6b"

void sendDebug(int count){
        String debugMsg = "V " + String(FIRMWARE_VERSION) + "," + String(count) + "," +WiFi.SSID() + "," + WiFi.RSSI() ;
        Spark.publish("debugmsg", debugMsg, 60, PRIVATE);
}

void setup()
{
    sendDebug(0) ;
    
    Particle.variable("timerange", actualTimeRangeIndex) ;
    Particle.variable("temperature", temperature);
    Particle.variable("humidity", humidity);
    Particle.variable("photoresist", photoresistor);
    Particle.variable("trimmer", trimmer);
    Particle.variable("switch", switch0) ;
    Particle.variable("relais", relais) ;
    Particle.variable("button1", button1) ;
    Particle.variable("button2", button2) ;
    Particle.variable("alarm", alarm) ;
    
    
    Particle.function("setrelais", setrelais); // accepts "on" to switch on the relais 
    Particle.function("setalarm", setalarm); // accept a nummber of beeps to emit (one per second)
    Particle.function("relaispulse", relaispulse); // accept a number of milliseconds for releais on
    //Particle.function("setpoint",setpoint);
    
    
    pinMode(BUZZER, OUTPUT);  
    pinMode(RELAIS_SET, OUTPUT);  
    pinMode(RELAIS_RESET, OUTPUT);     
    
    pinMode(RELAIS_FDB,INPUT_PULLUP);  
    pinMode(PULSANTE_1, INPUT_PULLUP);     
    pinMode(PULSANTE_2, INPUT_PULLUP);   
    pinMode(SWITCH, INPUT_PULLUP); 
    
    digitalWrite(BUZZER, LOW);
    digitalWrite(RELAIS_SET, LOW);
    digitalWrite(RELAIS_RESET, LOW);    

    Serial.begin(115200);
    Wire.begin();              // si connette col bus i2c
    
}

void loop(){
    
    actualTimeRangeIndex = getTimeRangeForNow();
    if (actualTimeRangeIndex != lastTimeRangeIndex){
        String actualTimeRange = "NO TIME RANGE" ; 
        if (actualTimeRangeIndex > -1){
            actualTimeRange = timeRanges[actualTimeRangeIndex] ;
        }
        Particle.publish("timeRangeEntered",actualTimeRange,60,PRIVATE) ;
        lastTimeRangeIndex = actualTimeRangeIndex ;
    }
    
    i++ ;
    if (i > DEBUG_INTERVAL_SECONDS){
        i = 0;
        l++ ;
        sendDebug(l) ;
    }

    if (alarm > 0){
        sound(1000,250);
        alarm-- ;
    }
    
    
    temperature = temperatureRead() ;
    if (abs(lastTemp - temperature) > tempDeltaEvent) {
        lastTemp = temperature ;
        Particle.publish("temperatureChanged", String(temperature), 60, PRIVATE);
    }
    

    humidity = humidityRead() ;
    if (abs(lastHumi - humidity) > humiDeltaEvent) {
        lastHumi = humidity ;
        Particle.publish("humidityChanged", String(humidity), 60, PRIVATE);
    }
    

    photoresistor = analogRead(A1) ;
    if (abs(lastPhotoRes - photoresistor) > photoDeltaEvent) {
        lastPhotoRes = photoresistor;
        Particle.publish("photoresistChanged", String(photoresistor), 60, PRIVATE);
    }
    
    
    trimmer = analogRead(A2) ;
    if (abs(lastTrimmer - trimmer) > trimmerDeltaEvent) {
        lastTrimmer = trimmer;
        Particle.publish("trimmerChanged", String(trimmer), 60, PRIVATE);
    }
    
    int lastSwitch = switch0 ;
    switch0 = digitalRead(SWITCH) == HIGH ;
    if (lastSwitch != switch0) Particle.publish("switchChanged", switch0 ? "true" : "false", 60, PRIVATE);
    
    int lastRelais = relais ;
    relais = digitalRead(RELAIS_FDB) == HIGH ;
    if (lastRelais != relais) Particle.publish("relaisChanged", relais ? "true" : "false", 60, PRIVATE);
    

    if(digitalRead(SWITCH)==LOW) sound(500,340) ;
    
    if (digitalRead(PULSANTE_1)==LOW){
        sound(1000,170) ;
        button1 = !button1 ;
        Particle.publish("button1Changed", button1 ? "true" : "false", 60, PRIVATE);
    }
    
    if (digitalRead(PULSANTE_2)==LOW){
        sound(1000,170) ;
        button2 = !button2 ;
        Particle.publish("button2Changed", button2 ? "true" : "false", 60, PRIVATE);
    }

/*
    if(digitalRead(PULSANTE_2)==PREMUTO) digitalWrite(RELAIS_SET, HIGH); else digitalWrite(RELAIS_SET, LOW);   
    Serial.print("TEMPERATURA: "); Serial.print(temperature);   Serial.println(" C");     
    Serial.print("UMIDITA'   : "); Serial.print(umidity);   Serial.println(" %");         
    Serial.print("FOTORESISTENZA:-> ");  Serial.println(photores);
    Serial.print("POTENZIOMETRO :-> ");  Serial.println(trimmer);   
    Serial.print("PULSANTE 'P1' "); if(digitalRead(PULSANTE_1)==HIGH) Serial.print("*NON*"); Serial.println(" PREMUTO");   
    Serial.print("PULSANTE 'P2' "); if(digitalRead(PULSANTE_2)==HIGH) Serial.print("*NON*"); Serial.println(" PREMUTO");       
    Serial.print("SWITCH "); if(digitalRead(SWITCH)==HIGH) Serial.println(" *OFF*"); else Serial.println(" *ON* - BUZZER ATTIVO");     
    Serial.print("IL RELAIS RISULTA "); if(digitalRead(RELAIS_FDB)==LOW) Serial.print(" *NON* "); Serial.println(" ECCITATO");     
    Serial.println(""); Serial.println(""); 
*/
    workTimeRange() ;
    delay(1000); 
}

void evaluateRelaisOn(){
    
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
    for(byte n=0;n<NUMERO_LETTURE;n++)
        {
            Wire.beginTransmission(INDIRIZZO_Si7020);   // "start" i2c
            Wire.write(0xF3);                           // comando lettura temperatura
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(INDIRIZZO_Si7020, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    double res = ((175.72*(accumulo/NUMERO_LETTURE))/65536)-46.85;   // calcola la media e converte la lettura grezza in temperatura (gradi centigradi)
    return round(res*10)/10 ;
}

double humidityRead(void){
    int accumulo=0;
    for(byte n=0;n<NUMERO_LETTURE;n++)
        {
            Wire.beginTransmission(INDIRIZZO_Si7020);   // "start" i2c
            Wire.write(0xF5);                           // comando lettura umidita'
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(INDIRIZZO_Si7020, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    
    return ((125*(accumulo/NUMERO_LETTURE))/65536)-6;   // calcola la media e converte la lettura grezza in umidita' (% relativa)     
}

int setalarm(String command){
    alarm = command.toInt();
    if (alarm > 0) {
        sound(1000,250);
        alarm-- ;
    }
    return command.toInt() ;
}

int setrelais(String command){
    int cmd = LOW ;
    if (command == "on")  {
        cmd = HIGH ;
    } 
    digitalWrite(RELAIS_SET, cmd);
    return cmd ;
}

int relaispulse(String command){
    int duration = command.toInt() ;
    digitalWrite(RELAIS_SET, HIGH);
    delay(duration) ;
    digitalWrite(RELAIS_SET, LOW);
    return duration ;
}


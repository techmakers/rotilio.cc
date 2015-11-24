/*
    Rotilio.cc firmware
    carlo@techmakers.io

*/

#define FIRMWARE_CLASS      "ROTILIO THERMO"
#define FIRMWARE_VERSION    0.29

//STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));


#include "Particle_BaroSensor/Particle_BaroSensor.h"

// Uncomment following line to activate DS18B20 external temperature sensor 
//#define DS18B20

#ifdef DS18B20
#include "spark-dallas-temperature/spark-dallas-temperature.h"
#include "OneWire/OneWire.h"
DallasTemperature dallas(new OneWire(D7));
#endif

#define UICONFIGARRAYSIZE 10
#define UICONFIGVERSION 3

String uiConfig[UICONFIGARRAYSIZE] = {
    // page title
    //"[{'t':'head','text':'General purpose application'}]",
    
    // timerange
    "[{'n':'timerangeon','l':'Timerange ON','t':'switch'}]",
    "[{'n':'timerange','l':'Active time range', 't':'timerange'}]",
    
    // sensors
    "[{'n':'temperature','l':'Temperature 1'},{'n':'exttemp','l':'Temperature 2'}]", // no 't' means, default:text, no 'l' means use 'n' as label
    "[{'n':'temperaturesetpoint','step':0.5, 'min':-10,'max':30,'l':'Set temperature','t':'slider'},{'n':'relais','t':'led','l':'Relais status'}]",
    "[{'n':'humidity','l':'Humidity'},{'n':'pressure','l':'Pressure'}]",    
    "[{'n':'photoresistor','l':'Light'}]",
    //"[{'n':'button1','t':'led','l':'Button 1'},{'n':'button2','t':'led','l':'Button 2'},{'n':'switch','l':'Switch','t':'switch-readonly'}]", 
    "[{'n':'switch','l':'Switch','t':'switch-readonly'}]", 
    "[{'n':'relaisIsInManualMode','l':'Relais is manual','t':'switch'}]",
    
    // manual mode for relais
    
    
    // actions
    "[{'t':'button','l':'Manual relais on','m':'setrelais:on'},{'t':'button','l':'Manual relais off','m':'setrelais:off'}]",  // Relais on or off, normally open, button label for open: Warm up, button label for close: Off
    //"[{'t':'button','l':'Relais on for 1 second','m':'setrelais:1000'}]",    // Relais pulse on click, for 100 msec, button label: Open door
    "[{'t':'button','l':'2 Beeps','m':'setalarm:2'}]",
    //"[{'t':'button','l':'3 Beeps','m':'setalarm:3'}]",
    //"[{'t':'button','l':'Reset','m':'reset:now'}]"
};

#define TIMERANGEARRAYSIZE  8
String timeRanges[TIMERANGEARRAYSIZE] = { 
    //"00:00-23:59|MTWTFSS|JFMAMJJASOND", // alwais on, alwais off is : 00:00-00:00|mtwtfss|jfmamjjasond
    "0:06:00-07:30|sMTWTFs|JFmamjjasOND|T=20.5", // working days morning
    "1:07:31-12:59|sMTWTFs|JFmamjjasOND|T=19.5",
    "2:13:00-14:00|SMTWTFS|JFmamjjasOND|T=20.5", // lunch time every day
    "3:14:01-18:59|SMTWTFS|JFmamjjasOND|T=19.5",
    "4:19:00-22:00|SMTWTFS|JFmamjjasOND|T=20.5", // dinner time every day
    "5:22:00-05:59|sMTWTFs|JFmamjjasOND|T=19.5", // sleeping time working days
    "6:23:30-08:59|SmtwtfS|JFmamjjasOND|T=19.5", // sleeping time weekend
    "7:09:00-10:00|SmtwtfS|JFmamjjasOND|T=20.5", // weekend morning
};

String expireDates[TIMERANGEARRAYSIZE] = {
    "0:2015-06-30|08:00:00|2015-07-30|16:00:00|T=22", // set temperature setpoint to 22°C from 30 June 2015 8 o'clock untile 30 July 16 o'clock
    "",
    "",
    "",
    "",
    "",
    "",
    ""
};


String api_help = "setrelais:on|off|auto|<msec on>, setalarm:<beeps>, auto:on, getuiconfig:now, temperaturesetpoint:<temperaturesetpoint>" ;

// relais override command
// 0 : no override
// 1 : override to value by setRalais



#define DEBUG_INTERVAL_SECS 30

#define INDIRIZZO_Si7020    0x40
#define NUMERO_LETTURE      1

#define PULSANTE_1          D2
#define PULSANTE_2          D6
#define SWITCH              D5
#define BUZZER              A0
#define RELAIS_SET          D4
#define RELAIS_RESET        D3
#define RELAIS_FDB          A3

#define PREMUTO             LOW
#define NON_PREMUTO         HIGH


#define tempDeltaEvent      0.2
#define humiDeltaEvent      1
#define pressureDeltaEvent  1
#define photoDeltaEvent     50
#define trimmerDeltaEvent   50

#define RELAIS_MIN_PERIOD   60
#define TEMP_ADJUST_OFFSET  -0.7

#define LOOP_DELAY          1000.0


// variable Inizialization
int temperatureSampleCount = 0 ;
double temperatureSampleAcc = 0 ;
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
int actualTimeRangeIndex = -1 ;
int actualExpireDateIndex = -1 ;

String status = "{}" ;
String status_template = "{\"relaisIsInManualMode\":<relaisIsInManualMode>,\"timerangeon\":<timerangeon>,\"timerange\":\"<timerange>\",\"expiredate\":\"<expiredate>\",\"temperature\":<temperature>,\"exttemp\":<exttemp>,\"humidity\":<humidity>,\"pressure\":<pressure>,\"photoresistor\":<photoresistor>,\"trimmer\":<trimmer>,\"button1\":<button1>,\"button2\":<button2>,\"switch\":<switch>,\"relais\":<relais>,\"alarm\":<alarm>,\"temperaturesetpoint\":<temperaturesetpoint>,\"humiditysetpoint\":<humiditysetpoint>,\"pressuresetpoint\":<pressuresetpoint>,\"trimmersetpoint\":<trimmersetpoint>,\"photoresistorsetpoint\":<photoresistorsetpoint>}" ;


// storage for last published values 
double lastTemp = -999999 ;  
double lastExtTemp = -99999 ;
int lastHumi = -999999 ;
double lastPressure = -99999 ;
int lastPhotoRes = -999999 ;
int lastTrimmer = -999999 ;
int lastTimeRangeIndex = -1 ;
int lastExpireDateIndex = -1 ;

// setpoints
float TempSetPoint = -271 ;
int HumiSetPoint = -1 ;
int PressureSetPoint = -1 ;
int TrimmerSetPoint = -1 ;
int LightSetPoint = -1 ;

float TempSetPoint_prev = -271 ;
int HumiSetPoint_prev = -1 ;
int PressureSetPoint_prev = -1 ;
int TrimmerSetPoint_prev = -1 ;
int LightSetPoint_prev = -1 ;


// counter for debug function
int l=0;
int i=0;

int timeZone = 1 ;

// prevent fast change of relais 
int lastRelaisSetup = 0 ;

// when relais is set in manual mode it works regardless setpoint
int relaisIsInManualMode = 0 ;

int timeRangeOn = 1;

int debugVar = -1 ;


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


int workMessage(String message){
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
        
    } else if (command.equalsIgnoreCase("getuiconfig")){
        return sendUIConfig();
        
    } else if (command.equalsIgnoreCase("temperaturesetpoint")){
        TempSetPoint = argument.toFloat();
        if (TempSetPoint != TempSetPoint_prev) sendVariableChanged("temperaturesetpoint",String(TempSetPoint)) ;
        TempSetPoint_prev = TempSetPoint ;
        return 0;
        
    } else if (command.equalsIgnoreCase("humiditysetpoint")){
        HumiSetPoint = argument.toFloat();
        if (HumiSetPoint != HumiSetPoint_prev) sendVariableChanged("humiditysetpoint",String(HumiSetPoint)) ;
        HumiSetPoint_prev = HumiSetPoint ;
        return 0;
        
    } else if (command.equalsIgnoreCase("pressuresetpoint")){
        PressureSetPoint = argument.toFloat();
        if (PressureSetPoint != PressureSetPoint_prev) sendVariableChanged("pressuresetpoint",String(PressureSetPoint)) ;
        PressureSetPoint_prev = PressureSetPoint ;
        return 0;
        
    } else if (command.equalsIgnoreCase("trimmersetpoint")){
        TrimmerSetPoint = argument.toFloat();
        if (TrimmerSetPoint != TrimmerSetPoint_prev) sendVariableChanged("trimmersetpoint",String(TrimmerSetPoint)) ;
        TrimmerSetPoint_prev = TrimmerSetPoint ;
        return 0;
        
    } else if (command.equalsIgnoreCase("photoresistorsetpoint")){
        LightSetPoint = argument.toFloat();
        if (LightSetPoint != LightSetPoint_prev) sendVariableChanged("photoresistorsetpoint",String(LightSetPoint)) ;
        LightSetPoint_prev = LightSetPoint ;
        return 0;
        
    } else if (command.equalsIgnoreCase("timerangeon")){
        timeRangeOn = argument.toInt();
        sendVariableChanged("timerangeon",String(timeRangeOn)) ;
        return 0;
        
    } else if (command.equalsIgnoreCase("relaisIsInManualMode")){
        relaisIsInManualMode = argument.toInt();
        sendVariableChanged("relaisIsInManualMode",String(relaisIsInManualMode)) ;
        return 0;
        
    } else if (command.equalsIgnoreCase("reset")){
        System.reset();
        return 0;
        
    } else {
        return -2 ;                         // command not found
    }
}

void setup(){
    
    sendDebug(0) ;
    
    #ifdef DS18B20
    dallas.begin();
    #endif
    
    BaroSensor.begin();
    
    preparePerMinuteTimeLine();
    
    Particle.variable("status", status) ;
    Particle.variable("stats",stats) ;
    Particle.variable("message_help", api_help) ;
    Particle.variable("debug",debugVar) ;
    
    Particle.function("message", workMessage);      // see workMessage function 
    Particle.function("setTimeRange",setTimeRange); // accept a string to set the desiderd time range
    Particle.function("setExpDate", setExpireDate); // accept a number of milliseconds for releais on
    
    
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
    
    if (alarm > 0){
        sound(1000,250);
        alarm-- ;
    }
    
    actualTimeRangeIndex = getTimeRangeForNow();
    if (actualTimeRangeIndex != lastTimeRangeIndex){
        String actualTimeRange = actualTimeRangeIndex > -1 ? timeRanges[actualTimeRangeIndex] : "NO ACTIVE TIME RANGE" ;
        sendVariableChanged("timerange",actualTimeRange) ;
        lastTimeRangeIndex = actualTimeRangeIndex ;
    }

    actualExpireDateIndex = getExpireDateForNow();
    if (actualExpireDateIndex != lastExpireDateIndex){
        String actualExpireDate = actualExpireDateIndex > -1 ? expireDates[actualExpireDateIndex] : "NO EXPIRE DATE" ;
        sendVariableChanged("expiredate",actualExpireDate) ;
        lastExpireDateIndex = actualExpireDateIndex ;
    }
    
    i++ ;
    if (i > DEBUG_INTERVAL_SECS){
        i = 0;
        l++ ;
        sendDebug(l) ;
    }
    
    double delta = 0;
    // mean temperature over 20 samples (20 seconds if loop is of one second)
    temperatureSampleCount++ ;
    if (temperatureSampleCount > 20) {
        temperatureSampleCount = 1 ;
        temperatureSampleAcc = 0 ;
    }
    temperatureSampleAcc += temperatureRead() + TEMP_ADJUST_OFFSET ;
    temperature = round(temperatureSampleAcc / temperatureSampleCount * 10.0)/10.0 ;
    delta = lastTemp - temperature ;
    if (delta < 0) delta = delta * -1 ;
    if (delta >= tempDeltaEvent) {
        lastTemp = temperature ;
        sendVariableChanged("temperature",String(temperature)) ;
    }
    
    pressure = round(BaroSensor.getPressure(OSR_8192)*10.0)/10.0 ;
    delta = lastPressure - pressure ;
    if (delta < 0) delta = delta * -1 ;
    if (delta >= pressureDeltaEvent){
        lastPressure = pressure ;
        sendVariableChanged("pressure",String(pressure)) ;
    }
    
    // external Temperature
    #ifdef DS18B20
    float celsius = readCelsiusFromExternalSensor() ;
    if (celsius != -127){ // no temp received
        extTemperature = celsius ;
    }
    #else
        extTemperature = BaroSensor.getTemperature() + TEMP_ADJUST_OFFSET;
        extTemperature = round(extTemperature*10)/10 ;
    #endif
    
    
    delta = lastExtTemp - extTemperature ;
    if (delta < 0) delta = delta * -1 ;
    if (delta >= tempDeltaEvent) {
        lastExtTemp = extTemperature ;
        sendVariableChanged("extTemperature",String(extTemperature)) ;
    }
    

    humidity = humidityRead() ;
    delta = lastHumi - humidity ;
    if (abs(delta) > humiDeltaEvent) {
        lastHumi = humidity ;
        sendVariableChanged("humidity",String(humidity)) ;
    }
    

    photoresistor = analogRead(A1) ;
    if (abs(lastPhotoRes - photoresistor) >= photoDeltaEvent) {
        lastPhotoRes = photoresistor;
        sendVariableChanged("photoresistor",String(photoresistor)) ;
    }
    
    
    trimmer = analogRead(A2) ;
    if (abs(lastTrimmer - trimmer) >= trimmerDeltaEvent) {
        lastTrimmer = trimmer;
        sendVariableChanged("trimmer",String(trimmer)) ;
    }
    
    int lastSwitch = switch0 ;
    switch0 = digitalRead(SWITCH) == HIGH ;
    if (lastSwitch != switch0) sendVariableChanged("switch", switch0 ? "1" : "0") ;
    
    int lastRelais = relais ;
    relais = digitalRead(RELAIS_FDB) == HIGH ;
    if (lastRelais != relais) sendVariableChanged("relais", relais ? "1" : "0") ;
    

    //if(digitalRead(SWITCH)==LOW) sound(500,340) ;
    
    if (digitalRead(PULSANTE_1)==LOW){
        sound(1000,170) ;
        button1 = !button1 ;
        sendVariableChanged("button1", button1 ? "1" : "0");
    }
    
    if (digitalRead(PULSANTE_2)==LOW){
        sound(1000,170) ;
        button2 = !button2 ;
        sendVariableChanged("button2", button2 ? "1" : "0");
    }
    
    if (actualExpireDateIndex > -1){
        // priority to ExpireDate on time range
        workExpireDate() ;
    } else {
        workTimeRange() ;
    }
    
    
    if (!relaisIsInManualMode){
    // no changes to relais upon setpoint values, for less of 30 seconds (avoid fast on/off commuting to protect the attached utility)
        unsigned long now = millis() ;
        unsigned long deltaT = (now - lastRelaisSetup)/1000 ;
        if (deltaT > RELAIS_MIN_PERIOD){
            lastRelaisSetup = now ;
            #ifdef DS18B20
            if (extTemperature < TempSetPoint || humidity < HumiSetPoint || pressure < PressureSetPoint || trimmer < TrimmerSetPoint || photoresistor < LightSetPoint) {
            #else
            if (temperature < TempSetPoint || humidity < HumiSetPoint || pressure < PressureSetPoint || trimmer < TrimmerSetPoint || photoresistor < LightSetPoint) {
            #endif
                setRelaisOn(false) ; // if the master switch is off don't set the relais to on
            } else {
                setRelaisOff() ;
            }
        }        
    }
    

    status = String(status_template) ; // copying ;

    status.replace("<expiredate>",actualExpireDateIndex > -1 ? expireDates[actualExpireDateIndex] : "NO EXPIRE DATE");
    status.replace("<timerange>",actualTimeRangeIndex > -1 ? timeRanges[actualTimeRangeIndex] : "NO ACTIVE TIME RANGE");
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
    status.replace("<temperaturesetpoint>",String(TempSetPoint));
    status.replace("<humiditysetpoint>",String(HumiSetPoint));
    status.replace("<pressuresetpoint>",String(PressureSetPoint));
    status.replace("<trimmersetpoint>",String(TrimmerSetPoint));
    status.replace("<photoresistorsetpoint>",String(LightSetPoint));
    status.replace("<relaisIsInManualMode>",String(relaisIsInManualMode));
    status.replace("<timerangeon>",String(timeRangeOn));

    
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

    updateTimeLinePerMinute() ;
    
    delay(LOOP_DELAY); 
}


// data logging section
// we prepare a matrix for datalogging
// rows are timeline
// columns are different metrics: temperature, relais status, humidiy, ...

#define TIMELINE_PERMINUTE_SIZE 60

#define TIMELINE_TEMPERATURE_INDEX 0
#define TIMELINE_RELAIS_INDEX 1

#define TIMELINE_INDEX_SIZE 2

int perMinuteTimeLine[TIMELINE_PERMINUTE_SIZE][TIMELINE_INDEX_SIZE] = {} ;
int perMinuteCounter = 0 ;
int lastMinute = -1 ;
int perMinuteTimeLinePosition = -1 ;

void preparePerMinuteTimeLine(){
    for (int r=0;r<TIMELINE_PERMINUTE_SIZE;r++){
        for (int i=0;i<TIMELINE_INDEX_SIZE;i++){
            perMinuteTimeLine[r][i] = 0 ;
        }
    }
}

int calcMeanValue(int actualValue, int newValue, int meanCounter){
    double meanValue = (actualValue * meanCounter + newValue) / (meanCounter+1);
    return round(meanValue) ;
}

template <size_t N, size_t M>
void shiftBackValues(int (&array)[M][N],int maxrows){
    for (int row=0;row<maxrows-1;row++){
        for (int col=0;col<TIMELINE_INDEX_SIZE;col++){
            array[row][col] = array[row+1][col] ;
        }
    }
}

template <size_t N, size_t M>
void updateCell(int (&array)[M][N], int row, int col, int newValue, int meanCounter){
    int actualValue = array[row][col] ;
    array[row][col] = calcMeanValue(actualValue,newValue,meanCounter);
}

void updateTimeLinePerMinute(){
    // selecting witch position in the timeline we need to update.
    // we advance one position every minute
    // arrived at the end of the timeline (TIMELINE_PERMINUTE_SIZE) 
    // we will start to shift all the values one position back, to preserve last N values
    
    unsigned long now = Time.now() ;
    int minute = Time.minute(now) ;
    
    // if the actual minute is different from the last tracked minute we advance of one position in the time line 
    if (minute != lastMinute){
        perMinuteTimeLinePosition++ ;
        perMinuteCounter = 0 ;
    } 
    
    if (perMinuteTimeLinePosition >= TIMELINE_PERMINUTE_SIZE){
        shiftBackValues(perMinuteTimeLine,TIMELINE_PERMINUTE_SIZE); // shift all timeline values back one position
        perMinuteTimeLinePosition = TIMELINE_PERMINUTE_SIZE-1 ;     // returning to last timeline position
    }
    
    int newValue = 0 ;
    
    newValue = round(temperature*10) ; // using 123 for storing 12.3
    updateCell(perMinuteTimeLine,perMinuteTimeLinePosition,TIMELINE_TEMPERATURE_INDEX,newValue,perMinuteCounter) ;
    
    newValue = round(relais*10) ; // using 10 for storing 1, we will can see 0.5 as 5 and so on
    updateCell(perMinuteTimeLine,perMinuteTimeLinePosition,TIMELINE_RELAIS_INDEX,newValue,perMinuteCounter) ;
    
    perMinuteCounter++ ;
}

int daysInMonthOfYear(int month, int year){
    int months[12] = {
        31,
        28,
        31,
        30,
        31,
        30,
        31,
        31,
        30,
        31,
        30,
        31
    };
    bool isLeapYear = (year/4) != round(year/4) ; 
    if (month == 2 && isLeapYear) return 29 ;
    return months[month] ;
}

long unsigned millisAtDate(String atDate){

    // 2015-06-30|08:00:00
    // 0123456789012345678
    //           1       

    int millisInSecond = 1000 ;
    int millisInMinute = 60 * millisInSecond ;
    int millisInHour = 60 * millisInMinute ;
    int millisInDay = 24 * millisInHour ;

    int year = atDate.substring(0,4).toInt();
    int month = atDate.substring(5,7).toInt();
    int day = atDate.substring(8,10).toInt();
    int hour = atDate.substring(11,13).toInt();
    int minute = atDate.substring(14,16).toInt();
    int second = atDate.substring(17,19).toInt();

    bool isLeapYear = (year/4) != round(year/4) ; 

    long unsigned int totDays = ((year - 1) * 365 + isLeapYear ? 1 : 0) + daysInMonthOfYear(month-1,year) + day - 1 ;
    long unsigned int millisAt = totDays * millisInDay + hour * millisInHour + minute * millisInMinute + second * millisInSecond ;

    return millisAt ;
}

bool isInExpireDate(unsigned long now, String eDate){
    /*
        eDate example: "0:2015-06-30|08:00:00|2015-07-30|16:00:00|T=22"
                        01234567890123456789012345678901234567890123456
                                  1         2         3         4
    */
    // calculating milliseconds from
    long unsigned millisFrom = millisAtDate(eDate.substring(2,21)) ;
    long unsigned millisTo = millisAtDate(eDate.substring(22,41)) ;

    bool yesItIs = (millisFrom <= now) && (now <= millisTo) ;

    return yesItIs ;

}

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
    if (hourNow > 23) hourNow = 0 ;
    int minuteNow = Time.minute(now) ;
                                                            
    int hourMinuteNow = minuteNow + hourNow * 60 ;          

    if (hourMinuteFrom > hourMinuteTo){
        if (hourMinuteNow > hourMinuteTo){
            int midNight = 24*60 ; 
            yesItIs = (hourMinuteFrom <= hourMinuteNow) && (hourMinuteNow <= midNight) ;
        } else {
            yesItIs = (0 <= hourMinuteNow) && (hourMinuteNow <= hourMinuteTo) ;
        }
    } else {
        yesItIs = (hourMinuteFrom <= hourMinuteNow) && (hourMinuteNow <= hourMinuteTo) ;
    }

    return yesItIs ;
}

int getTimeRangeForNow(){
    
    if (timeRangeOn == 0) return -3 ; // no local time range but command only from the cloud
    
    unsigned long now = Time.now() ;
    int res = -1 ;
    for (int ii=0;ii<TIMERANGEARRAYSIZE;ii++){
        if (!timeRanges[ii].equals("")){
            if (isInTimeRange(now,timeRanges[ii])){
                res = ii ;
            }
        }
    }
    return res ;
}

int getExpireDateForNow(){
    
    if (timeRangeOn == 0) return -3 ; // no local expire date but command only from the cloud
    
    unsigned long now = Time.now() ;
    int res = -1 ;
    for (int ii=0;ii<TIMERANGEARRAYSIZE;ii++){
        if (!expireDates[ii].equals("")){
            if (isInExpireDate(now,expireDates[ii])){
                res = ii ;
            }
        }
    }
    return res ;
}

int setTimeRange(String timeRange){
    int index = timeRange.substring(0,1).toInt() ;
    if (index >= TIMERANGEARRAYSIZE){
        Particle.publish("error: timerange overflow",String(index),60,PRIVATE) ;
        return -1;
    }
    timeRanges[index] = timeRange ;
    return index ;
}

int setExpireDate(String expireDate){
    int index = expireDate.substring(0,1).toInt() ;
    if (index >= TIMERANGEARRAYSIZE){
        Particle.publish("error: expire date overflow",String(index),60,PRIVATE) ;
        return -1;
    }
    expireDates[index] = expireDate ;
    return index ;
}

void workTimeRange(){
    if (actualTimeRangeIndex < 0 ) return ; // no time range active
    String actualTimeRange = timeRanges[actualTimeRangeIndex] ;
    char c = actualTimeRange.charAt(35) ;
    switch (c)
    {
        
        case 'T': { // temp
            TempSetPoint = actualTimeRange.substring(37).toFloat() ;
            if (TempSetPoint != TempSetPoint_prev) sendVariableChanged("temperaturesetpoint",String(TempSetPoint)) ;
            TempSetPoint_prev = TempSetPoint ;
            break;
        }
        
        case 'H': { // humidity
            HumiSetPoint = actualTimeRange.substring(37).toInt() ;
            if (HumiSetPoint != HumiSetPoint_prev) sendVariableChanged("humiditysetpoint",String(HumiSetPoint)) ;
            HumiSetPoint_prev = HumiSetPoint ;
            break;
        }
        
        case 'P': { // pressure
            PressureSetPoint = actualTimeRange.substring(37).toInt() ;
            if (PressureSetPoint != PressureSetPoint_prev) sendVariableChanged("pressuresetpoint",String(PressureSetPoint)) ;
            PressureSetPoint_prev = PressureSetPoint ;
            break;
        }

        case 'R': { // trimmer
            TrimmerSetPoint = actualTimeRange.substring(37).toInt() ;
            if (TrimmerSetPoint != TrimmerSetPoint_prev) sendVariableChanged("trimmersetpoint",String(TrimmerSetPoint)) ;
            TrimmerSetPoint_prev = TrimmerSetPoint ;
            break;
        }

        case 'L': { // PhotoResistor
            LightSetPoint = actualTimeRange.substring(37).toInt() ;
            if (LightSetPoint != LightSetPoint_prev) sendVariableChanged("photoresistorsetpoint",String(LightSetPoint)) ;
            LightSetPoint_prev = LightSetPoint ;
            break;
        }
        
        default: {
            Particle.publish("error:unknown setpoint in timeRange",actualTimeRange,60,PRIVATE) ;
        }
    }
}

void workExpireDate(){
    if (actualExpireDateIndex < 0 ) return ; // no expire date active
    String actualExpireDate = expireDates[actualExpireDateIndex] ;
    char c = actualExpireDate.charAt(42) ;
    switch (c)
    {
        
        case 'T': { // temp
            float TempSetPoint = actualExpireDate.substring(44).toFloat() ;
            break;
        }
        
        case 'H': { // humidity
            int HumiSetPoint = actualExpireDate.substring(44).toInt() ;
            break;
        }
        
        case 'P': { // pressure
            int PressuerSetPoint = actualExpireDate.substring(44).toInt() ;
            break;
        }

        case 'R': { // 
            int TrimmerSetPoint = actualExpireDate.substring(44).toInt() ;
            break;
        }

        case 'L': { // PhotoResistor
            int LightSetPoint = actualExpireDate.substring(44).toInt() ;
            break;
        }
        
        case 'S': { // Switch
            int SwitchSetPoint = actualExpireDate.substring(44).toInt() ;
            break;
        }

        case 'A': { // button1
            int Button1SetPoint = actualExpireDate.substring(44).toInt() ;
            break;
        }

        case 'B': { // button2
            int Button2SetPoint = actualExpireDate.substring(44).toInt() ;
            break;
        }
        default: {
            Particle.publish("error:unknown setpoint in expireDate",actualExpireDate,60,PRIVATE) ;
        }
    }
}


void sendDebug(int count){
        String debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(count) + "," +WiFi.SSID() + "," + WiFi.RSSI() ;
        Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
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
    sendVariableChanged("alarm",String(alarm)) ;
    return alarm ;
}

/*

    Accept this command:
    on : relais on, regardless timerange or expire date
    off: relais off, regardless timerange or expire date
    
    auto: rules from timerange or expire date

*/

void setRelaisOn(bool force){
    // master switch used to block relais
    if (!force){
        if (switch0==LOW) return ;
    }
    digitalWrite(RELAIS_SET, HIGH);
}

void setRelaisOff(){
    digitalWrite(RELAIS_SET, LOW);
}

/*
    if relaisIsInManuaMode setpoint will be ignored.
    
    on : relais on regardless setpoint
    off : relais off regardless setpoint
    auto: relais on or off if setpoint is over or under the measured value
    
*/
int setrelais(String command){
    int cmd = LOW ;
    if (command.equalsIgnoreCase("on"))  {
        relaisIsInManualMode = 1 ;
        cmd = HIGH ;
        setRelaisOn(true) ;
    } else if (command.equalsIgnoreCase("off")){
        relaisIsInManualMode = 1 ;
        setRelaisOff() ;
    } else if (command.equalsIgnoreCase("auto")){
        relaisIsInManualMode = 0 ;
    } else {
        relaisIsInManualMode = 1 ;
        cmd = command.toInt() ;
        setRelaisOn(true) ;
        delay(cmd) ;
        setRelaisOff() ;
        relaisIsInManualMode = 0 ;
    }
    sendVariableChanged("relaisIsInManualMode",String(relaisIsInManualMode));
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

float readCelsiusFromExternalSensor(){
    #ifdef DS18B20
    dallas.requestTemperatures();
    float celsius = dallas.getTempCByIndex( 0 );
    #else
    float celsius = -999 ;
    #endif
    return celsius ;
}

void sendVariableChanged(String name, String value){
    Particle.publish("variableChanged",name+":"+value,60,PRIVATE) ;
}
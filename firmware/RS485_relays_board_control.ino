/*

    Techmakers RS485 Relays boord control
    Binary RS485 protocol
    Baud rate: 9600 baud

*/

#define PTT_RS485   A5
#define RX_RS485    A4
#define ON 1
#define OFF 0

void setup()
{
    
    pinMode(RX_RS485, OUTPUT); 
    pinMode(PTT_RS485, OUTPUT); 
    
    Serial1.begin(9600);    
        
    digitalWrite(RX_RS485,LOW);         
    digitalWrite(PTT_RS485,LOW);
    

    // initialization: shutdown all relays of board "1" 
    for(int n=1;n<9;n++)
    {
        relays(1,n,OFF);
        delay(100);
    }

}
        
void loop()
{

    // lights all relays one by one
    for(int n=1;n<9;n++)
        {
            relays(1,n,ON);
            delay(800);
        }

    // shutdown all relays one by one
    for(int n=8;n>=1;n--)
        {
            relays(1,n,OFF);
            delay(800);            
        }
    

}

int relays(int ID_BOARD, int RELAYS, int COMMAND)
    {

            /*
            "ID_BOARD"  : manual setup by DIP_SWITCH on Techmakers RS485 Relays board (from 1 to 15)
            "RELAYS"        : relays position on the specified board (from 1 to 8) 
            "COMMAND"   : 0 -> OFF, 1 -> ON 
            ie: relays(1,4,ON); //  lights relays number 4 of board "1" 
            
        */

        int indirizzo_relays = ((ID_BOARD-1) * 8 ) + RELAYS;
        digitalWrite(PTT_RS485,HIGH);   // active the "PTT"  of the transceiver RS485
        Serial1.write(0xFF);    
        Serial1.write(indirizzo_relays);    
        Serial1.write(COMMAND);
        delay(5);   
        digitalWrite(PTT_RS485,LOW);    // releases the "PTT" of the transceiver RS485
        return 1;

    } 
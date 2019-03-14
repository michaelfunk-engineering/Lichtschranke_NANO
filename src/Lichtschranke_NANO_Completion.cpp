
 #include <avr/io.h>
 #include <util/delay.h>
 #include <SPI.h>
 #include <Arduino.h>

// 16MHz clock
//#define F_CPU 16000000UL

// Outputs, pin definitions
/* SCK Pin 13 - MOSI Pin 11 - SS Pin 10
*/
#define PIN_SCK                   PB5
#define PIN_MOSI                  PB3
#define PIN_SS1                   PB2
#define PIN_SS2                   PB1
#define PIN_SS3                   PB0
#define PIN_SS4                   PD2

#define ON                        1
#define OFF                       0

#define MAX7219_LOAD11             PORTB |= (1<<PIN_SS1)
#define MAX7219_LOAD10             PORTB &= ~(1<<PIN_SS1)

#define MAX7219_LOAD21             PORTB |= (1<<PIN_SS2)
#define MAX7219_LOAD20             PORTB &= ~(1<<PIN_SS2)

#define MAX7219_LOAD31             PORTB |= (1<<PIN_SS3)
#define MAX7219_LOAD30             PORTB &= ~(1<<PIN_SS3)

#define MAX7219_LOAD41             PORTD |= (1<<PIN_SS4)
#define MAX7219_LOAD40             PORTD &= ~(1<<PIN_SS4)

#define MAX7219_MODE_DECODE       0x09
#define MAX7219_MODE_INTENSITY    0x0A
#define MAX7219_MODE_SCAN_LIMIT   0x0B
#define MAX7219_MODE_POWER        0x0C
#define MAX7219_MODE_TEST         0x0F
#define MAX7219_MODE_NOOP         0x00

volatile boolean decodeMode1 = false;
volatile boolean decodeMode2 = false;
volatile boolean decodeMode3 = false;
volatile boolean decodeMode4 = false;

// we use all 8 Digits
const char digitsInUse = 8;

#define MAX7219_DIGIT0            0x01
#define MAX7219_DIGIT1            0x02
#define MAX7219_DIGIT2            0x03
#define MAX7219_DIGIT3            0x04
#define MAX7219_DIGIT4            0x05
#define MAX7219_DIGIT5            0x06
#define MAX7219_DIGIT6            0x07
#define MAX7219_DIGIT7            0x08

#define MAX7219_CHAR_BLANK        0xF
#define MAX7219_CHAR_NEGATIVE     0xA

//Ansteuerung LED's mit Hilfe eines Encoders
/*Encoder braucht 3,3V Versorgung an +. GND auf GND.
*/
const int keyPin = 5; //MS
const int RoAPin = 4; //DT
const int RoBPin = 3; //CLK
//Andere Variablen für Encoder
int x = 0;
int y = 0;
int flag = 0;
//startWert für Encoder-Drehung
long count = 0;

//enterHits
long enterHits = 0;

//Variablen für Lichtschranke
int litPin = 12;


//fixed Variables
// 5 Letter per Row - Capital Letters
char A=0x77; char B=0x7F; char C=0x4E; char E=0x4F; char F=0x47;
char G=0x5E; char H=0x37; char I=0x06; char J=0x3C; char L=0x0E;
char O=0x7E; char P=0x67; char Q=0xFE; char R=0xF7; char S=0x5B;
char U=0x3E; char V=0x2A; char X=0xB7; char Y=0x27; char Z=0x6D;
// 5 Letter per Row - small Letters
char a=0x9D; char b=0x1F; char c=0x0D; char d=0x3D; char h=0x17;
char i=0x04; char j=0x38; char n=0x15; char o=0x1D; char q=0x73;
char r=0x05; char u=0x1C; char v=0x14;

char space=0x00;

const int charSetSize = 34;
const char charSet[charSetSize] = {A,B,C,E,F,G,H,I,J,L,O,P,Q,R,S,U,V,X,Y,Z,a,b,c,d,h,i,j,n,o,q,r,u,v,space};

//Words that are commonly used as 'String' of chars
char stringHALLO[5] = {H,A,L,L,O};
char stringFLO[3]   = {F,L,O};
char stringLASEr[5] = {L,A,S,E,r};
char stringCHECK[5] = {C,H,E,C,H};
char stringPLAYEr[6] = {P,L,A,Y,E,r};
char stringPLAYERS[7] = {P,L,A,Y,E,R,S};
char stringLOSEr[5] = {L,O,S,E,r};
char stringLOS[3] = {L,O,S};
char stringREADY[5] = {R,E,A,d,Y};

//Daten-Variablen fürs Programm
int players = 0;

char player0[digitsInUse]; //Name von Spieler 0
char player1[digitsInUse]; //Name von SPieler 1
char player2[digitsInUse]; //Name von Spieler 2
char player3[digitsInUse]; //Name von SPieler 3
char player4[digitsInUse]; //Name von Spieler 4
char player5[digitsInUse]; //Name von SPieler 5
char player6[digitsInUse]; //Name von Spieler 6
char player7[digitsInUse]; //Name von SPieler 7
char player8[digitsInUse]; //Name von Spieler 8
char player9[digitsInUse]; //Name von SPieler 9

long best0 = 11;
long best1 = 22;
long best2 = 33;
long best3 = 44;
long best4 = 55;
long best5 = 66;
long best6 = 77;
long best7 = 88;
long best8 = 99;
long best9 = 12345;

long dailyBest = 0;

volatile int index = 0;

volatile int buttonHits = 0;

//Num always positv, if you need the negative count too use count
volatile long num = 0;

//Functions
void spiSendByte (char databyte)
{
    // Copy data into the SPI data register
    SPDR = databyte;
    // Wait until transfer is complete
    while (!(SPSR & (1 << SPIF)));
}

void MAX7219_writeData(char data_register, char data)
{
    MAX7219_LOAD10;
    MAX7219_LOAD20;
    MAX7219_LOAD30;
    MAX7219_LOAD40;
        // Send the register where the data will be stored
        spiSendByte(data_register);
        // Send the data to be stored
        spiSendByte(data);
    MAX7219_LOAD11;
    MAX7219_LOAD21;
    MAX7219_LOAD31;
    MAX7219_LOAD41;
}//MAX7219_writeData

void MAX7219_writeDataSingle(long displaySelect, char data_register, char data)
{
  //Displays can be selected with integer from 1 to 4
  switch (displaySelect)
  {
    case 1:
      MAX7219_LOAD10;
          // Send the register where the data will be stored
          spiSendByte(data_register);
          // Send the data to be stored
          spiSendByte(data);
      MAX7219_LOAD11; break;

    case 2:
      MAX7219_LOAD20;
          // Send the register where the data will be stored
          spiSendByte(data_register);
          // Send the data to be stored
          spiSendByte(data);
      MAX7219_LOAD21; break;

    case 3:
      MAX7219_LOAD30;
          // Send the register where the data will be stored
          spiSendByte(data_register);
          // Send the data to be stored
          spiSendByte(data);
      MAX7219_LOAD31; break;

    case 4:
      MAX7219_LOAD40;
          // Send the register where the data will be stored
          spiSendByte(data_register);
          // Send the data to be stored
          spiSendByte(data);
      MAX7219_LOAD41; break;

    default:
    break;
  }
}//MAX7219_writeDataSingle

void MAX7219_clearDisplay()
{
  char i = digitsInUse;
    //put no decodeMode on for every Display, to clear all.
  MAX7219_writeData(MAX7219_MODE_DECODE, 0x00);
  decodeMode1=false;decodeMode2=false;decodeMode3=false;decodeMode4=false;

    // Loop until 0, but don't run for zero
    do {
        // Set each display in use to blank
        MAX7219_writeData(i, 0x00);
    } while (--i);
}//MAX7219_clearDisplay

void MAX7219_clearDisplaySingle(int displaySelect)
{
  char i = digitsInUse;

  switch (displaySelect) {
    case 1:
      if(decodeMode1){
        do {MAX7219_writeDataSingle(displaySelect, i, MAX7219_CHAR_BLANK);} while (--i);
      }
      if(!decodeMode1){
        do {MAX7219_writeDataSingle(displaySelect, i, 0x00);} while (--i);
      } break;

    case 2:
      if(decodeMode2){
        do {MAX7219_writeDataSingle(displaySelect, i, MAX7219_CHAR_BLANK);} while (--i);
      }
      if(!decodeMode2){
        do {MAX7219_writeDataSingle(displaySelect, i, 0x00);} while (--i);
      } break;

    case 3:
      if(decodeMode3){
        do {MAX7219_writeDataSingle(displaySelect, i, MAX7219_CHAR_BLANK);} while (--i);
      }
      if(!decodeMode3){
        do {MAX7219_writeDataSingle(displaySelect, i, 0x00);} while (--i);
      } break;

      case 4:
        if(decodeMode4){
          do {MAX7219_writeDataSingle(displaySelect, i, MAX7219_CHAR_BLANK);} while (--i);
        }
        if(!decodeMode4){
          do {MAX7219_writeDataSingle(displaySelect, i, 0x00);} while (--i);
        } break;

      default: break;
    }
}//MAX7219_clearDisplaySingle

void MAX7219_displayNumber(long displaySelect, volatile long number)
{
    switch (displaySelect) {
      case 1:
        if (!decodeMode1){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode1 = true; break;
      case 2:
        if (!decodeMode2){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode2 = true; break;
      case 3:
        if (!decodeMode3){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode3 = true; break;
      case 4:
        if (!decodeMode4){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode4 = true; break;
      default: break;
    }
    char negative = 0;
    // Convert negative to positive.
    // Keep a record that it was negative so we can
    // sign it again on the display.
    if (number < 0) {negative = 1; number *= -1;}
    MAX7219_clearDisplaySingle(displaySelect);
    // If number = 0, only show one zero then exit
    if (number == 0) {MAX7219_writeDataSingle(displaySelect, MAX7219_DIGIT0, 0); }
    // Initialization to 0 required in this case,
    // does not work without it. Not sure why.
    char i = 0x00;
    // Loop until number is 0.
    do {
        MAX7219_writeDataSingle(displaySelect, ++i, number % 10);
        // Actually divide by 10 now.
        number /= 10;
    } while (number);
    // Bear in mind that if you only have three digits, and
    // try to display something like "-256" all that will display
    // will be "256" because it needs an extra fourth digit to
    // display the sign.
    if (negative) {
        MAX7219_writeDataSingle(displaySelect, (0x01+i), MAX7219_CHAR_NEGATIVE);
    }
}//MAX7219_displayNumber


//displayTime is like displayNumber but adds a dot for milliseconds
//and zeros to fit that "look"
void MAX7219_displayTime(long displaySelect, volatile long number)
{
    switch (displaySelect) {
      case 1:
        if (!decodeMode1){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode1 = true; break;
      case 2:
        if (!decodeMode2){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode2 = true; break;
      case 3:
        if (!decodeMode3){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode3 = true; break;
      case 4:
        if (!decodeMode4){MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0xFF);}
        decodeMode4 = true; break;
      default: break;
    }
    //calculate some Stuff
    char negative = 0;
    if (number < 0) {negative = 1; number *= -1;}

    MAX7219_clearDisplaySingle(displaySelect);
    // If number = 0, only show one zero then exit
    if (number == 0) {MAX7219_writeDataSingle(displaySelect, MAX7219_DIGIT0, 0); }
    // Initialization to 0 required in this case,
    // does not work without it. Not sure why.
    char i = 0x00;
    // Loop until number is 0.
    do {
        //add point at 3. Digit
        if(i==3){MAX7219_writeDataSingle(displaySelect, ++i, 0x80+(number % 10));}

        else{MAX7219_writeDataSingle(displaySelect, ++i, number % 10);}

        // Actually divide by 10 now.
        number /= 10;

    } while (number);
    // Bear in mind that if you only have three digits, and
    // try to display something like "-256" all that will display
    // will be "256" because it needs an extra fourth digit to
    // display the sign.
    if (negative) {
        MAX7219_writeDataSingle(displaySelect, (0x01+i), MAX7219_CHAR_NEGATIVE);
    }
}//MAX7219_displayTime


void MAX7219_displayLetter(long displaySelect, long digit , char letter)
{
  MAX7219_writeDataSingle(displaySelect, MAX7219_MODE_DECODE, 0x00);
  switch (displaySelect) {
    case 1: decodeMode1 = false; break;
    case 2: decodeMode2 = false; break;
    case 3: decodeMode3 = false; break;
    case 4: decodeMode4 = false; break;
    default: break;
  }

  if (digit > digitsInUse){digit = digitsInUse;}
  if (digit < 0){digit = 0;}

  char i = digitsInUse;

  MAX7219_writeDataSingle(displaySelect, (i-digit), letter);

}//MAX7219_displayLetter

void MAX7219_displayString(long displaySelect, char string[], long size)
{
  int i = 0;

  while(i < size)
  {
    MAX7219_displayLetter(displaySelect, i, string[i]);
    i++;
  }
  while(i < digitsInUse)
  {
    MAX7219_displayLetter(displaySelect, i, space);
    i++;
  }
}

boolean getEnter(){
    if(digitalRead(keyPin)){ return true; }
    else
    {return false;}
}

int getEncoderValue(){
   x = digitalRead(RoBPin);

   while (!digitalRead(RoAPin)) {
      y = digitalRead(RoBPin);
      flag = 1;
      }

    if (flag == 1) {
      flag = 0;
      if ((x == 0) && (y == 1)) {
        count ++;
      }
      if ((x == 1) && (y == 0)) {
        count --;
      }
    }

    if (count < 0) {count = charSetSize;}
    return count%charSetSize;
}//getEncoderValue

boolean getLit(){
  boolean lit = true;
  if(digitalRead(litPin))
    {lit = false;}

  return lit;
}


void setup() {
  //  setup code here, to run once:
Serial.begin(9600);

// SCK MOSI CS/LOAD/SS
DDRB |= (1 << PIN_SCK) | (1 << PIN_MOSI) | (1 << PIN_SS1) | (1 << PIN_SS2) | (1 << PIN_SS3);
DDRD |= (1 << PIN_SS4);
// SPI Enable, Master mode
SPCR |= (1 << SPE) | (1 << MSTR)| (1 << SPR1);

// Scan limit runs from 0
MAX7219_writeData(MAX7219_MODE_SCAN_LIMIT, (digitsInUse) - 1);
MAX7219_writeData(MAX7219_MODE_INTENSITY, 3);
MAX7219_writeData(MAX7219_MODE_POWER, ON);
Serial.begin(9600);

MAX7219_writeData(MAX7219_MODE_DECODE, 0x00);

MAX7219_clearDisplay();



//Initialisierung von den Encoder Pins
pinMode(RoAPin, INPUT);
pinMode(RoBPin, INPUT);
pinMode(keyPin, INPUT);

pinMode(litPin, INPUT);


boolean check = false;

//Zustand 0 - Lichtkalibrierung
MAX7219_displayString(1,stringLASEr,5);

  while (!check){
    if(getLit()){
      check = true;
    }
  }

MAX7219_displayString(2,stringCHECK,5);

long t0 = millis();

for (long i = 3; i > -1; i--) {
  MAX7219_displayNumber(3,i);
  while ( (millis()-t0) < 999 ) {}
  t0 = millis();
}
MAX7219_clearDisplay();


}//Setup

void loop() {
  // put your main code here, to run repeatedly

Serial.println(millis());


  // Q1: Spieler Anzahl wählen
  while (enterHits==0) {
    MAX7219_displayString(1,stringPLAYERS,7);


    MAX7219_displayNumber(2,getEncoderValue()%11);

    //ButtonHit Standard Routine
    if(digitalRead(keyPin)){enterHits++; players = (getEncoderValue()%11); while(digitalRead(keyPin)){} }
  }//Q1

  MAX7219_clearDisplay();

  //Q2 - Spielernamen Eingeben
  while(buttonHits < (8*players) && enterHits ==1){

    num = getEncoderValue();

    MAX7219_displayLetter(1, index, charSet[num%charSetSize]);
    MAX7219_displayString(3,stringPLAYEr,6);
    MAX7219_displayNumber(4, (buttonHits/8)+1);


    if(digitalRead(keyPin)){

      buttonHits++;

      if (buttonHits <= 8){player0[index] = charSet[num%charSetSize];}
      if (buttonHits > 8 && buttonHits <= 16){player1[index] = charSet[num%charSetSize];}
      if (buttonHits > 16 && buttonHits <= 24){player2[index] = charSet[num%charSetSize];}
      if (buttonHits > 24 && buttonHits <= 32){player3[index] = charSet[num%charSetSize];}
      if (buttonHits > 32 && buttonHits <= 40){player4[index] = charSet[num%charSetSize];}
      if (buttonHits > 40 && buttonHits <= 48){player5[index] = charSet[num%charSetSize];}
      if (buttonHits > 48 && buttonHits <= 56){player6[index] = charSet[num%charSetSize];}
      if (buttonHits > 56 && buttonHits <= 64){player7[index] = charSet[num%charSetSize];}
      if (buttonHits > 64 && buttonHits <= 72){player8[index] = charSet[num%charSetSize];}
      if (buttonHits > 72 && buttonHits <= 80){player9[index] = charSet[num%charSetSize];}

      index = (index + 1);
      if(index >= 8){index = 0; MAX7219_clearDisplaySingle(1);}

      while(digitalRead(keyPin)){}
    }

    if (buttonHits == (8*players)) {
      enterHits++;
      buttonHits = 0;
      MAX7219_displayLetter(2,4,P);
    }
}//Q2

MAX7219_clearDisplay();

//Q3 - Spielüberblick
while (enterHits==2) {

  //ButtonHit Standard Routine
  if(digitalRead(keyPin)){enterHits++; while(digitalRead(keyPin)){} }

  MAX7219_displayString(4, stringREADY, 5);
  MAX7219_displayTime(1,dailyBest);

  switch (getEncoderValue()%players){
    case 0: MAX7219_displayString(2, player0, 8); MAX7219_displayTime(3,best0); break;
    case 1: MAX7219_displayString(2, player1, 8); MAX7219_displayTime(3,best1); break;
    case 2: MAX7219_displayString(2, player2, 8); MAX7219_displayTime(3,best2); break;
    case 3: MAX7219_displayString(2, player3, 8); MAX7219_displayTime(3,best3); break;
    case 4: MAX7219_displayString(2, player4, 8); MAX7219_displayTime(3,best4); break;
    case 5: MAX7219_displayString(2, player5, 8); MAX7219_displayTime(3,best5); break;
    case 6: MAX7219_displayString(2, player6, 8); MAX7219_displayTime(3,best6); break;
    case 7: MAX7219_displayString(2, player7, 8); MAX7219_displayTime(3,best7); break;
    case 8: MAX7219_displayString(2, player8, 8); MAX7219_displayTime(3,best8); break;
    case 9: MAX7219_displayString(2, player9, 8); MAX7219_displayTime(3,best9); break;
    default: break;
  }

}//Q3

//Q4 - Zeitmessung pro SPieler




}//Loop

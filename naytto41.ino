// ver. 4.1 (beta)
// mokin naytto versio johon lisätty myös lampötila-anturi
// koodia korjattu 24.2.218: 10.0 lampotila, sekä lisätty tekstin naytto
// teksti: NDTr1234567, jossa r=rivi. odotetaan 11 riviä, sitten tulostus
// teksti poistuu 2 minuutin jälkeen tai kun painaa nappia

// 4.5.2019: koodiin lisätty lampotila funktioon reset jos lämpötila -127, ei kokeiltu tätä koodia

//01 3V3
//02 D1/Tx
//03 D0/Rx
//04 A0
//05 RESET
//06 D12/MISO
//07 D11/MOSI    disp
//08 D13/SCK     disp
//09 D10/SS/PWM  
//10 GND
//11 A5/SCL
//12 A4/SDA
//13 A3
//14 A2          nappi
//15 A1          liikesen
//16 D9/PWM      disp
//17 D6/PWM      disp
//18 D5/PWM      disp_led
//19 D3/PWM/int1 ds18b20
//20 D2/int0     disp

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <DallasTemperature.h>

char viesti[12],teksti[100];
int liiketta=0,liikeled=1,rivi,arvot[3];
int liike=0,valo=0,nappi=0,liikeherkkyys=9,tekstiOn=0;
int kontrasti=50,paluuvalikosta=0;
unsigned long aikaleima=0, aikaleima_t=0, aikaleimaTeksti=0, liike_aika, viive=0, tempCycle=1200000;

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 12 // 0.12 c tarkkuus
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

Adafruit_PCD8544 display = Adafruit_PCD8544(9, 2, 6);

static const unsigned char PROGMEM plus [] = {12,12,63,63,12,12};

static const unsigned char PROGMEM miinus [] = {12,12,12,12,12,12};

static const unsigned char PROGMEM fontti [][2][10] = {
{ {0x00, 0xF0, 0xFE, 0x0E, 0x06, 0x06, 0x06, 0xFE, 0xF8, 0x00}, {0x00, 0x07, 0x1F, 0x30, 0x30, 0x30, 0x38, 0x3F, 0x0F, 0x00}},
{ {0x00, 0x00, 0x18, 0x0C, 0x0C, 0xFE, 0xFE, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x30, 0x30, 0x30, 0x3F, 0x3F, 0x30, 0x00, 0x00}}, 
{ {0x00, 0x0C, 0x06, 0x06, 0x06, 0x86, 0xFE, 0x7C, 0x00, 0x00}, {0x00, 0x30, 0x3C, 0x3E, 0x37, 0x33, 0x31, 0x30, 0x30, 0x00}}, 
{ {0x00, 0x0C, 0x06, 0xC6, 0xC6, 0xEE, 0xFE, 0x3C, 0x18, 0x00}, {0x00, 0x18, 0x30, 0x30, 0x30, 0x31, 0x3B, 0x1F, 0x0E, 0x00}}, 
{ {0x00, 0x80, 0xF0, 0x78, 0x1C, 0x06, 0x06, 0xFE, 0xFE, 0x00}, {0x00, 0x07, 0x07, 0x06, 0x06, 0x06, 0x06, 0x3F, 0x3F, 0x00}}, 
{ {0x00, 0x7E, 0x7E, 0x66, 0x66, 0x66, 0xE6, 0xC6, 0x00, 0x00}, {0x00, 0x10, 0x30, 0x30, 0x30, 0x30, 0x30, 0x1F, 0x0F, 0x00}}, 
{ {0x00, 0xF0, 0xFC, 0x8C, 0xC6, 0xC6, 0xC6, 0xC6, 0x86, 0x00}, {0x00, 0x0F, 0x1F, 0x39, 0x30, 0x30, 0x30, 0x39, 0x1F, 0x00}}, 
{ {0x00, 0x06, 0x06, 0x06, 0x06, 0xC6, 0xF6, 0x7E, 0x1E, 0x00}, {0x00, 0x00, 0x30, 0x3C, 0x3F, 0x0F, 0x03, 0x00, 0x00, 0x00}}, 
{ {0x00, 0x38, 0xFC, 0xCE, 0x86, 0x86, 0xCE, 0xFC, 0x38, 0x00}, {0x00, 0x0F, 0x1F, 0x39, 0x30, 0x30, 0x39, 0x1F, 0x0F, 0x00}}, 
{ {0x00, 0x78, 0xFC, 0x86, 0x86, 0x86, 0x86, 0xFE, 0xFC, 0x00}, {0x00, 0x00, 0x30, 0x31, 0x31, 0x31, 0x31, 0x1F, 0x0F, 0x00}}, 
{ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
};

const unsigned char PROGMEM tausta [][84] = {
{0x00, 0x00, 0x20, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0xFC, 0xFC, 0x3C, 0x78, 0xF0, 0xE0, 0xC0, 0x00, 0x00, 0xFC, 0xFC, 0x40, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

{0x00, 0x00, 0x0C, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x04, 0x00, 0x00, 0x01, 0x03, 0x0F, 0x3C, 0x3F, 0x3F, 0x03, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

{0x00, 0x00, 0xF8, 0xF8, 0x0C, 0x0C, 0x0C, 0x1C, 0x1C, 0x38, 0xF8, 0x00, 0x00, 0x60, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFC, 
0xFC, 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x0C, 0xFC, 0xFC, 0xFC, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

{0x00, 0x00, 0x03, 0x0F, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x1E, 0x38, 0x38, 0x1E, 0x1F, 
0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x3F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

{0x00, 0xFE, 0xFE, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0xFC, 0x9E, 0x8E, 0xFE, 0xF8, 0xC0, 0x00, 0x00, 0x00, 0xFE, 
0xFE, 0xF0, 0xB8, 0x3C, 0x1E, 0x0E, 0x06, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xC6, 0xC6, 0xC6, 0xC6, 0xC4, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

{0x00, 0x1F, 0x1F, 0x18, 0x18, 0x18, 0x00, 0x00, 0x1F, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x1C, 0x00, 0x00, 0x1F, 
0x1F, 0x01, 0x03, 0x03, 0x07, 0x0E, 0x1E, 0x18, 0x00, 0x00, 0x1F, 0x1F, 0x18, 0x38, 0x38, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} 
};


void setup()   
{
  int merkkic;
  
  arvot[0]=999;arvot[1]=999;arvot[2]=999;
  
  liike_aika=0;
//  pinMode(3, OUTPUT); // liikeled
  pinMode(5, OUTPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  
  display.begin();
  display.setContrast(50);
  display.clearDisplay();
  
  pinMode(8, OUTPUT);    // initialize pin 8 to control the radio
  digitalWrite(8, HIGH); // enable the radio
  delay(1000);		        // allow the radio to startup
  Serial.begin(115200);  // set the baude rate of the RFu-328
//  Serial.begin(9600);

  delay(1000);
  Serial.print("aNDstarted--");
  delay(500);
  Serial.print("aNDstarted--");
  delay(500);
  Serial.print("aNDstarted--");
  delay(500);
  
  display.clearDisplay();
  display.setRotation(3);
  naytatausta();
  display.display();

  digitalWrite(5, HIGH);   
  delay(200);
  digitalWrite(5, LOW);   
  delay(200);
  digitalWrite(5, HIGH);   
  delay(200);
  digitalWrite(5, LOW);   
  
  // Start up sensor library
  sensors.begin();
}

void loop() 
{
  char merkki; 
  
  // jos millisekunttilaskuri on mennyt nollaan (n. 50 vrk)
  if (millis()<aikaleima)
  {
    aikaleima=0;
    aikaleima_t=0;
  }
  
  // jos tulee viesti, tulostetaan naytto
  if (Serial.available()>0) 
  { 
    merkki=Serial.read();
    if (merkki=='a')
    {
      {
        for (int i=0; i<11; i++)
        {
          delay(10);
          viesti[i]=Serial.read();
        }
        if ((viesti[0]=='N') && (viesti[1]=='D'))
        {
          if (viesti[2]=='X')
          {
            if (viesti[3]=='L') liikeherkkyys=viesti[4]-'0';            
          } else
          if (viesti[2]=='T') // nyt tulee teksti 
          {
            rivi=viesti[3]-'0';
            for (int lp=0; lp<7; lp++) teksti[rivi*7+lp]=viesti[4+lp];
            teksti[(rivi+1)*7]='\0';
            if (rivi==11) tulostaTeksti();
          } else
          {
            arvot[0]=((viesti[2]-'0')*100) + ((viesti[3]-'0')*10) + (viesti[4]-'0');
            arvot[1]=((viesti[5]-'0')*100) + ((viesti[6]-'0')*10) + (viesti[7]-'0');
            arvot[2]=((viesti[8]-'0')*100) + ((viesti[9]-'0')*10) + (viesti[10]-'0');
            if (tekstiOn==0) // jos näytöllä on viesti, älä tulosta lämötiloja
            {
              display.clearDisplay();
              display.setRotation(3);
              naytatausta();
              naytalampotila(arvot[0],1);
              naytalampotila(arvot[1],2);
              naytalampotila(arvot[2],3);
              aikaleima=millis();
              viive=0;
            }
          }
        }
      }
    }
  }

  // tuleeko liikeanturilta signaali
  if (digitalRead(A1))
  {
    if (liike==0) liikeviesti();
    liike=1;
  } else liike=0;

//  if (liikeled) digitalWrite(3,liike);
  
  // onko nappi painettuna
  if (digitalRead(A2))
  {
    if ((nappi>0) && (paluuvalikosta==0))
    {
      if (tekstiOn) // näytössä on viesti, kuitataan pois
      {
        display.clearDisplay();
        display.setRotation(3);
        naytatausta();
        naytalampotila(arvot[0],1);
        naytalampotila(arvot[1],2);
        naytalampotila(arvot[2],3);
        tekstiOn=0;
      } else
      {
        valo=!valo;
        digitalWrite(5,valo);
      }
    }
    nappi=0; paluuvalikosta=0;
  }
  else
  {
    nappi++;
    if (nappi>40)
    {
      nappi=0; paluuvalikosta=1;
      nappipainettu();
    }
  }
  
  // nayttoon virheviesti jos viestiä ei ole tullut pitkään aikaan
  if ((millis()-aikaleima)>((viive+1)*60*1000))
  {
    viive++;
    if (viive>20)
    {
      display.clearDisplay();
      display.setRotation(2);
      display.println("");
      display.println("NoSignal");
      display.print(viive);
      display.println(" minutes");
      display.display();
    }
  }
  
  // 20min välien lähetetään lampotilaviesti
  if (((millis()-aikaleima_t)>tempCycle) || (aikaleima_t==0))
  {
    aikaleima_t=millis();
    lampotila();
  }
  
  // tekstiviesti poistetaan näytöltä 3 minuutin päästi
  if (tekstiOn==1)
  {
    if ((millis()-aikaleimaTeksti)>120000)
    {
      display.clearDisplay();
      display.setRotation(3);
      naytatausta();
      naytalampotila(arvot[0],1);
      naytalampotila(arvot[1],2);
      naytalampotila(arvot[2],3);
      tekstiOn=0;
    }
  }

  delay (50);
}

void tulostaTeksti()
{
  display.clearDisplay();
  display.setRotation(2);
//  teksti[70]='\0';
  display.print(teksti);
//  display.println("koe");
  display.display();
//  delay (10000);
  tekstiOn=1;
  aikaleimaTeksti=millis();
}


void lampotila()
{
  char lviesti[13];
  float lampo;
  int luku,siirtyminen=0;
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  lampo=sensors.getTempCByIndex(0); // Why "byIndex"? You can have more than one IC on the same bus.  
//  lampo=lampo-40;
//  Serial.print(lampo);
//  Serial.print("\n");
  lviesti[0]='a';lviesti[1]='N';lviesti[2]='D'; lviesti[3]='T'; lviesti[4]='M'; lviesti[5]='P'; lviesti[6]='A'; 

  if (lampo<-120)
  {
    WDTCR=0x18; 
    WDTCR=0x08; 
    #asm("wdr") 
    while(1); 
  }

  if (lampo<0) 
  { 
    lviesti[7]='-'; 
    siirtyminen=1;
    lampo=-1*lampo; 
  }
  if ((lampo>10) || (lampo=10)) // tämä korjattu
  {
    luku=(int)(lampo/10);
    lampo=lampo-luku*10;
    lviesti[7+siirtyminen]='0'+luku;
    siirtyminen++;
  } 
  
  luku=(int)lampo; 
  lampo=(lampo-luku)*10;
  lviesti[7+siirtyminen]='0'+luku;
  
  lviesti[8+siirtyminen]='.'; 

  luku=(int)(lampo);
  lampo=(lampo-luku)*10;
  lviesti[9+siirtyminen]='0'+luku; 
  
  if (siirtyminen<2)
  {
    luku=(int)(lampo);
    lampo=(lampo-luku)*10;
    lviesti[10+siirtyminen]='0'+luku; 
  }  
  
  if (siirtyminen<1)
  {
    luku=(int)(lampo);
    lviesti[11+siirtyminen]='0'+luku; 
  }  

  lviesti[12]='\0';
  Serial.print(lviesti);
  delay(300);
}

void nappipainettu()
{
  int kierros=0;
  char lviesti[13],sviesti[13],merkki;
  int lopetus=0, valikko=1, nappic=0;

  lviesti[0]='a';lviesti[1]='N';lviesti[2]='D'; lviesti[3]='V'; lviesti[5]='-'; lviesti[6]='-';
  lviesti[7]='-'; lviesti[8]='-'; lviesti[9]='-'; lviesti[10]='-'; lviesti[11]='-'; lviesti[12]='\0';
  
  naytavalikko(valikko);
  do { delay(10); } while (!digitalRead(A2));
  do
  {
    if (digitalRead(A2))
    {
      if (nappic>0)
      {
        valikko++; 
        if (valikko==5) valikko=1;
        nappic=0;
        naytavalikko(valikko);
      }
    }
    else 
    { 
      nappic++;
      if (nappic>200)
      {
        lopetus=1;
        nappic=0;
      }
    }
    delay(10);
  } while (lopetus==0);
  
  if (valikko!=4)
  {
    display.clearDisplay();
    display.println(" ");
    display.println(" ");
    display.println("Viisari:");
    display.println(" ");
    if (valikko==1) display.println(" Ulkona");
    if (valikko==2) display.println(" Sisalla");
    if (valikko==3) display.println(" Jarvessa");
    display.display();
    delay (1000);
    if (valikko==1) lviesti[4]='U';
    if (valikko==2) lviesti[4]='S';
    if (valikko==3) lviesti[4]='J';
//    Serial.print(lviesti);
//    delay(500);
//    Serial.print(lviesti);
//    delay(500);
    Serial.print(lviesti);
    
    lopetus=0;
    do
    {
      if (Serial.available()>0) 
      { 
        merkki=Serial.read();
        if (merkki=='a')
        {
          for (int i=0; i<11; i++)
          {
            delay(10);
            sviesti[i]=Serial.read();
          }
          if ((sviesti[0]=='N') && (sviesti[1]=='D') && (sviesti[2]=='X') && (sviesti[3]=='V')) lopetus=1;
        }  
      } else { delay(10); kierros++; }
    } while ((kierros<200) && (lopetus==0));
    
    if (lopetus)  display.println(" CONFIRMED"); else  display.println(" ERROR");
    display.display();
    delay(5000);
  } else naytakontrasti();
  
  display.clearDisplay();
  display.setRotation(3);
  naytatausta();
  naytalampotila(arvot[0],1);
  naytalampotila(arvot[1],2);
  naytalampotila(arvot[2],3);
  display.display();
}

void naytakontrasti()
{
  int nappik=0,lopetusk=0;

  display.setTextColor(BLACK, WHITE); 
  display.clearDisplay(); display.println(" ");  display.print("kontrasti:"); display.print(kontrasti);  display.display();
  
  do { delay(10); } while (!digitalRead(A2));
  do
  {
    if (digitalRead(A2))
    {
      if (nappik>0)
      {
        kontrasti=kontrasti+5;
        if (kontrasti==80) kontrasti=30; 
        nappik=0;
        display.setContrast(kontrasti);
        display.clearDisplay(); display.println(" ");  display.print("kontrasti:"); display.print(kontrasti);  display.display();
      }
    }
    else 
    { 
      nappik++;
      if (nappik>200)
      {
        lopetusk=1;
        nappik=0;
      }
    }
    delay(10);
  } while (lopetusk==0);
  naytaliikeled();  
}

void naytaliikeled()
{
  int nappil=0,lopetusl=0;

  display.clearDisplay();
  display.setRotation(2);
  display.setTextColor(BLACK, WHITE); 
  display.println(" Liike LED");
  display.println(" ");
  if (liikeled) display.println(" ON"); else display.println(" OFF");
  display.display();
  do { delay(10); } while (!digitalRead(A2));
  
  do
  {
    if (digitalRead(A2))
    {
      if (nappil>0)
      {
        liikeled=!liikeled;
        nappil=0;
        display.clearDisplay();
        display.setRotation(2);
        display.setTextColor(BLACK, WHITE); 
        display.println(" Liike LED");
        display.println(" ");
        if (liikeled) display.println(" ON"); else display.println(" OFF");
        display.display();
      }
    }
    else 
    { 
      nappil++;
      if (nappil>200)
      {
        lopetusl=1;
        nappil=0;
      }
    }
    delay(10);
  } while (lopetusl==0);

  if (liikeled==0) digitalWrite(3,LOW);
}

void naytavalikko(int valikko)
{
  display.clearDisplay();
  display.setRotation(2);
  display.setTextColor(BLACK, WHITE); 
  display.println(" Viisari");
  display.println(" ");
  
  if (valikko==1) display.setTextColor(WHITE, BLACK); else display.setTextColor(BLACK, WHITE); 
  display.println(" Ulkona");
  if (valikko==2) display.setTextColor(WHITE, BLACK); else display.setTextColor(BLACK, WHITE); 
  display.println(" Sisalla");
  if (valikko==3) display.setTextColor(WHITE, BLACK); else display.setTextColor(BLACK, WHITE); 
  display.println(" Jarvessa");
  if (valikko==4) display.setTextColor(WHITE, BLACK); else display.setTextColor(BLACK, WHITE); 
  display.println(" Kontrasti");
  display.display();
}

void liikeviesti()
{
  unsigned long aika;
  String viesti;
  aika=millis();
  
  if (aika>(liike_aika+120000))
  {
    liiketta++;
    viesti="aNDL";
    viesti=viesti+liiketta;
    int pituus=viesti.length();
    for (int x=0; x<(12-pituus); x++)
    {
      viesti=viesti+'-';
    } 
    liike_aika=aika;
    delay(500);
    Serial.print(viesti);
    delay(500);
  }
  if (aika<liike_aika) liike_aika=aika;
  
//  liiketta++;
//  if (liiketta>liikeherkkyys)
//  {
//    liiketta=0;
//    Serial.write(lviesti);
//  }

}

void naytanumero (int numero, int rivi, int sarake)
{
  int paikka;
  if (sarake==1) paikka=51;
  if (sarake==2) paikka=61;
  if (sarake==3) paikka=74;
  display.drawBitmap(56-(rivi*16), paikka, fontti[numero][0], 8, 10, 1);
  display.drawBitmap(48-(rivi*16), paikka, fontti[numero][1], 8, 10, 1);
  display.display();
}

void naytatausta()
{
  display.clearDisplay();
  display.drawBitmap(40, 0, tausta[0], 8, 84, 1);
  display.drawBitmap(32, 0, tausta[1], 8, 84, 1);
  display.drawBitmap(24, 0, tausta[2], 8, 84, 1);
  display.drawBitmap(16, 0, tausta[3], 8, 84, 1);
  display.drawBitmap(8, 0, tausta[4], 8, 84, 1);
  display.drawBitmap(0, 0, tausta[5], 8, 84, 1);
  display.display();
}

void naytalampotila (int numero, int rivi)
{
  int luku1,luku2,luku3;

  if (numero>499) 
  { 
    numero=numero-500;
    display.drawBitmap(50-(rivi*16),44,plus,8,6,1);
  } else 
  { 
    numero=-(numero-500);
   display.drawBitmap(50-(rivi*16),44,miinus,8,6,1);
  }
  if (numero<499)
  {
    luku1=numero/100;
    luku2=(numero-(luku1*100))/10;
    luku3=numero-(luku1*100)-(luku2*10);
    if (luku1==0) luku1=10;
    display.drawBitmap(56-(rivi*16), 51, fontti[luku1][0], 8, 10, 1);
    display.drawBitmap(48-(rivi*16), 51, fontti[luku1][1], 8, 10, 1);
    display.drawBitmap(56-(rivi*16), 61, fontti[luku2][0], 8, 10, 1);
    display.drawBitmap(48-(rivi*16), 61, fontti[luku2][1], 8, 10, 1);
    display.drawBitmap(56-(rivi*16), 74, fontti[luku3][0], 8, 10, 1);
    display.drawBitmap(48-(rivi*16), 74, fontti[luku3][1], 8, 10, 1);
  } else
  {
    display.drawBitmap(56-(rivi*16), 51, fontti[10][0], 8, 10, 1);
    display.drawBitmap(48-(rivi*16), 51, fontti[10][1], 8, 10, 1);
    display.drawBitmap(56-(rivi*16), 61, fontti[10][0], 8, 10, 1);
    display.drawBitmap(48-(rivi*16), 61, fontti[10][1], 8, 10, 1);
    display.drawBitmap(56-(rivi*16), 74, fontti[10][0], 8, 10, 1);
    display.drawBitmap(48-(rivi*16), 74, fontti[10][1], 8, 10, 1);
  }
  display.display();
}

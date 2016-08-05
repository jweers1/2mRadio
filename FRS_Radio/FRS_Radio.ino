#include "U8glib.h"
#include <SoftwareSerial.h>
#include <Rotary.h>
#include <OneButton.h>

// Radio module
SoftwareSerial radioSerial(4,5);
// Display
U8GLIB_64128N u8g(13, 11, 10, 9, 8);    //SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9
//Rotary encoder
Rotary r = Rotary(2,3);
OneButton button(A1,true);

//Debug Mode   1 is on. 0 is off.
int DEBUG = 1;

// smart delay variables
double draw_delay=5000; // in millis
double draw_start_millis = 0;

int zzMode=0;
int zzEnter=0;
float zzMult=1;
String zzinp;
String zzset = "Mode";

struct settings_t
{
  float receive_f = 146.9400;
  float transmit_f = 146.3400;
  int bandwidth = 1;
  int rxcss = 0;
  int txcss = 12 ;
  int squelch = 2;
  int volume = 8;
} current_settings;

char radio_status[32];
int radio_update_needed=1;
int volume_update_needed=1;
int display_update_needed=0;
String command;
  
//**************************************************************************//

void singleclick() {//change position in field
 if(DEBUG==1){
  Serial.println("SingleClick");
 }
 // if (doubleClicked) {
 //   posInput++;
 //   posInput = posInput % 7;
 // }
  display_update_needed = 1;
  //if (zzMode == 0)

  if (zzMult == 1){ zzMult = .01; } 
  else{ 
    zzMult = 1;
  }

} 

void doubleclick() { //change field
if(DEBUG==1){
  Serial.println("DoubleClick");
}
//  doubleClicked = !doubleClicked;
//  if (!doubleClicked) eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));;
   display_update_needed = 1;
 //if (zzMode == 0)

  if (zzMult == 1){ zzMult = .01; } 
  else{ 
    zzMult = 1;
  }

} 

void longclick() { //rotate displays
if(DEBUG==1){
  Serial.println("LongClick");
}
//  posInput = 0;
//  doubleClicked = 0;
 display_update_needed = 1;
 if (zzEnter==0) {
   zzEnter = 1;
   if (zzMode==1){
      zzset="Rx F";
   } else if (zzMode == 2){
      zzset= "Tx F";
   } else if (zzMode == 3){
      zzset= "Rx C";
   } else if (zzMode == 4){
      zzset="Tx C";
   } else if (zzMode == 5){
      zzset="Vol";
   }
   
 } else if (zzEnter == 1){
   zzEnter = 0;
   zzset="Mode";
 }

 if (zzMode == 5){
   volume_update_needed = 1;
 } else {
   radio_update_needed=1;
 }
} 

void processRotary(int result){


  if (DEBUG == 1){
    Serial.print("E: ");
    Serial.print(zzEnter);
    Serial.print(" R: ");
    Serial.print(result);
    Serial.print(" M: ");
    Serial.print(zzMode);
    Serial.println();
  }

   if (zzEnter == 0) {
    if (result == 16) {
      zzMode --;
      if (zzMode<0) zzMode=0;
    }
    if (result == 32) {
      zzMode ++;
      if (zzMode>5) zzMode=5;
    }
    switch(zzMode){
      case 1: zzinp="Rx F"; break;
      case 2: zzinp="Tx F"; break;
      case 3: zzinp="Rx C"; break;
      case 4: zzinp="Tx C"; break;
      case 5: zzinp="Vol"; break;
    }
   } else {
     if (result == 16){
      //left
      switch (zzMode){
        case 1: 
           current_settings.receive_f = current_settings.receive_f - zzMult;
           break;
        case 2:
           current_settings.transmit_f = current_settings.transmit_f - zzMult;
           break;
        case 3:
           current_settings.rxcss -= 1;
           if (current_settings.rxcss < 0){
            current_settings.rxcss = 0;
           }
           break;
        case 4:
           current_settings.txcss -= 1;
           if (current_settings.txcss < 0){
            current_settings.txcss = 0;
           }
           break;
        case 5:
           current_settings.volume -= 1;
           if (current_settings.volume <0){
            current_settings.volume = 0;
           }
           break; 
      }
     } else if (result == 32) {
      //right
      switch (zzMode){
        case 1: 
           current_settings.receive_f = current_settings.receive_f + zzMult;
           break;
        case 2:
           current_settings.transmit_f = current_settings.transmit_f + zzMult;
           break;
        case 3:
           current_settings.rxcss += 1;
           if (current_settings.rxcss > 12) {
            current_settings.rxcss = 12;
           }
           break;
        case 4:
           current_settings.txcss += 1;
           if (current_settings.txcss > 12) {
            current_settings.txcss = 12;
           }
           break;
        case 5:
           current_settings.volume += 1;
           if (current_settings.volume > 8){
            current_settings.volume = 8;
           }
           break; 
      }
      
     }
    

    
   }

   
    display_update_needed=1;
}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  
  //USB Serial
  Serial.begin(9600);
  while (!Serial){
   ; //delay until it connects.
  }

  Serial.println("Connecting to Radio");
  radioSerial.begin(9600);
  radioSerial.println("AT+DMOCONNECT");
  
  // init display
  Serial.println("Setup Display.");
  u8g.setFont(u8g_font_5x7);
  u8g.setColorIndex(1);
  u8g.setDefaultBackgroundColor();
  u8g.setDefaultForegroundColor();

  // rotary encoder
  button.attachDoubleClick(doubleclick);
  button.attachClick(singleclick);
  button.attachPress(longclick);
  
}


//ISR(PCINT2_vect) {
//  Serial.println(result);
  //if (doubleClicked)
  //  {
  //    if((select_display == 0) && modSpan){
  //      updateSpan(result);
  //    }
  //    else 
  //    settings.freqCenter = updateFreq(result, settings.freqCenter);
  //  }
  //else if(select_display == 0) modSpan = !modSpan;
// 
//  display_update_needed = 1;
//}

//void updateSpan(unsigned char r){
//  if (r == DIR_NONE ) {
//  // do nothing
//  }
//  else if (r == DIR_CW) {
//    settings.spanIdx++;
//  }
//  else if (r == DIR_CCW) {
//    settings.spanIdx--;
//    if (settings.spanIdx < 0) settings.spanIdx = totSpan -1;
//  }
//  settings.spanIdx = settings.spanIdx % totSpan;
//}

void setRadio(){
  char radioCommand[64];
  char txf[10];
  char rxf[10];

  dtostrf(current_settings.transmit_f,4,4,txf);
  dtostrf(current_settings.receive_f,4,4,rxf);
  
  sprintf(radioCommand,"AT+DMOSETGROUP=%d,%s,%s,%d,%d,%d,1\0",current_settings.bandwidth,txf,rxf,current_settings.rxcss,current_settings.squelch,current_settings.txcss);

 if(DEBUG==1){
  Serial.println(radioCommand);
 }
  radioSerial.println(radioCommand);

  radio_update_needed=0;
}


void setRadioVolume(){
  char radioCommand[32];
  sprintf(radioCommand,"AT+DMOSETVOLUME=%d",current_settings.volume);
  radioCommand[17]='\0';
  
  radioSerial.println(radioCommand);

 if(DEBUG==1){
  Serial.println(radioCommand);
 }  
 volume_update_needed=0;

}

void parseInput(){
  int x=0;
  char c;

  if(DEBUG == 1){
    Serial.print("Entering parseInput: ");
    Serial.println(command);
  }
  
  while (Serial.available() && (c != '=') && (command != "Help")) { 
    delay(3);
    c = Serial.read();
    if ((c != '=') || (c == '\n')) {
      command += c; 
    }
  }

  
  if (command == "Help") {
       Serial.println("Usage:");
       Serial.println("Tx=<x> - set Transmit Frequency");
       Serial.println("Rx=<x> - set Receive Frequency");
       Serial.println("Rxcss=<x> - set receive Ctcss 0-12");
       Serial.println("Txcss=<x> - set transmit ctcss 0-12");
       Serial.println("squelch=<x> - set squelch 0-8");
       Serial.println("vol=<x> - set volume 0-10");
       command = "";
  } else if (command == "Tx") {
     //  Serial.println("TX");
       float fl = Serial.parseFloat();
       current_settings.transmit_f = fl;
       radio_update_needed=1;
       command = "";
  } else if (command == "Rx") {
     //  Serial.println("RX");
       float fl = Serial.parseFloat();
       current_settings.receive_f = fl;
       radio_update_needed=1;
       
  } else if (command == "Rxcss"){
       int fl=Serial.parseInt();
       current_settings.rxcss = fl;
       radio_update_needed=1;
  } else if (command == "Txcss"){
      int fl = Serial.parseInt();
      current_settings.txcss = fl;
      radio_update_needed=1;
  } else if (command == "squelch") {
      int fl = Serial.parseInt();
      current_settings.squelch = fl;
      radio_update_needed=1;
  } else if (command == "vol"){
      int fl = Serial.parseInt();
      current_settings.volume=fl;
      radio_update_needed=1;
  }

   command = "";
  
}

// theloop function runs over and over again forever
void loop() {
  button.tick();
  unsigned char result = r.process();
  if((DEBUG==1) && (result != 0)){
  //  Serial.println(result);
  }
  if (result !=0){
     processRotary(result);
  }
  
  int i=0;
  if (radioSerial.available()) {
          radio_status[i] = radioSerial.read();
          Serial.write(radio_status[i]);
          i++;
   }
   if (Serial.available()) {
          parseInput();
          //radioSerial.write(Serial.read());
   }

 if (radio_update_needed) {
  if(DEBUG==1){
    Serial.println("Radio Update is needed..");
  }
  setRadio();
 }
 if (volume_update_needed) {
  if(DEBUG==1){
    Serial.println("Vol Update is needed..");
  }
  setRadioVolume();
 }
 
 // only update the display periodically
 if ((millis() - draw_start_millis > draw_delay) || (display_update_needed == 1)){
  draw_start_millis = millis();
  display_update_needed=0;
  if(DEBUG == 1){
//    Serial.println("u8g draw routine.");
  }
  u8g.firstPage();  
    do {
      draw();
    } while( u8g.nextPage() );
    // after you draw, save the current time...
   
  }

}





// overall draw routine
void draw(void) {
   char tmp[12];
  u8g.setColorIndex(0);
  u8g.drawBox(1,1,63,63);

  u8g.setColorIndex(255);
  u8g.drawStr(0,8,"2m Radio");

  u8g.drawStr(0,16,"Rx : ");
  u8g.setPrintPos(24,16);
  u8g.print(current_settings.receive_f);
  
  u8g.drawStr(0,24,"Tx : ");
  u8g.setPrintPos(24,24);
  u8g.print(current_settings.transmit_f);

  u8g.drawStr(0,32,"Txc/Rxc : ");
  u8g.setPrintPos(48,32);
  u8g.print(current_settings.txcss);
  u8g.setPrintPos(64,32);
  u8g.print(current_settings.rxcss);
  u8g.setPrintPos(72,32);
  u8g.print(current_settings.volume);

  u8g.drawStr(0,48,"Set> ");
  zzset.toCharArray(tmp,12);
  u8g.drawStr(24,48,tmp);
  
  u8g.drawStr(0,56,"Val> ");
  if (zzEnter == 0){
  zzinp.toCharArray(tmp,12);
  u8g.drawStr(24,56,tmp);
  } else if (zzMode > 2) {
    u8g.drawStr(24,56,"1");
  } else{
    u8g.setPrintPos(24,56);
    u8g.print(zzMult);
  }
}

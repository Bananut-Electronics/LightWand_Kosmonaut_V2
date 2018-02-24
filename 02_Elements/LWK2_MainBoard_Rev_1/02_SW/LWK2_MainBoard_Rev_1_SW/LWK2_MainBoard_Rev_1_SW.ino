/*
LightWand Kosmonaut V2

Code based on Michael Ross original LightWand(2014) and LightWand Kosmanut V1(2015) first written in 2010 and enhaced
by Is0-Mick in 2012.
Apart from the main idea, the following functions have been literally borrowed from the original code:
   setupSDcard()
   GetFileNamesFromSD(File dir)

http://mrossphoto.com/wordpress32/

This code has been written by Pablo de Miguel (pablodmm.isp@gmail.com) in 2018.

This code is intended to run with LightWand Kosmonaut V2 Controller (LWK2_MainBoard_Rev_1), All the information about this
controller can be find in:

TODO LINK GIRHUB ***

This code support direct reading of .pnm RAW files stored in the root directory of a microSD-Card
*/

// Libraries needed
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Adafruit_NeoPixel.h>

// LWK2_MainBoard_Rev_1 Pin Definition
const int LCD_SCK =  7;   // Definition of LCD Pins
const int LCD_DIN =  6;   // Definition of LCD Pins
const int LCD_DC  =  5;   // Definition of LCD Pins
const int LCD_CS  =  4;   // Definition of LCD Pins
const int LCD_RST =  3;   // Definition of LCD Pins
const int LCD_BL  = 15;   // Definition of LCD Pins
const int SD_MOSI = 51;   // Definition of SD Pins (Just for Info, already set in the SD library)
const int SD_MISO = 50;   // Definition of SD Pins (Just for Info, already set in the SD library)
const int SD_CLK  = 52;   // Definition of SD Pins (Just for Info, already set in the SD library)
const int SD_CS   = 44;   // Definition of SD Pins (Just for Info, already set in the SD library)
const int SWPad_A = 56;   // SW LED PAD A (TOP-LEFT)
const int SWPad_B = 57;   // SW LED PAD B (BOT-LEFT)
const int SWPad_C = 58;   // SW LED PAD C (TOP-RIGHT)
const int SWPad_D = 59;   // SW LED PAD D (BOT-RIGHT)
const int SW_L    = 17;   // SW Tactile LEFT
const int SW_C    = 18;   // SW Tactile CENTER
const int SW_R    = 19;   // SW Tactile RIGHT
const int BUZZ    = 16;   // Buzzer Speaker

// Constant Parameters and values
const int LED_SWPAD    =   4; // Number of WS2812B LEDs in the SW LED Pads
const int LED_STR      = 144; // Number of WS2812B LEDs in the LED Stripe
const int BUZZ_INI_NoteN = 4; // Number of Notes in Start Melody
const int BUZZ_END_NoteN = 4; // Number of Notes in Ending Melody
const int BUZZ_STP_NoteN = 4; // Number of Notes in Stop Melody
int ValUP = 0;
int ValDW = 0;

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);  //LCD Display Declaration
Adafruit_NeoPixel strip = Adafruit_NeoPixel((LED_STR+LED_SWPAD), 2, NEO_RGB + NEO_KHZ800);         //LED Strip Declaration  

// Control Arrays for SW_Pad LEDS
// ---------------------------------------------------------------------------------------------------------------------
// COLOR RED [0:255]        || COLOR GREEN [0:255]        || COLOR BLUE [0:255]           || Comments
//----------------------------------------------------------------------------------------------------------------------
const byte GEN_R_OFF =  0    ;const byte GEN_G_OFF =  0    ;const byte GEN_B_OFF =   0;    // SW_GENERAL OFF LED COLOR
const byte GEN_R_ON  =  255  ;const byte GEN_G_ON  =  255  ;const byte GEN_B_ON  =   255;  // SW_GENERAL OFF LED COLOR
const byte MENU_0_R  =  100  ;const byte MENU_0_G  =  100  ;const byte MENU_0_B  =   100;  // SW_GENERAL OFF LED COLOR
const byte MENU_1_R  =  100  ;const byte MENU_1_G  =  0    ;const byte MENU_1_B  =   100;  // SW_SCALE ON LED COLOR
const byte MENU_2_R  =  0    ;const byte MENU_2_G  =  100  ;const byte MENU_2_B  =   100;  // SW_SCALE ON LED COLOR
const byte SEL_R     =  0    ;const byte SEL_G     =  100  ;const byte SEL_B     =   100;  // SW_SCALE ON LED COLOR
const byte EXT_R     =  150  ;const byte EXT_G     =  0    ;const byte EXT_B     =   50;   // SW_SCALE ON LED COLOR



byte color_MenuLVL[3][3] = {{MENU_0_R, MENU_0_G, MENU_0_B},
                            {MENU_1_R, MENU_1_G, MENU_1_B},
                            {MENU_2_R, MENU_2_G, MENU_2_B}};

// Flags for the Pulse Conformer
int SWPad[4]         = {SWPad_A, SWPad_B, SWPad_C, SWPad_D}; // SWPad Pin Array
boolean SWPad_p[4]   = {false, false, false, false};         // SWPad Pulse Conformer Array

int SW[3]            = {SW_L, SW_C, SW_R};                   // SW Pin Array
boolean SW_p[3]      = {false, false, false};                // SW Pulse Conformer Array 

int menuLVL    = 0;
int menuIDX[3] = {0,0,0};

// Menu Display Definitions

File root;
File dataFile;
String m_CurrentFilename = "";        //Root Filename
int m_FileIndex = 0;                  //Variable for the FileIndex  
int m_NumberOfFiles = 0;              //Varialbe for the number of  files in the SD Storage
String m_FileNames[200];              //Variable for the File names(Max 200)
int Bright = 5;                       //Predetermined Brigthness
int Delay = 0;                        //Predetermined Delay
bool Active = 0;                      //Active Option
int MenuIndex = 0;                    //Variable for the MenuIndex
int BrightLimit = 0;                  //Variable for the Maximum BrithLimit
int FreqTone = 2;                     //Variable for the Frequenzy Tone emitted 
int BuzzMode = 1;                     //Predetermined Buzzer Mode
int FuncMode = 1;                     //Predetermined Function Mode
//int iniMelody[INIMelodyNote][2]={{2093,500},{1568,500},{0,300},{1319,600}}; //Start Melody
//int endMelody[ENDMelodyNote][2]={{2093,500},{1568,500},{0,300},{1319,600}}; //End Melody
//int stpMelody[STOPMelodyNote][2]={{1568,500},{1319,500},{0,300},{2093,600}}; //Stop Melody
int red = 0;
int green = 0; ///WHOT THE FUCK ARE WE USING INT?!?!?!?!?!?!?!?!
int blue = 0;

bool dispActive = false; // Active LED Stripe Flag

int paramBUZZ   = 0; // Buzzer Parameter
int paramFUNC   = 0; // General Function Parameter
int paramBRT    = 5; // Brightness Parameter
int paramDLY    = 0; // Delay Parameter
int paramACC    = 0; // Accelerometer Parameter



void setup()
{
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  digitalWrite(43,HIGH); //WHOT??
  setupLCDdisplay();
  setupSDcard();
  strip.begin();
  strip.show();
}

void loop()
{
   displayMenuLVL_0();   
   //SwitchControl();
   //if(Active){DisplayStrip();}
   readSWPad_LED();
   updateSWPad_LED();
   delay(50);
}

void setupLCDdisplay() {
  display.begin();
  display.setContrast(60);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.display();          //Show SplashScreen (Can this be removed??)
  //delay(500);
  display.clearDisplay();
  display.println("LIGHTWAND");
  display.println("KOSMONAUT V2");
  display.println("INI SD card...");
  display.display(); 
}

void setupSDcard() {
  pinMode(SD_CS, OUTPUT);
  // SD Initialization Routine
  if (!SD.begin(SD_CS)) 
  {
    display.clearDisplay();
    display.println("LIGHTWAND");
    display.println("KOSMONAUT V2");
    display.println("INI SD FAILED");
    display.display();
    return;
  }
  display.clearDisplay();
  display.println("LIGHTWAND");
  display.println("KOSMONAUT V2");
  display.println("INI SD DONE");
  display.display();
  // Files Scan
  root = SD.open("/");
  display.clearDisplay();
  display.println("LightWand");
  display.println("KOSMONAUT V2");
  display.println("FILE SCAN");
  display.display();
  delay(500);
  GetFileNamesFromSD(root);
  isort(m_FileNames, m_NumberOfFiles);
  m_CurrentFilename = m_FileNames[0];
  DisplayControl();
}

void GetFileNamesFromSD(File dir) {
  int fileCount = 0;
  String CurrentFilename = "";
  while(1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      m_NumberOfFiles = fileCount;
    entry.close();
      break;
    }
    else {
      if (entry.isDirectory()) {
        //GetNextFileName(root);
      }
      else {
        CurrentFilename = entry.name();
        if (CurrentFilename.endsWith(".pnm") || CurrentFilename.endsWith(".PNM")) { //find files with our extension only
          m_FileNames[fileCount] = entry.name();
          fileCount++;
        }
      }
    }
    entry.close();
  }
}

void displayMenuLVL_0(){
  m_CurrentFilename = m_FileNames[m_FileIndex];
  display.clearDisplay();

  if(menuIDX[0]==0){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.println("CURRENT FILE:");
  display.println(m_CurrentFilename);

  if(menuIDX[0]==1){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("BRIGHT: "); display.print(Bright); display.println(" %");

  if(menuIDX[0]==2){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("DELAY: "); display.print(Delay); display.println(" ms");

  if(menuIDX[0]==3){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("BUZZER: "); display.println(" ");// HERE STUFF

  if(menuIDX[0]==4){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("MODE: "); display.println(" ");// HERE STUFF

  if(menuIDX[0]==5){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("ACC: "); display.println(" ");// HERE STUFF

  if(menuIDX[0]==6){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.println("FACTORY RST");

    
  display.setTextColor(BLACK, WHITE);
  display.display();
}

void DisplayControl() {
  m_CurrentFilename = m_FileNames[m_FileIndex];
  display.clearDisplay();
  //display.println("LightWand"); // LOOP EXPERIMENTAL
  
  if(menuIDX==0){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("Bright: ");
  display.print(Bright);
  display.println(" %");
  
  if(menuIDX==1){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("Delay: ");
  display.print(Delay);
  display.println(" ms");
  

  
  if(menuIDX==3){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("Buzz: ");
  switch(BuzzMode)
  {
    case 0:
      display.println("OFF");
      break;
    case 1:
      display.println("LIMIT");
      break;
    case 2:
      display.println("PROGR");
      break;
  }
  
  if(menuIDX==4){display.setTextColor(WHITE, BLACK);}else{display.setTextColor(BLACK, WHITE);}
  display.print("Mode: ");
  switch(FuncMode)
  {
    case 0:
      display.println("NORMAL");
      break;
    case 1:
      display.println("LOOP");
      break;
  }
  
  display.setTextColor(BLACK, WHITE);
  display.display();
}

void isort(String *filenames, int n) {
  for (int i = 1; i < n; ++i) {
    String j = filenames[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < filenames[k]); k--) {
      filenames[k + 1] = filenames[k];
    }
    filenames[k + 1] = j;
  }
}

void DisplayStrip()
{
  // Set of Option Variables
  BrightLimit = map(Bright,0,100,0,255);      // Sets the maximum Brightness value
  digitalWrite(LCD_BL, LOW);                  //LCD Backlight Control OFF to avoid it appearing in the image;
  //Light=false;                                //LCD Backlight Control OFF;
  //if(BuzzMode!=0){for(int z=0; z<INIMelodyNote; z++){tone(BUZZ,iniMelody[z][0],iniMelody[z][1]);}} // Initial melody Play
  //FreqTone=INIFREQ;                           // Initial Frequency Set
  //delay(INIDELAY);                            // Initial Delay
  
  char temp[14];
  m_CurrentFilename.toCharArray(temp,14);
  File dataFile = SD.open(temp);
  // Rutine to avoid the header of the file
  int countRet=0; // Number of Carriage Return counted
  while(countRet < 4)
  {
    if(dataFile.read()=='\n'){countRet++;}
  }
  while(dataFile.available())
  {
    // Stop Control
    if(digitalRead(SW_L))
    {
      Active=0;
      for(int j=LED_SWPAD; j<LED_STR+LED_SWPAD; j++){strip.setPixelColor(j,0,0,0);} // Turn OFF LED Stripe
      strip.show();
      //if(BuzzMode!=0){for(int z=0; z<STOPMelodyNote; z++){tone(BUZZ,stpMelody[z][0],stpMelody[z][1]);}} // Stop Melody Play
      return;
    }
    // Control of progressive beat (Only emmitted in Buzz Progressive Mode)
    if(BuzzMode==2){
       //FreqTone=FreqTone+5;
       //noTone(BUZZ);
       //tone(BUZZ,FreqTone);
       //digitalWrite(MOT_A_DIG,LOW);
       //speedBuzz(analogRead(MOT_B_ANAL));
    }
    // File Row Read and Display
    for(int j=(LED_SWPAD); j<(LED_STR+LED_SWPAD); j++)
    {  
      //int red = dataFile.read();
      //int green = dataFile.read();
      //int blue = dataFile.read();
      red = map(dataFile.read(),0,255,0,BrightLimit);     // Bright Adjustment
      green = map(dataFile.read(),0,255,0,BrightLimit);   // Bright Adjustment
      blue = map(dataFile.read(),0,255,0,BrightLimit);    // Bright Adjustment
      strip.setPixelColor(j, green,red,blue);
    }
    delay(Delay);
    strip.show();
  }  
  // File Close
  noTone(BUZZ);
  
  dataFile.close();
    Active=0;
    // LED Strip OFF
    for(int j=(LED_SWPAD); j<(LED_STR+LED_SWPAD); j++){  strip.setPixelColor(j,0,0,0);}
    strip.show();  
  
  if(FuncMode == 1){if(m_FileIndex<(m_NumberOfFiles-1)){m_FileIndex++;}else{m_FileIndex=0;}} // Experimental GIF AID in LOOP MODE
  //if(BuzzMode!=0){for(int z=0; z<ENDMelodyNote; z++){tone(BUZZ,endMelody[z][0],endMelody[z][1]);}} // End Melody Play
}

void SwitchControl()
{
  // Active Switch Control
  /*if(!digitalRead(SW_C)){
    if(!SW_C_p)
    {
      SW_C_p=true;
      Active=1;
    }
  }else{
    if(SW_C){SW_C_p=false;}
  }
  
  // Menu Index Control
  if(!digitalRead(SWPad_A)){
    if(!SWPad_A_p)
    {
      SWPad_A_p=true;
      if(MenuIndex>0){MenuIndex--;}
    }
  }else{
    if(SWPad_A){SWPad_A_p=false;}
  }
  
  if(!digitalRead(SWPad_B)){ 
    if(!SWPad_B_p){
      SWPad_B_p=true;
      if(MenuIndex<4){MenuIndex++;}
    }
  }else{
    if(!SWPad_B){SWPad_B_p=false;}
  }
  
  // Light Control
  /*if(digitalRead(SWLight)){ 
    if(!SWLight_p){
      SWLight_p=true;
      Light=!Light;
    }
  }else{
    if(SWLight){SWLight_p=false;}
  }
  digitalWrite(LCD_BL, Light); //LCD Backlight Control
  */
  // Value Control
  /*if(!digitalRead(SWPad_D)){ 
    if(!SWPad_D_p){
      SWPad_D_p=true;
      ValUP=true;
    }
  }else{
    if(SWPad_D){SWPad_D_p=false;}
  }
  
  if(!digitalRead(SWPad_C)){ 
    if(!SWPad_C_p){
      SWPad_C_p=true;
      ValDW=true;
    }
  }else{
    if(!SWPad_C){SWPad_C_p=false;}
  }


  switch(MenuIndex){
  case 4: // FuncMode Control
    if(ValUP){if(FuncMode<1){FuncMode++;}else{FuncMode=0;}}
    if(ValDW){if(FuncMode>0){FuncMode--;}else{FuncMode=1;}}
    break;  
  case 3: // BuzzMode Control
    if(ValUP){if(BuzzMode<2){BuzzMode++;}else{BuzzMode=0;}}
    if(ValDW){if(BuzzMode>0){BuzzMode--;}else{BuzzMode=2;}}
    break;
  case 2: // File Control
    if(ValUP){if(m_FileIndex<(m_NumberOfFiles-1)){m_FileIndex++;}else{m_FileIndex=0;}}
    if(ValDW){if(m_FileIndex>0){m_FileIndex--;}else{m_FileIndex=(m_NumberOfFiles-1);}}
    break;
  case 1: // Delay Control
    if(ValUP){if(Delay<100){Delay=Delay+5;}else{Delay=0;}}
    if(ValDW){if(Delay>0){Delay=Delay-5;}else{Delay=100;}}
    break;
  case 0: // Bright Control
    if(ValUP){if(Bright<100){Bright=Bright+5;}else{Bright=5;}}
    if(ValDW){if(Bright>5){Bright=Bright-5;}else{Bright=100;}}
    break;
  }
  ValUP=false;
  ValDW=false;*/
}

void speedBuzz(int speedBuzz){
  //noTone(BUZZ);
  /*if(speedBuzz < (CENTRAL_SPEED - SPEED_TOL)){
    //FreqTone = LOW_SPEED_BUZZ;}
    noTone(BUZZ);
  }
  else if(speedBuzz > (CENTRAL_SPEED + SPEED_TOL)){
    //FreqTone = HIGH_SPEED_BUZZ;}
    noTone(BUZZ);
  }else{
    tone(BUZZ,CENTRAL_SPEED);
  }*/
}


void readSWPad_LED(){
  for (int i = 0; i < 4; i++){
    if(!digitalRead(SWPad[i])){
      SWPad_p[i]=true;
      //Here the actions!!
    }else{if(SWPad_p[i]){SWPad_p[i]=false;}}
  }
}


void updateSWPad_LED(){
  for (int i = 0; i < 2; i++){strip.setPixelColor(i,color_MenuLVL[menuIDX][1],color_MenuLVL[menuIDX][0],color_MenuLVL[menuIDX][2]);}
  strip.setPixelColor(2, EXT_G, EXT_R, EXT_B);
  strip.setPixelColor(3, SEL_G, SEL_R, SEL_B);
  for (int i = 0; i < LED_SWPAD; i++){if(SWPad_p[i]){strip.setPixelColor(i, GEN_G_ON, GEN_R_ON, GEN_B_ON);}}
  strip.show();  
}

void actionSWPad_LED(int SWPad_n){
  switch(SWPad_n){
    case 0:        // CTRL UP
      if (menuIDX[
  }
}


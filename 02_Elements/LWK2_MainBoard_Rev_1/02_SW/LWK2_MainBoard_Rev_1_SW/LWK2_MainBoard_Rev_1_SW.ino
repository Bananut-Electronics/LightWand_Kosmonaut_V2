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
const int SWPad_0 = 56;   // SW LED PAD A (TOP-LEFT)
const int SWPad_1 = 57;   // SW LED PAD B (BOT-LEFT)
const int SWPad_2 = 58;   // SW LED PAD C (TOP-RIGHT)
const int SWPad_3 = 59;   // SW LED PAD D (BOT-RIGHT)
const int SW_2    = 17;   // SW Tactile LEFT
const int SW_1    = 18;   // SW Tactile CENTER
const int SW_0    = 19;   // SW Tactile RIGHT
const int BUZZ    = 16;   // Buzzer Speaker

// Constant Parameters and values
const int LED_SWPAD    =   4; // Number of WS2812B LEDs in the SW LED Pads
const int LED_STR      = 144; // Number of WS2812B LEDs in the LED Stripe
const int BUZZ_INI_NoteN = 4; // Number of Notes in Start Melody
const int BUZZ_END_NoteN = 4; // Number of Notes in Ending Melody
const int BUZZ_STP_NoteN = 4; // Number of Notes in Stop Melody
int ValUP = 0;
int ValDW = 0;

// Contant Menu IDs
const int menuIDX_0_MAX = 7;
const int IDX_0_FLE    = 0;
const int IDX_0_BRT    = 1;
const int IDX_0_DLY    = 2;
const int IDX_0_MOD    = 3;
const int IDX_0_SEN    = 4;
const int IDX_0_BUZ    = 5;
const int IDX_0_ACC    = 6;
const int IDX_0_STT    = 7;

// Constant Buzzer Tones
const int BUZZ_SWPad_UP = 1500;
const int BUZZ_SWPad_DW = 1000;
const int BUZZ_ERROR_H = 5000;
const int BUZZ_ERROR_L = 800;
const int BUZZ_Duration = 50;

// Constant Brightness BRT
const int BRT_Max   = 100;
const int BRT_Min   = 0;

// Constant Delay DLY
const int DLY_Max   = 90;
const int DLY_Min   = 0;

// Constant Mode MOD
const int MOD_Max      = 2;
const int MOD_Min      = 0;
const int MOD_Normal   = 0;
const int MOD_Gif      = 1;
const int MOD_Loop     = 2;

// Constant Buzzer BZZ
const int BUZ_Max     = 2;
const int BUZ_Min     = 0;
const int BUZ_Limit   = 0;
const int BUZ_Scale   = 1;
const int BUZ_AllOff  = 2;

// Constant Buzzer BZZ
const int SEN_0_ON     = 0;
const int SEN_0_MIN    = 0;
const int SEN_0_MAX    = 1024;
const int SEN_1_ON     = 0;
const int SEN_1_MIN    = 0;
const int SEN_1_MAX    = 1024;

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);  //LCD Display Declaration
Adafruit_NeoPixel strip = Adafruit_NeoPixel((LED_STR+LED_SWPAD), 2, NEO_RGB + NEO_KHZ800);         //LED Strip Declaration  

// Control Arrays for SW_Pad LEDS
// ---------------------------------------------------------------------------------------------------------------------
// COLOR RED [0:255]        || COLOR GREEN [0:255]        || COLOR BLUE [0:255]           || Comments
//----------------------------------------------------------------------------------------------------------------------
const byte GEN_R_OFF =  0    ;const byte GEN_G_OFF =  0    ;const byte GEN_B_OFF =   0;    // SW_GENERAL OFF LED COLOR
const byte GEN_R_ON  =  100  ;const byte GEN_G_ON  =  100  ;const byte GEN_B_ON  =   100;  // SW_GENERAL OFF LED COLOR
const byte MENU_0_R  =  50   ;const byte MENU_0_G  =  50   ;const byte MENU_0_B  =   50;  // SW_GENERAL OFF LED COLOR
const byte MENU_1_R  =  0    ;const byte MENU_1_G  =  0    ;const byte MENU_1_B  =   100;  // SW_SCALE ON LED COLOR
const byte MENU_2_R  =  0    ;const byte MENU_2_G  =  50   ;const byte MENU_2_B  =   50;  // SW_SCALE ON LED COLOR
const byte SEL_R     =  0    ;const byte SEL_G     =  50   ;const byte SEL_B     =   50;  // SW_SCALE ON LED COLOR
const byte EXT_R     =  50   ;const byte EXT_G     =  0    ;const byte EXT_B     =   50;   // SW_SCALE ON LED COLOR

const byte color_MenuLVL[3][3] = {{MENU_0_R, MENU_0_G, MENU_0_B},
                                  {MENU_1_R, MENU_1_G, MENU_1_B},
                                  {MENU_2_R, MENU_2_G, MENU_2_B}};

// Flags for the Pulse Conformer
int SWPad[4]         = {SWPad_0, SWPad_1, SWPad_2, SWPad_3}; // SWPad Pin Array
boolean SWPad_p[4]   = {false, false, false, false};         // SWPad Pulse Conformer Array

int SW[3]            = {SW_2, SW_1, SW_0};                   // SW Pin Array
boolean SW_p[3]      = {false, false, false};                // SW Pulse Conformer Array 

int menuLVL    = 0;
int menuIDX[3] = {0,0,0};

// Menu Display Definitions

File root;
File dataFile;
String m_CurrentFilename = "";        //Root Filename
int paramFileIDX = 0;                  //Variable for the FileIndex  
int paramFileMAX = 0;              //Varialbe for the number of  files in the SD Storage
String m_FileNames[200];              //Variable for the File names(Max 200)
int Bright = 50;                       //Predetermined Brigthness
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

int paramBUZ    = 0; // Buzzer Parameter
int paramMOD    = 0; // General Function Parameter
int paramBRT    = 50; // Brightness Parameter
int paramDLY    = 0; // Delay Parameter
int paramACC    = 0; // Accelerometer Parameter

int paramA0_ON  = 1; // A0 Sensor ON/OFF
int paramA0_MIN = 0; // A0 Sensor MIN
int paramA0_MAX = 1000; // A0 Sensor MAX
int paramA1_ON  = 1; // A0 Sensor ON/OFF
int paramA1_MIN = 0; // A0 Sensor MIN
int paramA1_MAX = 1000; // A0 Sensor MAX

int paramFLE_W      = 144;  // Actual Image File Width
int paramFLE_H      = 200;  // Actual Image File Width
int paramFLW_Format = 0;    // Actual Image File Width



void setup(){
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  digitalWrite(43,HIGH); //WHOT??
  setupLCDdisplay();
  setupSDcard();
  strip.begin();
  strip.show();
}

void loop(){
   displayM();   
   readSWPad();
   if(!digitalRead(SW_0)){displayStrip_PNM();}
   delay(50);
}

void setupLCDdisplay(){
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

void setupSDcard(){
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
  isort(m_FileNames, paramFileMAX);
  m_CurrentFilename = m_FileNames[0];
  //DisplayControl();
}

void GetFileNamesFromSD(File dir){
  int fileCount = 0;
  String CurrentFilename = "";
  while(1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      paramFileMAX = fileCount;
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

void isort(String *filenames, int n){
  for (int i = 1; i < n; ++i) {
    String j = filenames[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < filenames[k]); k--) {
      filenames[k + 1] = filenames[k];
    }
    filenames[k + 1] = j;
  }
}

void displayStrip_PNM(){
  displayBUSY();   // Display in LCD Busy Message
  int BRT_Limit = map(paramBRT,0,BRT_Max,0,255);      // Sets the maximum Brightness value
  digitalWrite(LCD_BL, LOW);                  //LCD Backlight Control OFF to avoid it appearing in the image;
  
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
    // File Row Read and Display
    for(int j=LED_SWPAD; j<(LED_STR+LED_SWPAD); j++)
    {  
      red = map(dataFile.read(),0,255,0,BRT_Limit);     // Bright Adjustment
      green = map(dataFile.read(),0,255,0,BRT_Limit);   // Bright Adjustment
      blue = map(dataFile.read(),0,255,0,BRT_Limit);    // Bright Adjustment
      strip.setPixelColor(j, green,red,blue);
    }
    delay(paramDLY);
    strip.show();
  }  
  // File Close  
  dataFile.close();
  Active=0;
  for(int j=LED_SWPAD; j<(LED_STR+LED_SWPAD); j++){  strip.setPixelColor(j,0,0,0);}
  strip.show();  
}

void buzzerSWPad(int SWPad_n){
  switch(SWPad_n){
    case 0: tone(BUZZ, BUZZ_SWPad_UP, BUZZ_Duration);break;
    case 1: tone(BUZZ, BUZZ_SWPad_DW, BUZZ_Duration);break;
    case 2: tone(BUZZ, BUZZ_SWPad_UP, 2*BUZZ_Duration);tone(BUZZ, BUZZ_SWPad_DW, 2*BUZZ_Duration); break;
    case 3: tone(BUZZ, BUZZ_SWPad_DW, 2*BUZZ_Duration);tone(BUZZ, BUZZ_SWPad_UP, 2*BUZZ_Duration); break;
    default: break;
  }
}

void buzzerSWPad_ERROR(){ //TODO
   tone(BUZZ, BUZZ_ERROR_H, 10*BUZZ_Duration);
   tone(BUZZ, BUZZ_ERROR_L, 10*BUZZ_Duration);
}

void readSWPad(){
  for (int i = 0; i < 4; i++){
    if(!digitalRead(SWPad[i])){
      if(!SWPad_p[i]){
        SWPad_p[i]  = true;
        actionSWPad(i);
        updateSWPad();
        buzzerSWPad(i);
      }
    }else{
      if(SWPad_p[i]){
        SWPad_p[i]  = false;
        updateSWPad();
      }
    }
  }
}

void readSW(){
  for (int i = 0; i < 4; i++){
    if(!digitalRead(SWPad[i])){
      SW_p[i]=true;
            //Here the actions!!
    }else{if(SW_p[i]){SWPad_p[i]=false;}}
  }
}

void updateSWPad(){
  for (int i = 0; i < 2; i++){strip.setPixelColor(i,color_MenuLVL[menuLVL][1],color_MenuLVL[menuLVL][0],color_MenuLVL[menuLVL][2]);}
  strip.setPixelColor(2, EXT_G, EXT_R, EXT_B);
  strip.setPixelColor(3, SEL_G, SEL_R, SEL_B);
  for (int i = 0; i < LED_SWPAD; i++){if(SWPad_p[i]){strip.setPixelColor(i, GEN_G_ON, GEN_R_ON, GEN_B_ON);}}
  strip.show();  
}

void actionSWPad(int SWPad_n){
  if(SWPad_n < 2){  // For SWPad UP and SWPad DW
    switch(menuLVL){
      case 0: // Root Main Menu LVL 1
        switch(SWPad_n){
          case 0: // SWPad UP 0
            if(menuIDX[0]==menuIDX_0_MAX){menuIDX[0] = 0;}
            else{menuIDX[0]++;}
          break;
          case 1: // SWPad DW 1
            if(menuIDX[0]==0){menuIDX[0] = menuIDX_0_MAX;}
            else{menuIDX[0]--;}
          break;
          default: break;     
        }     
      break;
      default: // Submenu LVL 1, 2
        switch(menuIDX[0]){
          case IDX_0_FLE: actionSM_FLE(SWPad_n); break; // display File Options Submenu
          case IDX_0_BRT: actionSM_BRT(SWPad_n); break; // display Brightness Submenu
          case IDX_0_DLY: actionSM_DLY(SWPad_n); break; // display Delay Submenu
          case IDX_0_MOD: actionSM_MOD(SWPad_n); break; // display Mode Submenu
          case IDX_0_SEN: actionSM_SEN(SWPad_n); break; // display Sensor Submenu
          case IDX_0_BUZ: actionSM_BUZ(SWPad_n); break; // display Buzzer Submenu
          case IDX_0_ACC: actionSM_ACC(SWPad_n); break; // display Accelerometer Submenu
          case IDX_0_STT: actionSM_STT(SWPad_n); break; // display Settings Submenu
          default: break;
        }
      break;
    }
  }else{ // For SWPad SEL and SWPad EXIT
    switch(SWPad_n){
      case 2: // For SWPad EXIT
         if(menuLVL > 0){menuLVL--;}
         else{buzzerSWPad_ERROR();}
      break;
      case 3: // For SWPad SEL
         if(menuLVL < 2){menuLVL++;}
         else{buzzerSWPad_ERROR();}
      break;
      default: break;
    }
  }
}

void displayM(){
  m_CurrentFilename = m_FileNames[paramFileIDX];
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.println(m_CurrentFilename);

  switch(menuIDX[0]){
    case IDX_0_FLE: displaySM_FLE(); break; // display File Options Submenu
    case IDX_0_BRT: displaySM_BRT(); break; // display Brightness Submenu
    case IDX_0_DLY: displaySM_DLY(); break; // display Delay Submenu
    case IDX_0_MOD: displaySM_MOD(); break; // display Mode Submenu
    case IDX_0_SEN: displaySM_SEN(); break; // display Sensor Submenu
    case IDX_0_BUZ: displaySM_BUZ(); break; // display Buzzer Submenu
    case IDX_0_ACC: displaySM_ACC(); break; // display Accelerometer Submenu
    case IDX_0_STT: displaySM_STT(); break; // display Settings Submenu
    default: break;
  }
  display.display();
}

void displayBUSY(){
  m_CurrentFilename = m_FileNames[paramFileIDX];
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.println(m_CurrentFilename);
  display.setTextSize(2);
  display.print("STRIPEBUSY");
  display.display();
}

void displaySM_FLE(){
  display.setTextColor(WHITE, BLACK);
  display.print("FILE INFO:\n");
  display.setTextColor(BLACK, WHITE);
  display.print("FORMAT: ");
  if(!paramFLW_Format){display.println(".BMP");}
  else{display.println(".PNM");}
  display.print("WIDTH: "); display.println(paramFLE_W);
  display.print("HEIGHT: "); display.println(paramFLE_H);
  display.setTextSize(1);
}

void displaySM_BRT(){
  display.setTextColor(WHITE, BLACK);
  display.println("BRIGHTNESS:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(3);
  display.print(paramBRT);
  display.println("%");
  display.setTextSize(1);
}

void displaySM_DLY(){
  display.setTextColor(WHITE, BLACK);
  display.println("DELAY:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(3);
  display.print(paramDLY);
  display.println("ms");
  display.setTextSize(1);
}

void displaySM_MOD(){
  display.setTextColor(WHITE, BLACK);
  display.print("MODE:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
  switch(paramMOD){
    case MOD_Normal:
      display.print("NORMAL\n");
      display.setTextSize(1);
      display.print("One image pershot");
    break;
    case MOD_Gif:
      display.print("GIF\n");
      display.setTextSize(1);
      display.print("Image index  increased");
    break;
    case MOD_Loop:
      display.print("LOOP\n");
      display.setTextSize(1);
      display.print("Image displayin loop");
    break;
  }
}

void displaySM_SEN(){
  display.setTextColor(WHITE, BLACK);
  display.print("SENSOR:\n");
  display.setTextSize(1);
  // Info of Analog Sensor 0
  display.print("A0:");
  display.setTextColor(BLACK, WHITE);
  if(paramA0_ON){display.print("R:");display.println(analogRead(A6));}
  else{display.println("OFF");}
  display.print("MN:");display.print(paramA0_MIN);
  display.print(" MX:");display.println(paramA0_MAX);
  // Info of Analog Sensor 0
  display.setTextColor(WHITE, BLACK);
  display.print("A1:");
  display.setTextColor(BLACK, WHITE);
  if(paramA1_ON){display.print("R:");display.println(analogRead(A7));}
  else{display.println("OFF");}
  display.print("MN:");display.print(paramA1_MIN);
  display.print(" MX:");display.println(paramA1_MAX);
  
  //Here Submenu
}

void displaySM_BUZ(){
  display.print("BUZZER:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
  switch(paramBUZ){
    case BUZ_Limit:
      display.print("LIMIT\n");
      display.setTextSize(1);
      display.print("Buzz Sound atinit and end");
    break;
    case BUZ_Scale:
      display.print("SCALE\n");
      display.setTextSize(1);
      display.print("Scale Sound  when display");
    break;
    case BUZ_AllOff:
      display.print("OFF\n");
      display.setTextSize(1);
      display.print("No Buzzer    Sounds");
    break;
  }
}

void displaySM_ACC(){
  display.setTextColor(WHITE, BLACK);
  display.println("ACC:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
  display.print("NO :-(\n");
  display.setTextSize(1);
  display.print("\nSorry :-(");
}

void displaySM_STT(){
  display.setTextColor(WHITE, BLACK);
  display.println("SETTINGS:  \n");
  display.setTextColor(BLACK, WHITE);
  //Here Submenu
}

void actionSM_FLE(bool SWPad_n){
  switch(SWPad_n){
    case 0: // For SWPad UP
      if(paramFileIDX < paramFileMAX-1){paramFileIDX++;}
      else{paramFileIDX = 0;}
    break;
    case 1: // For SWPad DW
      if(paramFileIDX > 0){paramFileIDX--;}
      else{paramFileIDX = paramFileMAX;}
    break;
  }
}

void actionSM_BRT(bool SWPad_n){
  switch(SWPad_n){
    case 0: // For SWPad UP
      if(paramBRT < BRT_Max){paramBRT += 5;}
      else{buzzerSWPad_ERROR();}
    break;
    case 1: // For SWPad DW
      if(paramBRT > BRT_Min){paramBRT -= 5;}
      else{buzzerSWPad_ERROR();}
    break;
  }
}

void actionSM_DLY(bool SWPad_n){
  switch(SWPad_n){
    case 0: // For SWPad UP
      if(paramDLY < DLY_Max){paramDLY += 5;}
      else{buzzerSWPad_ERROR();}
    break;
    case 1: // For SWPad DW
      if(paramDLY > DLY_Min){paramDLY -= 5;}
      else{buzzerSWPad_ERROR();}
    break;
  }
}

void actionSM_MOD(bool SWPad_n){
  switch(SWPad_n){
    case 0: // For SWPad UP
      if(paramMOD < MOD_Max){paramMOD++;}
      else{buzzerSWPad_ERROR();}
    break;
    case 1: // For SWPad DW
      if(paramMOD > MOD_Min){paramMOD--;}
      else{buzzerSWPad_ERROR();}
    break;
  }
}

void actionSM_SEN(bool SWPad_n){

}

void actionSM_BUZ(bool SWPad_n){
  switch(SWPad_n){
    case 0: // For SWPad UP
      if(paramBUZ < BUZ_Max){paramBUZ++;}
      else{buzzerSWPad_ERROR();}
    break;
    case 1: // For SWPad DW
      if(paramBUZ > BUZ_Min){paramBUZ--;}
      else{buzzerSWPad_ERROR();}
    break;
  }
}

void actionSM_ACC(bool SWPad_n){

}

void actionSM_STT(bool SWPad_n){

}


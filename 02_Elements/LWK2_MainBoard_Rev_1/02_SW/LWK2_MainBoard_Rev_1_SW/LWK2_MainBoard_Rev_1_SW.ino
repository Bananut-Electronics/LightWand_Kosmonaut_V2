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
const int SWPad_0 = 56;   // SW LED PAD A SWPad_UP (TOP-LEFT)
const int SWPad_1 = 57;   // SW LED PAD B SWPad_DW (BOT-LEFT)
const int SWPad_2 = 58;   // SW LED PAD C SWPad_EXT (TOP-RIGHT)
const int SWPad_3 = 59;   // SW LED PAD D SWPad_SEL (BOT-RIGHT)
const int SW_2    = 17;   // SW Tactile LEFT
const int SW_1    = 18;   // SW Tactile CENTER
const int SW_0    = 19;   // SW Tactile RIGHT
const int BUZZ    = 16;   // Buzzer Speaker

// Constant Parameters and values
const int LED_SWPAD    =   4; // Number of WS2812B LEDs in the SW LED Pads
const int LED_STR      = 144; // Number of WS2812B LEDs in the LED Stripe

// Constant BMP
//const int byteBMPHeader  = 138; // Header for the Windows BitMap with Opacity
const int byteBMPHeader  = 54; // Header for the Windows BitMap with Opacity
const int byteBMPHeight  = 22;  // First Byte of File Image Height Header
const int byteBMPWidth   = 18;  // First Byte of File Image Width Header

// Constant Menu IDs
const int menuIDX_MAX  = 7;
const int IDX_0_FLE    = 0;
const int IDX_0_MOD    = 1;
const int IDX_0_BRT    = 2;
const int IDX_0_DLY    = 3;
const int IDX_0_BUZ    = 4;
const int IDX_0_LGT    = 5;
const int IDX_0_ACC    = 6;
const int IDX_0_SEN    = 7;

// Constant Sensor Submenu IDs
const int menuIDX_SEN_MAX   = 6;
const int IDX_SEN_MOD       = 0;
const int IDX_SEN_A0_ONOFF  = 1;
const int IDX_SEN_A0_MIN    = 2;
const int IDX_SEN_A0_MAX    = 3;
const int IDX_SEN_A1_ONOFF  = 4;
const int IDX_SEN_A1_MIN    = 5;
const int IDX_SEN_A1_MAX    = 6;

// Constant SEN Sensor Control
const int SEN_MOD_Max   = 2;
const int SEN_MOD_Point = 0;
const int SEN_MOD_Bar   = 1;
const int SEN_MOD_Color = 2;
const int SEN_A_MAX     = 1000;
const int SEN_A_MIN     = 0;

// Constant BRT Brightness Control
const int BRT_Max   = 100;
const int BRT_Min   = 0;
const int BRT_Step  = 5;

// Constant DLY Delay Control
const int DLY_Max   = 90;
const int DLY_Min   = 0;
const int DLY_Step  = 5;

// Constant MOD Mode Control
const int MOD_Max      = 2;
const int MOD_Min      = 0;
const int MOD_Normal   = 0;
const int MOD_Gif      = 1;
const int MOD_Loop     = 2;

// Constant BZZ Buzzer Control
const int BUZ_Max     = 2;
const int BUZ_Min     = 0;
const int BUZ_Limit   = 0;
const int BUZ_Scale   = 1;
const int BUZ_AllOff  = 2;

// Constant BUZ Buzzer Tones
const int BUZZ_SWPad_UP = 1500;
const int BUZZ_SWPad_DW = 1000;
const int BUZZ_ERROR_H = 5000;
const int BUZZ_ERROR_L = 800;
const int BUZZ_Duration = 50;

// Constant LGT Light Control
const int LGT_Max     = 1;
const int LGT_Min     = 0;
const int LGT_Auto    = 0;
const int LGT_AllOff  = 1;

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);  //LCD Display Declaration
Adafruit_NeoPixel strip = Adafruit_NeoPixel((LED_STR+LED_SWPAD), 2, NEO_RGB + NEO_KHZ800);         //LED Strip Declaration  

// Control Arrays for SW_Pad LEDS
// ---------------------------------------------------------------------------------------------------------------------
// COLOR RED [0:255]        || COLOR GREEN [0:255]        || COLOR BLUE [0:255]           || Comments
//----------------------------------------------------------------------------------------------------------------------
const byte GEN_R_OFF =  0    ;const byte GEN_G_OFF =  0    ;const byte GEN_B_OFF =   0;    // SW_GENERAL OFF LED COLOR
const byte GEN_R_ON  =  100  ;const byte GEN_G_ON  =  100  ;const byte GEN_B_ON  =   100;  // SW_GENERAL OFF LED COLOR
const byte MENU_0_R  =  50   ;const byte MENU_0_G  =  50   ;const byte MENU_0_B  =   50;   // SW_GENERAL OFF LED COLOR
const byte MENU_1_R  =  0    ;const byte MENU_1_G  =  0    ;const byte MENU_1_B  =   100;  // SW_SCALE ON LED COLOR
const byte MENU_2_R  =  0    ;const byte MENU_2_G  =  100   ;const byte MENU_2_B  =   0;   // SW_SCALE ON LED COLOR
const byte SEL_R     =  0    ;const byte SEL_G     =  50   ;const byte SEL_B     =   50;   // SW_SCALE ON LED COLOR
const byte EXT_R     =  50   ;const byte EXT_G     =  0    ;const byte EXT_B     =   50;   // SW_SCALE ON LED COLOR
const byte ERR_R     =  100  ;const byte ERR_G     =  0    ;const byte ERR_B     =   0;    // SW_SCALE ON LED COLOR

const byte color_MenuLVL[3][3] = {{MENU_0_R, MENU_0_G, MENU_0_B},
                                  {MENU_1_R, MENU_1_G, MENU_1_B},
                                  {MENU_2_R, MENU_2_G, MENU_2_B}};

// Flags for the Pulse Conformer
int SWPad[4]         = {SWPad_0, SWPad_1, SWPad_2, SWPad_3}; // SWPad Pin Array
boolean SWPad_p[4]   = {false, false, false, false};         // SWPad Pulse Conformer Array

const int SWPad_UP      = 0;
const int SWPad_DW      = 1;
const int SWPad_EXT     = 2;
const int SWPad_SEL     = 3;

int menuLVL    = 0;
int menuIDX    = 0;

// Menu Display Definitions
File root;
File dataFile;
String m_CurrentFilename = "";   // Root Filename
int paramFileIDX = 0;            // Variable for the FileIndex  
int paramFileMAX = 0;            // Varialbe for the number of  files in the SD Storage
String m_FileNames[200];         // Variable for the File names(Max 200)
unsigned char R,G,B = 0;

bool dispActive = false; // Active LED Stripe Flag

int paramBUZ    = BUZ_Limit; // Buzzer Parameter
int paramMOD    = 0; // General Function Parameter
int paramBRT    = 10; // Brightness Parameter
int paramDLY    = 20; // Delay Parameter
int paramACC    = 0; // Accelerometer Parameter
int paramLGT    = 0; // Light Mode Parameter

int paramSEN        = 0;    // SEN SubMenu Index
int paramSEN_MOD    = 0;    // SEN Effect Mode
int paramSEN_A0_ON  = 1;    // SEN Analog 0 ON/OFF
int paramSEN_A0_MIN = SEN_A_MIN;    // SEN Analog 0 MIN
int paramSEN_A0_MAX = SEN_A_MAX; // SEN Analog 0 MAX
int paramSEN_A1_ON  = 1;    // SEN Analog 1 ON/OFF
int paramSEN_A1_MIN = SEN_A_MIN;    // SEN Analog 1 MIN
int paramSEN_A1_MAX = SEN_A_MAX; // SEN Analog 1 MAX

int paramFLE_W      = 0;  // Actual Image File Width
int paramFLE_H      = 0;  // Actual Image File Width
int paramFLW_Format = 0;    // Actual Image File Width

void setup(){
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  digitalWrite(43,HIGH); //WHOT??
  setupLCDdisplay();
  setupSDcard();
  strip.begin();
  strip.show();
  displaySWPad();
  buzzerStripe_ON();
}

void loop(){
   displayM();   
   readSWPad();
   if(!digitalRead(SW_0)){displayStripe();}
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

void displayStripe(){
  if(menuIDX == IDX_0_SEN){
    switch(paramSEN_MOD){
      case(SEN_MOD_Point): displayStripe_SEN_MOD_Point(); break;
      case(SEN_MOD_Bar): displayStripe_SEN_MOD_Bar(); break;
      case(SEN_MOD_Color): displayStripe_SEN_MOD_Color(); break;
    }
  }else{displayStripe_PNM();}
}

void displayStripe_SEN_MOD_Point(){
  turnOffLights(); // Turn off SWPad and Display Backlight
  buzzerStripe_START();
  while(digitalRead(SW_1)){
    displayBUSY();   // Display in LCD Busy Message
    int BRT_Limit = map(paramBRT,0,BRT_Max,0,255);      // Sets the maximum Brightness value
    int pointPos = map(analogRead(A6), paramSEN_A0_MIN, paramSEN_A0_MAX, 0, LED_STR);
    for(int j = LED_SWPAD; j<(LED_SWPAD+LED_STR); j++){ 
      strip.setPixelColor(j, 0, 0, 0);
    }
    strip.setPixelColor(pointPos, G, R, B);
    strip.show();
    delay(paramDLY); 
  }
  interruptDisplayStripe();// Interrupt Stripe Display
  buzzerStripe_END();
}

void displayStripe_SEN_MOD_Bar(){
  turnOffLights(); // Turn off SWPad and Display Backlight
  buzzerStripe_START();
  while(digitalRead(SW_1)){
    displayBUSY();   // Display in LCD Busy Message
    int BRT_Limit = map(paramBRT,0,BRT_Max,0,255);      // Sets the maximum Brightness value
    int barLength = map(analogRead(A6), paramSEN_A0_MIN, paramSEN_A0_MAX, 0, LED_STR);
    for(int j=LED_SWPAD; j<(LED_SWPAD+barLength); j++){ 
      B = map(analogRead(A7),paramSEN_A1_MIN,paramSEN_A1_MAX,0,BRT_Limit);
      G = map(analogRead(A7),paramSEN_A1_MIN,paramSEN_A1_MAX,0,BRT_Limit);
      R = map(analogRead(A7),paramSEN_A1_MIN,paramSEN_A1_MAX,0,BRT_Limit);
      strip.setPixelColor(j, G, R, B);
    }
    for(int j = (LED_SWPAD + barLength); j<(LED_SWPAD+LED_STR); j++){ 
      strip.setPixelColor(j, 0, 0, 0);
    }
    strip.show();
    delay(paramDLY); 
  }
  interruptDisplayStripe();// Interrupt Stripe Display
  buzzerStripe_END();
}

void displayStripe_SEN_MOD_Color(){
  turnOffLights(); // Turn off SWPad and Display Backlight
  buzzerStripe_START();
  while(digitalRead(SW_1)){
    displayBUSY();   // Display in LCD Busy Message
    int BRT_Limit = map(paramBRT,0,BRT_Max,0,255);      // Sets the maximum Brightness value
    for(int j=LED_SWPAD; j<(LED_SWPAD+LED_STR); j++){ 
      B = map(analogRead(A7),paramSEN_A1_MIN,paramSEN_A1_MAX,0,BRT_Limit);
      G = 0;
      R = map(analogRead(A6),paramSEN_A0_MIN,paramSEN_A0_MAX,0,BRT_Limit);
      strip.setPixelColor(j, G, R, B);
    }
    strip.show();
    delay(paramDLY); 
  }
  interruptDisplayStripe();// Interrupt Stripe Display
  buzzerStripe_END();
  
}

void displayStripe_BMP(){
  // Init Routine
  displayBUSY();   // Display in LCD Busy Message
  turnOffLights(); // Turn off SWPad and Display Backlight
  buzzerStripe_START();
  readHeader_BMP(); 
  int BRT_Limit = map(paramBRT,0,BRT_Max,0,255);      // Sets the maximum Brightness value
  while(dataFile.available()){
    if(!digitalRead(SW_1)){interruptDisplayStripe();return;} // Interrupt Stripe Display
    // File Row Read and Display
    for(int j=LED_SWPAD; j<(LED_STR+LED_SWPAD); j++){ 
      dataFile.read(); // Opacity Avoidance 
      B = map(dataFile.read(),0,255,0,BRT_Limit);
      G = map(dataFile.read(),0,255,0,BRT_Limit);
      R = map(dataFile.read(),0,255,0,BRT_Limit);
      strip.setPixelColor(j, G, R, B);
    }
    delay(paramDLY);
    strip.show();
  }  
  // File Close
  dataFile.close();
  // Turn Off LED Stripe
  for(int j=LED_SWPAD; j<(LED_STR+LED_SWPAD); j++){strip.setPixelColor(j,0,0,0);} // Turn Off LED Stripe
  strip.show();  
  // End Routine
  buzzerStripe_END();
  if(paramMOD == MOD_Loop){displayStripe_BMP();} // Display the image in LOOP
  if(paramMOD == MOD_Gif){paramFileIDX++;}       // Place Inde for next image
  turnOnLights(); // Turn off SWPad and Display Backlight
}

void displayStripe_PNM(){
  // Init Routine
  displayBUSY();   // Display in LCD Busy Message
  turnOffLights(); // Turn off SWPad and Display Backlight
  buzzerStripe_START();
  //readHeader_PNM(); 
  int BRT_Limit = map(paramBRT,0,BRT_Max,0,255);      // Sets the maximum Brightness value
  char temp[14];
  m_CurrentFilename.toCharArray(temp,14);
  File dataFile = SD.open(temp);
  // Rutine to avoid the header of the file
  int countRet=0; // Number of Carriage Return counted
  while(countRet < 4){if(dataFile.read()=='\n'){countRet++;}}
  while(dataFile.available()){
    if(!digitalRead(SW_1)){interruptDisplayStripe();return;} // Interrupt Stripe Display
    // File Row Read and Display
    for(int j=LED_SWPAD; j<(LED_STR+LED_SWPAD); j++){ 
      R = map(dataFile.read(),0,255,0,BRT_Limit);
      G = map(dataFile.read(),0,255,0,BRT_Limit);
      B = map(dataFile.read(),0,255,0,BRT_Limit);
      strip.setPixelColor(j, G, R, B);
    }
    delay(paramDLY);
    strip.show();
  }  
  // File Close
  dataFile.close();
  // Turn Off LED Stripe
  for(int j=LED_SWPAD; j<(LED_STR+LED_SWPAD); j++){strip.setPixelColor(j,0,0,0);} // Turn Off LED Stripe
  strip.show();  
  // End Routine
  buzzerStripe_END();
  if(paramMOD == MOD_Loop){displayStripe_BMP();} // Display the image in LOOP
  if(paramMOD == MOD_Gif){paramFileIDX++;}       // Place Inde for next image
  turnOnLights(); // Turn off SWPad and Display Backlight
}

void readHeader_BMP(){
  paramFLE_W,paramFLE_H = 0;
  // Opens File
  char temp[14];
  m_CurrentFilename.toCharArray(temp,14);
  dataFile = SD.open(temp);
  // Rutine to avoid the header of the file
  int byteRead = 0; // Number of Bytes read
  while(byteRead < byteBMPHeader){
    if(byteRead == byteBMPWidth){paramFLE_W = (int)dataFile.read();}
    else if(byteRead == byteBMPHeight){paramFLE_H =(int)dataFile.read();}
    else if(byteRead == (byteBMPHeight + 1)){paramFLE_H += (256*(int)dataFile.read());}
    else{dataFile.read();}
    byteRead++;
  }
}

void readHeader_PNM(){
  paramFLE_W,paramFLE_H = 0;
  // Opens File
  char temp[14];
  m_CurrentFilename.toCharArray(temp,14);
  File dataFile = SD.open(temp);
  // Rutine to avoid the header of the file
  int countRet = 0; // Number of Carriage Return counted
  while(countRet < 4){
    if(dataFile.read()=='\n'){countRet++;}
  }
}

void interruptDisplayStripe(){
  buzzer_ERROR();
  for(int j=LED_SWPAD; j<(LED_STR+LED_SWPAD); j++){strip.setPixelColor(j,0,0,0);} // Turn Off LED Stripe
  strip.show(); 
  turnOnLights(); // Turn off SWPad and Display Backlight
}

void buzzerSWPad(int SWPad_n){
  if(paramBUZ!=BUZ_AllOff){
    switch(SWPad_n){
      case SWPad_UP: tone(BUZZ, BUZZ_SWPad_UP, BUZZ_Duration);break;
      case SWPad_DW: tone(BUZZ, BUZZ_SWPad_DW, BUZZ_Duration);break;
      case SWPad_EXT: tone(BUZZ, BUZZ_SWPad_UP, 2*BUZZ_Duration);tone(BUZZ, BUZZ_SWPad_DW, 2*BUZZ_Duration); break;
      case SWPad_SEL: tone(BUZZ, BUZZ_SWPad_DW, 2*BUZZ_Duration);tone(BUZZ, BUZZ_SWPad_UP, 2*BUZZ_Duration); break;
      default: break;
    }
  }
}

void buzzerStripe_ON(){
  if(paramBUZ!=BUZ_AllOff){
   tone(BUZZ, 500, 100);
   tone(BUZZ, 1000, 100);
   tone(BUZZ, 4000, 100);
  }
}

void buzzerStripe_START(){
  if(paramBUZ!=BUZ_AllOff){
     tone(BUZZ, 3*BUZZ_ERROR_L, 10*BUZZ_Duration);
     tone(BUZZ, BUZZ_ERROR_H, 10*BUZZ_Duration);
  }
}

void buzzerStripe_END(){
   if(paramBUZ!=BUZ_AllOff){
     tone(BUZZ, 3*BUZZ_ERROR_H, 10*BUZZ_Duration);
     tone(BUZZ, BUZZ_ERROR_L, 10*BUZZ_Duration);
   }
}

void buzzer_ERROR(){
  if(paramBUZ!=BUZ_AllOff){
   tone(BUZZ, BUZZ_ERROR_H, 10*BUZZ_Duration);
   tone(BUZZ, BUZZ_ERROR_L, 10*BUZZ_Duration);
  }
}

void readSWPad(){
  for (int i = 0; i < 4; i++){
    if(!digitalRead(SWPad[i])){
      if(!SWPad_p[i]){
        SWPad_p[i]  = true;
        actionSWPad(i);
        displaySWPad();
        buzzerSWPad(i);
      }
    }else{
      if(SWPad_p[i]){
        SWPad_p[i]  = false;
        displaySWPad();
      }
    }
  }
}

void displaySWPad(){
  if(paramLGT == LGT_Auto){ // Light All Off Control
    for (int i = 0; i < 2; i++){strip.setPixelColor(i,color_MenuLVL[menuLVL][1],color_MenuLVL[menuLVL][0],color_MenuLVL[menuLVL][2]);}
    strip.setPixelColor(2, EXT_G, EXT_R, EXT_B);
    strip.setPixelColor(3, SEL_G, SEL_R, SEL_B);
    for (int i = 0; i < LED_SWPAD; i++){if(SWPad_p[i]){strip.setPixelColor(i, GEN_G_ON, GEN_R_ON, GEN_B_ON);}}
    strip.show();  
  }
}

void turnOffLights(){
  for (int i = 0; i < 4; i++){strip.setPixelColor(i,0,0,0);} // SWPad Off
  digitalWrite(LCD_BL, LOW);  //LCD Backlight Control OFF;
  strip.show();  
}

void turnOnLights(){
  displaySWPad();
  digitalWrite(LCD_BL, HIGH);  //LCD Backlight Control OFF; 
}

void displayM(){
  m_CurrentFilename = m_FileNames[paramFileIDX];
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  if(menuIDX != IDX_0_SEN){display.println(m_CurrentFilename);}

  switch(menuIDX){
    case IDX_0_FLE: displaySM_FLE(); break; // display File Options Submenu
    case IDX_0_BRT: displaySM_BRT(); break; // display Brightness Submenu
    case IDX_0_DLY: displaySM_DLY(); break; // display Delay Submenu
    case IDX_0_MOD: displaySM_MOD(); break; // display Mode Submenu
    case IDX_0_SEN: displaySM_SEN(); break; // display Sensor Submenu
    case IDX_0_BUZ: displaySM_BUZ(); break; // display Buzzer Submenu
    case IDX_0_ACC: displaySM_ACC(); break; // display Accelerometer Submenu
    case IDX_0_LGT: displaySM_LGT(); break; // display Settings Submenu
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
  dataFile.close();  //Get File Image Info
  if(menuLVL>0){display.setTextColor(WHITE, BLACK);}
  display.print("FILE INFO:\n");
  display.print("INDEX: ");
  display.print(paramFileIDX+1); display.print("/"); display.println(paramFileMAX);
  display.setTextColor(BLACK, WHITE);
  display.print("FORMAT: ");
  if(!paramFLW_Format){display.println(".PNM");}
  else{display.println(".PNM");}
  display.print("WIDTH: "); display.println(paramFLE_W);
  display.print("HEIGHT: "); display.println(paramFLE_H);
  display.setTextSize(1);
}

void displaySM_BRT(){
  if(menuLVL>0){display.setTextColor(WHITE, BLACK);}
  display.println("BRIGHTNESS:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(3);
  display.print(paramBRT);
  display.println("%");
  display.setTextSize(1);
}

void displaySM_DLY(){
  if(menuLVL>0){display.setTextColor(WHITE, BLACK);}
  display.println("DELAY:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(3);
  display.print(paramDLY);
  display.println("ms");
  display.setTextSize(1);
}

void displaySM_MOD(){
  if(menuLVL>0){display.setTextColor(WHITE, BLACK);}
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
  if(menuLVL>0){display.setTextColor(WHITE, BLACK);}
  display.print("SENSOR:\n");
  display.setTextColor(BLACK, WHITE);
  // Option MOD
  if(paramSEN == IDX_SEN_MOD){display.setTextColor(WHITE, BLACK);}
  display.print("MODE: ");
  switch(paramSEN_MOD){
    case SEN_MOD_Point: display.println("POINT"); break;
    case SEN_MOD_Bar: display.println("BAR"); break;
    case SEN_MOD_Color: display.println("COLOR"); break;
    default: break;
  }
  // Info of Analog Sensor 0
  // General ID
  display.setTextColor(WHITE, BLACK);
  display.print("A0:");
  display.setTextColor(BLACK, WHITE);
  // Option ON/OFF
  if(paramSEN == IDX_SEN_A0_ONOFF){display.setTextColor(WHITE, BLACK);}
  if(paramSEN_A0_ON){display.print("R:");display.println(analogRead(A6));}
  else{display.println("OFF");}
  display.setTextColor(BLACK, WHITE);
  // Option MIN
  if(paramSEN == IDX_SEN_A0_MIN){display.setTextColor(WHITE, BLACK);}
  display.print("MN:");display.print(paramSEN_A0_MIN);
  display.setTextColor(BLACK, WHITE);
  // Option MAX
  if(paramSEN == IDX_SEN_A0_MAX){display.setTextColor(WHITE, BLACK);}
  display.print(" MX:");display.println(paramSEN_A0_MAX);
  display.setTextColor(BLACK, WHITE);
  
  // Info of Analog Sensor 1
  // General ID
  display.setTextColor(WHITE, BLACK);
  display.print("A1:");
  display.setTextColor(BLACK, WHITE);
  // Option ON/OFF
  if(paramSEN == IDX_SEN_A1_ONOFF){display.setTextColor(WHITE, BLACK);}
  if(paramSEN_A1_ON){display.print("R:");display.println(analogRead(A7));}
  else{display.println("OFF");}
  display.setTextColor(BLACK, WHITE);
  // Option MIN
  if(paramSEN == IDX_SEN_A1_MIN){display.setTextColor(WHITE, BLACK);}
  display.print("MN:");display.print(paramSEN_A1_MIN);
  display.setTextColor(BLACK, WHITE);
  // Option MAX
  if(paramSEN == IDX_SEN_A1_MAX){display.setTextColor(WHITE, BLACK);}
  display.print(" MX:");display.println(paramSEN_A1_MAX);
  display.setTextColor(BLACK, WHITE);
}

void displaySM_BUZ(){
  if(menuLVL>0){display.setTextColor(WHITE, BLACK);}
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

void displaySM_LGT(){
  if(menuLVL>0){display.setTextColor(WHITE, BLACK);}
  display.print("LIGHTS:\n");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
  switch(paramLGT){
    case LGT_Auto:
      display.print("AUTO\n");
      display.setTextSize(1);
      display.print("SWPad and LCDBL Auto");
      turnOnLights();
    break;
    case LGT_AllOff:
      display.print("OFF\n");
      display.setTextSize(1);
      display.print("All lights   OFF always");
      turnOffLights();
    break;
  }
}

void actionSWPad(int SWPad_n){
  switch(menuLVL){
    case 0: // Root Main Menu LVL 1
      menuIDX = swipeParamLinear(SWPad_n, menuIDX, 0, menuIDX_MAX, 1, 0, 1); break; // display Brightness Submenu
    break;
    default: // Submenu LVL 1
      switch(menuIDX){
        case IDX_0_FLE: paramFileIDX = swipeParamLinear(SWPad_n, paramFileIDX, 0, (paramFileMAX-1), 1, 1, 0); readHeader_BMP(); break; // display Delay Submenu
        case IDX_0_BRT: paramBRT = swipeParamLinear(SWPad_n, paramBRT, BRT_Min, BRT_Max, BRT_Step, 1, 0); break; // display Brightness Submenu
        case IDX_0_DLY: paramDLY = swipeParamLinear(SWPad_n, paramDLY, DLY_Min, DLY_Max, DLY_Step, 1, 0);  break; // display Delay Submenu
        case IDX_0_MOD: paramMOD = swipeParamLinear(SWPad_n, paramMOD, MOD_Min, MOD_Max, 1, 1, 0);  break; // display Delay Submenu
        case IDX_0_SEN: actionSM_SEN(SWPad_n); break; // display Sensor Submenu
        case IDX_0_BUZ: paramBUZ = swipeParamLinear(SWPad_n, paramBUZ, BUZ_Min, BUZ_Max, 1, 1, 0);  break; // display Delay Submenu
        case IDX_0_ACC: break; // display Accelerometer Submenu NOT SUPPORTED YET
        case IDX_0_LGT: paramLGT = swipeParamLinear(SWPad_n, paramLGT, LGT_Min, LGT_Max, 1, 1, 0);  break; // display Delay Submenu
        default: break;
      }
    break;
  }
}

void actionSM_SEN(int SWPad_n){
  switch(menuLVL){
    case 1:
      paramSEN = swipeParamLinear(SWPad_n, paramSEN, 0, menuIDX_SEN_MAX, 1, 1, 1); break; // display Brightness Submenu
    break;
    default:
      switch(paramSEN){
        case IDX_SEN_MOD: paramSEN_MOD = swipeParamLinear(SWPad_n, paramSEN_MOD, 0, SEN_MOD_Max, 1, 1, 0); readHeader_BMP(); break; // display Delay Submenu
        case IDX_SEN_A0_ONOFF: paramSEN_A0_ON = swipeParamLinear(SWPad_n, paramSEN_A0_ON, 0, 1, 1, 1, 0); break; // display Brightness Submenu
        case IDX_SEN_A0_MIN: paramSEN_A0_MIN = swipeParamLinear(SWPad_n, paramSEN_A0_MIN, SEN_A_MIN, SEN_A_MAX, 50, 1, 0);  break; // display Delay Submenu
        case IDX_SEN_A0_MAX: paramSEN_A0_MAX = swipeParamLinear(SWPad_n, paramSEN_A0_MAX, SEN_A_MIN, SEN_A_MAX, 50, 1, 0);  break; // display Delay Submenu
        case IDX_SEN_A1_ONOFF: paramSEN_A1_ON = swipeParamLinear(SWPad_n, paramSEN_A1_ON, 0, 1, 1, 1, 0); break; // display Brightness Submenu
        case IDX_SEN_A1_MIN: paramSEN_A1_MIN = swipeParamLinear(SWPad_n, paramSEN_A1_MIN, SEN_A_MIN, SEN_A_MAX, 50, 1, 0);  break; // display Delay Submenu
        case IDX_SEN_A1_MAX: paramSEN_A1_MAX = swipeParamLinear(SWPad_n, paramSEN_A1_MAX, SEN_A_MIN, SEN_A_MAX, 50, 1, 0);  break; // display Delay Submenu
        default: break;
      }
    break;
  }
}

int swipeParamLinear(int SWPad_n, int actualValue, int minValue, int maxValue, int stepValue, bool lessSub, bool moreSub){
  int t_actualValue = actualValue;
  switch(SWPad_n){
    case SWPad_UP: // For SWPad UP
      if(t_actualValue < maxValue){t_actualValue += stepValue;}
      else{t_actualValue = minValue;}
    break;
    case SWPad_DW: // For SWPad DW
      if(t_actualValue > minValue){t_actualValue -= stepValue;}
      else{t_actualValue = maxValue;}
    break;
    case SWPad_EXT:   // For SWPad EXT
      if(lessSub){menuLVL--;}
      else{buzzer_ERROR();}
    break;
    case SWPad_SEL:   // For SWPad SEL
      if(moreSub){menuLVL++;}
      else{buzzer_ERROR();}
    break;
  }
  return t_actualValue;
}


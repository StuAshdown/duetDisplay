#include <Arduino.h>
/*
 3 Serial Ports on Feather M0 board.
 1.  USB serial (Serial)
 2.  TTL using Rx Tx pins 0 & 1 (serDuet)  this goes to Duet PanelDue port
 3.  Then add another sercom thing following: 
      https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
      TTL Rx Tx = D11 D10 (serNextion added in code here) this goes to Nextion display

 */
//#include <Arduino.h>   // required before wiring_private.h
//#include "wiring_private.h" // pinPeripheral() function
#include <ArduinoJson.h>
#include <Nextion.h>
#include <NextionPage.h>
#include <NextionText.h>
#include <NextionButton.h>
#include <NextionProgressBar.h>



#define serDebug Serial
#define serDuet Serial2
#define serNextion Serial3
//#define LED_BUILTIN PB4

//SoftwareSerial serDuet (D3,D4); //rx,tx
//SoftwareSerial serNextion (D1,D2); //rx,tx

//void SERCOM1_Handler()
//{
//  serNextion.IrqHandler();
//}


Nextion nex(serNextion);
NextionProgressBar progressBar(nex, 1, 4, "percent");
//NextionPage pgText(nex, 1, 0, "pgText");
// Declare a text object [page id:0,component id:1, component name: "t0"].
NextionText t_bedSet(nex, 0, 8, "bedSet");  // 
NextionText t_bedTemp(nex, 0, 9, "bedTemp");  // 
NextionText t_hotSet(nex, 0, 10, "hotSet");  // 
NextionText t_hotTemp(nex, 0, 11, "hotTemp");  // 
NextionText t_timeleft(nex, 1, 5, "timeleft");  // 
NextionText t_temp_enc(nex, 1, 3, "temp_enc");  // 
NextionText t_set_hot(nex, 1, 10, "set_hot");  // 
NextionText t_temp_hot(nex, 1, 1, "temp_hot");  // 
NextionText t_set_bed(nex, 1, 11, "set_bed");  // 
NextionText t_temp_bed(nex, 1, 2, "temp_bed");  // 
// button object: Page 0, ID:2, name "b0"
NextionButton b_bedPlus(nex,0,7,"bedPlus");
NextionButton b_bedMin(nex,0,8,"bedMin");
NextionButton b_hotPlus(nex,0,9,"hotPlus");
NextionButton b_hotMin(nex,0,10,"hotMin");
NextionButton b_home(nex,0,11,"home");
NextionButton b_pause(nex,2,1,"pause");
NextionButton b_resume(nex,2,2,"resume");
NextionButton b_load(nex,2,3,"load");
// new buttons for screen 3
NextionButton b_navXY1(nex,3,3,"navXY1");
NextionButton b_navXY2(nex,3,4,"navXY2");
NextionButton b_navXY3(nex,3,5,"navXY3");
NextionButton b_navXY4(nex,3,6,"navXY4");
NextionButton b_navCen(nex,3,9,"navCen");
NextionButton b_zp5(nex,3,7,"zp5");
NextionButton b_zm5(nex,3,8,"zm5");

String textBuffer = "Hello World!";
char textbuf[300];

unsigned long lasttime=0;
int ii=0;
int in = 0; 

int bedset = 0;
int hotset = 0;
float bedtemp, hottemp, encltemp;
float percent, secondsleft;

  int hrs,mins,secs;
  String minstring;
  //char state[3];
  char state;
  char last_state;

void update_displays() {
    textBuffer = "(" + String(bedset) + ")";
    textBuffer.toCharArray(textbuf,10); // typeconvert
    t_set_bed.setText(textbuf);
    t_bedSet.setText(textbuf);
    
    textBuffer = "(" + String(hotset) + ")";
    textBuffer.toCharArray(textbuf,10); // typeconvert
    t_set_hot.setText(textbuf);
    t_hotSet.setText(textbuf);
    
    textBuffer = String(bedtemp,1);
    textBuffer.toCharArray(textbuf,10); // typeconvert
    t_temp_bed.setText(textbuf);
    t_bedTemp.setText(textbuf);
  
    textBuffer = String(hottemp,1);
    textBuffer.toCharArray(textbuf,10); // typeconvert
    t_temp_hot.setText(textbuf);
    t_hotTemp.setText(textbuf);
  
    textBuffer = String(encltemp,1);
    textBuffer.toCharArray(textbuf,10); // typeconvert
    t_temp_enc.setText(textbuf);
  
    hrs = secondsleft/3600;
    mins = secondsleft/60 - hrs*60;
    secs = secondsleft - hrs*3600 - mins*60;
    minstring = String(mins);
    if (mins < 10) minstring = "0" + minstring;
  
    if (secs < 10) {
      textBuffer = String(hrs) + ":" + minstring + ":0" + String(secs);
    } else {
      textBuffer = String(hrs) + ":" + minstring + ":" + String(secs);
    }
    textBuffer.toCharArray(textbuf,15); // typeconvert
    t_timeleft.setText(textbuf);
  
    progressBar.setValue(int(percent*100));

    if  ((state =='P') && (last_state=='I')) {
      // switch to other page
      serNextion.println("page 1");
    } 
    last_state = state;

  
}



void call_bedPlus(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    if (bedset==0) {
      bedset=75;
    } else {
      bedset += 5;
    }
    textBuffer = "M140 S" + String(bedset);
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void call_bedMin(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    if (bedset==0) {
      bedset=75;
    } else {
      bedset -= 5;
    }
    textBuffer = "M140 S" + String(bedset);
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void call_hotPlus(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    if (hotset==0) {
      hotset=190;
    } else {
      hotset += 5;
    }
    textBuffer = "M104 S" + String(hotset);
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void call_hotMin(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    if (hotset==0) {
      hotset=190;
    } else {
      hotset -= 5;
    }
    textBuffer = "M104 S" + String(hotset);
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void call_home(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    textBuffer = "G28";
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void call_pause(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    textBuffer = "M25";
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void call_resume(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    textBuffer = "M24";
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void call_load(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    textBuffer = "G1 E50 F60"; //Feed 50 mm at 60 mm/minute (units?)
    serDuet.println(textBuffer);
    serDebug.println(textBuffer);
    update_displays();
  }
}

void callbackNav(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
      //serDebug.println(widget->getComponentID());
      switch (widget->getComponentID()) 
      {
        case 3: //xy1 - 10,10
          textBuffer = "G1 X10 Y10 F800"; 
          serDuet.println(textBuffer);
          serDebug.println(textBuffer);
          break;
        case 4: //xy2 - 10,290
          textBuffer = "G1 X10 Y290 F800"; 
          serDuet.println(textBuffer);
          serDebug.println(textBuffer);
          break;
        case 5: //xy3 - 290,10
          textBuffer = "G1 X290 Y10 F800"; 
          serDuet.println(textBuffer);
          serDebug.println(textBuffer);
          break;
        case 6: //xy4 - 290,290
          textBuffer = "G1 X290 Y290 F800"; 
          serDuet.println(textBuffer);
          serDebug.println(textBuffer);
          break;
        case 9: //xy4 - 290,290
          textBuffer = "G1 X150 Y150 F800"; 
          serDuet.println(textBuffer);
          serDebug.println(textBuffer);
          break;
        case 7: //z up 0.05mm
          textBuffer = "G91";  
          serDuet.println(textBuffer);
          textBuffer = "G1 Z0.05 F800";
          serDuet.println(textBuffer);
          serDebug.println(textBuffer);
          textBuffer = "G90";
          serDuet.println(textBuffer);
          break;
        case 8: //z down 0.05mm
          textBuffer = "G91";  
          serDuet.println(textBuffer);
          textBuffer = "G1 Z-0.05 F800";
          serDuet.println(textBuffer);
          serDebug.println(textBuffer);
          textBuffer = "G90";
          serDuet.println(textBuffer);
          break;

      }

  }
}
 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize both serial ports:
  serDebug.begin(9600);
  serDuet.begin(57600); // panel due connection

  serNextion.begin(9600); 
  
  // Assign pins 10 & 11 SERCOM functionality
//  pinPeripheral(10, PIO_SERCOM);
//  pinPeripheral(11, PIO_SERCOM);

  serDebug.println("Testing 123...");
  serNextion.print("t0.txt=\"hi there\"");
  serNextion.write(0xFF);
  serNextion.write(0xFF);
  serNextion.write(0xFF);


  nex.init();

  t_timeleft.setText("Hello!");
  t_bedSet.setText("com");
  t_bedTemp.setText("err");
  b_bedPlus.attachCallback(&call_bedPlus);
  b_bedMin.attachCallback(&call_bedMin);
  b_hotPlus.attachCallback(&call_hotPlus);
  b_hotMin.attachCallback(&call_hotMin);
  b_home.attachCallback(&call_home);
  b_pause.attachCallback(&call_pause);
  b_resume.attachCallback(&call_resume);
  b_load.attachCallback(&call_load);
  b_navXY1.attachCallback(&callbackNav);
  b_navXY2.attachCallback(&callbackNav);
  b_navXY3.attachCallback(&callbackNav);
  b_navXY4.attachCallback(&callbackNav);
  b_navCen.attachCallback(&callbackNav);
  b_zp5.attachCallback(&callbackNav);
  b_zm5.attachCallback(&callbackNav);
  serDebug.println("Init Done!");
}


void update_printer() {
  //digitalWrite(LED_BUILTIN, LOW);

  //StaticJsonDocument<2048> jsonBuffer;
  StaticJsonDocument<2048> root;
  serDuet.println("M408");
  serDebug.print("M408:->");
  textBuffer = serDuet.readStringUntil('\n');
  serDebug.println(textBuffer);
  //textBuffer = "{\"status\":\"P\",\"heaters\":[75.0,194.8,22.7],\"active\":[75.0,195.0,0.0],\"standby\":[0.0,0.0,0.0],\"hstat\":[2,2,0],\"pos\":[-35.248,-15.877,54.700],\"sfactor\":100.00,\"efactor\":[100.00,100.00],\"babystep\":0.000,\"tool\":0,\"probe\":\"0\",\"fanPercent\":[50.00,100.00,100.00,0.00,0.00,0.00,0.00,0.00,0.00],\"fanRPM\":0,\"homed\":[1,1,1],\"fraction_printed\":0.7257,\"msgBox.mode\":-1,\"timesLeft\":[7527.1,7728.8,7837.4]}";
  //char txbuf[] = "{\"status\":\"P\",\"heaters\":[75.8,194.8,22.7],\"active\":[75.0,195.0,0.0],\"standby\":[0.0,0.0,0.0],\"hstat\":[2,2,0],\"pos\":[-35.248,-15.877,54.700],\"fraction_printed\":0.7257,\"msgBox.mode\":-1,\"timesLeft\":[7527.1,7728.8,7837.4]}";
  //char txbuf[]  =  "{\"status\":\"P\",\"heaters\":[75.8,194.8,22.7],\"active\":[75.0,195.0,0.0],\"standby\":[0.0,0.0,0.0],\"hstat\":[2,2,0],\"pos\":[-35.248,-15.877,54.700]}";
  //char txbuf[] =      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
  DeserializationError err = deserializeJson(root,textBuffer);
  //JsonObject& root = jsonBuffer.parseObject(textBuffer);
  //textBuffer.toCharArray(textbuf,300); // typeconvert
  //serDebug.println(txbuf);
  //JsonObject& root = jsonBuffer.parseObject(txbuf);

  digitalWrite(LED_BUILTIN, LOW);
  serDebug.println("done reading");
  //textBuffer = String("sent:") + textBuffer;
  //textBuffer.toCharArray(textbuf,100); // typeconvert
  //serDebug.println(textBuffer);
  //serDebug.println(txbuf);
  

  if (err) {
    serDebug.println("Parse fail");
  } else {
    //serDebug.println("Parse success!");
  
    bedset = root["active"][0];
    serDebug.print("Bedset:");
    serDebug.println(bedset);
    hotset = root["active"][1]; //190;
    bedtemp = root["heaters"][0]; //74.2;
    hottemp = root["heaters"][1]; //191.3;
    encltemp = root["heaters"][2]; //22.1;
    percent = root["fraction_printed"]; //0.43;
    secondsleft = root["timesLeft"][1]; //7000-millis()/1000;
    //strcpy(state,root["status"]); //"I";
    strncpy(&state,root["status"],1); //1st character
    serDebug.print("State:");
    serDebug.println(state);
    //last_state = state;
    //digitalWrite(LED_BUILTIN, HIGH);
    update_displays(); 
    //digitalWrite(LED_BUILTIN, LOW);
  }  
//NextionText t_timeleft(nex, 1, 5, "timeleft");  // 

}

void loop() {
  nex.poll();

  if (millis() > lasttime) {
    serDebug.print("cycle ");
    serDebug.println(ii++);
    digitalWrite(LED_BUILTIN, HIGH);
    update_printer();
    yield();
    digitalWrite(LED_BUILTIN, LOW);
    lasttime = millis()+1000;
//    textBuffer = "time is:" + String(millis());
//    textBuffer.toCharArray(textbuf,250); // typeconvert
//    text.setText(textbuf); 
  }
}

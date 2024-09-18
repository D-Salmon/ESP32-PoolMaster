/*
  NEXTION TFT related code, based on EasyNextion library by Seithan / Athanasios Seitanis (https://github.com/Seithan/EasyNextionLibrary)
  The trigger(s) functions at the end are called by the Nextion library on event (buttons, page change).

  Completely reworked and simplified.
*/

#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"
#include "EasyNextionLibrary.h"

static volatile int CurrentPage = 0;
static volatile bool TFT_ON = true; // display status

static String temp;
static unsigned long LastAction = 0; // Last action time done on TFT. Go to sleep after TFT_SLEEP
static char HourBuffer[9];

// Variables used to refresh buttons only on status change
static bool TFT_Automode = false;
static bool TFT_Filt = false;
static bool TFT_Robot = false;
static bool TFT_Lights = false;
static bool TFT_Relay = false;
static bool TFT_Winter = false;

// Nextion TFT object. Choose which ever Serial port
// you wish to connect to (not "Serial" which is used for debug), here Serial2 UART
static EasyNex myNex(Serial2);

// Functions prototypes
void InitTFT(void);
void ResetTFT(void);
void UpdateTFT(void);
void UpdateWiFi(bool);

// Reset TFT at start of controller - Change transmission rate to 115200 bauds on both side (Nextion then ESP)
// could have been not in HMI file, but it is good to know that after reset the Nextion goes back to 9600 bauds
void ResetTFT()
{
  myNex.begin(115200);
  myNex.writeStr("sleep=0");
  myNex.writeStr(F("rest"));
  myNex.writeStr(F("wup=0"));
  delay(1000);
}

void InitTFT()
{
  TFT_Automode = storage.AutoMode;
  TFT_Filt     = storage.FiltrationOn;
  TFT_Winter   = storage.WinterMode;
  TFT_Robot    = storage.RobotOn;
  TFT_Lights   = storage.LightOn; 
  TFT_Relay    = storage.RelayOn; 
#ifdef ELECTROLYSE 
  myNex.writeNum(F("page0.vaElectro.val"), 1);
#else
  myNex.writeNum(F("page0.vaElectro.val"), 0);
#endif
  
  myNex.writeStr(F("page3.vaMCFW.txt"), FIRMW);
  myNex.writeStr(F("page3.vaTFTFW.txt"), TFT_FIRMW); 
  myNex.writeNum(F("page0.vaMode.val"), storage.AutoMode);
  myNex.writeNum(F("page1.vaLights.val"), storage.LightOn);
  myNex.writeNum(F("page1.vaFilt.val"), storage.FiltrationOn);
  myNex.writeNum(F("page1.vaRobot.val"), storage.RobotOn);
  myNex.writeNum(F("page1.vaWinter.val"), storage.WinterMode);
  myNex.writeNum(F("page1.vaRelay.val"), storage.RelayOn);
}

void UpdateWiFi(bool wifi)
{
  if (wifi)
  {
    temp = "WiFi: " + WiFi.SSID();
    myNex.writeStr("page3.vaSSID.txt", temp);
    temp = "IP: " + WiFi.localIP().toString();
    myNex.writeStr("page3.vaIP.txt", temp);
  }
  else
  {
    myNex.writeStr("page3.vaSSID.txt", "Not connected");
    myNex.writeStr("page3.vaIP.txt", "");
  }
}


// Function to update TFT display
// update the global variables of the TFT + the widgets of the active page
// call this function at least every second to ensure fluid display
void UpdateTFT()
{

  // Has any button been touched? If yes, one of the trigger routines
  // will fire
  myNex.NextionListen();

  // Updates done only if TFT ON, useless (and not taken into account) if not
  if (TFT_ON)
  {
    sprintf(HourBuffer, "%02d:%02d:%02d", hour(), minute(), second());
    myNex.writeStr("page0.vaTime.txt", HourBuffer);
    myNex.writeNum("page0.vaNetW.val", MQTTConnection ? 1 : 0);
    temp = String(storage.FiltrationStart) + F("/") + String(storage.FiltrationStop) + F("h");
    myNex.writeStr(F("page0.vaStaSto.txt"), temp);

    if(TFT_Automode != storage.AutoMode) 
    { myNex.writeNum(F("page0.vaMode.val"),storage.AutoMode);
      TFT_Automode = storage.AutoMode;}
    if(TFT_Winter != storage.WinterMode) 
    { myNex.writeNum(F("page1.vaWinter.val"),storage.WinterMode);
      TFT_Winter = storage.WinterMode;}  

    myNex.writeNum(F("page0.vapHLevel.val"), PhPump.TankLevel() ? 0 : 1);
    myNex.writeNum(F("page0.vaChlLevel.val"), ChlPump.TankLevel() ? 0 : 1);
    myNex.writeNum(F("page0.vaPSIErr.val"), PSIError ? 1 : 0);
    myNex.writeNum(F("page0.vaChlUTErr.val"), ChlPump.UpTimeError ? 1 : 0);
    myNex.writeNum(F("page0.vapHUTErr.val"), PhPump.UpTimeError ? 1 : 0);

    if (CurrentPage == 0)
    {
      myNex.writeStr(F("page0.vapH.txt"), String(storage.PhValue, 2));
      myNex.writeStr(F("page0.vaOrp.txt"), String(storage.OrpValue, 0));
      temp = "(" + String(storage.Ph_SetPoint, 1) + ")";
      myNex.writeStr(F("page0.vapHSP.txt"), temp);
      temp = "(" + String((int)storage.Orp_SetPoint) + ")";
      myNex.writeStr(F("page0.vaOrpSP.txt"), temp);
      temp = (PhPID.GetMode() == 1) ? F("PID") : F("---");
      myNex.writeStr(F("page0.vapHPID.txt"), temp);
      temp = (OrpPID.GetMode() == 1) ? F("PID") : F("---");
      myNex.writeStr(F("page0.vaOrpPID.txt"), temp);
      temp = String(storage.TempValue, 1) + (char)176 + F("C");
      myNex.writeStr(F("page0.vaWT.txt"), temp);
      temp = String(storage.TempExternal, 1) + (char)176 + F("C");
      myNex.writeStr(F("page0.vaAT.txt"), temp);
      temp = String(storage.PSIValue, 2) + F("b");
      myNex.writeStr(F("page0.vaPSI.txt"), temp);
      temp = String((int)round(PhPump.GetTankFill())) + (char)37 + F(" / ") + String(float(PhPump.UpTime) / 1000. / 60., 1) + F("min");
      myNex.writeStr(F("page0.vapHTk.txt"), temp);
      temp = String((int)round(ChlPump.GetTankFill())) + (char)37 + F(" / ") + String(float(ChlPump.UpTime) / 1000. / 60., 1) + F("min");
      myNex.writeStr(F("page0.vaChlTk.txt"), temp);

      if (abs(storage.PhValue - storage.Ph_SetPoint) <= 0.1)
        myNex.writeNum("page0.vapHErr.val", 0);
      if (abs(storage.PhValue - storage.Ph_SetPoint) > 0.1 && abs(storage.PhValue - storage.Ph_SetPoint) <= 0.2)
        myNex.writeNum("page0.vapHErr.val", 1);
      if (abs(storage.PhValue - storage.Ph_SetPoint) > 0.2)
        myNex.writeNum("page0.vapHErr.val", 2);
      if (abs(storage.OrpValue - storage.Orp_SetPoint) <= 20.)
        myNex.writeNum("page0.vaOrpErr.val", 0);
      if (abs(storage.OrpValue - storage.Orp_SetPoint) > 20. && abs(storage.OrpValue - storage.Orp_SetPoint) <= 40.)
        myNex.writeNum("page0.vaOrpErr.val", 1);
      if (abs(storage.OrpValue - storage.Orp_SetPoint) > 40.)
        myNex.writeNum("page0.vaOrpErr.val", 2);
    }

    if(CurrentPage == 1)
    {
      if(TFT_Filt != storage.FiltrationOn)
      { myNex.writeNum(F("page1.vaFilt.val"), storage.FiltrationOn);
        TFT_Filt = storage.FiltrationOn;}
      if(TFT_Robot != storage.RobotOn)
      { myNex.writeNum(F("page1.vaRobot.val"), storage.RobotOn);
        TFT_Robot = storage.RobotOn;}
      if(TFT_Lights != storage.LightOn)
      { myNex.writeNum(F("page1.vaLights.val"), storage.LightOn);
        TFT_Lights = storage.LightOn;}
      if(TFT_Relay != storage.RelayOn)
      { myNex.writeNum(F("page1.vaRelay.val"), storage.RelayOn);
        TFT_Relay = storage.RelayOn;}
    } 

    if (CurrentPage == 2)
    {
      myNex.writeStr(F("page0.vapH.txt"), String(storage.PhValue, 2));
      myNex.writeStr(F("page0.vaOrp.txt"), String(storage.OrpValue, 0));
    }

    if (CurrentPage == 3)
    {
      UpdateWiFi(WiFi.status() == WL_CONNECTED);
      myNex.writeStr(F("page3.vaMCFW.txt"), FIRMW);
      myNex.writeStr(F("page3.vaTFTFW.txt"), TFT_FIRMW);
    }

    // put TFT in sleep mode with wake up on touch and force page 0 load to trigger an event
    if ((unsigned long)(millis() - LastAction) >= TFT_SLEEP && TFT_ON && CurrentPage != 2)
    {
      myNex.writeStr("thup=1");
      myNex.writeStr("wup=0");
      myNex.writeStr("sleep=1");
      TFT_ON = false;
    }
  }
}

// Page 0 has finished loading
// printh 23 02 54 01
void trigger1()
{
  CurrentPage = 0;
  if (!TFT_ON)
  {
    UpdateWiFi(WiFi.status() == WL_CONNECTED);
    TFT_ON = true;
  }
  LastAction = millis();
}

// Page 1 has finished loading
// printh 23 02 54 02
void trigger2()
{
  CurrentPage = 1;
  LastAction = millis();
}

// Page 2 has finished loading
// printh 23 02 54 03
void trigger3()
{
  CurrentPage = 2;
  LastAction = millis();
}

// Page 3 has finished loading
// printh 23 02 54 04
void trigger4()
{
  CurrentPage = 3;
  LastAction = millis();
}

// MODE button was toggled
// printh 23 02 54 05
void trigger5()
{
  char Cmd[100] = "{\"Mode\":1}";
  TFT_Automode = !TFT_Automode;
  bool Toggle = (bool)myNex.readNumber("page0.vaMode.val");
  if(!Toggle) Cmd[8] = '0';
  xQueueSendToBack(queueIn,&Cmd,0);
  Debug.print(DBG_INFO, Cmd);
  LastAction = millis();
}


// FILT button was toggled
// printh 23 02 54 06
void trigger6()
{
  char Cmd[100] = "{\"FiltPump\":1}";
  TFT_Filt = !TFT_Filt;
  bool Toggle = (bool)myNex.readNumber("page1.vaFilt.val");
  if (!Toggle) Cmd[12] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  Debug.print(DBG_INFO, Cmd);
  LastAction = millis();
}

// Robot button was toggled
// printh 23 02 54 07
void trigger7()
{
  char Cmd[100] = "{\"RobotPump\":1}";
  TFT_Robot = !TFT_Robot;
  bool Toggle = (bool)myNex.readNumber("page1.vaRobot.val");
  if (!Toggle) Cmd[13] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  Debug.print(DBG_INFO, Cmd);
  LastAction = millis();
}

// Light tbutton was toggled 
// printh 23 02 54 08
void trigger8()
{
  char Cmd[100] = "{\"Light\":1}";
  TFT_Lights = !TFT_Lights;
  bool Toggle = (bool)myNex.readNumber("page1.vaLights.val");
  if (!Toggle) Cmd[9] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  Debug.print(DBG_INFO, Cmd); 
  LastAction = millis();
}

// ajout
// Relay or Electrolyser button was toggled
// printh 23 02 54 09
void trigger9()
{
  char Cmd[100] = "{\"Relay\":1}";
  TFT_Relay = !TFT_Relay;
  bool Toggle = (bool)myNex.readNumber("page1.vaRelay.val");
  if (!Toggle) Cmd[9] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  Debug.print(DBG_INFO, Cmd);
  LastAction = millis();
}


// Winter button was toggled
// printh 23 02 54 0A
void trigger10()
{
  char Cmd[100] = "{\"Winter\":1}";
  TFT_Winter = !TFT_Winter;
  bool Toggle = (bool)myNex.readNumber("page1.vaWinter.val");
  if (!Toggle) Cmd[10] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  Debug.print(DBG_INFO, Cmd);
  LastAction = millis();
}

// Probe calibration completed or new pH, Orp or Water Temp setpoints or New tank
// printh 23 02 54 0B
void trigger11()
{
  char Cmd[100] = "";
  strcpy(Cmd, myNex.readStr(F("pageCalibs.vaCommand.txt")).c_str());
  xQueueSendToBack(queueIn, &Cmd, 0);
  Debug.print(DBG_VERBOSE, "Nextion cal page command: %s", Cmd);
  LastAction = millis();
}

// Clear Errors button pressed
// printh 23 02 54 0C
void trigger12()
{
  char Cmd[100] = "{\"Clear\":1}";
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// pH PID button pressed
// printh 23 02 54 0D
void trigger13()
{
  char Cmd[100] = "{\"PhPID\":1}";
  if (PhPID.GetMode() == 1)
    Cmd[9] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Orp PID button pressed
// printh 23 02 54 0E
void trigger14()
{
  char Cmd[100] = "{\"OrpPID\":1}";
  if (OrpPID.GetMode() == 1)
    Cmd[10] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}
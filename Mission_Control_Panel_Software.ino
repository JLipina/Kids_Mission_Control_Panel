/**
 * \file demo.ino
 *
 * \brief Example sketch of using the VS1053 Arduino driver, demonstrating all methods and functions.
 * \remarks comments are implemented with Doxygen Markdown format
 *
 * \author Bill Porter
 * \author Michael P. Flaga
 *
 * This sketch listens for commands from a serial terminal (like the Serial
 * Monitor in the Arduino IDE). If it sees 1-9 it will try to play an MP3 file
 * named track00x.mp3 where x is a number from 1 to 9. For eaxmple, pressing
 * 2 will play 'track002.mp3'. A lowe case 's' will stop playing the mp3.
 * 'f' will play an MP3 by calling it by it's filename as opposed to a track
 * number.
 *
 * Sketch assumes you have MP3 files with filenames like "track001.mp3",
 * "track002.mp3", etc on an SD card loaded into the shield.
 */

#include <SPI.h>
#include <FreeStack.h>
#include <Adafruit_MCP23X17.h>

//Add the SdFat Libraries
#include <SdFat.h>

//and the MP3 Shield Library
#include <vs1053_SdFat.h>

/**
 * \brief Object instancing the SdFat library.
 *
 * principal object for handling all SdCard functions.
 */
SdFat sd;

/**
 * \brief Object instancing the vs1053 library.
 *
 * principal object for handling all the attributes, members and functions for the library.
 */
vs1053 MP3player;

const uint8_t VOLUME =  0; //0x0000 is maximum volume or 0db attenuation. Total silence is 0xFEFE

#include <Wire.h>
#include <SparkFun_Alphanumeric_Display.h>  //Click here to get the library: http://librarymanager/All#SparkFun_Qwiic_Alphanumeric_Display by SparkFun
HT16K33 display;

/**
 * \brief Object instancing the MCP23X17 library.
 *
 * principal object for handling all the attributes, members and functions for the library.
 */
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;
Adafruit_MCP23X17 mcp3;
Adafruit_MCP23X17 mcp4;

#define O2_FAN_LED              8
#define O2_FAN_SW               9
#define C2_FAN_LED              10
#define C2_FAN_SW               11
#define PUMPS_LED               12
#define PUMPS_SW                13
#define HEAT_LED                14
#define HEAT_SW                 15

#define DOCKING_LED             0
#define DOCKING_SW              1
#define LIGHTING_LED            2
#define LIGHTING_SW             3
#define LM_POWER_LED            4
#define LM_POWER_SW             5
#define WASTE_DUMP_LED          6
#define WASTE_DUMP_SW           7

#define BOOSTER_LOW_PITCH_LED   14
#define BOOSTER_LOW_PITCH_SW    15
#define BOOSTER_LOW_YAW_LED     12
#define BOOSTER_LOW_YAW_SW      13
#define BOOSTER_LOW_ROLL_LED    10
#define BOOSTER_LOW_ROLL_SW     11
#define BOOSTER_MED_PITCH_LED   8
#define BOOSTER_MED_PITCH_SW    9
#define BOOSTER_MED_YAW_LED     14
#define BOOSTER_MED_YAW_SW      15
#define BOOSTER_MED_ROLL_LED    12
#define BOOSTER_MED_ROLL_SW     13
#define BOOSTER_HIGH_PITCH_LED  10
#define BOOSTER_HIGH_PITCH_SW   11
#define BOOSTER_HIGH_YAW_LED    8
#define BOOSTER_HIGH_YAW_SW     9
#define BOOSTER_HIGH_ROLL_LED   6
#define BOOSTER_HIGH_ROLL_SW    7

#define PYRO_MAIN               1
#define PYRO_STAGE              0
#define PYRO_CM_LM              3
#define PYRO_ROVER              2
#define PYRO_PARACHUTE          4

#define GO_FOR_LAUNCH_LED       4
#define GO_FOR_LAUNCH_SW        5
#define LUNAR_LANDING_LED       6
#define LUNAR_LANDING_SW        7
#define HEAD_HOME_LED           0
#define HEAD_HOME_SW            1
#define SPLASHDOWN_LED          2
#define SPLASHDOWN_SW           3
#define MASTER_ALARM_LED        4
#define MASTER_ALARM_SW         5
#define GO_FOR_LAUNCH_LED_SPEED 300
unsigned long launchLedBlinkStart;
unsigned long launchLedBlinkCurr;
bool GO_LAUNCH_LED_State =      1;

#define LAUNCH_SW               5
#define ABORT_SW                6

#define PATH_LED_1              3
#define PATH_LED_2              2
#define PATH_LED_3              15
#define PATH_LED_4              10
#define PATH_LED_5              9
#define PATH_LED_6              8
#define PATH_LED_7              12
#define PATH_LED_8              13
#define PATH_LED_9              14
#define PATH_LED_10             11
#define PATH_LED_11             0
#define PATH_LED_12             1
#define PATH_SPEED              2000
int pathPosition =              1;
bool changePosition =           FALSE;

bool O2_FAN_Played = FALSE;
bool C2_FAN_Played = FALSE;
bool PUMPS_Played = FALSE;
bool HEAT_Played = FALSE;
bool GO_FOR_LAUNCH_Played = FALSE;
bool MASTER_ALARM_Played = FALSE;
bool BOOSTER_LOW_PITCH_Played = FALSE;
bool BOOSTER_LOW_YAW_Played = FALSE;
bool BOOSTER_LOW_ROLL_Played = FALSE;
bool BOOSTER_MED_PITCH_Played = FALSE;
bool BOOSTER_MED_YAW_Played = FALSE;
bool BOOSTER_MED_ROLL_Played = FALSE;
bool BOOSTER_HIGH_PITCH_Played = FALSE;
bool BOOSTER_HIGH_YAW_Played = FALSE;
bool BOOSTER_HIGH_ROLL_Played = FALSE;

unsigned long startMillis;
unsigned long currentMillis;

//------------------------------------------------------------------------------
/**
 * \brief Setup the Arduino Chip's feature for our use.
 *
 * After Arduino's kernel has booted initialize basic features for this
 * application, such as Serial port and MP3player objects with .begin.
 * Along with displaying the Help Menu.
 *
 * \note returned Error codes are typically passed up from MP3player.
 * Whicn in turns creates and initializes the SdCard objects.
 *
 * \see
 * \ref Error_Codes
 */
void setup() {

  uint8_t result; //result code from some function as to be tested at later time.

  Serial.begin(115200);

  Serial.print(F("F_CPU = "));
  Serial.println(F_CPU);
  Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
  Serial.print(FreeStack(), DEC);  // FreeStack() is provided by SdFat
  Serial.println(F(" Should be a base line of 1028, on ATmega328 when using INTx"));

  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_FULL_SPEED)) sd.initErrorHalt();
  // depending upon your SdCard environment, SPI_HAVE_SPEED may work better.
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");

  //Initialize the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
    if( result == 6 ) {
      Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
      Serial.println(F("Use the \"d\" command to verify SdCard can be read")); // can be removed for space, if needed.
    }
  }

  MP3player.setVolume(VOLUME); //Set panel volume once since there's no method of adjusting in software while in normal use.

  Wire.begin(); //Display joins I2C bus
  display.begin(0x71, 0x70);
  display.print("DYNETICS");

  mcp1.begin_I2C(0X20);
  mcp2.begin_I2C(0X21);
  mcp3.begin_I2C(0X22);
  mcp4.begin_I2C(0X23);

  mcp1.pinMode(O2_FAN_LED, OUTPUT);
  mcp1.pinMode(O2_FAN_SW, INPUT_PULLUP);
  mcp1.pinMode(C2_FAN_LED, OUTPUT);
  mcp1.pinMode(C2_FAN_SW, INPUT_PULLUP);
  mcp1.pinMode(PUMPS_LED, OUTPUT);
  mcp1.pinMode(PUMPS_SW, INPUT_PULLUP);
  mcp1.pinMode(HEAT_LED, OUTPUT);
  mcp1.pinMode(HEAT_SW, INPUT_PULLUP);

  mcp1.digitalWrite(O2_FAN_LED, LOW);
  mcp1.digitalWrite(C2_FAN_LED, LOW);
  mcp1.digitalWrite(PUMPS_LED, LOW);
  mcp1.digitalWrite(HEAT_LED, LOW);

  mcp3.pinMode(GO_FOR_LAUNCH_LED, OUTPUT);
  mcp3.pinMode(GO_FOR_LAUNCH_SW, INPUT_PULLUP);
  mcp3.pinMode(LUNAR_LANDING_LED, OUTPUT);
  mcp3.pinMode(LUNAR_LANDING_SW, INPUT_PULLUP);
  mcp2.pinMode(HEAD_HOME_LED, OUTPUT);
  mcp2.pinMode(HEAD_HOME_SW, INPUT_PULLUP);
  mcp2.pinMode(SPLASHDOWN_LED, OUTPUT);
  mcp2.pinMode(SPLASHDOWN_SW, INPUT_PULLUP);
  mcp2.pinMode(MASTER_ALARM_LED, OUTPUT);
  mcp2.pinMode(MASTER_ALARM_SW, INPUT_PULLUP);
  
  mcp3.digitalWrite(GO_FOR_LAUNCH_LED, HIGH);
  mcp3.digitalWrite(LUNAR_LANDING_LED, HIGH);
  mcp2.digitalWrite(HEAD_HOME_LED, HIGH);
  mcp2.digitalWrite(SPLASHDOWN_LED, HIGH);
  mcp2.digitalWrite(MASTER_ALARM_LED, LOW);

  mcp4.pinMode(LAUNCH_SW, INPUT_PULLUP);
  mcp4.pinMode(ABORT_SW, INPUT_PULLUP);

  mcp3.pinMode(BOOSTER_LOW_PITCH_LED, OUTPUT);
  mcp3.pinMode(BOOSTER_LOW_PITCH_SW, INPUT_PULLUP);
  mcp3.pinMode(BOOSTER_LOW_YAW_LED, OUTPUT);
  mcp3.pinMode(BOOSTER_LOW_YAW_SW, INPUT_PULLUP);
  mcp3.pinMode(BOOSTER_LOW_ROLL_LED, OUTPUT);
  mcp3.pinMode(BOOSTER_LOW_ROLL_SW, INPUT_PULLUP);
  mcp3.pinMode(BOOSTER_MED_PITCH_LED, OUTPUT);
  mcp3.pinMode(BOOSTER_MED_PITCH_SW, INPUT_PULLUP);
  mcp2.pinMode(BOOSTER_MED_YAW_LED, OUTPUT);
  mcp2.pinMode(BOOSTER_MED_YAW_SW, INPUT_PULLUP);
  mcp2.pinMode(BOOSTER_MED_ROLL_LED, OUTPUT);
  mcp2.pinMode(BOOSTER_MED_ROLL_SW, INPUT_PULLUP);
  mcp2.pinMode(BOOSTER_HIGH_PITCH_LED, OUTPUT);
  mcp2.pinMode(BOOSTER_HIGH_PITCH_SW, INPUT_PULLUP);
  mcp2.pinMode(BOOSTER_HIGH_YAW_LED, OUTPUT);
  mcp2.pinMode(BOOSTER_HIGH_YAW_SW, INPUT_PULLUP);
  mcp2.pinMode(BOOSTER_HIGH_ROLL_LED, OUTPUT);
  mcp2.pinMode(BOOSTER_HIGH_ROLL_SW, INPUT_PULLUP);

  mcp3.digitalWrite(BOOSTER_LOW_PITCH_LED, HIGH);
  mcp3.digitalWrite(BOOSTER_LOW_YAW_LED, HIGH);
  mcp3.digitalWrite(BOOSTER_LOW_ROLL_LED, HIGH);
  mcp3.digitalWrite(BOOSTER_MED_PITCH_LED, HIGH);
  mcp2.digitalWrite(BOOSTER_MED_YAW_LED, HIGH);
  mcp2.digitalWrite(BOOSTER_MED_ROLL_LED, HIGH);
  mcp2.digitalWrite(BOOSTER_HIGH_PITCH_LED, HIGH);
  mcp2.digitalWrite(BOOSTER_HIGH_YAW_LED, HIGH);
  mcp2.digitalWrite(BOOSTER_HIGH_ROLL_LED, HIGH);

  mcp3.pinMode(PATH_LED_1, OUTPUT);
  mcp3.pinMode(PATH_LED_2, OUTPUT);
  mcp4.pinMode(PATH_LED_3, OUTPUT);
  mcp4.pinMode(PATH_LED_4, OUTPUT);
  mcp4.pinMode(PATH_LED_5, OUTPUT);
  mcp4.pinMode(PATH_LED_6, OUTPUT);
  mcp4.pinMode(PATH_LED_7, OUTPUT);
  mcp4.pinMode(PATH_LED_8, OUTPUT);
  mcp4.pinMode(PATH_LED_9, OUTPUT);
  mcp4.pinMode(PATH_LED_10, OUTPUT);
  mcp3.pinMode(PATH_LED_11, OUTPUT);
  mcp3.pinMode(PATH_LED_12, OUTPUT);

  mcp3.digitalWrite(PATH_LED_1, HIGH);
  mcp3.digitalWrite(PATH_LED_2, LOW);
  mcp4.digitalWrite(PATH_LED_3, LOW);
  mcp4.digitalWrite(PATH_LED_4, LOW);
  mcp4.digitalWrite(PATH_LED_5, LOW);
  mcp4.digitalWrite(PATH_LED_6, LOW);
  mcp4.digitalWrite(PATH_LED_7, LOW);
  mcp4.digitalWrite(PATH_LED_8, LOW);
  mcp4.digitalWrite(PATH_LED_9, LOW);
  mcp4.digitalWrite(PATH_LED_10, LOW);
  mcp3.digitalWrite(PATH_LED_11, LOW);
  mcp3.digitalWrite(PATH_LED_12, LOW);

  startMillis = millis();
  launchLedBlinkStart = millis();

}

//------------------------------------------------------------------------------
/**
 * \brief Main Loop the Arduino Chip
 *
 * This is called at the end of Arduino kernel's main loop before recycling.
 * And is where the user's serial input of bytes are read and analyzed by
 * parsed_menu.
 *
 * Additionally, if the means of refilling is not interrupt based then the
 * MP3player object is serviced with the availaible function.
 *
 * \note Actual examples of the libraries public functions are implemented in
 * the parse_menu() function.
 */
void loop() {

// FUEL SWITCHES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

  if(!mcp1.digitalRead(O2_FAN_SW) && O2_FAN_Played == FALSE){
    mcp1.digitalWrite(O2_FAN_LED, HIGH);
    MP3player.playMP3("fuel_o2_fan.mp3");
    O2_FAN_Played = TRUE;
    Serial.println(F("O2 FAN ON"));
  } 
  if(mcp1.digitalRead(O2_FAN_SW) && O2_FAN_Played == TRUE){
    mcp1.digitalWrite(O2_FAN_LED, LOW);
    O2_FAN_Played = FALSE;
    Serial.println(F("O2 FAN OFF"));
  }

    if(!mcp1.digitalRead(C2_FAN_SW) && C2_FAN_Played == FALSE){
    mcp1.digitalWrite(C2_FAN_LED, HIGH);
    MP3player.playMP3("fuel_c2_fan.mp3");
    C2_FAN_Played = TRUE;
    Serial.println(F("C2 FAN ON"));
  } 
  if(mcp1.digitalRead(C2_FAN_SW) && C2_FAN_Played == TRUE){
    mcp1.digitalWrite(C2_FAN_LED, LOW);
    C2_FAN_Played = FALSE;
    Serial.println(F("C2 FAN OFF"));
  }

    if(!mcp1.digitalRead(PUMPS_SW) && PUMPS_Played == FALSE){
    mcp1.digitalWrite(PUMPS_LED, HIGH);
    MP3player.playMP3("fuel_pump.mp3");
    PUMPS_Played = TRUE;
    Serial.println(F("PUMPS ON"));
  } 
  if(mcp1.digitalRead(PUMPS_SW) && PUMPS_Played == TRUE){
    mcp1.digitalWrite(PUMPS_LED, LOW);
    PUMPS_Played = FALSE;
    Serial.println(F("PUMPS OFF"));
  }

  if(!mcp1.digitalRead(HEAT_SW) && HEAT_Played == FALSE){
    mcp1.digitalWrite(HEAT_LED, HIGH);
    MP3player.playMP3("fuel_heat.mp3");
    HEAT_Played = TRUE;
    Serial.println(F("HEAT ON"));
  } 
  if(mcp1.digitalRead(HEAT_SW) && HEAT_Played == TRUE){
    mcp1.digitalWrite(HEAT_LED, LOW);
    HEAT_Played = FALSE;
    Serial.println(F("HEAT OFF"));
  }

// EVENT SEQUENCE BUTTONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

  if(!mcp3.digitalRead(GO_FOR_LAUNCH_SW) && GO_FOR_LAUNCH_Played == FALSE){
    MP3player.playMP3("laser001.mp3");
    GO_FOR_LAUNCH_Played = TRUE;
    Serial.println(F("GO FOR LAUNCH PRESSED"));
  } 
  if(mcp3.digitalRead(GO_FOR_LAUNCH_SW) && GO_FOR_LAUNCH_Played == TRUE){
    GO_FOR_LAUNCH_Played = FALSE;
  }

  launchLedBlinkCurr = millis();
  if(launchLedBlinkCurr - launchLedBlinkStart >= GO_FOR_LAUNCH_LED_SPEED){
    GO_LAUNCH_LED_State = !GO_LAUNCH_LED_State;
    mcp3.digitalWrite(GO_FOR_LAUNCH_LED, GO_LAUNCH_LED_State);
    launchLedBlinkStart = millis();
  }

  if(!mcp2.digitalRead(MASTER_ALARM_SW) && MASTER_ALARM_Played == FALSE){
    mcp2.digitalWrite(MASTER_ALARM_LED, HIGH);
    MP3player.playMP3("event_seq_master_alarm.mp3");
    MASTER_ALARM_Played = TRUE;
    Serial.println(F("MASTER ALARM PRESSED"));
  } 
  if(mcp2.digitalRead(MASTER_ALARM_SW) && MASTER_ALARM_Played == TRUE){
    mcp2.digitalWrite(MASTER_ALARM_LED, LOW);
    MASTER_ALARM_Played = FALSE;
  }

// BOOSTER LOW BUTTONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if(!mcp3.digitalRead(BOOSTER_LOW_PITCH_SW) && BOOSTER_LOW_PITCH_Played == FALSE){
    mcp3.digitalWrite(BOOSTER_LOW_PITCH_LED, LOW);
    MP3player.playMP3("booster_pitch_low.mp3");
    BOOSTER_LOW_PITCH_Played = TRUE;
    Serial.println(F("BOOSTER LOW PITCH TOGGLED"));
  } 
  if(mcp3.digitalRead(BOOSTER_LOW_PITCH_SW) && BOOSTER_LOW_PITCH_Played == TRUE){
    mcp3.digitalWrite(BOOSTER_LOW_PITCH_LED, HIGH);
    BOOSTER_LOW_PITCH_Played = FALSE;
  }

  if(!mcp3.digitalRead(BOOSTER_LOW_YAW_SW) && BOOSTER_LOW_YAW_Played == FALSE){
    mcp3.digitalWrite(BOOSTER_LOW_YAW_LED, LOW);
    MP3player.playMP3("booster_yaw_low.mp3");
    BOOSTER_LOW_YAW_Played = TRUE;
    Serial.println(F("BOOSTER LOW YAW TOGGLED"));
  } 
  if(mcp3.digitalRead(BOOSTER_LOW_YAW_SW) && BOOSTER_LOW_YAW_Played == TRUE){
    mcp3.digitalWrite(BOOSTER_LOW_YAW_LED, HIGH);
    BOOSTER_LOW_YAW_Played = FALSE;
  }

  if(!mcp3.digitalRead(BOOSTER_LOW_ROLL_SW) && BOOSTER_LOW_ROLL_Played == FALSE){
    mcp3.digitalWrite(BOOSTER_LOW_ROLL_LED, LOW);
    MP3player.playMP3("booster_roll_low.mp3");
    BOOSTER_LOW_ROLL_Played = TRUE;
    Serial.println(F("BOOSTER LOW ROLL TOGGLED"));
  } 
  if(mcp3.digitalRead(BOOSTER_LOW_ROLL_SW) && BOOSTER_LOW_ROLL_Played == TRUE){
    mcp3.digitalWrite(BOOSTER_LOW_ROLL_LED, HIGH);
    BOOSTER_LOW_ROLL_Played = FALSE;
  }

  // BOOSTER MEDIUM BUTTONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if(!mcp3.digitalRead(BOOSTER_MED_PITCH_SW) && BOOSTER_MED_PITCH_Played == FALSE){
    mcp3.digitalWrite(BOOSTER_MED_PITCH_LED, LOW);
    MP3player.playMP3("booster_pitch_med.mp3");
    BOOSTER_MED_PITCH_Played = TRUE;
    Serial.println(F("BOOSTER MED PITCH TOGGLED"));
  } 
  if(mcp3.digitalRead(BOOSTER_MED_PITCH_SW) && BOOSTER_MED_PITCH_Played == TRUE){
    mcp3.digitalWrite(BOOSTER_MED_PITCH_LED, HIGH);
    BOOSTER_MED_PITCH_Played = FALSE;
  }

  if(!mcp2.digitalRead(BOOSTER_MED_YAW_SW) && BOOSTER_MED_YAW_Played == FALSE){
    mcp2.digitalWrite(BOOSTER_MED_YAW_LED, LOW);
    MP3player.playMP3("booster_yaw_med.mp3");
    BOOSTER_MED_YAW_Played = TRUE;
    Serial.println(F("BOOSTER MED YAW TOGGLED"));
  } 
  if(mcp2.digitalRead(BOOSTER_MED_YAW_SW) && BOOSTER_MED_YAW_Played == TRUE){
    mcp2.digitalWrite(BOOSTER_MED_YAW_LED, HIGH);
    BOOSTER_MED_YAW_Played = FALSE;
  }

  if(!mcp2.digitalRead(BOOSTER_MED_ROLL_SW) && BOOSTER_MED_ROLL_Played == FALSE){
    mcp2.digitalWrite(BOOSTER_MED_ROLL_LED, LOW);
    MP3player.playMP3("booster_roll_med.mp3");
    BOOSTER_MED_ROLL_Played = TRUE;
    Serial.println(F("BOOSTER MED ROLL TOGGLED"));
  } 
  if(mcp2.digitalRead(BOOSTER_MED_ROLL_SW) && BOOSTER_MED_ROLL_Played == TRUE){
    mcp2.digitalWrite(BOOSTER_MED_ROLL_LED, HIGH);
    BOOSTER_MED_ROLL_Played = FALSE;
  }

  // BOOSTER HIGH BUTTONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if(!mcp2.digitalRead(BOOSTER_HIGH_PITCH_SW) && BOOSTER_HIGH_PITCH_Played == FALSE){
    mcp2.digitalWrite(BOOSTER_HIGH_PITCH_LED, LOW);
    MP3player.playMP3("booster_pitch_high.mp3");
    BOOSTER_HIGH_PITCH_Played = TRUE;
    Serial.println(F("BOOSTER HIGH PITCH TOGGLED"));
  } 
  if(mcp2.digitalRead(BOOSTER_HIGH_PITCH_SW) && BOOSTER_HIGH_PITCH_Played == TRUE){
    mcp2.digitalWrite(BOOSTER_HIGH_PITCH_LED, HIGH);
    BOOSTER_HIGH_PITCH_Played = FALSE;
  }

  if(!mcp2.digitalRead(BOOSTER_HIGH_YAW_SW) && BOOSTER_HIGH_YAW_Played == FALSE){
    mcp2.digitalWrite(BOOSTER_HIGH_YAW_LED, LOW);
    MP3player.playMP3("booster_yaw_high.mp3");
    BOOSTER_HIGH_YAW_Played = TRUE;
    Serial.println(F("BOOSTER HIGH YAW TOGGLED"));
  } 
  if(mcp2.digitalRead(BOOSTER_HIGH_YAW_SW) && BOOSTER_HIGH_YAW_Played == TRUE){
    mcp2.digitalWrite(BOOSTER_HIGH_YAW_LED, HIGH);
    BOOSTER_HIGH_YAW_Played = FALSE;
  }

  if(!mcp2.digitalRead(BOOSTER_HIGH_ROLL_SW) && BOOSTER_HIGH_ROLL_Played == FALSE){
    mcp2.digitalWrite(BOOSTER_HIGH_ROLL_LED, LOW);
    MP3player.playMP3("booster_roll_high.mp3");
    BOOSTER_HIGH_ROLL_Played = TRUE;
    Serial.println(F("BOOSTER HIGH ROLL TOGGLED"));
  } 
  if(mcp2.digitalRead(BOOSTER_HIGH_ROLL_SW) && BOOSTER_HIGH_ROLL_Played == TRUE){
    mcp2.digitalWrite(BOOSTER_HIGH_ROLL_LED, HIGH);
    BOOSTER_HIGH_ROLL_Played = FALSE;
  }

// FLIGHT PATH LEDS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  currentMillis = millis();
  if(currentMillis - startMillis >= PATH_SPEED){
    changePosition = TRUE;
    pathPosition++;
    if(pathPosition >= 13){
      pathPosition = 1;
    }
    startMillis = millis();
  }

  if(changePosition == TRUE){
    if(pathPosition == 1){
      mcp3.digitalWrite(PATH_LED_12, LOW);
      mcp3.digitalWrite(PATH_LED_1, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 2){
      mcp3.digitalWrite(PATH_LED_1, LOW);
      mcp3.digitalWrite(PATH_LED_2, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 3){
      mcp3.digitalWrite(PATH_LED_2, LOW);
      mcp4.digitalWrite(PATH_LED_3, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 4){
      mcp4.digitalWrite(PATH_LED_3, LOW);
      mcp4.digitalWrite(PATH_LED_4, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 5){
      mcp4.digitalWrite(PATH_LED_4, LOW);
      mcp4.digitalWrite(PATH_LED_5, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 6){
      mcp4.digitalWrite(PATH_LED_5, LOW);
      mcp4.digitalWrite(PATH_LED_6, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 7){
      mcp4.digitalWrite(PATH_LED_6, LOW);
      mcp4.digitalWrite(PATH_LED_7, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 8){
      mcp4.digitalWrite(PATH_LED_7, LOW);
      mcp4.digitalWrite(PATH_LED_8, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 9){
      mcp4.digitalWrite(PATH_LED_8, LOW);
      mcp4.digitalWrite(PATH_LED_9, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 10){
      mcp4.digitalWrite(PATH_LED_9, LOW);
      mcp4.digitalWrite(PATH_LED_10, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 11){
      mcp4.digitalWrite(PATH_LED_10, LOW);
      mcp3.digitalWrite(PATH_LED_11, HIGH);
      changePosition = FALSE;
    }
    else if(pathPosition == 12){
      mcp3.digitalWrite(PATH_LED_11, LOW);
      mcp3.digitalWrite(PATH_LED_12, HIGH);
      changePosition = FALSE;
    }
  }


}




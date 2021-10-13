//This is MClock V1 software for Liligo T-Watch 2020 V1 by M.I.N.I.A. program studio.
//Please, be sure that all of necessary libraries are installed!
//Finished (13.11.2021)
//Created with PlatformIO in Atom.
//Kyiv. Ukraine.

#include <iostream>
#include <string>
#include <stdlib.h>
#include "config.h"

using namespace std;

TTGOClass *ttgo;

#define BRIGHTNESS 50 // percentage

#define LENGTH_OF_TOUCH_BUFER 50
#define TIME_OF_TOUCH_BUFER 500 // milliseconds
#define DELAY_OF_TOUCH_BUFER (TIME_OF_TOUCH_BUFER/LENGTH_OF_TOUCH_BUFER)*1000

#define MAX_PROGRAMS 2
#define DEFAULT_PROGRAM 0

#define MAX_SETTINGS 2
#define DEFAULT_SETTING 0

#define BACKGROUND_COLOR TFT_BLACK
#define MAIN_COLOR TFT_WHITE
#define UNMAIN_COLOR1 TFT_GREEN
#define UNMAIN_COLOR2 TFT_RED

bool rtcIrq = false;
char buf[128];

bool in_range(int input, int point, int range ){
  if( input < point + range && input > point - range ){
    return true;
  }else {
    return false;
  }
}





class T_touch
{
  int16_t bufer_x[LENGTH_OF_TOUCH_BUFER];
  int16_t bufer_y[LENGTH_OF_TOUCH_BUFER];
  int start_x = -1;
  int start_y = -1;

  public:


  void begin(){
    int i = 0;
    while( i < LENGTH_OF_TOUCH_BUFER ){
      bufer_x[i] = -1;
      bufer_y[i] = -1;
      i++;
    }
  }//void begin end

  void update(){
    int i = 0;
    while( i < LENGTH_OF_TOUCH_BUFER - 1 ){
      bufer_x[i+1] = bufer_x[i];
      bufer_y[i+1] = bufer_y[i];
      i++;
    }
    int16_t newx, newy;
    ttgo->getTouch( newx, newy );
    if( newx < 240 && newy < 240 && newx >= 0 && newy >= 0 && ttgo->touched()){
      bufer_x[0] = newx;
      bufer_y[0] = newy;
      if(bufer_x[1] == -1 && bufer_y[1] == -1){
        start_x = newx;
        start_y = newy;
      }
    }else{
      bufer_x[0] = -1;
      bufer_y[0] = -1;
      if(bufer_x[1] == -1 && bufer_y[1] == -1){
        start_x = -1;
        start_y = -1;
      }
    }
  }//void update() end

  bool is_touched( int xin, int yin, int range ){
    if( in_range( bufer_x[0], xin, range ) && in_range( bufer_y[0], xin, range ) && bufer_x[0] != -1 && bufer_y[0] != -1 ){
      return true;
    }else {
      return false;
    }
  }//bool is_touched( int xin, int yin, int range ) end

  bool was_touched( int xin, int yin, int range ){
    if( bufer_x[0] != -1 || bufer_y[0] != -1 ){
      return false;
    }
    int i = -1;
    bool still_zero = true;
    bool still_touched = true;
    bool was_in_range = false;
    while( i < LENGTH_OF_TOUCH_BUFER-1 ){
      i++;
      if( ( bufer_x[i] == -1 || bufer_y[i] == -1 ) && still_zero ){
        continue;
      }else{
        still_zero = false;
      }
      if( in_range( bufer_x[i], xin, range ) && in_range( bufer_y[i], yin, range ) && still_touched ){
        was_in_range = true;
        continue;
      }else {
        still_touched = false;
      }
      if( bufer_x[i] != -1 || bufer_y[i] != -1 ){
        return false;
      }

    }
    if( was_in_range ) return true;
    return false;
  }//bool was_touched( int xin, int yin, int range ) end

  bool swipe( int xin0, int yin0, int xin1, int yin1, int rangex, int rangey ){
    if( start_x == -1 || start_y == -1 ) return false;
    if( bufer_x[0] != -1 || bufer_y[0] != -1 ) return false;
    if( in_range( bufer_x[1], xin1, rangex ) && in_range( bufer_y[1], yin1, rangey ) && in_range( start_x, xin0, rangex ) && in_range( start_y, yin0, rangey ) ){
      return true;
    }else{
      return false;
    }
  }//bool swipe( int xin0, int yin0, int xin1, int yin1, int rangex, int rangey ) end


} ts;

void update_all(){
  delayMicroseconds( DELAY_OF_TOUCH_BUFER - ( micros() % DELAY_OF_TOUCH_BUFER ) );
  ts.update();
}

class T_programs{
public:
  int current = DEFAULT_PROGRAM;

  void begin(){
    //clock start

    //clock end
  }//void begin() end

  unsigned long last_time_get = millis();
  void clock(){
      ttgo->tft->fillScreen( BACKGROUND_COLOR );
      ttgo->tft->drawCircle( 120, 100, 100, MAIN_COLOR );
    while(true){
      update_all();
      if(ts.swipe( 0, 0, 239, 239, 60, 60 )){
        break;
      }

      if( last_time_get > millis() + 1 ) last_time_get = millis();
      if( last_time_get + 1000 > millis() ) continue;
      last_time_get = millis();
      snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
      String time_bufer = String( buf );
      float hours = atoi( time_bufer.c_str() );
      String S_minutes = time_bufer.substring(3,5);
      float minutes = atoi( S_minutes.c_str() );
      String S_seconds = time_bufer.substring(6,8);
      float seconds = atoi( S_seconds.c_str() );
      float seconds_angle = seconds * PI * 2 / 60;
      float minutes_angle = ( minutes + ( seconds / 60 ) ) * PI * 2 / 60;
      float hours_angle = ( hours + ( minutes + ( seconds / 60 ) ) / 60 ) * PI * 2 / 12;

      float old_seconds_angle = ( seconds - 1 ) * PI * 2 / 60;
      float old_minutes_angle = ( minutes + ( ( seconds - 1 ) / 60 ) ) * PI * 2 / 60;
      float old_hours_angle = ( hours + ( minutes + ( ( seconds - 1) / 60 ) ) / 60 ) * PI * 2 / 12;

      float very_old_seconds_angle = ( seconds - 2 ) * PI * 2 / 60;
      float very_old_minutes_angle = ( minutes + ( ( seconds - 2) / 60 ) ) * PI * 2 / 60;
      float very_old_hours_angle = ( hours + ( minutes + ( ( seconds - 2 ) / 60 ) ) / 60 ) * PI * 2 / 12;

      ttgo->tft->drawLine(120, 100, 120 + sin( very_old_hours_angle ) * 75, 100 - cos( very_old_hours_angle ) * 75, BACKGROUND_COLOR );
      ttgo->tft->drawLine(120, 100, 120 + sin( very_old_minutes_angle ) * 95, 100 - cos( very_old_minutes_angle ) * 95, BACKGROUND_COLOR );
      ttgo->tft->drawLine(120, 100, 120 + sin( very_old_seconds_angle ) * 95, 100 - cos( very_old_seconds_angle ) * 95, BACKGROUND_COLOR );

      ttgo->tft->drawLine(120, 100, 120 + sin( old_hours_angle ) * 75, 100 - cos( old_hours_angle ) * 75, BACKGROUND_COLOR );
      ttgo->tft->drawLine(120, 100, 120 + sin( old_minutes_angle ) * 95, 100 - cos( old_minutes_angle ) * 95, BACKGROUND_COLOR );
      ttgo->tft->drawLine(120, 100, 120 + sin( old_seconds_angle ) * 95, 100 - cos( old_seconds_angle ) * 95, BACKGROUND_COLOR );




      ttgo->tft->drawLine(120, 100, 120 + sin( hours_angle ) * 75, 100 - cos( hours_angle ) * 75, MAIN_COLOR);
      ttgo->tft->drawLine(120, 100, 120 + sin( minutes_angle ) * 95, 100 - cos( minutes_angle ) * 95, MAIN_COLOR);
      ttgo->tft->drawLine(120, 100, 120 + sin( seconds_angle ) * 95, 100 - cos( seconds_angle ) * 95, UNMAIN_COLOR2);

      ttgo->tft->setCursor( 50, 210 );
      ttgo->tft->setTextFont( 0 );
      ttgo->tft->setTextColor( MAIN_COLOR, BACKGROUND_COLOR );
      ttgo->tft->setTextSize( 3 );
      ttgo->tft->print( ttgo->rtc->formatDateTime() );
    }
  }//void clock() end

  void settings(){

      ttgo->tft->fillScreen( BACKGROUND_COLOR );
      int now_setting = DEFAULT_SETTING;
      ttgo->tft->print("settings");

      bool needs_update = true;
      int time_setting = 0;
      int new_year = 2021;
      int new_month = 6;
      int new_day = 15;
      int new_hour = 12;
      int new_minute = 30;

    while( true ){
      update_all();
      if(ts.swipe( 0, 0, 239, 239, 60, 60 )) return;
      if( ts.swipe( 0, 120, 239, 120, 60, 120 ) ){
        now_setting--;
        needs_update = true;
        if( now_setting < 0 ) now_setting = MAX_SETTINGS - 1;
      }
      if( ts.swipe( 239, 120, 0, 120, 60, 120 ) ){
        now_setting++;
        needs_update = true;
        if( now_setting >= MAX_SETTINGS ) now_setting = 0;
      }
      switch( now_setting ){
        case 0:
          switch( time_setting ){
            case 0:
              if( needs_update ){
                ttgo->tft->fillRect( 0, 0, 240, 240, BACKGROUND_COLOR );
                ttgo->tft->setCursor( 0, 0 );
                ttgo->tft->setTextSize( 3 );
                ttgo->tft->setTextFont( 0 );
                ttgo->tft->setTextColor( MAIN_COLOR );
                ttgo->tft->print( "setting year" );
                ttgo->tft->setCursor( 60, 100 );
                ttgo->tft->setTextFont( 7 );
                ttgo->tft->setTextSize( 1 );
                ttgo->tft->setTextColor( MAIN_COLOR );
                ttgo->tft->print( new_year );
                needs_update = false;
              }
              if( ts.swipe( 120, 239, 120, 0, 120, 60 )){
                needs_update = true;
                new_year++;
                break;
              }
              if( ts.swipe( 120, 0, 120, 239, 120, 60 )){
                needs_update = true;
                new_year--;
                break;
              }
              if( ts.was_touched( 120, 120, 30 ) ){
                needs_update = true;
                time_setting++;
                break;
              }
            case 1:
            if( needs_update ){
              ttgo->tft->fillRect( 0, 0, 240, 240, BACKGROUND_COLOR );
              ttgo->tft->setCursor( 0, 0 );
              ttgo->tft->setTextSize( 3 );
              ttgo->tft->setTextFont( 0 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              ttgo->tft->print( "setting month" );
              ttgo->tft->setCursor( 60, 100 );
              ttgo->tft->setTextFont( 7 );
              ttgo->tft->setTextSize( 1 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              if( new_month < 10 ) ttgo->tft->print( 0 );
              ttgo->tft->print( new_month );
              needs_update = false;
            }
            if( ts.swipe( 120, 239, 120, 0, 120, 60 )){
              needs_update = true;
              new_month++;
              if( new_month > 12 ) new_month = 0;
              break;
            }
            if( ts.swipe( 120, 0, 120, 239, 120, 60 )){
              needs_update = true;
              new_month--;
              if( new_month < 1 ) new_month = 12;
              break;
            }
            if( ts.was_touched( 120, 120, 30 ) ){
              needs_update = true;
              time_setting++;
              break;
            }break;
            case 2:
            if( needs_update ){
              ttgo->tft->fillRect( 0, 0, 240, 240, BACKGROUND_COLOR );
              ttgo->tft->setCursor( 0, 0 );
              ttgo->tft->setTextSize( 3 );
              ttgo->tft->setTextFont( 0 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              ttgo->tft->print( "setting day" );
              ttgo->tft->setCursor( 60, 100 );
              ttgo->tft->setTextFont( 7 );
              ttgo->tft->setTextSize( 1 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              if( new_day < 10 ) ttgo->tft->print( 0 );
              ttgo->tft->print( new_day );
              needs_update = false;
            }
            if( ts.swipe( 120, 239, 120, 0, 120, 60 )){
              needs_update = true;
              new_day++;
              if( new_day > 28 ){
                switch ( new_month ){
                  case 1: if( new_day > 31 ) new_day = 0; break;
                  case 2: if( ( new_day > 28 && new_year % 4 != 0 ) || new_day > 28 ) new_day = 0; break;
                  case 3: if( new_day > 31 ) new_day = 0; break;
                  case 4: if( new_day > 30 ) new_day = 0; break;
                  case 5: if( new_day > 31 ) new_day = 0; break;
                  case 6: if( new_day > 30 ) new_day = 0; break;
                  case 7: if( new_day > 31 ) new_day = 0; break;
                  case 8: if( new_day > 31 ) new_day = 0; break;
                  case 9: if( new_day > 30 ) new_day = 0; break;
                  case 10: if( new_day > 31 ) new_day = 0; break;
                  case 11: if( new_day > 30 ) new_day = 0; break;
                  case 12: if( new_day > 31 ) new_day = 0; break;
                }
              }
              break;
            }
            if( ts.swipe( 120, 0, 120, 239, 120, 60 )){
              needs_update = true;
              new_day--;
              if( new_day < 1 ){
                switch ( new_month ){
                  case 1: new_day = 31; break;
                  case 2: if( new_year % 4 == 0 ){ new_day = 29; }else{ new_day = 28; } break;
                  case 3: new_day = 31; break;
                  case 4: new_day = 30; break;
                  case 5: new_day = 31; break;
                  case 6: new_day = 30; break;
                  case 7: new_day = 31; break;
                  case 8: new_day = 31; break;
                  case 9: new_day = 30; break;
                  case 10: new_day = 31; break;
                  case 11: new_day = 30; break;
                  case 12: new_day = 31; break;
                }
              }
              break;
            }
            if( ts.was_touched( 120, 120, 30 ) ){
              needs_update = true;
              time_setting++;
              break;
            }
            break;
            case 3:
            if( needs_update ){
              ttgo->tft->fillRect( 0, 0, 240, 240, BACKGROUND_COLOR );
              ttgo->tft->setCursor( 0, 0 );
              ttgo->tft->setTextSize( 3 );
              ttgo->tft->setTextFont( 0 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              ttgo->tft->print( "setting hour" );
              ttgo->tft->setCursor( 60, 100 );
              ttgo->tft->setTextFont( 7 );
              ttgo->tft->setTextSize( 1 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              if( new_hour < 10 ) ttgo->tft->print( 0 );
              ttgo->tft->print( new_hour );
              needs_update = false;
            }
            if( ts.swipe( 120, 239, 120, 0, 120, 60 )){
              needs_update = true;
              new_hour++;
              if( new_hour > 23 ) new_hour = 0;
              break;
            }
            if( ts.swipe( 120, 0, 120, 239, 120, 60 )){
              needs_update = true;
              new_hour--;
              if( new_hour < 0 ) new_hour = 23;
              break;
            }
            if( ts.was_touched( 120, 120, 30 ) ){
              needs_update = true;
              time_setting++;
              break;
            }
            break;
            case 4:
            if( needs_update ){
              ttgo->tft->fillRect( 0, 0, 240, 240, BACKGROUND_COLOR );
              ttgo->tft->setCursor( 0, 0 );
              ttgo->tft->setTextSize( 3 );
              ttgo->tft->setTextFont( 0 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              ttgo->tft->print( "setting minute" );
              ttgo->tft->setCursor( 60, 100 );
              ttgo->tft->setTextFont( 7 );
              ttgo->tft->setTextSize( 1 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              if( new_minute < 10 ) ttgo->tft->print( 0 );
              ttgo->tft->print( new_minute );
              needs_update = false;
            }
            if( ts.swipe( 120, 239, 120, 0, 120, 60 )){
              needs_update = true;
              new_minute++;
              if( new_minute > 59 ) new_minute = 0;
              break;
            }
            if( ts.swipe( 120, 0, 120, 239, 120, 60 )){
              needs_update = true;
              new_minute--;
              if( new_minute < 0 ) new_minute = 59;
              break;
            }
            if( ts.was_touched( 120, 120, 30 ) ){
              needs_update = true;
              time_setting = 0;
              pinMode( RTC_INT_PIN, INPUT_PULLUP );
              attachInterrupt( RTC_INT_PIN, [] {
                  rtcIrq = 1;
              }, FALLING);
              ttgo->rtc->setDateTime( new_year, new_month, new_day, new_hour, new_minute, 0 );
              return;
            }
            break;
              default: break;
            }

            break;

          case 1:
            if( needs_update ){
              ttgo->tft->fillRect( 0, 0, 240, 240, BACKGROUND_COLOR );
              ttgo->tft->setCursor( 0, 0 );
              ttgo->tft->setTextSize( 3 );
              ttgo->tft->setTextFont( 0 );
              ttgo->tft->setTextColor( MAIN_COLOR );
              ttgo->tft->print( "Motor test" );
              needs_update = false;
            }
            if( ts.is_touched(120, 120, 60) ){
              digitalWrite( 4, HIGH );
            }else{
              digitalWrite( 4, LOW );
            }
        break;
        default: break;
      }
    }
  }//void settings() end

}program;

class T_Icons {
public:
  void clock(){
    ttgo->tft->drawCircle( 120, 120, 75, MAIN_COLOR );
    ttgo->tft->drawLine( 120, 120, 120, 50, MAIN_COLOR );
    ttgo->tft->drawLine( 120, 120, 160, 120, MAIN_COLOR );
  }
  void settings(){
    ttgo->tft->fillRect( 67, 67, 106, 106, MAIN_COLOR );
    ttgo->tft->fillTriangle(120, 45, 195, 120, 120, 195, MAIN_COLOR );
    ttgo->tft->fillTriangle(120, 45, 45, 120, 120, 195, MAIN_COLOR );
    ttgo->tft->fillCircle( 120, 120, 25, BACKGROUND_COLOR );
  }
}icons;


void setup() {
  pinMode( 4, OUTPUT );
  digitalWrite( 4, LOW );

  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->tft->fillScreen( BACKGROUND_COLOR );

  ts.begin();
  program.begin();

  Serial.begin( 115200 );
  ttgo->tft->setTextFont(2);
  ttgo->tft->setTextSize(1);
  ttgo->setBrightness( BRIGHTNESS );
  ttgo->displayWakeup();

}//setup end


void loop() {
  ttgo->tft->fillScreen( BACKGROUND_COLOR );
  ttgo->tft->setTextColor( UNMAIN_COLOR1 );
  ttgo->tft->setCursor( 0, 0 );
  ttgo->tft->setTextFont( 0 );
  ttgo->tft->setTextSize( 1 );
  ttgo->tft->print("MClock V0 menu");
  ttgo->tft->setTextColor( MAIN_COLOR );
  switch( program.current ){
    case 0: icons.clock(); break;
    case 1: icons.settings(); break;
    default: ttgo->tft->print(" UNDEFINED");break;
  }
  while( true ){
    update_all();
    if( ts.swipe( 0, 120, 239, 120, 60, 90 ) ){
      program.current--;
      if( program.current < 0 ) program.current = MAX_PROGRAMS - 1;
      break;
    }
    if( ts.swipe( 239, 120, 0, 120, 60, 90 ) ){
      program.current++;
      if( program.current >= MAX_PROGRAMS ) program.current = 0;
      break;
    }
    if(ts.was_touched(120,120,30)){
      switch(program.current){
        case 0: program.clock(); break;
        case 1: program.settings(); break;
      }
      break;
    }
  }
}

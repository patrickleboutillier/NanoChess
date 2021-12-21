#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include "SoftwareSerial.h"
#include "AVRMAX.hpp"

#define I2C_ADDRESS 0x3C
#define MAX_NODES   1024
#define MAX_MOVES   99

SSD1306AsciiAvrI2c oled ;
AVRMAX am ;
SoftwareSerial *port = NULL ;
bool is_black = 0 ;
char move[5] ;
int moves = 0 ;


void reset(){
  moves = 0 ;
  if (is_black){
    delay(2000) ;
    port->write("0") ;
  }
  else {
    while (port->available() < 1){} ;
    port->read() ;
    oled.begin(&Adafruit128x64, I2C_ADDRESS) ;
    oled.setFont(Adafruit5x7) ;
    print_board(" ") ;
    strcpy(move, "\n") ;
    am.do_move(move, MAX_NODES) ;
    moves++ ;
    print_board(" ") ;
    port->write(move) ;
  }
}


void setup(){
  pinMode(12, INPUT_PULLUP) ;
  srand(analogRead(A0) << 10 | analogRead(A1)) ;

  if (digitalRead(12)){ // black
    pinMode(A3, OUTPUT) ;
    digitalWrite(A3, HIGH) ; // reset white
    oled.begin(&Adafruit128x64, I2C_ADDRESS) ;
    oled.setFont(Adafruit5x7) ;
    oled.clear() ;
    oled.setCursor(0, 1) ;
    oled.print("      NanoChess") ;
    port = new SoftwareSerial(6, 7) ;
    is_black = 1 ;
  }
  else {
    port = new SoftwareSerial(A3, A2) ;
  }
  port->begin(9600) ;

  reset() ;
}
  

void print_board(const char *del){
  oled.setCursor(0, 0) ;
  char sp[] = " " ;
  for (int r = 7 ; r >=0 ; r--){
    if (! is_black){
      oled.print(sp) ;
      oled.print(sp) ;
    }
    else {
      switch (r){
        case 7:
        case 6:
        case 5:
          oled.print(sp) ;
          oled.print(del) ;
          break ;
        case 3:
          oled.print(move[0]) ;
          oled.print(move[1]) ;
          break ;      
        case 2:
          oled.print(move[2]) ;
          oled.print(move[3]) ;
          break ;      
        case 0:
          oled.print(moves / 10) ;
          oled.print(moves % 10) ;
          break ;
        default:
          oled.print(sp) ;
          oled.print(sp) ;
      }
    }
    oled.print(sp) ;

    char row[17] ;
    am.print_row(r, row) ;
    oled.print(row) ;

    if (is_black){
      oled.print(sp) ;      
      oled.print(sp) ;
    } 
    else {
      switch (r){
        case 7:
        case 6:
        case 5:
          oled.print(sp) ;
          oled.print(del) ;
          break ;
        case 3:
          if (!is_black){
            oled.print(move[0]) ;
            oled.print(move[1]) ;
          }
          break ;      
        case 2:
          if (!is_black){
            oled.print(move[2]) ;
            oled.print(move[3]) ;
          }
          break ;      
        case 0:
          if (!is_black){
            oled.print(moves / 10) ;
            oled.print(moves % 10) ;
          }
          break ;
        default:
          oled.print(sp) ;
          oled.print(sp) ;
      }  
    }
    oled.println() ;
  }
}


void wait(){
    while(1){} ;  
}


void loop(){
  // Wait for opponent move.
  while (port->available() < 4){} ;
  for (int i = 0 ; i < 4 ; i++){
    move[i] = port->read() ;
  }
  move[4] = '\0' ;
  
  am.do_move(move, MAX_NODES) ;
  strcpy(move, "\n") ;
  am.do_move(move, MAX_NODES) ;
  if (move[0] == 'X'){
    strcpy(move, "    ") ;
    print_board(is_black ? ">" : "<") ;
    wait() ;
  }
  else if (move[0] == '='){
    strcpy(move, "    ") ;
    print_board("=") ;
    wait() ;
  }
  else {
    moves++ ;
    if (moves >= MAX_MOVES){
      strcpy(move, "    ") ;
      print_board("=") ;
      wait() ;    
    }
    else {
      print_board(" ") ;
      port->write(move) ;
    }
  }
}

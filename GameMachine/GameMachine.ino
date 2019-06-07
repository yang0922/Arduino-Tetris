/***********************************************************************
 * A game machine implementation of Tetris.
 * 
 *  -Controlled by a handheld device made by a 3d accelerometer MPU-6050
 *  -Display on 32x16 Adafruit RGB LED matrix panel
 *  -Use Arduino UNO as controller
 *  
 * By Mengwei Yuan & Yang Wang 
 ***********************************************************************/

// RGEmatrix - Gaming Didsplay 
#include <RGBmatrixPanel.h>
#define CLK  8
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// All types of colors to be used
#define BLACK  matrix.Color333(0, 0, 0)
#define GRAY   matrix.Color333(1, 1, 1)
#define YELLOW matrix.Color333(2, 1, 0)
#define BLUE   matrix.Color333(0, 0, 1)
#define GREEN  matrix.Color333(0, 1, 0)
#define ORANGE matrix.Color333(2, 1, 0)
#define RED    matrix.Color333(1, 0, 0)
#define PURPLE matrix.Color333(1, 0, 1)
#define CYAN   matrix.Color333(0, 1, 1)

/*
 * Tetrominoes representation in binary bits, within a 4x4 digit area.
 * All possible shapes in 4 directions of 7 types of tetrominoes 
 */
const uint16_t tetrominoes[28] = { 
  // xxxx
  0b1111000000000000, 0b0100010001000100, 0b1111000000000000, 0b0010001000100010,
  // xx
  // xx
  0b0110011000000000, 0b0110011000000000, 0b0110011000000000, 0b0110011000000000,
  // xx
  //  xx
  0b0110001100000000, 0b0010011001000000, 0b0110001100000000, 0b0010011001000000,
  //  xx
  // xx
  0b0110110000000000, 0b0100011000100000, 0b0110110000000000, 0b0100011000100000,
  // xxx
  // x
  0b0111010000000000, 0b0100010001100000, 0b0001011100000000, 0b0110001000100000,
  // xxx
  //   x
  0b1110001000000000, 0b0110010001000000, 0b0100011100000000, 0b0010001001100000,
  // xxx
  //  x
  0b1110010000000000, 0b0100011001000000, 0b0100111000000000, 0b0010011000100000};

/*
 * Numbers representation in binary bits.
 * Each number is represented in a 3x5 digit area, so the first bit is left empty.
 */
const uint16_t numbers[10] ={
  0b0111101101101111,     // 0
  0b0010010010010010,     // 1
  0b0111001111100111,     // 2
  0b0111001111001111,     // 3
  0b0101101111001001,     // 4
  0b0111100111001111,     // 5
  0b0111100111101111,     // 6
  0b0111001001001001,     // 7
  0b0111101111101111,     // 8
  0b0111101111001111 };   // 9

#define  WIDTH        16
#define  HEIGHT       23
#define  EDGE         3
#define  TETRO_SIZE   4

#define  OPERATION_PERIOD      200
#define  TETRO_FALLING_PERIOD  1000
unsigned long lastOp    = 0;
unsigned long lastFall  = 0;

uint8_t  left      = 0,
         right     = 0,
         rotation  = 0,
         speedup   = 0;

int      buzzer = 13;
int      button = 12;
boolean  pixels[HEIGHT][WIDTH];
boolean  block[TETRO_SIZE][TETRO_SIZE];
  
uint8_t  blockCursor_x;
uint8_t  blockCursor_y;

uint16_t background_color = GRAY;
uint16_t score_color      = BLUE;
boolean  gameOver         = false;
uint8_t  scores           = 0;

/*******************************************************************************************
 * Tetromino class.
 * 
 * All tetrominoes have their information stored in this kind of objects. 
 * Containing information including the index in the tetromino table, the binary 
 * representaion of the shape, and the color.
 *******************************************************************************************/
class Tetromino 
{
  uint16_t tetro;
  uint16_t color;
  uint8_t  index;

  public: 
  // Constructor
  Tetromino () {
  }
  
  // Update this tetromino object with provided information.
  void Update( uint8_t n_index, uint16_t n_color){
    tetro = tetrominoes[n_index];
    color = n_color;
  }
  
  // Assign new tetromino values to this object.
  void getNew() {
    index = millis()%28;
    tetro = tetrominoes[index];
    switch (millis()%8) {
      case 0: color = GRAY;    break;
      case 1: color = YELLOW;  break;
      case 2: color = BLUE;    break;
      case 3: color = GREEN;   break;
      case 4: color = ORANGE;  break;
      case 5: color = RED;     break;
      case 6: color = PURPLE;  break;
      case 7: color = CYAN;    break;
    }  
  }
  
  // Return information accordingly.
  uint8_t retIndex(){
    return index;
  }
  uint16_t retColor(){
    return color;
  }
  uint16_t retTetro(){
    return tetro;
  }
};

Tetromino current_tetro;
Tetromino next_tetro;

/*********************************************************************************************
 * Main flow of the project.
 * 
 * Set up all peripherals and start and control the game.
 * This is the main structure of the project and is mostly calling other functions.
 **********************************************************************************************/
void setup() {                
  matrix.begin();
  Serial.begin(9600);
  pinMode(button, INPUT);
  pinMode(buzzer,OUTPUT);
  accelerometer_setup();
  current_tetro.getNew();
  next_tetro.getNew();
  reset();
//  timercount_setup();
}

/*
 * Reset current game and start a new game.
 *  -Set up the background display when the game starts
 *  -Set up the margins and clean up the playground
 *  -Drop a tetromino from middle top to start the game at last 
 */
void reset () {
  // draw the background of upper part where scores and next tetromino are displayed
  // horizontal lines
  matrix.drawLine(24, 0, 24, 15, background_color);
  matrix.drawLine(23, 0, 23, 15, background_color);
  matrix.drawLine(25, 0, 25, 15, background_color);
  matrix.drawLine(31, 0, 31, 15, background_color);
  // vertical lines
  matrix.drawLine(31, 0,  26, 0,  background_color);
  matrix.drawLine(31, 8,  26, 8,  background_color);
  matrix.drawLine(31, 10, 23, 10, background_color);
  matrix.drawLine(31, 15, 23, 15, background_color);

  // set up the playground in the lower part of RGB matrix panel
  for(uint8_t i = EDGE; i < HEIGHT; i++){
    for(uint8_t j = EDGE; j < WIDTH - EDGE; j++){
      pixels[i][j] = false;
    }
  }
  for(uint8_t i = 0; i < HEIGHT; i++){
    for(uint8_t j = 0; j < EDGE; j++){
      pixels[i][j] = true;
      pixels[i][15-j] = true;
    }
  }
  for(uint8_t i = 0; i < EDGE; i++){
    for(uint8_t j = EDGE; j < WIDTH - EDGE; j++){
      pixels[i][j] = true;
    }
  }
  updatedisp();
  
  gameOver = false;
  scores = 0;
  printScore();
  animation();
  delay(1000);
  dropnew();
}

/*
 * Drop the next tetromino..
 * Update the current tetromino and randomly generate a new tetromino.
 */
void dropnew () {
  current_tetro.Update(next_tetro.retIndex(), next_tetro.retColor());
  next_tetro.getNew();
  previewtetro();
  uint2bool(current_tetro.retTetro());
  blockCursor_x = 6;
  blockCursor_y = 22;
  drawtetro(true);
  if(ifCollision('d') == true){
    gameOver = true;
  } else {
    drawtetro(true);
  }
}

/*
 * Repeatedly called to continue the game.
 * If the game is over, play the animation and wait for the button to restart the game.
 * Otherwise, the system continues to read operations and keep the current tetromino moving down.
 */
void loop () {
  if(!gameOver){
    determineOperation();
    operationHandler();
    if((millis()-lastFall)>TETRO_FALLING_PERIOD){
      moveDown();
      lastFall = millis(); 
    } 
  } else {
    gameOver = true;
    animation();
    while(1){
     if(digitalRead(button)==HIGH){
       gameOver = false;
       reset();
       break;
     }
    }
  }
}

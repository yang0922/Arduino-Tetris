/*
 * A game machine implementation of Tetris 
 *  -Controlled by a handheld device made by a 3d accelerometer MPU6050
 *  -Display on 32x16 Adafruit RGB LED matrix panel
 *  -Use Arduino UNO as controller
 *  
 * By Mengwei Yuan & Yang Wang 
 */

// RGEmatrix - Gaming Didsplay 
#include <RGBmatrixPanel.h>
#define CLK  8
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);
#define BLACK  matrix.Color333(0, 0, 0)
#define GRAY   matrix.Color333(1, 1, 1)
#define YELLOW matrix.Color333(2, 1, 0)
#define BLUE   matrix.Color333(0, 0, 1)
#define GREEN  matrix.Color333(0, 1, 0)
#define ORANGE matrix.Color333(2, 1, 0)
#define RED    matrix.Color333(1, 0, 0)
#define PURPLE matrix.Color333(1, 0, 1)
#define CYAN   matrix.Color333(0, 1, 1)




int countt=0;
/*
 * Tetrominoes representation in binary bits, within a 4x4 digit area
 * All possible shapes in four directions of seven types of tetrominoes 
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
 * Numbers representation in binary bits
 * Each number is represented in a 3x5 digit area, so the first bit is left empty
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

unsigned long lastFall =0;
#define  OPERATION_PERIOD      200
#define  TETRO_FALLING_PERIOD  1000
unsigned long lastOp    = 0;
uint8_t  left      = 0,
         right     = 0,
         rotation  = 0,
         speedup   = 0;

         int buzzer=13;
boolean  pixels[HEIGHT][WIDTH];
boolean  block[TETRO_SIZE][TETRO_SIZE];
uint16_t current_tetro, next_tetro;
uint16_t current_color, next_color;
uint8_t  current_index, next_index;
uint8_t  blockCursor_x;
uint8_t  blockCursor_y;

uint16_t background_color = GRAY;
uint16_t score_color      = BLUE;
boolean  gameOver         = false;
uint8_t  scores           = 0;

void setup() {                
  matrix.begin();
  Serial.begin(9600);
  pinMode(12, INPUT);
  pinMode(buzzer,OUTPUT);
  accelerometer_setup();
  openingAnimation();
  reset();
//  timercount_setup();
}




void openingAnimation () {

}

void endingAnimation () {
  for(uint8_t i = EDGE; i < HEIGHT; i++){
    for(uint8_t j = EDGE; j < WIDTH - EDGE; j++){
      pixels[i][j] = false;
      matrix.drawPixel(i,j,GRAY);
    }
//matrix.drawLine(i,3,i,12,GRAY);
    delay(50);
  }
  for(uint8_t i = EDGE; i < HEIGHT; i++){
    for(uint8_t j = EDGE; j < WIDTH - EDGE; j++){
      matrix.drawPixel(i,j,BLACK);
    }
    delay(50);
  }
}

/*
 * Set up the background display when the game starts
 * Set up the margins and clean up the playground 
 * Drop a tetromino from middle top to start the game at last 
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
  endingAnimation();
  delay(1000);
  newblock();
  dropnew();
  newblock();
}

/*
 * Print the scores that player has gained in real time 
 * Update each time a line is finished
 * Parameters 'cursor-' indicate the upper left location on the RGB matrix panel
 */
void printScore () {
  byte cursorx = 30; 
  byte cursory1 = 1;    // for first digit
  byte cursory2 = 5;    // for second digit 
  uint16_t shifter = 0b0100000000000000;
  
  for(uint8_t i = 0; i < 5; i++){
    for(uint8_t j = 0; j < 3; j++){
      //numbers[scores%10])
      //print first digit
      if((shifter & numbers[scores/10]) > 0){
        matrix.drawPixel(cursorx-i, cursory1+j, score_color);    
      }else{
        matrix.drawPixel(cursorx-i, cursory1+j, BLACK);    
      }  
      //print the second digit
      if((shifter & numbers[scores%10]) > 0){
        matrix.drawPixel(cursorx-i, cursory2+j, score_color);    
      }else{
        matrix.drawPixel(cursorx-i, cursory2+j, BLACK);    
      }
      shifter = shifter >> 1; 
    }
  }
}

/*
 * Preview the next tetromino while playing 
 * Update each time the current tetromino collides and a new tetromino drops from the middle top
 * Parameters 'cursor-' indicate the upper right location on the RGB matrix panel
 */
void previewtetro (){
  byte cursorx = 29;
  byte cursory = 11;
  uint16_t shifter = 0b1000000000000000;
  for(uint8_t i = 0; i < TETRO_SIZE; i++){
    for(uint8_t j = 0; j < TETRO_SIZE; j++){
      if((shifter & next_tetro) > 0){
        matrix.drawPixel(cursorx-i, cursory+j, next_color);       
      }else{
        matrix.drawPixel(cursorx-i, cursory+j, BLACK);       
      }
      shifter = shifter >> 1; 
    }
  }
}

/*
 * Display the current tetromino on RGB matrix panel
 */
void drawtetro (boolean clean){
  for(uint8_t i = 0; i < TETRO_SIZE; i++){
    for(uint8_t j = 0; j < TETRO_SIZE; j++){
      if(block[i][j] == true){
        if(clean == true){
          matrix.drawPixel(blockCursor_y-i, blockCursor_x+j, BLACK);
        }else{
          matrix.drawPixel(blockCursor_y-i, blockCursor_x+j, current_color);
        }
      }         
    }
  }
}

/*
 * Display the background on RGB matrix panel
 * Update when a tetromino collides and stop dropping 
 */
void updatedisp (){
  for(uint8_t i = 0; i < HEIGHT; i++){
    for(uint8_t j = 0; j < WIDTH; j++){
      if(pixels[i][j] == true){
        matrix.drawPixel(i, j, background_color);
      }
      else{
        matrix.drawPixel(i, j, BLACK);
      }
    }
  }
}


/*
 * Execute operations correspondingly
 * Only when there is no collision on conduction of the operation 
 */
 void moveDown () {
  if(ifCollision('d') == false){
    drawtetro(true);
    blockCursor_y--;
    drawtetro(false);
  }else{
    // If collide, update current tetromino to the background and drop next tetromino
    for(uint8_t i = 0; i < TETRO_SIZE; i++){
      for(uint8_t j = 0; j < TETRO_SIZE; j++){
        if(block[i][j] == true){
          pixels[blockCursor_y-i][blockCursor_x+j] = true;
        }
      }
    }
    ifLine();
    updatedisp();
    dropnew();
    newblock();
  }
}
void moveRight () {
  if(ifCollision('r') == false){
    drawtetro(true);
    blockCursor_x++;
    drawtetro(false);
  }
}
void moveLeft () {
  if(ifCollision('l') == false){
    drawtetro(true);
    blockCursor_x--;
    drawtetro(false);
  }
}
void rotate () {
  if(ifCollision('o') == false){
     drawtetro(false); 
  } 
}

/*
 * Transfer the tetromino information from binary bits to a boolean matrix 
 */
void uint2bool (uint16_t value) {
  uint16_t shifter = 0b1000000000000000;
  for(uint8_t i = 0; i < TETRO_SIZE; i++){
    for(uint8_t j = 0; j < TETRO_SIZE; j++){
      if((shifter & value)>0){
        block[i][j] = true;        
      }else{
        block[i][j] = false;   
      }
      shifter = shifter >> 1; 
    }
  }
}

/*
 * Check if there is any collision on the conduction of operations
 * Retrun true if a collision is about to happen 
 */
boolean ifCollision (char instruction) {
  switch (instruction){
     // === Operation: Move Down === //
     case'd':                   
     for(uint8_t i = 0; i < TETRO_SIZE; i++){
       for(uint8_t j = 0; j < TETRO_SIZE; j++){
         if((block[i][j] == true)&&(pixels[blockCursor_y-i-1][blockCursor_x+j] == true)){
//            matrix.drawPixel(blockCursor_y-i-1, blockCursor_x+j, GREEN); 
            return true;
         }
       }
     }
     return false; 
     
     // === Operation: Move Left === //
     case'l':                  
     for(uint8_t i = 0; i < TETRO_SIZE; i++){
       for(uint8_t j = 0; j < TETRO_SIZE; j++){
         if((block[i][j] == true)&&(pixels[blockCursor_y-i][blockCursor_x+j-1]) == true){
//            matrix.drawPixel(blockCursor_y-i, blockCursor_x+j-1, RED); 
            return true;
         }           
       }
     }
     return false;
     
     // === Operation: Move Right === //
     case'r':                   
     for(uint8_t i = 0; i < TETRO_SIZE; i++){
       for(uint8_t j = 0; j < TETRO_SIZE; j++){
         if((block[i][j] == true)&&(pixels[blockCursor_y-i][blockCursor_x+j+1]) == true){
 //           matrix.drawPixel(blockCursor_y-i, blockCursor_x+j+1, ORANGE); 
            return true;
         }           
       }
     }
     return false;
     
     // === Operation: Rotate === //
     case'o':                   
     uint2bool(tetrominoes[(current_index/4)*4+(current_index+1)%4]);
     for(uint8_t i = 0; i < TETRO_SIZE; i++){
       for(uint8_t j = 0; j < TETRO_SIZE; j++){
         if((block[i][j] == true)&&(pixels[blockCursor_y-i][blockCursor_x+j]) == true){
 //         matrix.drawPixel(blockCursor_y-i, blockCursor_x+j, YELLOW);
          uint2bool(current_tetro);
          return true;
         }
       }
     }
     uint2bool(current_tetro);
     drawtetro(true);
     current_index = (current_index/4)*4+(current_index+1)%4;
     current_tetro = tetrominoes[current_index];
     uint2bool(current_tetro);
     return false;
  }   
}

/*
 * Check if a line is accomplished  
 * If so, eliminate the accomplished line and update the scores
 */
void ifLine () {
  int accomplishline = 1;
  for(uint8_t i = EDGE; i< HEIGHT; i++){
    accomplishline = 1;
    for(uint8_t j = EDGE; j < WIDTH-EDGE; j++){
      if(pixels[i][j] == false){
        accomplishline = 0;
        break;
      }
    }
    if(accomplishline == 1){
///      matrix.drawPixel(3, 3, matrix.Color333(7, 7, 7));
      delay(1000);
      for(uint8_t k = i; k < HEIGHT - 1; k++){
        for(uint8_t l = EDGE; l < WIDTH-EDGE; l++){
          if(pixels[k+1][l] == true){
             pixels[k][l] = true;
          } else {
            pixels[k][l] = false;
          }
        }
      }
      i = i-1;
      scores++;
      printScore();
    }
  }       
}

/* 
 *  Randomly generate a new tetromino and preview it
 */
void newblock () {
  next_index = millis()%28;
  next_tetro = tetrominoes[next_index];
  switch (millis()%8) {
    case 0: next_color = GRAY;    break;
    case 1: next_color = YELLOW;  break;
    case 2: next_color = BLUE;    break;
    case 3: next_color = GREEN;   break;
    case 4: next_color = ORANGE;  break;
    case 5: next_color = RED;     break;
    case 6: next_color = PURPLE;  break;
    case 7: next_color = CYAN;    break;
  }
  previewtetro();
}

/*
 * Drop the next tetromino
 */
void dropnew () {
  current_tetro = next_tetro;
  current_color = next_color;
  current_index = next_index;
  uint2bool(current_tetro);
  blockCursor_x = 6;
  blockCursor_y = 22;
  drawtetro(false);
  if(ifCollision('d') == true){
    gameOver = true;
  } else {
    drawtetro(false);
  }
}

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
    countt=0;
    endingAnimation();
    while(1){
     if(digitalRead(12)==HIGH){
       gameOver = false;
       reset();
       break;
     }
     countt=0;
     delay(100);
    }
  }
}

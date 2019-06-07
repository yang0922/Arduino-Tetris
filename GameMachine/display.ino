/*******************************************************************************
 * Display the game.
 * 
 * Including the background, the playground, socres, and preview of tetromino.
 *******************************************************************************/

// Transfer the tetromino information from binary bits to a boolean matrix.
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
 * Display the background on lower hals of the RGB matrix panel.
 * Update when a tetromino collides and stop dropping.
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

// Display the current tetromino on RGB matrix panel.
void drawtetro (boolean colorful){
  for(uint8_t i = 0; i < TETRO_SIZE; i++){
    for(uint8_t j = 0; j < TETRO_SIZE; j++){
      if(block[i][j] == true){
        if(colorful == true){
          matrix.drawPixel(blockCursor_y-i, blockCursor_x+j, current_tetro.retColor());
        }else{
          matrix.drawPixel(blockCursor_y-i, blockCursor_x+j, BLACK);
        }
      }         
    }
  }
}

/*
 * Preview the next tetromino while playing.
 * Update each time the current tetromino collides and a new tetromino drops from the middle top.
 * Parameters 'cursor-' indicate the upper right location on the RGB matrix panel.
 */
void previewtetro (){
  byte cursorx = 29;
  byte cursory = 11;
  uint16_t shifter = 0b1000000000000000;
  for(uint8_t i = 0; i < TETRO_SIZE; i++){
    for(uint8_t j = 0; j < TETRO_SIZE; j++){
      if((shifter & next_tetro.retTetro()) > 0){
        matrix.drawPixel(cursorx-i, cursory+j, next_tetro.retColor());       
      }else{
        matrix.drawPixel(cursorx-i, cursory+j, BLACK);       
      }
      shifter = shifter >> 1; 
    }
  }
}

/*
 * Print the scores that player has gained in real time.
 * Update each time a line is finished.
 * Parameters 'cursor-' indicate the upper left location on the RGB matrix panel.
 */
void printScore () {
  byte cursorx = 30; 
  byte cursory1 = 1;    // for first digit
  byte cursory2 = 5;    // for second digit 
  uint16_t shifter = 0b0100000000000000;
  
  for(uint8_t i = 0; i < 5; i++){
    for(uint8_t j = 0; j < 3; j++){
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
 * An animation to be displayed between games. 
 * Fill the playground and empty it row by row
 */
void animation () {
  for(uint8_t i = EDGE; i < HEIGHT; i++){
    for(uint8_t j = EDGE; j < WIDTH - EDGE; j++){
      pixels[i][j] = false;
      matrix.drawPixel(i,j,GRAY);
    }
    delay(50);
  }
  for(uint8_t i = EDGE; i < HEIGHT; i++){
    for(uint8_t j = EDGE; j < WIDTH - EDGE; j++){
      matrix.drawPixel(i,j,BLACK);
    }
    delay(50);
  }
}

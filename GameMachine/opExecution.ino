/********************************************************************************************
 * The execution of all operations.
 * 
 * Check if any collision before execution and check if a line is filled if collides.
 ********************************************************************************************/

/*
 * Execute operations correspondingly.
 * Only happens when there is no collision on the conduction of the operation.
 */
 void moveDown () {
  if(ifCollision('d') == false){
    drawtetro(false);
    blockCursor_y--;
    drawtetro(true);
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
  }
}
void moveRight () {
  if(ifCollision('r') == false){
    drawtetro(false);
    blockCursor_x++;
    drawtetro(true);
  }
}
void moveLeft () {
  if(ifCollision('l') == false){
    drawtetro(false);
    blockCursor_x--;
    drawtetro(true);
  }
}
void rotate () {
  if(ifCollision('o') == false){
     drawtetro(true); 
  } 
}

/*
 * Check if there is any collision on the conduction of operations.
 * Retrun true if a collision is about to happen.
 */
boolean ifCollision (char instruction) {
  switch (instruction){
     // === Operation: Move Down === //
     case'd':                   
     for(uint8_t i = 0; i < TETRO_SIZE; i++){
       for(uint8_t j = 0; j < TETRO_SIZE; j++){
         if((block[i][j] == true)&&(pixels[blockCursor_y-i-1][blockCursor_x+j] == true)){
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
            return true;
         }           
       }
     }
     return false;
     
     // === Operation: Rotate === //
     case'o':                   
     uint2bool(tetrominoes[(current_tetro.retIndex()/4)*4+(current_tetro.retIndex()+1)%4]);
     for(uint8_t i = 0; i < TETRO_SIZE; i++){
       for(uint8_t j = 0; j < TETRO_SIZE; j++){
         if((block[i][j] == true)&&(pixels[blockCursor_y-i][blockCursor_x+j]) == true){
          uint2bool(current_tetro.retTetro());
          return true;
         }
       }
     }
     uint2bool(current_tetro.retTetro());
     drawtetro(false);
     current_tetro.Update((current_tetro.retIndex()/4)*4+(current_tetro.retIndex()+1)%4, current_tetro.retColor());
     uint2bool(current_tetro.retTetro());
     return false;
  }   
}

/*
 * Check if a line is accomplished.
 * If so, eliminate the accomplished line and update the scores.
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

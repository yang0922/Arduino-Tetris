// Accelerometer - Handheld Game Controller 
#include <Wire.h>
#define MPU       0x68    // MPU6050 I2C address
#define PWR_MGMT  0x6B 
float AccX, AccY, AccZ;


/*
 * Power on the accelerometer
 */
void accelerometer_setup () {
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 
  Wire.write(PWR_MGMT);              // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  delay(20);
}


/*
 * Determine the operations according to the read values from the accelerometer
 * Use integers to represent the operations:
 * --- rotation        -> 1 ---
 * --- speed up        -> 2 ---
 * --- move left       -> 3 ---
 * --- fast move left  -> 4 ---
 * --- move right      -> 5 ---
 * --- fast move right -> 6 ---
 * Combined operations are allowed
 */
uint8_t determineOperation () {
  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);                         // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);           // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;            // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;            // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;            // Z-axis value
//  //Print the values on the serial monitor if needed 
//  Serial.print(AccX);
//  Serial.print("/");
//  Serial.print(AccY);
//  Serial.print("/");
//  Serial.println(AccZ);

  // === Determine operation === //
  uint8_t operation = 0;
  // Horizontal operations
  if (AccX < -0.3 && AccX > -0.6) {                //move right 
    operation = 5;
    right++;
    left = 0;
  } else if (AccX < -0.6) {                        //fast move right 
    operation = 6;  
    right++; 
    left = 0;
  } else if (AccX > 0.3 && AccX < 0.6) {           //move left 
    operation = 3;   
    left++;
    right = 0;
  } else if (AccX > 0.6) {                         //fast move left
    operation = 4;   
    left++;  
    right = 0;
  } else {                                         //no operation
    operation = 0; 
    left = 0;  
    right = 0;
  }
  // Vertical operations 
  if (AccY > 0.3) {                                //rotate
    operation += 20;  
    rotation++;  
    speedup = 0;
  } else if (AccY < -0.3){                         //speed up 
    operation += 10;  
    speedup++; 
    rotation = 0;
  } else {
    speedup = 0;  
    rotation = 0;
  }

  return operation;
}

/*
 * Read and execute corresponding operations periodically 
 */
void operationHandler (){    
  if(millis()-lastOp < OPERATION_PERIOD){
     return;
   }
  uint8_t operation = determineOperation();  
  //Execute the operations
  switch (operation){
    case 1:     //rotate
    rotate(); break;
    case 2:     //speed up
    moveDown();break;
    case 3:     //move left
    moveLeft();break;
    case 4:     //fast move left
    moveLeft();break;
    case 5:     //move right
    moveRight();break;
    case 6:     //fast move right
    moveRight();break;
  }
  if(left != 0){
    moveLeft();
    lastOp = millis();
  } else if (right != 0){
    moveRight();
    lastOp = millis();
  }

  if(speedup != 0){
    moveDown();
    lastOp = millis();
  } else if(rotation != 0){
    rotate();
    lastOp = millis(); 
  }
}

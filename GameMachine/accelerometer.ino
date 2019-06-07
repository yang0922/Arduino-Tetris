/*********************************************************************************
 * Accelerometer - Handheld Game Controller.   
 * 
 *  - read values from the accelerometer and determine operations accordingly 
 *  - pass informatin to the system to conduct the operations 
 *********************************************************************************/

#include <Wire.h>
#define MPU       0x68    // MPU6050 I2C address
#define PWR_MGMT  0x6B 
float AccX, AccY, AccZ;

// Power on the accelerometer
void accelerometer_setup () {
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 
  Wire.write(PWR_MGMT);              // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        // End the transmission
  delay(20);
}

/*
 * Determine the operations according to the read values from the accelerometer.
 * The information about the determined operations is store in relevant variables.
 * Combined operations in both directions (vertical & horizontal) are allowed.
 */
void determineOperation () {
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
//  Serial.print(AccX);   Serial.print("/");
//  Serial.print(AccY);   Serial.print("/");
//  Serial.println(AccZ);

  // === Determine operation === //
  // Horizontal operations---------------------------------------------
  if (AccX < -0.3 ) {                //move right 
    right++;
    left = 0;
  } else if (AccX > 0.3) {           //move left  
    left++;
    right = 0;
  } else {                           //no operation 
    left = 0;  
    right = 0;
  }
  
  // Vertical operations------------------------------------------------
  if (AccY > 0.3) {                  //rotate
    rotation++;  
    speedup = 0;
  } else if (AccY < -0.3){           //speed up 
    speedup++; 
    rotation = 0;
  } else {
    speedup = 0;  
    rotation = 0;
  }
}

/*
 * Read and execute corresponding operations periodically 
 */
void operationHandler (){    
  if(millis()-lastOp < OPERATION_PERIOD){
     return;
   }
   determineOperation();
  //Execute the operations
  // Horizontal operations------------------------------------------------
  if(left != 0){                        //move left
    moveLeft();
    tone(buzzer, 1000); 
    delay(10);        
    noTone(buzzer);     
    lastOp = millis();
  } else if (right != 0){               //move right
    moveRight();
    tone(buzzer, 1000); 
    delay(10);       
    noTone(buzzer);     
    lastOp = millis();
  }
  
  // Vertical operations----------------------------------------------------
  if(speedup != 0){                     //speed up 
    moveDown();
    tone(buzzer, 1000); 
    delay(10);        
    noTone(buzzer);    
    lastOp = millis();
  } 
  if(digitalRead(button)==HIGH){        //rotate 
    rotate();
    tone(buzzer, 1000); 
    delay(10);        
    noTone(buzzer); 
    lastOp = millis();    
    }
}

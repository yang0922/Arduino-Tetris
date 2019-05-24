


 
void timercount_setup() {
   TIMSK2 = (TIMSK2 & B11111001) | 0x02;
   TCCR2B = (TCCR2B & B11111000) | 0x06;
   OCR2A = 125;

}

 
ISR(TIMER2_COMPA_vect){
  TCNT2=0;
   countt++;
   if(countt>=500){
    moveDown();
    countt=0;
    }
}
 

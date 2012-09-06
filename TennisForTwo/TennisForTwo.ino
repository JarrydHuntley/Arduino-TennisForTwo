
/*

Copyright 2007 Windell H. Oskay
Distributed under the terms of the GNU General Public License, please see below.

-------------------------------------------------


Title:			tennis.c 
Author:			Windell Oskay
Date Created:   7/7/08
Last Modified:  08/31/12 by Jarryd Huntley 
Purpose:  Tennis for two

This is an AVR-GCC program designed for the Atmel ATmega168 series. 

More complete description here: http://www.evilmadscientist.com/article.php/tennis

Fuse settings: Default.

-------------------------------------------------
USAGE: How to compile and install


-------------------------------------------------

This code should be relatively straightforward, so not much documentation is provided.  If you'd like to ask 
questions, suggest improvements, or report success, please use the evilmadscientist forum:
http://www.evilmadscientist.com/forum/


-------------------------------------------------

 This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

-------------------------------------------------

*/


#include <avr/pgmspace.h>
 
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
 
void StreamPrint_progmem(Print &out,PGM_P format,...)
{
  // program memory version of printf - copy of format string and result share a buffer
  // so as to avoid too much memory use
  char formatString[128], *ptr;
  strncpy_P( formatString, format, sizeof(formatString) ); // copy in from program mem
  // null terminate - leave last char since we might need it in worst case for result's \0
  formatString[ sizeof(formatString)-2 ]='\0'; 
  ptr=&formatString[ strlen(formatString)+1 ]; // our result buffer...
  va_list args;
  va_start (args,format);
  vsnprintf(ptr, sizeof(formatString)-1-strlen(formatString), formatString, args );
  va_end (args);
  formatString[ sizeof(formatString)-1 ]='\0'; 
  out.print(ptr);
}
 
#define Serialprint(format, ...) StreamPrint_progmem(Serial,PSTR(format),##__VA_ARGS__)
#define Streamprint(stream,format, ...) StreamPrint_progmem(stream,PSTR(format),##__VA_ARGS__)

  
  //FIXME are these includes needed for this port?
  //#include <avr/io.h> 
  //#include <math.h> 
  //#include <stdlib.h>		//gives rand() function
   
   
  //gravitational acceleration (should be positive.) 
  #define g 0.8   
  
  // TimeStep
  #define ts 2.025 

  #define historyLength 7 
  
  
  //MAIN
  
  float sintable[64];
  float costable[64];
  
  uint8_t xOldList[historyLength];
  uint8_t yOldList[historyLength];
  
  float xOld; // a few x & y position values
  float yOld; // a few x & y position values
  
  
  float VxOld; //  x & y velocity values 
  float VyOld; //  x & y velocity values 
  
  float Xnew, Ynew, VxNew, VyNew;
  
  uint8_t  deadball = 0;
  
  uint8_t Langle, Rangle;
   
  
  uint8_t xp = 0;     
  uint8_t yp = 0;     
  
  
  
  unsigned int ADoutTemp;
  
   
  uint8_t NewBall = 101; 
  
  unsigned int NewBallDelay = 0;
  
  //Dummy variables: 
  uint8_t k = 0;
  uint8_t m = 0;
  
  uint8_t Server = 0;
  //uint8_t CheckNet = 0;
  
  uint8_t ballside;
  uint8_t Lused = 0;
  uint8_t Rused = 0;
 
 const int leftButton = 15;     //pin 15 is being used as a digital input for the left button
// String message;
 
 /* 
 //FIXME Are these all AVR C specific variables?
 
 //DataDiraction Ports
  //Outputs:
  DDRB = 255;
  DDRD = 255;
  
  //Internal Pull-up resistors off
  PORTB = 0;
  PORTD = 0;
  
  //Inputs:
  DDRC = 0;
  PORTC = 0; // Pull-ups off.  We have our own external pull-ups anyway.
  
  
  // ADC Setup
  
  PRR &=  ~(_BV(ICF1));  //Allow ADC to be powered up
  
  //ADC 3-5
  
  ADMUX = Server * 4;	 
  
  ADCSRA = 197  ;  // Enable ADC & start, prescale at 32

 */

 

void setup()
{
  // Create trig look-up table to keep things snappy.
  // 64 steps, total range: near pi.  Maybe a little more. 
  
   m = 0;
   while (m < 64)
   {
  	sintable[m] = sin((float) 0.0647 * (float) m - (float) 2.07);   
  	costable[m] = cos((float) 0.0647 * (float) m - (float) 2.07);
      m++;
  }
  
  yOld = 0;
  VyOld = 0;
  
  ballside = 0;
  
  Serial.begin(115200);
  
  //FIXME
   pinMode(leftButton, INPUT);

}

void loop() // main game loop	
{

    //message = "";
  
    if(ballside != (xOld >= 127))
    {
      ballside = (xOld >= 127);

      if(ballside) Rused = 0;
      else Lused = 0;

      //CheckNet = 1;

    }
      
    if(NewBall > 10) // IF ball has run out of energy, make a new ball!
    {

      NewBall = 0;
      deadball = 0;

      NewBallDelay = 1;

      Server = (ballside == 0);

      if(Server)
      {
        xOld = (float) 210;
        VxOld = 0; // (float) -2*g; 
        ballside = 1;
        Rused = 0;
        Lused = 1;
      }
      else
      {
        xOld = (float) 45;
        VxOld = 0; // (float) 2*g; 
        ballside = 0;
        Rused = 1;
        Lused = 0;
      }

      yOld = (float) 110;

      m = 0;
      while(m < historyLength)
      {
        xOldList[m] = xOld;
        yOldList[m] = yOld;

        m++;

      }

    }
    
    // Physics time!
    // x' = x + v*t + at*t/2
    // v' = v + a*t
    //
    // Horizontal (X) axis: No acceleration; a = 0.
    // Vertical (Y) axis: a = -g
    
    /*
    NOTE: ADC portion. Replaced by analog read functon
    
    if((ADCSRA & _BV(ADSC)) == 0) // If ADC conversion has finished
    {

      ADoutTemp = ADCW; // Read out ADC value	

      //We are using *ONE* ADC, but sequentially multiplexing it to sample the two different input lines.

      if(ADMUX == 0) Langle = ADoutTemp >> 4; //ADoutTemp >> 2;
      else Rangle = ADoutTemp >> 4; // ADoutTemp >> 2;

      // 64 angles allowed

      ADMUX = 4 * (ballside); // Either ch 0 or ch 4.

      ADCSRA |= _BV(ADSC); // Start new ADC conversion 
    }
    */
    //FIXME
    int tempL = analogRead(A0);
    Langle = tempL >> 4;
    
    //FIXME: Add analog reads here
    
    if(NewBallDelay)
    {
		//NOTE: if reset pin?  Nope. Controller button presses...?
      if(((PINC & 2U) == 0) || ((PINC & 32U) == 0)) NewBallDelay = 10000;

      NewBallDelay++;

      if(NewBallDelay > 5000) // was 5000
      NewBallDelay = 0;

      m = 0;
      while(m < 255)
      {

        PORTD = yp;
        PORTB = xp;
        
        //Serial.print(xp);
        //Serial.print(",");
        //Serial.println(yp);
       // message+= xp+","+yp;
        //message+=":";
        Serialprint("%d,%d:",xp,yp);
        
        m++;

      }

      VxNew = VxOld;
      VyNew = VyOld;
      Xnew = xOld;
      Ynew = yOld;

    }
    else
    {

      Xnew = xOld + VxOld;
      Ynew = yOld + VyOld - 0.5 * g * ts * ts;

      VyNew = VyOld - g * ts;
      VxNew = VxOld;

      // Bounce at walls

      if(Xnew < 0)
      {
        VxNew *= -0.05;
        VyNew *= 0.1;
        Xnew = 0.1;
        deadball = 1;
        NewBall = 100;

      }

      if(Xnew > 255)
      {
        VxNew *= -0.05;
        Xnew = 255;
        deadball = 1;
        NewBall = 100;

      }

      if(Ynew <= 0)
      {
        Ynew = 0;

        if(VyNew * VyNew < 10) NewBall++;

        if(VyNew < 0) VyNew *= -0.75;

      }

      if(Ynew >= 255)
      {
        Ynew = 255;
        VyNew = 0;

      }

      if(ballside)
      {

        if(Xnew < 127)
        {

          if(Ynew <= 63)
          {
            // Bounce off of net
            VxNew *= -0.5;
            VyNew *= 0.5;
            Xnew = 128.00;
            deadball = 1;

          }
        }
      }

      if(ballside == 0)
      {
        if(Xnew > 127)
        {

          if(Ynew <= 63)
          {
            // Bounce off of net
            VxNew *= -0.5;
            VyNew *= 0.5;
            Xnew = 126.00;
            deadball = 1;
          }
        }
      }

      // Simple routine to detect button presses: works, if the presses are slow enough.

      //if(xOld < 120)
      {
        //if((PINC & 2U) == 0)
        if(digitalRead(leftButton) == LOW)
        {
          if((Lused == 0) && (deadball == 0))
          {
            VxNew = 1.5 * g * costable[Langle];
            VyNew = g + 1.5 * g * sintable[Langle];

            //Lused = 1;
            NewBall = 0;

          }
        }
      }
      //else if(xOld > 134) // Ball on right side of screen
      {
        if((PINC & 32U) == 0)
        {

          if((Rused == 0) && (deadball == 0))
          {

            VxNew = -1.5 * g * costable[Rangle];
            VyNew = g + -1.5 * g * sintable[Rangle];

            Rused = 1;
            NewBall = 0;

          }
        }
      }

    }
    
    //Figure out which point we're going to draw. 

    xp = (int) floor(Xnew);

    yp = (int) floor(Ynew);

    //yp = 511 - (int) floor(Ynew);

    //Draw Ground and Net
    
    k = 0;
    
    //while(k < 20) FIXME
    while(k < 1)
    {
      k++;

      m = 0;
      while(m < 127)
      {

        PORTD = 0; // Y-position
        PORTB = m; // X-position
        //Serial.print(m);
        //Serial.print(",");
        //Serial.println(0);
        
        //message+= m+",0";
        //message+=":";
        //Serialprint("%d,%d:",m,0);
        
        m++;
      }

      PORTB = 127; // X-position of NET
      
      m = 0;
      while(m < 61)
      {

        PORTD = m; // Y-position
        //Serial.print(127);
        //Serial.print(",");
        //Serial.println(m);
        
        //message+= "127,"+m;
        //message+=":";
        //Serialprint("%d,%d:",127,m);

        m += 2;
      }

      while(m > 1)
      {

        PORTD = m; // Y-position
        //Serial.print(127);
        //Serial.print(",");
        //Serial.println(m);
        
        //message+= "127,"+m;
        //message+=":";
        //Serialprint("%d,%d:",127,m);
        
        m -= 2;
      }

      PORTD = 0; // Y-position
      PORTB = 127; //Redundant, but allows time for scope trace to catch up.
        //Serial.print(127);
        //Serial.print(",");
        //Serial.println(0);
        
        //message+= "127,0";
        //message+=":";
       Serialprint("%d,%d:",127,0);
      
      m = 127;
      while(m < 255)
      {

        PORTD = 0; // Y-position
        PORTB = m; // X-position
        //Serial.print(m);
        //Serial.print(",");
        //Serial.println(0);
        
        //message+= m+",0";
        //message+=":";
        //Serialprint("%d,%d:",m,0);
        
        m++;

      }

    }
    
    m = 0;
    while(m < historyLength)
    {

      k = 0;
      while(k < (4 * m * m))
      {
       PORTB = xOldList[m];
       PORTD = yOldList[m];
        
        //Serialprint("%d,%d:",xOldList[m],yOldList[m]);
        

        k++;

      }

      m++;

    }

    // Write the point to the buffer

    PORTD = yp;
    PORTB = xp;
        //Serial.print(xp);
        //Serial.print(",");
        //Serial.println(yp);
  
        //message+= xp+","+yp;
        //message+=":";

    //Serialprint("%d,%d:",xp,yp);
  
  
    m = 0;
    while(m < (historyLength - 1))
    {
      xOldList[m] = xOldList[m + 1];
      yOldList[m] = yOldList[m + 1];

      m++;

    }

    xOldList[(historyLength - 1)] = xp;
    yOldList[(historyLength - 1)] = yp;

    m = 0;
    //while(m < 100) FIXME
    while(m < 1)
    {

      PORTD = yp;
      PORTB = xp;
        //Serial.print(xp);
        //Serial.print(",");
        //Serial.println(yp);
        
       // message+= xp+","+yp;
        //message+=":";

      Serialprint("%d,%d:",xp,yp);
        
      m++;

    }

    //Age variables for the next iteration
    VxOld = VxNew;
    VyOld = VyNew;

    xOld = Xnew;
    yOld = Ynew;  


//delay(50);

        Serial.println("ENDMESSAGES");
}//end loop

void sendMessage(String messageBody)
{
  
  if(!messageBody.equals("ENDMESSAGES"))
  {
    Serial.print(messageBody);
  }
  else
  {
      Serial.println(messageBody);
  }
}

//Processing code for this example

//  This example code is in the public domain.

import processing.serial.*;

float redValue = 0; // red value

Serial myPort;

ArrayList<String> messages;
boolean shouldDraw = false;
void setup()
{
  size(300, 300);
  // set the background color
  background(255);
  
 // noLoop();
  
  messages = new ArrayList<String>();


  // List all the available serial ports
  println(Serial.list());
  // I know that the first port in the serial list on my mac
  // is always my  Arduino, so I open Serial.list()[0].
  // Open whatever port is the one you're using.
  myPort = new Serial(this, Serial.list()[2], 115200  );
  // don't generate a serialEvent() unless you get a newline character:
  myPort.bufferUntil('\n');
}

void draw()
{        //println("Start Draw"); 
      //if(shouldDraw)
  
      {
        for(int i = 0; i < messages.size(); i++)
        {
           if(messages.get(i)!=null)
           {
             int[] tempCoor = int(split(messages.get(i), ","));

           // println(messages.size());  
              //FIXME Add an include guard
            if(!(tempCoor.length < 2))
            point(tempCoor[0], tempCoor[1]);     
           }
        }         
        //println("Draw complete");     
        messages.clear();
        shouldDraw = false;
      }

}

void serialEvent(Serial myPort)
{
  // get the ASCII string:
  String inString = myPort.readStringUntil('\n');

  if(inString != null)
  {
    // trim off any whitespace:
    inString = trim(inString);
    
    // println(inString);
    
    String[] messageList = split(inString, ":");
    
    for(int i=0; i < messageList.length; i++)
    { 
      messageList[i] = messageList[i].trim();
      
        if(messageList[i].equals("ENDMESSAGES"))
        {
              //println("Caught ENDMASSAGES");
              //draw();
            shouldDraw = true;
                background(255);
        }
        else
        {
            messages.add(messageList[i]);
        }
    }
    // split the string on the commas and convert the 
    // resulting substrings into an integer array:
    /*float[] colors = float(split(inString, ","));
    // if the array has at least three elements, you know
    // you got the whole thing.  Put the numbers in the
    // color variables:
    if(colors.length >= 3)
    {
      // map them to the range 0-255:
      redValue = map(colors[0], 0, 1023, 0, 255);
      greenValue = map(colors[1], 0, 1023, 0, 255);
      blueValue = map(colors[2], 0, 1023, 0, 255);      
    }*/
    
    
  }
}

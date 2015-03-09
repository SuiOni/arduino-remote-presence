#include <Thread.h>
#include <ThreadController.h>
#include <EasyTransfer.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>

/*
	 requires a Timer Interrupt Library Timer1
 	 Basically same code for Arduino Uno in UART mode and Lilypad
         No DEBUG data send because the is only one Serial available which is used for bi directional communication.
         Need Terminalserver ncat to communicate 
         Example:
         sudo mkfifo  backpipe
         sudo ncat -l -k -vv 23 <backpipe | sudo ncat -l -k -vv 26 >backpipe
         
         Assuming one Xbee Wifi is sending to port 23 and the other one on 26

 */



// ThreadController that will controll all threads
ThreadController controll = ThreadController();

Thread smoothLight1Thread = Thread();
Thread smoothLight2Thread = Thread();
Thread sendDataThread = Thread();
Thread recDataThread = Thread();
Thread readDataThread = Thread();


//SoftwareSerial xb(0, 1);// RX, TX
const int analogInPin = A2; 
int led1 = 10; //ledpin for receiving pressure
int led2 = 11; //ledpin for local pressure
int led3 = 12; //ledpin for the biglight receiving pressure
int light1= 0; //current light state of receiving pressure
int light2= 0; //current light state of local pressure
volatile int light1be= 0; //received light state of receiving pressure - transition to this state
volatile int light2be= 0; //received light state of local pressure - transition to this state


//create object for SendData
EasyTransfer ETin, ETout; 



struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int pressure;
  //int pause;
};

struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int  pressure;
 
};

//give a name to the group of data
RECEIVE_DATA_STRUCTURE recdata;
SEND_DATA_STRUCTURE senddata;




// callback for sendDataThread sendData to Server
void sendDataCallback(){
 
  Timer1.stop();
  ETout.sendData();
  Timer1.resume();

}


// callback for recDataThread receivesData from Server
void recDataCallback(){
  
  if (ETin.receiveData()){
    Timer1.stop();
 
      light1be=recdata.pressure;
      smoothLight1Thread.enabled=true;
     
    Timer1.resume();
  }

}

//change Light1 smoothly this is called every 30 msek by the smoothLight1Thread  so no loop requiered
void smoothLight1Callback(){ 
   
  Timer1.stop();
  if (light1==light1be)
    smoothLight1Thread.enabled=false;
  if(light1>light1be){ 
    light1--;
    analogWrite(led1, light1);
    analogWrite(led3, light1);
  }

  if(light1<light1be) {
    light1++;
    analogWrite(led1, light1);
    analogWrite(led3, light1);
    

  }
    
  Timer1.resume();  
}

//change Light2 smoothly this is called every 30 msek by the smoothLight1Thread  so no loop requiered
void smoothLight2Callback(){

  Timer1.stop(); 
  if (light2==light2be)
    smoothLight2Thread.enabled=false;
  if(light2>light2be) 
    analogWrite(led2, --light2);

  if(light2<light2be) 
    analogWrite(led2, ++light2);
    Timer1.resume();


}



// callback for readDataThread readsPressure Data and stores to SEND_DATA_STRUCTURE senddata.pressure;
void readDataCallback(){
  int pressureData=0;

  pressureData = map(analogRead(analogInPin),0,600,0,255);
  if (pressureData>255)
    pressureData=255;
  senddata.pressure=pressureData;
  light2be=senddata.pressure;
  Timer1.resume();
  smoothLight2Thread.enabled=true;


}

// This is the callback for the Timer
void timerCallback(){
  controll.run();
}



void setup(){
  
  Serial.begin(9600);

  ETin.begin(details(recdata), &Serial);
  ETout.begin(details(senddata), &Serial);

  // Configure sendDataThread
  sendDataThread.onRun(sendDataCallback);
  sendDataThread.setInterval(600);

  // Configure recDataCallback
  recDataThread.onRun(recDataCallback);
  recDataThread.setInterval(300);

  // Configure Callback
  readDataThread.onRun(readDataCallback);
  readDataThread.setInterval(200);


  smoothLight1Thread.onRun(smoothLight1Callback);
  smoothLight1Thread.setInterval(30);


  smoothLight2Thread.onRun(smoothLight2Callback);
  smoothLight2Thread.setInterval(30);

  // Adds both threads to the controller
  controll.add(&sendDataThread); // & to pass the pointer to it
  controll.add(&recDataThread);
  controll.add(&readDataThread);
  controll.add(&smoothLight1Thread);
  controll.add(&smoothLight2Thread);

  //start the library, pass in the data details and the name of the serial port.
  
  
  /*
		If using TimerOne...
   	*/
   
  //Test both LEDs 
  analogWrite(led1, 255);
  analogWrite(led2, 255);
  delay(1000);
  analogWrite(led1, 0);
  analogWrite(led2, 0);
  delay(1000);

  //Initilize time which call the Thread controller
  Timer1.initialize(2000);
  Timer1.attachInterrupt(timerCallback);
  Timer1.start();
}




void loop(){
}












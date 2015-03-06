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
 Hearbeat sender       -~>   Heartbeat receiver      possibly no Portforwarding but both Xbees set on Internetserver IP, restart Sender will break Connectionpipe unless server kippsconnecton alive before 10 seconds past
 sudo ncat -l -k -vv 29 | sudo ncat -l -k -vv 26  
 
 Hearbeat sender       -~>   Heartbeat receiver     restart Sender without breaking Connection Pipe.. receiver Router has to Portforward 6066 to 192.168.10.111
 sudo ncat -l -k -vv 29 | ncat 192.168.10.111 6066
 
 
 Assuming one Xbee Wifi is sending to port 23 and the other one on 26
 
 */



// ThreadController that will controll all threads
ThreadController controll = ThreadController();


Thread recDataThread = Thread();
Thread keepAliveThread = Thread();
Thread onePulseThread = Thread();
Thread pulseLightThread = Thread();
Thread pulseEnddimThread = Thread();

int led1=10;
int led2=11;
 int closeIt=0;


//SoftwareSerial xb(0, 1);// RX, TX
const int analogInPin = A0; 

volatile int pulslight=0;

// The R value in the graph equation
float R;
// The number of Steps between the output being on and off
const int pwmIntervals = 255;


//create object for SendData
EasyTransfer ETin; 





struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int  bpm;

};

//give a name to the group of data
RECEIVE_DATA_STRUCTURE recdata;







// callback for recDataThread receivesData from Server
void recDataCallback(){

  if (ETin.receiveData()){
    Timer1.stop();
    float recbpm=0.00;
    //int millisecs=0;

    //light1be=recdata.pressure;
    //smoothLight1Thread.enabled=true;
    recbpm = recdata.bpm;
    Serial.println("BPM Before: ");
    Serial.println(recbpm);

    recbpm = recbpm/60;
    recbpm = 1000/recbpm;
    //recbpm = 1/recbpm;

    Serial.println("Millisec After: ");
    Serial.println(recbpm);

    //millisecs=
    //pulslight=255;
    
    
      //onePulseThread.enabled=false;
      onePulseThread.setInterval((int)recbpm);
      if (!onePulseThread.enabled)
       onePulseThread.enabled=true;


    Timer1.resume(); 
  }


}




void pulseEnddimCallback(){


  Serial.println("40 Sec over no Data received -> dimm light ");
  if (onePulseThread.enabled)
    onePulseThread.enabled=false;


}




void keepAliveCallback(){


  Serial.println("Keep Alive siganl for netcat Connection");
  


}


void onePulseCallback(){ //get the brithness up again

  Timer1.stop(); 


  pulslight=255;

  Timer1.resume();
}


void pulseLightCallback(){ //pull the light down slowly

  Timer1.stop(); 
  int brightness=0;
  
  //for linear led light... did not work korrektly
  if (pulslight>50){
    brightness = pow (2, (pulslight / R)) ;
    analogWrite(led1, brightness);
    pulslight--;
    // Calculate the required PWM value for this interval step
    
  }

 
  Timer1.resume();


}

// This is the callback for the Timer
void timerCallback(){
  controll.run();
}



void setup(){

  Serial.begin(9600);

  ETin.begin(details(recdata), &Serial);




  // Configure recDataCallback
  recDataThread.onRun(recDataCallback);
  recDataThread.setInterval(300);




  //Heartbeat up
  onePulseThread.onRun(onePulseCallback);
  onePulseThread.setInterval(500);

  //PULL The puls light down
  pulseLightThread.onRun(pulseLightCallback);
  pulseLightThread.setInterval(10);

  //after 10 sec end the pulse thread if no data comes in this will stay
  pulseEnddimThread.onRun(pulseEnddimCallback);
  pulseEnddimThread.setInterval(40000);




  keepAliveThread.onRun(keepAliveCallback);
  keepAliveThread.setInterval(5000);
 


  // Adds both threads to the controller
  //controll.add(&sendDataThread); // & to pass the pointer to it
  controll.add(&recDataThread);
  controll.add(&onePulseThread);
  controll.add(&pulseLightThread);  
  controll.add(&pulseEnddimThread);
  controll.add(&keepAliveThread);
  //start the library, pass in the data details and the name of the serial port.


  /*
		If using TimerOne...
   	*/
  // Calculate the R variable (only needs to be done once at setup)
  R = (pwmIntervals * log10(2))/(log10(255));
  //Test both LEDs 
  analogWrite(led1, 255);
  analogWrite(led2, 255);
  delay(1000);
  analogWrite(led1, 0);
  analogWrite(led2, 0);
  delay(1000);

  Serial.println("Ready to receive Pulsedata!");

  //Initilize time which call the Thread controller
  Timer1.initialize(2000);
  Timer1.attachInterrupt(timerCallback);
  Timer1.start();
}




void loop(){


}














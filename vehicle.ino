//////////////////////////////////////////////////////////////////////////////////
//
//Filename: Vehicle
//Developed By: Chris Chatzilenas Email: chrischatzilenas@gmail.com
//Last Code Revision: 09/02/2014
//
//////////////////////////////////////////////////////////////////////////////////
//
//Summary: With this code a robot with two DC motors can go  
//straight, turn right, turn left, go backwards and stop.
//This robot has also a webcamera up in a servo.
//With the camera the user has live stream and also can take 
//pictures.
//
//////////////////////////////////////////////////////////////////////////////////
//
//Digital Pins that used: 3,4,6,7,9,10,11,12
//Analog Pins that used: A1
//
//////////////////////////////////////////////////////////////////////////////////

#include <Bridge.h>
#include <Process.h>
#include <YunServer.h>
#include <YunClient.h>
#include <Servo.h>

YunServer server;
Process stream;
Process photo;
Process path;
Process closestream;
String photoshell;
String photoshell1;
String photoshell2;
String readString;
Servo myServo;

const int motor1Pin1 = 3;    // H-bridge leg1 (pin 2, 1A)
const int motor1Pin2 = 4;    // H-bridge leg2 (pin 7, 2A)
const int motor2Pin1= 7;     // H-bridge leg2 (pin 15, 4A)
const int motor2Pin2= 6;     // H-bridge leg1 (pin 10, 3A)
const int enablePin1 = 9;    // H-bridge enable motor1
const int enablePin2= 10;    // H-bridge enable motor2

const int irLeds=12;         //ir leds of camera

const int photoSensor=A1;    //photoresistor

int motorValue=0;
int potValue=0;
int angle=89;
int newangle=89;
int countp=0;


void setup() {
  
  Serial.begin(9600);
  myServo.attach(11);
  
  // set all the other pins you're using as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enablePin1, OUTPUT);
  pinMode(enablePin2, OUTPUT);
  
  //Ir Leds
  pinMode(irLeds,OUTPUT);
  digitalWrite(irLeds,LOW);
     
  Bridge.begin();
    
  //open live stream
  stream.runShellCommand("mjpg_streamer -i 'input_uvc.so -d /dev/video0 -r 640x480 -f 25' -o 'output_http.so -p 8080 -w /www/webcam' &");
    
  // set enablePin high so that motor can turn on:
  digitalWrite(enablePin1, HIGH);
  digitalWrite(enablePin2, HIGH);
    
  //put servo to 89
  myServo.write(angle);
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();

}

void loop() {
  
  int sensorValph=analogRead(photoSensor);
  delay(5);
  int sV=sensorValph/4;    //convert from 0-1023 to 0-255
 
  
  if(sV<127)
  {
   digitalWrite(irLeds,HIGH);   //enable ir leds
  }
  else
  {
    digitalWrite(irLeds,LOW);   //disable ir leds
  }

  
  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // read the command
    String command = client.readString();
    command.trim();        //kill whitespace
    Serial.println(command);
    
    if(command=="forward"){
       //drive straight: 
       //set leg 2 of the 2 motors HIGH to go straight
       digitalWrite(motor1Pin1,LOW);
       digitalWrite(motor1Pin2, HIGH);
    
       digitalWrite(motor2Pin1,HIGH);  
       digitalWrite(motor2Pin2, LOW);
    }
    else if(command=="stop"){
       //stop
      //set leg 1,2 of the 2 motors HIGH or LOW to stop
      digitalWrite(motor1Pin1,HIGH);   
      digitalWrite(motor1Pin2, HIGH);
   
      digitalWrite(motor2Pin1,HIGH);   
      digitalWrite(motor2Pin2, HIGH);
    }
    else if(command=="right"){
      //turn right:
      //set leg 2 of the 2 motors HIGH
      //set leg 1 of motor1 LOW
      //set leg 1 of motor2 Higher than LOW
      digitalWrite(motor1Pin1,LOW);  
      digitalWrite(motor1Pin2, HIGH);
   
      potValue=350;
      motorValue=map(potValue,0,1023,0,255);
   
      digitalWrite(motor2Pin1,HIGH);   
      analogWrite(motor2Pin2, motorValue);
    }
    else if(command=="left"){
       //turn left:
       //set leg 2 of the 2 motors HIGH
       //set leg 1 of motor2 LOW
       //set leg 1 of motor1 Higher than LOW
   
       potValue=350;
       motorValue=map(potValue,0,1023,0,255);
       analogWrite(motor1Pin1,motorValue);  
       digitalWrite(motor1Pin2, HIGH);
   
       digitalWrite(motor2Pin1,HIGH);   
       digitalWrite(motor2Pin2, LOW);
    }
    else if(command=="reverse"){
       //reverse
       //set leg 1 of the 2 motors HIGH to go backwards
       digitalWrite(motor1Pin2,LOW);   
       digitalWrite(motor1Pin1, HIGH);
   
       digitalWrite(motor2Pin2,HIGH);   
       digitalWrite(motor2Pin1, LOW);
    }
    else if(command=="servoLeft"){
      newangle=newangle-20;
      if(newangle+20<20){
         myServo.write(0);
      }
      myServo.write(newangle);
    }
    else if(command=="servoRight"){
       newangle=newangle+20;
       myServo.write(newangle);
    }
    else if(command=="servoCenter"){
       newangle=89.5;
       myServo.write(newangle);
    }  
    else if(command=="photo"){
       //close stream to take photo
       closestream.runShellCommand("kill -9 `pidof mjpg_streamer`");
       
       while(closestream.running());
       //take photo
       photoshell1="fswebcam /mnt/sda1/photo";
       photoshell2=".png";
       photoshell=photoshell1+countp+photoshell2;
       photo.runShellCommand(photoshell);
       //waits till the picture is saved
       while(photo.running());
       //restart stream
       stream.runShellCommand("mjpg_streamer -i 'input_uvc.so -d /dev/video0 -r 640x480 -f 25' -o 'output_http.so -p 8080 -w /www/webcam' &");
       
       countp++;
    }
    // Close connection and free resources.
    client.stop();
 }
delay(50); // Poll every 50ms
}

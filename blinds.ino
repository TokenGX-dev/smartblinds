#include <SimpleTimer.h>
#include <ESP8266WiFi.h> 
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <AH_EasyDriver.h> 

/*****************  INDIVIDUAL DETAILS  *********************************/
//All fields with the "EDIT" tag must be filled in with own details

#define USER_WIFI_SSID            "WIFI_SSID"          //EDIT
#define USER_WIFI_PASS            "WIFI_PASS"          //EDIT
#define USER_MQTT_SERVER          "MQTT_SERVER"        //EDIT
#define USER_MQTT_PORT             MQTT_PORT           //EDIT
#define USER_MQTT_USER            "MQTT_DEFAULT_USER"  //EDIT
#define USER_MQTT_PASS            "MQTT_DEFAULT_PASS"  //EDIT
#define USER_MQTT_CLIENT_NAME     "smartblinds"       

#define MOTOR_SPEED               35                  //RPM speed for motor
#define MOTOR_STEPS_PER_ROTATE    1028                //Number of pulses required for the stepper to rotate 360 degrees
#define MOTOR_MICROSTEPPING       0                   //0 = no microstepping, 1 = 1/2 stepping, 2 = 1/4 stepping 
#define DRIVER_INVERTED_SLEEP     1                   //0 = sleep pin, 1 = enable pin

#define STEPS_TO_CLOSE            12                  //Number of steps needed to completely open or close

#define DIR_PIN                   D6
#define STEP_PIN                  D7
#define SLEEP_PIN                 D5
#define MICROSTEP_PIN1            14
#define MICROSTEP_PIN2            12
 
/***********************************************************************/


WiFiClient wiFiClient;
PubSubClient client(wiFiClient);
SimpleTimer timer;
AH_EasyDriver shadeStepper(MOTOR_STEPS_PER_ROTATE, DIR_PIN, STEP_PIN, MICROSTEP_PIN1, MICROSTEP_PIN2, SLEEP_PIN);

bool boot                      = true;
int  currentPos                = 0;
int  newPos                    = 0;
bool moving                    = false;
char positionPublish[50];
char payload[50];

const char* ssid               = USER_WIFI_SSID ; 
const char* pass               = USER_WIFI_PASS ;
const char* mqtt_server        = USER_MQTT_SERVER ;
const int   mqtt_port          = USER_MQTT_PORT ;
const char *mqtt_user          = USER_MQTT_USER ;
const char *mqtt_pass          = USER_MQTT_PASS ;
const char *mqtt_client_name   = USER_MQTT_CLIENT_NAME ; 




//Functions
void setup_wifi() {
  // Connect to WiFi
 
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
   
    delay(500);
    Serial.print(".");
   
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
}

void reconnect() {
 
  int retryAttempts = 0;
 
  while (!client.connected()) {
   
    if(retryAttempts < 150) {
     
      Serial.print("Attempting MQTT connection...");
     
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
       
        Serial.println("Connected");
       
        if(boot == false) {
         
          client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Reconnected"); 
         
        }
       
        if(boot == true) {
         
          client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Rebooted");
         
        }
       
        client.subscribe(USER_MQTT_CLIENT_NAME"/blindsCommand");
        client.subscribe(USER_MQTT_CLIENT_NAME"/positionCommand");
       
      } else {
       
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        retryAttempts++;

        delay(5000);
       
      }
     
    }
   
    if(retryAttempts > 149) {
     
        ESP.restart();
     
    }
   
  }
 
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Processes MQTT commands
 
  Serial.print("Message arrived [");
  String newTopic = topic;
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
 
  String newPayload = String((char *)payload);
  int intPayload = newPayload.toInt();
  Serial.println(newPayload);
  Serial.println();
  newPayload.toCharArray(charPayload, newPayload.length() + 1);
 
  if (newTopic == USER_MQTT_CLIENT_NAME"/blindsCommand") {
   
    if (newPayload == "OPEN") {
     
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand", "0", true);
     
    } else if (newPayload == "CLOSE") {   
     
      int stepsToClose = STEPS_TO_CLOSE;
      String temp = String(stepsToClose);
      temp.toCharArray(charPayload, temp.length() + 1);
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand", charPayload, true);
     
    } else if (newPayload == "STOP") {
     
      String temp = String(currentPos);
      temp.toCharArray(positionPublish, temp.length() + 1);
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand", positionPublish, true);
     
    }
   
  }
 
  if (newTopic == USER_MQTT_CLIENT_NAME"/positionCommand") {
   
    if(boot == true) {
     
      newPos = intPayload;
      currentPos = intPayload;
      boot = false;
     
    }
   
    if(boot == false) {
     
      newPos = intPayload;
     
    }
   
  }
  
}

void processStepper() {
 
  if (newPos > currentPos) {
   
    #if DRIVER_INVERTED_SLEEP == 1
    shadeStepper.sleepON();
    #endif
    #if DRIVER_INVERTED_SLEEP == 0
    shadeStepper.sleepOFF();
    #endif
    shadeStepper.move(80, FORWARD);
    currentPos++;
    moving = true;
   
  }
 
  if (newPos < currentPos) {
   
    #if DRIVER_INVERTED_SLEEP == 1
    shadeStepper.sleepON();
    #endif
    #if DRIVER_INVERTED_SLEEP == 0
    shadeStepper.sleepOFF();
    #endif
    shadeStepper.move(80, BACKWARD);
    currentPos--;
    moving = true;
   
  }
 
  if (newPos == currentPos && moving == true) {
   
    #if DRIVER_INVERTED_SLEEP == 1
    shadeStepper.sleepOFF();
    #endif
    #if DRIVER_INVERTED_SLEEP == 0
    shadeStepper.sleepON();
    #endif
    String temp = String(currentPos);
    temp.toCharArray(positionPublish, temp.length() + 1);
    client.publish(USER_MQTT_CLIENT_NAME"/positionState", positionPublish); 
    moving = false;
   
  }
 
  Serial.println(currentPos);
  Serial.println(newPos);
 
}

void checkIn() {
  //Checks to make sure device is connected to server
  //If you can see this line from an MQTT webclient, you are connected
 
  client.publish(USER_MQTT_CLIENT_NAME"/checkIn","OK"); 
 
}


void setup() {
  //Run once setup
 
  Serial.begin(115200);
  shadeStepper.setMicrostepping(MOTOR_MICROSTEPPING);            // 0 -> Full Step                                
  shadeStepper.setSpeedRPM(MOTOR_SPEED);     // set speed in RPM, rotations per minute
  #if DRIVER_INVERTED_SLEEP == 1
  shadeStepper.sleepOFF();
  #endif
  #if DRIVER_INVERTED_SLEEP == 0
  shadeStepper.sleepON();
  #endif
 
  WiFi.mode(WIFI_STA);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  ArduinoOTA.setHostname(USER_MQTT_CLIENT_NAME);
  ArduinoOTA.begin(); 
  delay(10);
  timer.setInterval(((1 << MOTOR_MICROSTEPPING)*5800)/MOTOR_SPEED, processStepper);   
  timer.setInterval(90000, checkIn);
 
}

void loop() {
 
  if (!client.connected()) {
   
    reconnect();
   
  }
 
  client.loop();
  ArduinoOTA.handle();
  timer.run();
 
}

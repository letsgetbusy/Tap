/*

GPIO13  Blue WiFi LED, Active Low PWM1

GPIO0 Button Channel 1  E-FW
GPIO9 Button Channel 2  IO9
GPIO10  Button Channel 3  IO10
GPIO14  Button Channel 4  IO14

GPIO12  Relay + LED Channel 1, Active High  IO12
GPIO5 Relay + LED Channel 2, Active High  IO5
GPIO4 Relay + LED Channel 3, Active High  IO4
GPIO15  Relay + LED Channel 4, Active High  IO15

GPIO2 Pin1 Prog Header  SDA
GPIO7 Not connected, empty space for a 2.5mm jack (see TH16)  GPIO07
GPIO8 Not connected, empty space for a 2.5mm jack (see TH16)  GPIO08

https://www.arduino.cc/en/tutorial/debounce


PubSubClient:
A che servet client.loop() ?

https://pubsubclient.knolleary.net/api.html
https://gist.github.com/igrr/7f7e7973366fc01d6393


Restart:
https://github.com/esp8266/Arduino/issues/1722



Case	Choose		SumTot		SumCouple

0000	Nulla		0			0	0
0001	Go			1			0	1
0010	Go			1			0	1
0011	Reset		2			0	2*
0100	Go			1			1	0
0101	Go Go		2			1	1
0110	Go Go		2			1	1
0111	Break		3			1	2*
1000	Go			1			1	0
1001	Go Go		2			1	1
1010	Go Go		2			1	1
1011	Break		3			1	2*
1100	Reset		2			2*	0
1101	Break		3			2*	1
1110	Break		3			2*	1
1111	Break		4			2*	2*


Pointers:

int x = 25;
int* p = &x; // la locazione di memoria di x
cout << *p  // 25
cout <<  p  // la locazione di memoria

x  = x  + 5;
x  = *p + 5;
*p = *p + 5:

sono tutti è tre uguali. *p è alias di x



#include <stdio.h>
void scambio(int *,int *);
main()
{
 int x, y;
 printf ("\n Dammi due interi decimali");
 scanf("%d %d",&x,&y);
 printf("%d%d",x,y);
 scambio(&x,&y);
 printf("%d%d",x,y);
}
 void scambio(int *a,int *b)
{
 int z;
 z=*a;
 *a=*b;
 *b=z;
 printf("%d%d",*a,*b);
}




*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


const char* ssid = "WEBFAST";
const char* password = "qwertyuI";
const char* mqtt_server = "192.168.1.150";

const int LED0 = 13;

const int LED1 = 12;
const int LED2 = 5;
const int LED3 = 4;
const int LED4 = 15;

const int button1 = 0;
const int button2 = 9;
const int button3 = 10;
const int button4 = 14;

boolean LED0State = false;

boolean LED1State = false;
boolean LED2State = false;
boolean LED3State = false;
boolean LED4State = false;

int countbutton1 = 0; // Counter how many timse press
int countbutton2 = 0; // Counter how many timse press
int countbutton3 = 0; // Counter how many timse press
int countbutton4 = 0; // Counter how many timse press

int Sumcountbutton = 0; // Sum Counter how many timse press
int Lastcountbutton = 0; // Last Sum Counter how many timse press

long buttonTimer = 0; // millis() when pressed the button to analize if short or long press
long button1Cicle = 0; // millis() when pressed the button befoure reset count
long button2Timer = 0; // millis() when pressed the button to analize if short or long press

long longPressTime = 250; // minim duration to identify a Long pressed button
long lastDebounceTime = 0; // millis() when pressed the button to analize a Debounce press
long debounceDelay = 50; // minim duration to identify a Debounce
long WaitGoTap = 2000; // minim duration to Send Go to Tapparell
long ResetButtonTime = 3000; // minim duration to reset previous press
long TimeMoveTap = 10000; // duration move Tapparell
long EmergencyBreak = 0; // when stop move required

int lastButton1State = HIGH; // Default state SONOFF 4CH button si HIGH.
int lastButton2State = HIGH; // Default state SONOFF 4CH button si HIGH.
int lastButton3State = HIGH; // Default state SONOFF 4CH button si HIGH.
int lastButton4State = HIGH; // Default state SONOFF 4CH button si HIGH.

boolean buttonActive = false; // Flag: if true a button still press
boolean button2Active = false; // Flag: if true a button still press
boolean longPressActive = false; // Flag: if true long press detect
boolean longPress2Active = false; // Flag: if true long press detect

boolean move1 = false; // Flag: if some Tapperel is moving
boolean move2 = false; // Flag: if some Tapperel is moving
boolean move3 = false; // Flag: if some Tapperel is moving

boolean Emergency = false; // Flag: if stop Tapperel was required

WiFiClient espClient;

PubSubClient client(espClient); // sostituito con quello sotto
//PubSubClient client(mqtt_server, 1883, callback, espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
  digitalWrite(LED1, LED1State);
  digitalWrite(LED2, LED2State);
  digitalWrite(LED3, LED3State);
  digitalWrite(LED4, LED4State);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); //Messo sopra in una stringa
  client.setCallback(callback); //Messo sopra in una stringa

}

void setup_wifi() {

  delay(1000);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  delay(1000);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED0, HIGH);
}

/*

Subscription Callback

If the client is used to subscribe to topics, a callback function must be provided in the constructor. This function is called when new messages arrive at the client.

The callback function has the following signature:

void callback(const char[] topic, byte* payload, unsigned int length)

Parameters

topic - the topic the message arrived on (const char[])
payload - the message payload (byte array)
length - the length of the message payload (unsigned int)

Internally, the client uses the same buffer for both inbound and outbound messages. After the callback function returns, or if a call to either publish or subscribe is made from within the callback function, the topic and payload values passed to the function will be overwritten. The application should create its own copy of the values if they are required beyond this.


*/

void callback(char * topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char) payload[0] == '1') {
    digitalWrite(LED4, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(LED4, HIGH); // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    delay(1000);
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Hello World!!!");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void func1(boolean *longPressActive, boolean *LEDState, boolean *move, char str[100] ) {

  *longPressActive = false;
  *LEDState = false; // NEW: Long Press, need to Off the light when release
  digitalWrite(LED1, *LEDState); // NEW: Long Press, need to Off the light when release
  //Serial.println("Spengo dopo Premuto Lungo. Spengo Luce 1");
  Serial.println(str);
  *move = false;
	
}
	


void loop() {

  /*
  int connected ()
  Checks whether the client is connected to the server.
  Returns
  	false - the client is no longer connected
  	true - the client is still connected
  */

  if (!client.connected()) {
    reconnect();
  }

  /*
  boolean loop ()
  This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
  Returns
  	false - the client is no longer connected
  	true - the client is still connected

  */

  client.loop();

  int reading1 = digitalRead(button1);
  int reading2 = digitalRead(button2);
  int reading3 = digitalRead(button3);
  int reading4 = digitalRead(button4);
  
  if ( millis() - EmergencyBreak > ResetButtonTime && Emergency) {
	 Emergency = false;
	 EmergencyBreak = 0;	  
	  
  }
  
  if (reading1 != lastButton1State) {
    lastDebounceTime = millis();
  }

  if (reading1 == LOW && millis() - lastDebounceTime > debounceDelay && (!Emergency)) {
	  
	if ( (move3) || (move2))  {
	// Premo con qualcosa in movimento s da solo. Stop tutto e aspetto un reset time
	
	// STOPPA TUTTO  FUNC()
		
      LED1State = false;
      LED2State = false;
      LED3State = false;
      LED4State = false;
      digitalWrite(LED1, LED1State);
      digitalWrite(LED2, LED2State);
      digitalWrite(LED3, LED3State);
      digitalWrite(LED4, LED4State);
	  move1 = false;
	  move2 = false;
	  move3 = false;
	  EmergencyBreak = millis();
	  Emergency = true;
	  countbutton1 = 0;
	  button1Cicle = 0;
	  
	}	

    if (buttonActive == false) { // Here only the first moment when button has been pressed
	
			// Variabili.. 

      buttonActive = true;
      buttonTimer = millis();
      countbutton1++; // NEW: From 0 to 1

      if (button1Cicle == 0) {
        button1Cicle = buttonTimer; // button1Cicle = moment used to calculate if the Tap must bu move
      }

    }

    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {

      longPressActive = true;
      //LED1State = !LED1State;
      //digitalWrite(LED1, LED1State);
      Serial.println("Premuto a lungo. Aspetto WaitGoTap prima di accendere");
      client.publish("outTopic", "button1-LONG");

    }
 
 // Messo l'attesa prima di avviare dopo premuta a lungo
 

    if ((millis() - buttonTimer > WaitGoTap) && (longPressActive == true) && (!move1)) {

      LED1State = true;
      digitalWrite(LED1, LED1State);
      Serial.println("Premuto a lungo. Partiamo dopo WaitGoTap");
      client.publish("outTopic", "button1-LONG");
	  move1 = true;

    }

  } else {
    if (buttonActive == true) { // been pressed?

      if (longPressActive == true) {  //LLLUUUUUUUUUUUUUIIIIIIIIIIIIII
        
		func1(&longPressActive, &LED1State, &move1, "Spengo dopo Premuto Lungo. Spengo Luce 1" );
        
		// longPressActive = false;
        // LED1State = false; // NEW: Long Press, need to Off the light when release
        // digitalWrite(LED1, LED1State); // NEW: Long Press, need to Off the light when release
        // Serial.println("Spengo dopo Premuto Lungo. Spengo Luce 1");
        // move1 = false;
        

      } else {

        // LED2State = !LED2State;
        // digitalWrite(LED2, LED2State);
        Serial.println("Premuto corto");
        client.publish("outTopic", "button1-SHORT");

        if (countbutton1 == 1) {
          Serial.println("!!Premuto solo una volta");
        }

      }

      buttonActive = false;

    }

  }

  // If pressed more than 1, nothing is moving, i wait the correct waitng time, and not been a long pressed eventt ( Ex.: 1 short, 2 long... i want olny move the select tapparell)

  
  
  if ((countbutton1 == 2) && (!move2) && (millis() - buttonTimer > WaitGoTap) && (!longPressActive)) {
  //if ((countbutton1 == 2) && (!move2) && (millis() - button1Cicle > WaitGoTap) && (!longPressActive)) {
    Serial.println("!!Premeto due volte e dopo 2 secondi");
    LED1State = true; // NEW: Long Press, need to Off the light when release
    digitalWrite(LED1, LED1State); // NEW: Long Press, need to Off the light when release
    move2 = true;
  }

 // if ((countbutton1 == 3) && (!move3) && (millis() - button1Cicle > WaitGoTap) && (!longPressActive)) {
  if ((countbutton1 == 3) && (!move3) && (millis() - buttonTimer > WaitGoTap) && (!longPressActive)) {
    Serial.println("!!Premetu tre volte e dopo 2 secondi");
    LED1State = true; // NEW: Long Press, need to Off the light when release
    digitalWrite(LED1, LED1State); // NEW: Long Press, need to Off the light when release
    move3 = true;
  }

 // if ((countbutton1 == 2) && (move2) && (millis() - button1Cicle > TimeMoveTap) && (!longPressActive)) {
  if ((countbutton1 == 2) && (move2) && (millis() - buttonTimer > TimeMoveTap) && (!longPressActive)) {
    Serial.println("!!Premetu due volte e dopo 10 secondi sotto");
    LED1State = false; // NEW: Long Press, need to Off the light when release
    digitalWrite(LED1, LED1State); // NEW: Long Press, need to Off the light when release
    move2 = false;
  }

 // if ((countbutton1 == 3) && (move3) && (millis() - button1Cicle > TimeMoveTap) && (!longPressActive)) {
  if ((countbutton1 == 3) && (move3) && (millis() - buttonTimer > TimeMoveTap) && (!longPressActive)) {
    Serial.println("!!Premetu tre volte e dopo 10 secondi sotto");
    LED1State = false; // NEW: Long Press, need to Off the light when release
    digitalWrite(LED1, LED1State); // NEW: Long Press, need to Off the light when release
    move3 = false;
  }

  lastButton1State = reading1;

  Sumcountbutton = countbutton1 + countbutton2 + countbutton3 + countbutton4;

  if (Sumcountbutton != Lastcountbutton) {
    Lastcountbutton = Sumcountbutton;
    
    Serial.println();    
    Serial.print("countbutton: ");
    Serial.print(countbutton1);
    Serial.print(" ");
    Serial.print(countbutton2);
    Serial.print(" ");
    Serial.print(countbutton3);
    Serial.print(" ");
    Serial.print(countbutton4);
    Serial.print(" ");
    Serial.print(buttonTimer);
    Serial.print(" ");
    Serial.print(button1Cicle);
    Serial.println();
  }

  if ((millis() - buttonTimer > ResetButtonTime) && (countbutton1 != 0) && !move2 && !move3) {

    countbutton1 = 0;
    Serial.println("Resetto counter a 0");
    Serial.println(countbutton1);
    Serial.println();
    button1Cicle = 0;

  }
  
  
  
// BUTTON  Nr.2


  if (reading2 == LOW) {

    if (button2Active == false) { // Here only the first moment when button has been pressed

      button2Active = true;
      button2Timer = millis();

    }

    if ((millis() - button2Timer > longPressTime) && (longPress2Active == false)) {

      longPress2Active = true;
      LED4State = !LED4State;
      digitalWrite(LED4, LED4State);
      Serial.println("Premuto a lungo tasto 2");

    }

  } else {
    if (button2Active == true) { // been pressed?

      if (longPress2Active == true) {

        longPress2Active = false;
        LED4State = !LED4State; // NEW: Long Press, need to Off the light when release
        digitalWrite(LED4, LED4State); // NEW: Long Press, need to Off the light when release
        Serial.println("Spengo Premuto lungo tasto 2");
        ESP.restart();

      } else {

        LED4State = !LED4State;
        digitalWrite(LED4, LED4State);
        Serial.println("Premuto corto tasto 2");
        client.publish("outTopic", "button1-SHORT");

      }

      button2Active = false;

    }

  }

}

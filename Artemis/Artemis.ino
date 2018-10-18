//
// Artemis-Mega
//
// Description of the project
// Developed with [embedXcode](http://embedXcode.weebly.com)
//
// Author         Bob Smith
//                 Bob Smith
//
// Date            10/13/18 8:18 PM
// Version        0.1
//
// Copyright    © Bob Smith, 2018
// Licence        CC Share Alike
//
// See         ReadMe.txt for references
//


// Core library for code-sense - IDE-based
// !!! Help: http://bit.ly/2AdU7cu
#include "Arduino.h"
#include <Arduino.h>
#include "Ethernet.h"
#include <EthernetUdp.h>


// Set parameters
int debugMode = 0;
enum  pattern { NONE, IMPULSE, FADE, WARPCHASE, GAUGE, SHIELDS };
enum  direction { FORWARD, REVERSE };
// Include application, user and local libraries
#include "Adafruit_NeoPixel.h"
#include <Adafruit_NeoPixel.h>
// Define structures and classes

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
public:

// Member Variables:
pattern  ActivePattern;  // which pattern is running
direction Direction;     // direction to run the pattern

unsigned long Interval;   // milliseconds between updates
unsigned long lastUpdate; // last update of position

uint32_t Color1, Color2;  // What colors are in use
uint16_t TotalSteps;  // total number of steps in the pattern
uint16_t Index;  // current step within the pattern
uint16_t State;
uint16_t State2;
uint16_t Trail;
uint16_t Rate;
uint16_t Target;
uint16_t CurrentStep;
uint8_t Bounce;
float tState;
uint8_t StartIndex;



// Constructor - calls base-class constructor to initialize strip
NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type)
:Adafruit_NeoPixel(pixels, pin, type)
{
}
// Update the pattern
void Update()
{
if (State != 0)
{

if((millis() - lastUpdate) > Interval) // time to update
{
lastUpdate = millis();
switch(ActivePattern)
{
case IMPULSE:
ImpulseUpdate();
break;
case FADE:
FadeUpdate();
break;
case WARPCHASE:
WarpChaseUpdate();
break;
case GAUGE:
GaugeUpdate();
break;
case SHIELDS:
FrontShieldUpdate();
RearShieldUpdate();
default:
break;
}
}
}
}

// Increment the Index and reset at the end
void Increment()
{
if (Direction == FORWARD)
{
Index++;
if (Index >= TotalSteps)
{
Index = 0;
if (Bounce == 1)
{
Reverse(); // call the comlpetion callback
}
}
}
else // Direction == REVERSE
{
--Index;
if (Index <= 0)
{
Index = TotalSteps-1;
if (Bounce == 1)
{
Reverse(); // call the comlpetion callback
}
else
{
Index = 0;
}
}
}
}

// Reverse pattern direction
void Reverse()
{
if (Direction == FORWARD)
{
Direction = REVERSE;
Index = TotalSteps-1;
}
else
{
Direction = FORWARD;
Index = 0;
}
}

// Initialize for a Fade
void FadeConfig(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
{
if (debugMode == 1) {Serial.println("FadeConfig() detected");delay(1000);}
ActivePattern = FADE;
Interval = interval;
TotalSteps = steps;
Color1 = color1;
Color2 = color2;
Index = 0;
Direction = dir;
Bounce = 1;

}

// Update the Fade Pattern
void FadeUpdate()
{
if (debugMode == 1) {Serial.println("FadeUpdate() detected");
Serial.print((String)"Index = " + Index); delay(1000);}
// Calculate linear interpolation between Color1 and Color2
// Optimise order of operations to minimize truncation error
uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

ColorSet(Color(red, green, blue));
show();
StartIndex = Index;
Increment();
}

// Calculate a dimmed version of a color (used by ImpulseUpdate())
uint32_t DimColor(uint32_t color)
{
float shader = .2;
// Shift R, G and B components one bit to the right
//uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
uint32_t dimColor = Color(Red(color) * (1-shader) , Green(color) * (1-shader), Blue(color) * (1 - shader));
return dimColor;
}

// Set all pixels to a color (synchronously)
void ColorSet(uint32_t color)
{
for (int i = 0; i < numPixels(); i++)
{
setPixelColor(i, color);
}
show();
}

// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color)
{
return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
{
return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
{
return color & 0xFF;
}

//WarpChase setup
void WarpChaseConfig(uint32_t color,int len)
{

}

// Update the warpChase pattern
void WarpChaseUpdate()
{

}

// Initialize for Impulse power
void ImpulseConfig(uint32_t color1, uint8_t interval)
{
ActivePattern = IMPULSE;
Interval = interval;
TotalSteps = (numPixels() - 1) * 2;
Color1 = color1;
Index = 0;
}

// Update the Impulse Pattern
void ImpulseUpdate()
{
Interval = (1330/33)-((10*State)/33);  //thanks, Wolfram Alpha interpolate function!
for (int i = 0; i < numPixels(); i++)
{
if (i == Index)  // Scan Pixel to the right
{
setPixelColor(i, Color1);
}
else if (i == TotalSteps - Index) // Scan Pixel to the left
{
setPixelColor(i, Color1);
}
else // Fading tail
{
setPixelColor(i, DimColor(getPixelColor(i)));
}
}
show();
Increment();
}
//Configure Gauge parameters
void GaugeConfig(uint32_t color)
{
if (debugMode == 1) {Serial.println("GaugeConfig() detected"); delay(2000);}
}
//Update Gauge effect
void GaugeUpdate()
{
if (debugMode == 1) {Serial.println("GaugeUpdate() detected"); delay(2000);}
}
//Update front shield gauge
//will need to check logic for when one is up and the other down
void FrontShieldUpdate()
{
if (debugMode == 1) {Serial.println("FrontShieldUpdate() detected"); delay(2000);}
}
//update rear shield gauge
//will need to check logic for when one is up and the other down
void RearShieldUpdate()
{
if (debugMode == 1) {Serial.println("RearShieldUpdate() detected"); delay(2000);}
}
};

// Define variables and constants
NeoPatterns engines(60, 2, NEO_GRB + NEO_KHZ800);
NeoPatterns energy(60, 3, NEO_GRB + NEO_KHZ800);
NeoPatterns shields(60, 4, NEO_GRB + NEO_KHZ800);
NeoPatterns redalert(60, 6, NEO_GRB + NEO_KHZ800);
NeoPatterns environment(60, 7, NEO_GRB + NEO_KHZ800);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };    //manual MAC address of the ethernet port on the Arduino.
//This can be completely arbitrary in my environment as this will
//be directly connected to the arduino from the control PC on a private network*/
IPAddress ip(192, 168, 100, 100); //Using local Private as the arduino is directly connected to the computer
unsigned int localPort = 6454;      // local port to listen on for DMX packets
byte packetBuffer[600]; // buffer to hold incoming packet, DMX packet is pretty much always 530 bytes
EthernetUDP Udp; // An EthernetUDP instance to let us send and receive packets over UDP


// Prototypes
// !!! Help: http://bit.ly/2l0ZhTa


// Utilities


// Functions


// Initialize everything and prepare to start

void setup()
{
//start ethernet session
Ethernet.init(10);
Ethernet.begin(mac, ip);

//    start udp session
Udp.begin(localPort);

//    wait for serial connection to come up -- Only needed for testing
Serial.begin(115200);
while (!Serial){
;
}
// Initialize all the pixelStrips
engines.begin();
energy.begin();
shields.begin();
redalert.begin();
environment.begin();

//Initial config of effect objects
engines.ImpulseConfig(engines.Color(200,40,0), 10);
// engines.WarpChaseConfig(engines.Color(180,0,200), 15);
redalert.FadeConfig(redalert.Color(200,0,0), redalert.Color(0,0,0), 150, 15);
}





// Main loop
void loop()
{
// Check for packet in queue and read it into the buffer
int packetSize = Udp.parsePacket();
if (packetSize)
{
Udp.read(packetBuffer, packetSize);
Serial.println("Packet Received");
Serial.println();

//main actions to take upon finding a packet
// Populate State variables with the received DMX values
engines.State = packetBuffer[18];   //Impulse Power value: 0-100
engines.State2 = packetBuffer[19];  //Warp Power value: 0-100
energy.State = packetBuffer[20];    //Energy level: 0-100
shields.State = packetBuffer[21];   //Front shields level: 0-100
shields.State2 = packetBuffer[22];  //Rear shields level: 0-100
redalert.State = packetBuffer[23];  //Red alert state: 0-1
environment.State = packetBuffer[24];   //environment state: ??

for (int i = 18; i < 30; i++)
{
Serial.print(packetBuffer[i], HEX);
Serial.print(" ");
}
Serial.println();



}
engines.Update();
energy.Update();
shields.Update();
redalert.Update();
environment.Update();
}







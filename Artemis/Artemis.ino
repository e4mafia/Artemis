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
enum  pattern { NONE, IMPULSE, FADE, WARPCHASE, GAUGE, SHIELDS, ENVIRONMENT };
enum  direction { FORWARD, REVERSE };
enum enviro { NEBULA, DOCKED, HIT, SHIELDHIT, RUN };
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
    enviro Environment;
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;         // current step within the pattern
    uint16_t Index2;
    uint16_t State;
    uint16_t State2;
    uint16_t Trail;
    uint16_t Count;
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
                    break;
                case ENVIRONMENT:
                    EnvironmentUpdate();
                default:
                    break;
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
    void FadeConfig(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, int bounce, direction dir = FORWARD)
    {
        if (debugMode == 1) {Serial.println("FadeConfig() detected");delay(1000);}
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
        Bounce = bounce;
        
    }
    
        // Update the Fade Pattern
    void FadeUpdate()
    {
        if (debugMode == 1) {Serial.println("FadeUpdate() detected");
            Serial.print((String)"Index = " + Index); delay(1000);}
        if (State != 0)
        {
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
        else
        {
            effectReset();
            Index = 0;
        }
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
    void WarpChaseConfig(uint32_t color2, int len, direction dir = FORWARD)
    {
        if (debugMode == 1) {Serial.println("WarpChaseConfig() detected"); delay(2000);}
        ActivePattern = WARPCHASE;
        TotalSteps = numPixels();
        Trail = len;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
        // Update the warpChase pattern
    void WarpChaseUpdate()
    {
        if (debugMode == 1) {Serial.println("WarpChaseUpdate() detected"); delay(2000);}
        if (State2 != 0)
        {
            switch (State2)
            {
                case 25:
                    Interval = 25;
                    break;
                case 50:
                    Interval = 20;
                    break;
                case 75:
                    Interval = 13;
                    break;
                case 100:
                    Interval = 7;
                    break;
                default:
                    break;
            }
            
                    //Trail = (25 * State2) / 100;
                    
                    if (Index+Trail>numPixels())
                    {
                        setPixelColor(Index + Trail - numPixels(),Color2);
                    }
                    setPixelColor(Index + Trail, Color2);
                    setPixelColor(Index-1, 0,0,0);
                    show();
                    Increment();
                }
                    else
                    {
                        effectReset();
                        ImpulseConfig(Color1, 0);
                    }
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
        if (State != 0 && State2 == 0)
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
        else
        {
            effectReset();
            WarpChaseConfig(Color2, 10);
        }
        
    }
        //Configure Gauge parameters
    void GaugeConfig(uint32_t color)
    {
        if (debugMode == 1) {Serial.println("GaugeConfig() detected"); delay(2000);}
        ActivePattern = GAUGE;
        Color1 = color;
        Interval = 20;
        Index = 0;
        Count = 0;
        TotalSteps = numPixels();
        Direction = FORWARD;
    }
        //Update Gauge effect
    void GaugeUpdate()
    {
        if (debugMode == 1) {Serial.println("GaugeUpdate() detected"); delay(2000);}
        if (State == 0) { Count++; }
        if ( Count > 2) {effectReset(); Count = 0; Index = 0; Serial.println("Reset Gauge");}
        if (State != 0)
        {
            unsigned long ControlState = (State * numPixels()) / 100;
            if (Index <= ControlState)
            {
                setPixelColor(Index, Color1);
                show();
                Index++;
            }
            else if (Index > ControlState)
            {
                setPixelColor(Index, 0,0,0);
                show();
                Index--;
            }
        }
        
    }

        //Update Gauge effect
 /*   void GaugeUpdate()
    {
        //if (State == 0) { Count++; }
        //if ( Count > 2) {effectReset(); Count = 0;}
        if (State != 0)
        {
            unsigned long ControlState = (State * numPixels()) / 100;
            Serial.println((String)"Control = " + ControlState + " -- Index = " + Index);
            setPixelColor(Index,0,0,0);
            if (Index <=  ControlState)
                {
                    setPixelColor(Index,Color1);
                    Serial.println((String)"Set light #  " + Index + " to ON");
                }
            show();
            Increment();
            }
    }
*/
    void ShieldsConfig(uint32_t color, uint32_t color2)
    {
        if (debugMode == 4) {Serial.println("ShieldsConfig() detected"); delay(2000);}
        ActivePattern = SHIELDS;
        Interval = 40;
        Index = numPixels()/2 + 1;
        Index2 = numPixels()/2 - 1;
        Color1 = color;
        Color2 = color2;
        setPixelColor(numPixels()/2, 0, 0, 255);
        show();
    }
        
        //Update front shield gauge
        //will need to check logic for when one is up and the other down
    void FrontShieldUpdate()
    {
        if (debugMode == 4) {Serial.println("FrontShieldUpdate() detected"); delay(2000);}
        unsigned long ControlState =  numPixels()/2 + (State * .3);
        if (State != 0)
        {
            if (Index < ControlState)
            {
                setPixelColor(Index, Color1);
                show();
                Index++;
            }
            else if (Index > ControlState)
            {
                setPixelColor(Index, 0,0,0);
                show();
                Index--;
            }
        }
        else
        {
            if (Index > ControlState)
            {
                setPixelColor(Index,0,0,0);
                show();
                Index--;
            }
        }
        setPixelColor((numPixels() / 2), 0, 0, 255);
    
    }
        //update rear shield gauge
        //will need to check logic for when one is up and the other down
    void RearShieldUpdate()
    {
        if (debugMode == 4) {Serial.println("RearShieldUpdate() detected"); delay(2000);}
        {
            if (debugMode == 4) {Serial.println("FrontShieldUpdate() detected"); delay(2000);}
            
            unsigned long ControlState2 = numPixels() / 2 - (State2 * .3);
            if (State2 != 0)
            {
                if (Index2 > ControlState2)
                {
                    setPixelColor(Index2, Color1);
                    show();
                    Index2--;
                }
                else if (Index2 < ControlState2)
                {
                    setPixelColor(Index2, 0,0,0);
                    show();
                    Index2++;
                }
            }
            else
            {
                if (Index2 < ControlState2)
                {
                    setPixelColor(Index2,0,0,0);
                    show();
                    Index2++;
                }
            }
        }
    }
    
    void effectReset()
    {
        ColorSet(Color(0,0,0));
        show();
    }
    
        // Initialize for a Fade
    void EnvironmentConfig(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, int bounce, direction dir = FORWARD)
    {
        if (debugMode == 5) {Serial.println("EnvironmentConfig() detected");delay(1000);}
        ActivePattern = ENVIRONMENT;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
        Bounce = bounce;
        
    }
    
        // Update the Fade Pattern
    void EnvironmentUpdate()
    {
        if (debugMode == 3) {Serial.println("EnvironmentUpdate() detected");
            Serial.println((String)"Index = " + Index + "   State = " + State + "  State2 = " + State2); delay(1000);}
        if (State != 0 && State != State2)
        {
                switch (State)
                {
                    case 1:     //Within Nebula
                        EnvironmentConfig(Color(0,25,0), Color(50,0,150), 200, 5, 1);
                        break;
                    case 2:     //Docking
                        EnvironmentConfig(Color(180,180,0), Color(80,80,0), 100, 20, 1);
                        break;
                    case 3:     //Docked
                        EnvironmentConfig(Color(180,180,0), Color(180,180,0), 1, 100, 0);
                        break;
                    case 4:     //Shield Hit
                        EnvironmentConfig(Color(0,0,200), Color(0,0,100), 5, 5, 0);
                        break;
                    case 5:     //Ship hit
                        EnvironmentConfig(Color(255,0,0), Color(255,140,0), 5, 5, 0);
                        break;
                    default:
                        break;
                }
        }
        if (State != 0)
        {
            
                // Calculate linear interpolation between Color1 and Color2
                // Optimise order of operations to minimize truncation error
            uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
            uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
            uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
            
            ColorSet(Color(red, green, blue));
            show();
            //StartIndex = Index;
            Increment();
            State2 = State;
        }
        else
        {
            effectReset();
            Index = 0;
        }
    }
};

    // Define variables and constants
NeoPatterns engines(60, 6, NEO_GRB + NEO_KHZ800);
NeoPatterns energy(60, 5, NEO_GRB + NEO_KHZ800);
NeoPatterns shields(60, 4, NEO_GRB + NEO_KHZ800);
NeoPatterns redalert(60, 2, NEO_GRB + NEO_KHZ800);
NeoPatterns environment(60, 3, NEO_GRB + NEO_KHZ800);

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
    engines.ImpulseConfig(engines.Color(200,100,0), 0);
    engines.WarpChaseConfig(engines.Color(180,0,200), 10);
    redalert.FadeConfig(redalert.Color(200,0,0), redalert.Color(0,0,0),150, 15, 1);
    energy.GaugeConfig(energy.Color(10,100,10));
    environment.EnvironmentConfig(environment.Color(0,0,0), environment.Color(0,0,0), 1, 1, 1);
    shields.ShieldsConfig(shields.Color(0,0,150), shields.Color(0,0,150));
    
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
        
        //Stabilize Energy Gauge
        
        
        for (int i = 18; i < 30; i++)
        {
            Serial.print(packetBuffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        Serial.println((String)"Impulse: " + packetBuffer[18]);
        Serial.println((String)"Warp: " + packetBuffer[19]);
        Serial.println((String)"Energy: " + packetBuffer[20]);
        Serial.println((String)"Front Shields: " + packetBuffer[21]);
        Serial.println((String)"Rear Shields: " + packetBuffer[22]);
        Serial.println((String)"Red Alert: " + packetBuffer[23]);
        Serial.println((String)"Environment: " + packetBuffer[24]);
        
        
        
    }
    engines.Update();
    energy.Update();
    shields.Update();
    redalert.Update();
    environment.Update();
}








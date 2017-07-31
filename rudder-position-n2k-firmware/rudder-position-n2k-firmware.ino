/*
 * FOSS + OSHW NMEA 2000 Rudder Position Sensor
 *  (c) Daniel Harmsworth, 2017
 *  
 *  Based on Hall Effect rotary encoder (0-5v) mounted to replicate rudder position.
 *  
 */

#include <NMEA2000_CAN.h>
#include <N2kMessages.h>

#define VERSION 0.1a
#define HWVERSION prototype

#define SENSOR_PIN A1           // Pin connected to the position sensor
#define SENSOR_RANGE 3141       // Full sensor range in milliradians

#define UPDATE_PERIOD 1000      // How often to update the position and send to the network in milliseconds

#define DEBUG

const unsigned long TransmitMessages[] PROGMEM={127245L,0};

void setup() {
  pinMode(SENSOR_PIN, INPUT);
   
  NMEA2000.SetProductInformation( "00000001",                     // Manufacturer's Model serial code
                                  100,                            // Manufacturer's product code
                                  "OSHW Rudder Position Sensor",  // Manufacturer's Model ID
                                  "VERSION",                      // Manufacturer's Software version code
                                  "HWVERSION"                     // Manufacturer's Model version
  );
  
  NMEA2000.SetDeviceInformation(  112233, // Unique number. Use e.g. Serial number.
                                  155,    // Device function=Rudder. See codes at http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  40,     // Device class=Steering and Control Surfaces. See codes at  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  2040    // Just choose a free one from code list at http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
  );

  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly,22);      // Set the mode, NodeOnly as we're not listening on this device.
  
  #ifdef DEBUG
    Serial.begin(115200);
    NMEA2000.SetForwardStream(&Serial);
    NMEA2000.SetDebugMode(tNMEA2000::dm_Actisense);     // Uncomment if using ATMega2560
  #else
    NMEA2000.EnableForward(false);                      // Do not forward all N2K data to UART.
  #endif
  
  NMEA2000.ExtendTransmitMessages(TransmitMessages);  // Tell the N2K library what PGN's we're going to transmit.
  NMEA2000.Open();                                    // Let the fun begin!
}

void loop() {
  sendN2kRudderPosition();  // Detect the rudder position and transmit it to the N2K network, only runs every UPDATE_PERIOD milliseconds.
  NMEA2000.ParseMessages(); // Read and respond to any incoming messages on the N2K network
}

// Returns the rudder position in milliradians
int getRudderPosition() {
  int RudderPosRaw = analogRead(SENSOR_PIN);  // Read 0-5v value from encoder
  
  // Dead ahead is 0 radians turn, negative number is bow turning to port, positive bow turns to starboard.
  int RudderPos = map(RudderPosRaw, 0, 1024, -SENSOR_RANGE/2, SENSOR_RANGE/2);
  return RudderPos;
}

void sendN2kRudderPosition() {
   static unsigned long LastUpdated=millis();

   tN2kMsg N2kMsg;
   
   if (LastUpdated + UPDATE_PERIOD < millis()) {
     LastUpdated = millis();
     int RawRudderPosition = getRudderPosition();               // Get the rudder position in milliradians
     double RudderPosition = (double)RawRudderPosition / 1000;  // Convert to double precision in radians
     SetN2kRudder(N2kMsg,RudderPosition);                       // Set N2kMsg to a Rudder (PGN 127245) position message based on RudderPrecision
     NMEA2000.SendMsg(N2kMsg);                                  // Send the N2K message to the network
   }
}


#include <NMEA2000_CAN.h>
#include <N2kMessages.h>

#define VERSION 0.1a
#define HWVERSION 0.1a

#define SENSOR_PIN A1           // Pin connected to the position sensor
#define SENSOR_RANGE 3141       // Full sensor range in milliradians


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
                                  155,    // Device function=Rudder. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  40,     // Device class=Steering and Control Surfaces. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  2040    // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
  );

  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly,22);      // Set the mode, NodeOnly as we're not listening on this device.
  NMEA2000.EnableForward(false);                      // Do not forward all N2K data to UART.
  NMEA2000.ExtendTransmitMessages(TransmitMessages);  // Tell the N2K library what PGN's we're going to transmit.
  NMEA2000.Open();                                    // Let the fun begin!
}

void loop() {
  

}

float getHelmPos() {
  int helmPosRaw = analogRead(SENSOR_PIN);
  
  // Dead ahead is 0 radians turn, negative number is turning to port, positive to starboard.
  int helmPos = map(helmPosRaw, 0, 1024, -SENSOR_RANGE/2, SENSOR_RANGE/2);
}


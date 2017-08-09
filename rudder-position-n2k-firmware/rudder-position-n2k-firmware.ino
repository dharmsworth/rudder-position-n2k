/*
 * FOSS + OSHW NMEA 2000 Rudder Position Sensor
 *  (c) Daniel Harmsworth, 2017
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 *  Based on Hall Effect rotary encoder (0-5v) mounted to replicate rudder position.
 *  
 */

#define N2k_SPI_CS_PIN 10  // For DFRobot CAN Shield V2
#define USE_N2K_CAN 1      // tell the library to use the MCP2515 CAN Controller

#include <NMEA2000_CAN.h>
#include <N2kMessages.h>

#define VERSION "0.1a"
#define HWVERSION "prototype"
#define N2K_ADDRESS 23

#define AVERAGE_WINDOW 10       // How many samples to average for noise exclusion.
#define SENSOR_PIN A1           // Pin connected to the position sensor
#define SENSOR_RANGE 3141       // Full sensor range in milliradians
//#define INVERT_SENSOR         // Invert the mapping of sensor position +/- to milliradians +/-

#define UPDATE_PERIOD 500       // How often to update the position and send to the network in milliseconds
#define BEACON_PERIOD 5000      // How often to beacon the device info out. No periodic beacon if undefined.

//#define DEBUG

int rudderPosition = 0;         // Holds the current position of the rudder in milliradians.
int rudderWindow[AVERAGE_WINDOW];
int windowPosition = 0;
const unsigned long TransmitMessages[] PROGMEM={127245L,0}; // List of PGNs we're going to send from this device.

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  
  NMEA2000.SetProductInformation( "00000001",                     // Manufacturer's Model serial code
                                  100,                            // Manufacturer's product code
                                  "OSHW Rudder Sensor",           // Manufacturer's Model ID
                                  VERSION,                        // Manufacturer's Software version code
                                  HWVERSION                       // Manufacturer's Model version
  );
  
  NMEA2000.SetDeviceInformation(  112233, // Unique number. Use e.g. Serial number.
                                  155,    // Device function=Rudder. See codes at http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  40,     // Device class=Steering and Control Surfaces. See codes at  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  2020    // Just choose a free one from code list at http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
  );

  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, N2K_ADDRESS);      // Set the mode, NodeOnly as we're not listening on this device.
  
  #ifdef DEBUG
    Serial.begin(115200);
    NMEA2000.SetForwardStream(&Serial);
    //NMEA2000.SetDebugMode(tNMEA2000::dm_ClearText);     // Uncomment if using ATMega2560
  #else
    NMEA2000.EnableForward(false);                      // Do not forward all N2K data to UART.
  #endif
  
  NMEA2000.ExtendTransmitMessages(TransmitMessages);  // Tell the N2K library what PGN's we're going to transmit.
  NMEA2000.Open();                                    // Let the fun begin!
}

void loop() {
  getRudderPosition();      // Read the rudder sensor, recalculate the moving average and set rudderPosition.
  sendN2kRudderPosition();  // Transmit the rudder position to the N2K network, only runs every UPDATE_PERIOD milliseconds.
  
  #ifdef BEACON_PERIOD
    sendN2kBeacon();        // Send product information and ISO Address Claim, only runs every BEACON_PERIOD milliseconds.
  #endif
  
  NMEA2000.ParseMessages(); // Read and respond to any incoming messages on the N2K network
}

void getRudderPosition() {
  int RudderPosRaw = analogRead(SENSOR_PIN);  // Read 0-5v value from encoder
  
  // Dead ahead is 0 radians turn, negative number is bow turning to port, positive bow turns to starboard.
  #ifdef INVERT_SENSOR
    int RudderPos = map(RudderPosRaw, 0, 1024, SENSOR_RANGE/2, -SENSOR_RANGE/2);
  #else
    int RudderPos = map(RudderPosRaw, 0, 1024, -SENSOR_RANGE/2, SENSOR_RANGE/2);
  #endif

  if (windowPosition >= AVERAGE_WINDOW ) { windowPosition = 0; }
  rudderWindow[windowPosition] = RudderPos;
  windowPosition++;
  
  long rudderSum = 0;
  for ( int i = 0; i < AVERAGE_WINDOW; i++ ){
    rudderSum = rudderSum + (long)rudderWindow[i];
  }
  rudderPosition = rudderSum / AVERAGE_WINDOW;
}

#ifdef BEACON_PERIOD
  void sendN2kBeacon() {
    static unsigned long LastBeacon=millis();
    if (LastBeacon + BEACON_PERIOD < millis() )
    {
      LastBeacon = millis();
      NMEA2000.SendProductInformation();
      NMEA2000.SendIsoAddressClaim();
    }
  }
#endif

void sendN2kRudderPosition() {
   static unsigned long LastUpdated=millis();

   tN2kMsg N2kMsg;
   
   if (LastUpdated + UPDATE_PERIOD < millis()) {
     LastUpdated = millis();
     double rudderPositionRadians = (double)rudderPosition / 1000;  // Convert to double precision in radians
     SetN2kRudder(N2kMsg,rudderPositionRadians);                    // Set N2kMsg to a Rudder (PGN 127245) position message based on RudderPrecision
     NMEA2000.SendMsg(N2kMsg);                                      // Send the N2K message to the network
   }
}


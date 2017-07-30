#define SENSOR_PIN A1           // Pin connected to the position sensor
#define SENSOR_RANGE 3141       // Full sensor range in milliradians

void setup() {
  pinMode(SENSOR_PIN, INPUT);
}

void loop() {
  

}

float getHelmPos() {
  int helmPosRaw = analogRead(SENSOR_PIN);
  
  // Dead ahead is 0 radians turn, negative number is turning to port, positive to starboard.
  int helmPos = map(helmPosRaw, 0, 1024, -SENSOR_RANGE/2, SENSOR_RANGE/2);
}


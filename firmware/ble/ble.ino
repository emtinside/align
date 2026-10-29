#include <ArduinoBLE.h>
#include "Arduino_BMI270_BMM150.h"
#define MINIMUM_TILT 5    // Threshold for tilt detection in degrees
#define WAIT_TIME 500     // How often to run the code (in milliseconds)
float x, y, z;
int angleX = 0;
int angleY = 0;
unsigned long previousMillis = 0;

const char* serviceUUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"; // Service UUID: ID for the BLE module
const char* charUUID    = "beb5483e-36e1-4688-b7f5-ea07361b26a8"; // Characteristic UUID: ID for the packets being sent

// 2. Create the service and characteristic (BLENotify allows streaming)
BLEService dataService(serviceUUID);
BLEIntCharacteristic streamCharacteristic(charUUID, BLERead | BLENotify);

unsigned long lastStreamTime = 0;
int imuValue = 0;

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  BLE.setLocalName("Nano33Stream");
  BLE.setAdvertisedService(dataService);

  dataService.addCharacteristic(streamCharacteristic);
  BLE.addService(dataService);
  
  streamCharacteristic.writeValue(0);
  BLE.advertise();
  Serial.println("Peripheral active, waiting for central...");
  
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println("Hz");
}

void captureIMU() {
    IMU.readAcceleration(x, y, z);

    // Calculate tilt angles in degrees
    angleX = atan2(x, sqrt(y * y + z * z)) * 180 / PI;
    angleY = atan2(y, sqrt(x * x + z * z)) * 180 / PI;

    // Determine the tilting direction based on angleX and angleY
    if (angleX > MINIMUM_TILT) {  // Tilting up
      imuValue = angleX;
      Serial.print("Tilting up ");
      Serial.print(angleX);
      Serial.println(" degrees");
    } else if (angleX < -MINIMUM_TILT) {  // Tilting down
      imuValue = angleX;
      Serial.print("Tilting down ");
      Serial.print(angleX);
      Serial.println(" degrees");
    }

    if (angleY > MINIMUM_TILT) {  // Tilting right
      imuValue = angleY;
      Serial.print("Tilting right ");
      Serial.print(angleY);
      Serial.println(" degrees");
    } else if (angleY < -MINIMUM_TILT) {  // Tilting left
      imuValue = angleY;
      Serial.print("Tilting left ");
      Serial.print(angleY);
      Serial.println(" degrees");
    }
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address()); // This prints your PHONE'S MAC address in the serial monitor

    while (central.connected()) {
      unsigned long currentMillis = millis();
      
      // Stream data every 100ms (10Hz)
      if (currentMillis - lastStreamTime >= 100) {
        lastStreamTime = currentMillis;
        
        captureIMU();
        
        // Push the update to the phone
        streamCharacteristic.writeValue(imuValue); 
      }
    }
    Serial.println("Disconnected from central");
  }
}

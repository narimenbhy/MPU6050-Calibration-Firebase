#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>

#define _SSID "gnet-"          // Your WiFi SSID
#define _PASSWORD "73409415"      // Your WiFi Password
#define REFERENCE_URL "https://________________.firebaseio.com"  // Your Firebase project reference url

Firebase firebase(REFERENCE_URL);

Adafruit_MPU6050 mpu;

const int numSamples = 100;
float accelOffsetX = 0.0;
float accelOffsetY = 0.0;
float accelOffsetZ = 0.0;
float gyroOffsetX = 0.0;
float gyroOffsetY = 0.0;
float gyroOffsetZ = 0.0;

unsigned long duration=0;

void calibrateAccelerometer() {
  Serial.println("Calibrating accelerometer... Keep the MPU6050 still!");
  delay(6000);

  for (int i = 0; i < numSamples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    accelOffsetX += a.acceleration.x;
    accelOffsetY += a.acceleration.y;
    accelOffsetZ += a.acceleration.z;

    gyroOffsetX += g.gyro.x;
    gyroOffsetY += g.gyro.y;
    gyroOffsetZ += g.gyro.z;

    delay(10);
  }

  accelOffsetX /= numSamples;
  accelOffsetY /= numSamples;
  accelOffsetZ /= numSamples;
  gyroOffsetX /= numSamples;
  gyroOffsetY /= numSamples;
  gyroOffsetZ /= numSamples;

  Serial.println("Calibration complete!");
 /* Serial.print("AccelOffsetX: ");
  Serial.print(accelOffsetX);
  Serial.print(", AccelOffsetY: ");
  Serial.print(accelOffsetY);
  Serial.print(", AccelOffsetZ: ");
  Serial.print(accelOffsetZ);
  Serial.print(", GyroOffsetX: ");
  Serial.print(gyroOffsetX);
  Serial.print(", GyroOffsetY: ");
  Serial.print(gyroOffsetY);
  Serial.print(", GyroOffsetZ: ");
  Serial.println(gyroOffsetZ);
  Serial.println("\n"); */
  


}

void setup(void) {
  Serial.begin(57600);
  while (!Serial) {
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
  }

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  delay(100);

  calibrateAccelerometer(); // Call the accelerometer calibration function
  delay(100);

//Firebase
  
 WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(_SSID);
  WiFi.begin(_SSID, _PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  
 
}

float previousLinearAccelMagnitude = 0.0;
String State="";

void send_time(){
  if ((millis() - duration) >= 60000){
    duration= millis();
         Serial.print("Minutes:");
         String data1 =firebase.getString("minutes");
         int minutes= data1.toInt();
         minutes++;
         Serial.println(minutes);
         firebase.setString("minutes", (String)minutes);
  }
}
void MPU() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float linearAccelX = a.acceleration.x - accelOffsetX;
  float linearAccelY = a.acceleration.y - accelOffsetY;
  float linearAccelZ = a.acceleration.z - accelOffsetZ ;
  // Apply gyroscope calibration offsets
  float gyroX = g.gyro.x - gyroOffsetX;
  float gyroY = g.gyro.y - gyroOffsetY;
  float gyroZ = g.gyro.z - gyroOffsetZ;
  
  
  float linearAccelMagnitude = sqrt(pow(linearAccelX, 2) + pow(linearAccelY, 2) + pow(linearAccelZ, 2));

  //Send ACC to firebase
  firebase.setFloat("LACC", linearAccelMagnitude);
  
  Serial.print("LinearAccelMagnitude: ");
  Serial.print(linearAccelMagnitude);
  Serial.print(", ");
  

 if (linearAccelMagnitude > previousLinearAccelMagnitude + 0.01 ) {
    Serial.println("Acceleration");
    State="Acceleration";
  } else if (linearAccelMagnitude < previousLinearAccelMagnitude - 0.01 ) {
    Serial.println("Deceleration");
    State="Deceleration";
  }
  else {  Serial.println("Stable");
    State="Stable";
  }
 //Send state to firebase
  firebase.setString("State", State);

  // Save the current linear acceleration magnitude for the next iteration
  previousLinearAccelMagnitude = linearAccelMagnitude;
  Serial.println("\n");
  Serial.print("GyroMagnitude: ");
  Serial.print(sqrt(pow(gyroX, 2) + pow(gyroY, 2) + pow(gyroZ, 2)));  
  Serial.println("\n");
}
void loop() {

 MPU();
 send_time();
  
}

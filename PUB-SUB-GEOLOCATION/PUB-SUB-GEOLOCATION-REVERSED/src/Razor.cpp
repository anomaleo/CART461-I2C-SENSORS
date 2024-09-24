#include "Razor.h"
#include <math.h>

void IMU::read_sensors() {
  Read_Gyro(); // Read gyroscope
  Read_Accel(); // Read accelerometer
  Read_Magn(); // Read magnetometer
}

// Read every sensor and record a time stamp
// Init DCM with unfiltered orientation
// TODO re-init global vars?
void IMU::reset_sensor_fusion() {
  float temp1[3];
  float temp2[3];
  float xAxis[] = {1.0f, 0.0f, 0.0f};

  read_sensors();
  timestamp = millis();
  
  // GET PITCH
  // Using y-z-plane-component/x-component of gravity vector
  pitch = -atan2(accel[0], sqrt(accel[1] * accel[1] + accel[2] * accel[2]));
	
  // GET ROLL
  // Compensate pitch of gravity vector 
  Vector_Cross_Product(temp1, accel, xAxis);
  Vector_Cross_Product(temp2, xAxis, temp1);
  // Normally using x-z-plane-component/y-component of compensated gravity vector
  // roll = atan2(temp2[1], sqrt(temp2[0] * temp2[0] + temp2[2] * temp2[2]));
  // Since we compensated for pitch, x-z-plane-component equals z-component:
  roll = atan2(temp2[1], temp2[2]);
  
  // GET YAW
  Compass_Heading();
  yaw = MAG_Heading;
  
  // Init rotation matrix
  init_rotation_matrix(DCM_Matrix, yaw, pitch, roll);
}

// Apply calibration to raw sensor readings
void IMU::compensate_sensor_errors() {
    // Compensate accelerometer error
    accel[0] = (accel[0] - ACCEL_X_OFFSET) * ACCEL_X_SCALE;
    accel[1] = (accel[1] - ACCEL_Y_OFFSET) * ACCEL_Y_SCALE;
    accel[2] = (accel[2] - ACCEL_Z_OFFSET) * ACCEL_Z_SCALE;

    // Compensate magnetometer error
#if CALIBRATION__MAGN_USE_EXTENDED == true
    for (int i = 0; i < 3; i++)
      magnetom_tmp[i] = magnetom[i] - magn_ellipsoid_center[i];
    Matrix_Vector_Multiply(magn_ellipsoid_transform, magnetom_tmp, magnetom);
#else
    magnetom[0] = (magnetom[0] - MAGN_X_OFFSET) * MAGN_X_SCALE;
    magnetom[1] = (magnetom[1] - MAGN_Y_OFFSET) * MAGN_Y_SCALE;
    magnetom[2] = (magnetom[2] - MAGN_Z_OFFSET) * MAGN_Z_SCALE;
#endif

    // Compensate gyroscope error
    gyro[0] -= GYRO_AVERAGE_OFFSET_X;
    gyro[1] -= GYRO_AVERAGE_OFFSET_Y;
    gyro[2] -= GYRO_AVERAGE_OFFSET_Z;
}

// Reset calibration session if reset_calibration_session_flag is set
void IMU::check_reset_calibration_session()
{
  // Raw sensor values have to be read already, but no error compensation applied

  // Reset this calibration session?
  if (!reset_calibration_session_flag) return;
  
  // Reset acc and mag calibration variables
  for (int i = 0; i < 3; i++) {
    accel_min[i] = accel_max[i] = accel[i];
    magnetom_min[i] = magnetom_max[i] = magnetom[i];
  }

  // Reset gyro calibration variables
  gyro_num_samples = 0;  // Reset gyro calibration averaging
  gyro_average[0] = gyro_average[1] = gyro_average[2] = 0.0f;
  
  reset_calibration_session_flag = false;
}

void IMU::turn_output_stream_on()
{
  output_stream_on = true;
  digitalWrite(STATUS_LED_PIN, HIGH);
}

void IMU::turn_output_stream_off()
{
  output_stream_on = false;
  digitalWrite(STATUS_LED_PIN, LOW);
}

// Blocks until another byte is available on serial port
char IMU::readChar()
{
  while (Serial.available() < 1) { } // Block
  return Serial.read();
}

void IMU::readInput() { }

void IMU::loop() {
  if((millis() - timestamp) >= OUTPUT__DATA_INTERVAL)
  {
   timestamp_old = timestamp;
    timestamp = millis();
    if (timestamp > timestamp_old)
      G_Dt = (float) (timestamp - timestamp_old) / 1000.0f; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
    else G_Dt = 0;

    // Update sensor readings
    read_sensors();

    if (output_mode == OUTPUT__MODE_CALIBRATE_SENSORS)  // We're in calibration mode
    {
      check_reset_calibration_session();  // Check if this session needs a reset
      if (output_stream_on || output_single_on) output_calibration(curr_calibration_sensor);
    }
    else if (output_mode == OUTPUT__MODE_ANGLES)  // Output angles
    {
      // Apply sensor calibration
      compensate_sensor_errors();
    
      // Run DCM algorithm
      Compass_Heading(); // Calculate magnetic heading
      Matrix_update();
      Normalize();
      Drift_correction();
      Euler_angles();
      
      //if (output_stream_on || output_single_on) output_angles();
    }
    else  // Output sensor values
    {      
      //if (output_stream_on || output_single_on) output_sensors();
    }
    
    output_single_on = true;
    
#if DEBUG__PRINT_LOOP_TIME == true
    Serial.print("loop time (ms) = ");
    Serial.println(millis() - timestamp);
#endif
}
}


IMU::IMU()
        : gyro_num_samples(0)
        , yaw(0)
        , pitch(0)
        , roll(0)
        , MAG_Heading(0)
        , timestamp(0)
        , timestamp_old(0)
        , G_Dt(0)
        , output_stream_on(true)
        , output_single_on(true)
        , curr_calibration_sensor(0)
        , reset_calibration_session_flag(true)
        , num_accel_errors(0)
        , num_magn_errors(0)
        , num_gyro_errors(0) {

    accel[0] = accel_min[0] = accel_max[0] = magnetom[0] = magnetom_min[0] = magnetom_max[0] = gyro[0] = gyro_average[0] = 0;
    accel[1] = accel_min[1] = accel_max[1] = magnetom[1] = magnetom_min[1] = magnetom_max[1] = gyro[1] = gyro_average[1] = 0;
    accel[2] = accel_min[2] = accel_max[2] = magnetom[2] = magnetom_min[2] = magnetom_max[2] = gyro[2] = gyro_average[2] = 0;

    Accel_Vector[0] = Gyro_Vector[0] = Omega_Vector[0] = Omega_P[0] = Omega_I[0] = Omega[0] = errorRollPitch[0] = errorYaw[0] = 0;
    Accel_Vector[1] = Gyro_Vector[1] = Omega_Vector[1] = Omega_P[1] = Omega_I[1] = Omega[1] = errorRollPitch[1] = errorYaw[1] = 0;
    Accel_Vector[2] = Gyro_Vector[2] = Omega_Vector[2] = Omega_P[2] = Omega_I[2] = Omega[2] = errorRollPitch[2] = errorYaw[2] = 0;

    DCM_Matrix[0][0] = 1;
    DCM_Matrix[0][1] = 0;
    DCM_Matrix[0][2] = 0;
    DCM_Matrix[1][0] = 0;
    DCM_Matrix[1][1] = 1;
    DCM_Matrix[1][2] = 0;
    DCM_Matrix[2][0] = 0;
    DCM_Matrix[2][1] = 0;
    DCM_Matrix[2][2] = 1;

    Update_Matrix[0][0] = 0;
    Update_Matrix[0][1] = 1;
    Update_Matrix[0][2] = 2;
    Update_Matrix[1][0] = 3;
    Update_Matrix[1][1] = 4;
    Update_Matrix[1][2] = 5;
    Update_Matrix[2][0] = 6;
    Update_Matrix[2][1] = 7;
    Update_Matrix[2][2] = 8;

    Temporary_Matrix[0][0] = 0;
    Temporary_Matrix[0][1] = 0;
    Temporary_Matrix[0][2] = 0;
    Temporary_Matrix[1][0] = 0;
    Temporary_Matrix[1][1] = 0;
    Temporary_Matrix[1][2] = 0;
    Temporary_Matrix[2][0] = 0;
    Temporary_Matrix[2][1] = 0;
    Temporary_Matrix[2][2] = 0;

  // Init serial output
  //Serial.begin(OUTPUT__BAUD_RATE);
  
  // Init status LED
  //pinMode (STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  // Init sensors
  delay(50);  // Give sensors enough time to start
  I2C_Init();
  Accel_Init();
  Magn_Init();
  Gyro_Init();
  
  // Read sensors, init DCM algorithm
  delay(20);  // Give sensors enough time to collect data
  reset_sensor_fusion();

  // Init output
#if (OUTPUT__HAS_RN_BLUETOOTH == true) || (OUTPUT__STARTUP_STREAM_ON == false)
  turn_output_stream_off();
#else
  turn_output_stream_on();
#endif
}

// int main() {
//     IMU imu;
//     
//     Ticker looper;
//     looper.attach(&imu, &IMU::loop, (float)OUTPUT__DATA_INTERVAL / 1000.0f); // 50Hz update
//     
//     while (1)
//     {
//         imu.readInput();
//         wait_ms(5);
//     }
//     
//     return 0;
// }

// Computes the dot product of two vectors
float IMU::Vector_Dot_Product(const float v1[3], const float v2[3])
{
  float result = 0;
  
  for(int c = 0; c < 3; c++)
  {
    result += v1[c] * v2[c];
  }
  
  return result; 
}

// Computes the cross product of two vectors
// out has to different from v1 and v2 (no in-place)!
void IMU::Vector_Cross_Product(float out[3], const float v1[3], const float v2[3])
{
  out[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  out[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  out[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

// Multiply the vector by a scalar
void IMU::Vector_Scale(float out[3], const float v[3], float scale)
{
  for(int c = 0; c < 3; c++)
  {
    out[c] = v[c] * scale; 
  }
}

// Adds two vectors
void IMU::Vector_Add(float out[3], const float v1[3], const float v2[3])
{
  for(int c = 0; c < 3; c++)
  {
    out[c] = v1[c] + v2[c];
  }
}

// Multiply two 3x3 matrices: out = a * b
// out has to different from a and b (no in-place)!
void IMU::Matrix_Multiply(const float a[3][3], const float b[3][3], float out[3][3])
{
  for(int x = 0; x < 3; x++)  // rows
  {
    for(int y = 0; y < 3; y++)  // columns
    {
      out[x][y] = a[x][0] * b[0][y] + a[x][1] * b[1][y] + a[x][2] * b[2][y];
    }
  }
}

// Multiply 3x3 matrix with vector: out = a * b
// out has to different from b (no in-place)!
void IMU::Matrix_Vector_Multiply(const float a[3][3], const float b[3], float out[3])
{
  for(int x = 0; x < 3; x++)
  {
    out[x] = a[x][0] * b[0] + a[x][1] * b[1] + a[x][2] * b[2];
  }
}

// Init rotation matrix using euler angles
void IMU::init_rotation_matrix(float m[3][3], float yaw, float pitch, float roll)
{
  float c1 = cos(roll);
  float s1 = sin(roll);
  float c2 = cos(pitch);
  float s2 = sin(pitch);
  float c3 = cos(yaw);
  float s3 = sin(yaw);

  // Euler angles, right-handed, intrinsic, XYZ convention
  // (which means: rotate around body axes Z, Y', X'') 
  m[0][0] = c2 * c3;
  m[0][1] = c3 * s1 * s2 - c1 * s3;
  m[0][2] = s1 * s3 + c1 * c3 * s2;

  m[1][0] = c2 * s3;
  m[1][1] = c1 * c3 + s1 * s2 * s3;
  m[1][2] = c1 * s2 * s3 - c3 * s1;

  m[2][0] = -s2;
  m[2][1] = c2 * s1;
  m[2][2] = c1 * c2;
}

float IMU::constrain(float in, float min, float max)
{
    in = in > max ? max : in;
    in = in < min ? min : in;
    return in;
}

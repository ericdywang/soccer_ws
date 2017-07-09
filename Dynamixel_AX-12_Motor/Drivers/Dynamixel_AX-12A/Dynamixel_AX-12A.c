/***** Includes *****/
#include "Dynamixel_AX-12A.h"
#include "stm32f4xx_hal_conf.h"

/********** Functions **********/
/********** Setters **********/
void Dynamixel_SetPosition(Dynamixel_HandleTypeDef *hdynamixel, double angle){
	/* Takes a double between 0 and 300, encodes this position in an
	 * upper and low hex byte pair (with a maximum of 1023 as defined in the AX-12
	 * user manual), and sends this information (along with requisites) over UART.
	 * Low byte is 0x1E in motor RAM, high byte is 0x1F in motor RAM.
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  angle, the desired angular position. Arguemnts between 0 and 300 are valid
	 *
	 * Returns: none
	 */

	// Define array for transmission
	uint8_t arrTransmit[9];

	// Do assignments and computations
	arrTransmit[0] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[1] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[2] = hdynamixel -> _ID; // Motor ID
	arrTransmit[3] = 0x05; // Length of message minus the obligatory bytes
	arrTransmit[4] = 0x03; // WRITE instruction
	arrTransmit[5] = 0x1e; // Address of goal position

	// Translate the angle from degrees into a 10-bit number
	int normalized_value = (int)(angle / 300 * 1023); // maximum angle of 1023
	arrTransmit[6] = normalized_value & 0xFF; // Low byte of goal position
	arrTransmit[7] = (normalized_value >> 8) & 0xFF; // High byte of goal position

	// Checksum = 255 - (sum % 256)
	arrTransmit[8] = Dynamixel_ComputeChecksum(arrTransmit, 9);

	// Set data direction
	__DYNAMIXEL_TRANSMIT();

	// Transmit
	HAL_UART_Transmit(hdynamixel -> _UART_Handle, arrTransmit, 9, 100);
}

void Dynamixel_SetVelocity(Dynamixel_HandleTypeDef *hdynamixel, double velocity){
	/* Sets the goal velocity of the motor in RAM.
	 * Low byte is 0x20 in motor RAM, high byte is 0x21 in motor RAM.
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  velocity, the goal velocity in RPM. Arguments of 0-114 are valid. If 0, max is used.
	 *
	 * Returns: none
	 */

	// Define array for transmission
	uint8_t arrTransmit[9];

	// Do assignments and computations
	arrTransmit[0] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[1] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[2] = hdynamixel -> _ID; // Motor ID
	arrTransmit[3] = 0x05; // Length of message minus the obligatory bytes
	arrTransmit[4] = 0x03; // WRITE instruction
	arrTransmit[5] = 0x20; // Address of goal velocity

	// Translate the position from rpm into a 10-bit number
	int normalized_value = 0;
	if(hdynamixel -> _isJointMode){
		normalized_value = (int)(velocity / 114 * 1023); // maximum angle of 1023
	}
	else{
		normalized_value = (int)(velocity / 114 * 2047); // maximum angle of 1023
	}

	arrTransmit[6] = normalized_value & 0xFF; // Low byte of goal position
	arrTransmit[7] = (normalized_value >> 8) & 0xFF; // High byte of goal position

	// Checksum = 255 - (sum % 256)
	arrTransmit[8] = Dynamixel_ComputeChecksum(arrTransmit, 9);

	// Set data direction
	__DYNAMIXEL_TRANSMIT();

	// Transmit
	HAL_UART_Transmit(hdynamixel -> _UART_Handle, arrTransmit, 9, 100);
}

void Dynamixel_SetBaudRate(Dynamixel_HandleTypeDef *hdynamixel, double baud){
	/* Sets the baud rate of a particular motor. Register address is 0x04 in motor EEPROM.
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  baud, the baud rate. Arguments in range [7844, 1000000] are valid
	 */

	// Define array for transmission
	uint8_t arrTransmit[8];

	// Do assignments and computations
	arrTransmit[0] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[1] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[2] = hdynamixel -> _ID; // Motor ID
	arrTransmit[3] = 0x04; // Length of message minus the obligatory bytes
	arrTransmit[4] = 0x03; // WRITE instruction
	arrTransmit[5] = 0x04; // Baud rate register address

	// Set _baud equal to the hex code corresponding to baud. Default to 1 Mbps
	if(baud > 0){
		// Valid for baud in range [7844, 1000000]. Will be converted to 8-bit resolution
		hdynamixel -> _BaudRate = (uint8_t)((2000000 / baud) - 1);
	}
	else{
		// Default to 1 Mbps
		hdynamixel -> _BaudRate = 0x01;
	}
	arrTransmit[6] = hdynamixel -> _BaudRate; // The hex code corresponding to the desired baud rate

	// Checksum = 255 - (sum % 256)
	arrTransmit[7] = Dynamixel_ComputeChecksum(arrTransmit, 8);

	// Set data direction
	__DYNAMIXEL_TRANSMIT();

	// Transmit
	HAL_UART_Transmit(hdynamixel -> _UART_Handle, arrTransmit, 8, 100);
}

void Dynamixel_SetID(Dynamixel_HandleTypeDef *hdynamixel, int ID){
	/* Sets motor ID. Register address is 0x03 in motor EEPROM
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  ID, the unique number between 0 and 252 to identify the motor. If 0xFE (254), then
	 * 			  	  any messages broadcasted to that ID will be broadcasted to all motors
	 *
	 * Returns: none
	 */

	// Define array for transmission
	uint8_t arrTransmit[8]; // Obligatory bytes for setting baud rate

	// Do assignments and computations
	arrTransmit[0] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[1] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[2] = hdynamixel -> _ID; // Motor ID
	arrTransmit[3] = 0x04;
	arrTransmit[4] = 0x03;
	arrTransmit[5] = 0x03; // ID register address
	arrTransmit[6] = ID; // ID to be assigned

	// Checksum = 255 - (sum % 256)
	arrTransmit[7] = Dynamixel_ComputeChecksum(arrTransmit, 8);

	// Set data direction
	__DYNAMIXEL_TRANSMIT();

	// Transmit
	HAL_UART_Transmit(hdynamixel -> _UART_Handle, arrTransmit, 8, 100);
}

// UNIMPLEMENTED
void Dynamixel_SetMaxTorque(Dynamixel_HandleTypeDef *hdynamixel, double maxTorque){
	/* Sets the maximum torque limit for all motor operations.
	 * Low byte is addr 0x0E in motor RAM, high byte is addr 0x0F in motor RAM.
	 *
	 * Arguments: hdynamixel, the motor handle
	 *			  maxTorque, the maximum torque as a percentage (max: 100). Gets converted to 10-bit number
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_SetGoalTorque(Dynamixel_HandleTypeDef *hdynamixel, double goalTorque){
	/* Sets the goal torque for the current motor. Initial value is taken from 0x0E and 0x0F (max torque in EEPROM)
	 * Low byte is 0x22 in motor RAM, high byte is 0x23 in motor RAM.
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  goalTorque, the percentage of the maximum possible torque (max: 100). Gets converted into 10-bit number
	 *
	 * Returns: none
	 */
}

void Dynamixel_SetCWAngleLimit(Dynamixel_HandleTypeDef *hdynamixel, double minAngle){
	/* Sets the clockwise angle limit for the current motor.
	 * Register 0x06 in EEPROM for low byte, 0x07 in EEPROM for high byte
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  minAngle, the minimum angle for all motor operations. Arguments between 0 and 300 are valid
	 *
	 * If maxAngle for CCW angle limit is 0 AND minAngle for CW angle limit is 0,
	 * then motor is in wheel mode where it can continuously rotate. Otherwise, motor is in joint mode where its
	 * motion is constrained between the set bounds.
	 *
	 * Returns: none
	 */

	// Define array for transmission
	uint8_t arrTransmit[9];

	// Do assignments and computations
	arrTransmit[0] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[1] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[2] = hdynamixel -> _ID; // Motor ID
	arrTransmit[3] = 0x05; // Length of message minus the obligatory bytes
	arrTransmit[4] = 0x03; // WRITE instruction
	arrTransmit[5] = 0x06; // Address of min angle

	// Translate the angles from degrees into 8-bit numbers
	int normalized_value = (int)(minAngle / 300 * 1023);

	arrTransmit[6] = normalized_value & 0xFF; // Low byte of CW angle limit
	arrTransmit[7] = (normalized_value >> 8) & 0xFF; // High byte of CW angle limit

	// Checksum = 255 - (sum % 256)
	arrTransmit[8] = Dynamixel_ComputeChecksum(arrTransmit, 9);

	// Set data direction
	__DYNAMIXEL_TRANSMIT();

	// Transmit
	HAL_UART_Transmit(hdynamixel -> _UART_Handle, arrTransmit, 9, 100);
}

void Dynamixel_SetCCWAngleLimit(Dynamixel_HandleTypeDef *hdynamixel, double maxAngle){
	/* Sets the counter-clockwise angle limit for the current motor.
	 * Register 0x08 in EEPROM for low byte, 0x09 in EEPROM for high byte
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  maxAngle, the maximum angle for all motor operations. Arguments between 0 and 300 are valid.
	 *
	 * If maxAngle for CCW angle limit is 0 AND minAngle for CW angle limit is 0,
	 * then motor is in wheel mode where it can continuously rotate. Otherwise, motor is in joint mode where its
	 * motion is constrained between the set bounds.
	 *
	 * Returns: none
	 */

	// Define array for transmission
	uint8_t arrTransmit[9];

	// Do assignments and computations
	arrTransmit[0] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[1] = 0xff; // Obligatory bytes for starting communication
	arrTransmit[2] = hdynamixel -> _ID; // Motor ID
	arrTransmit[3] = 0x05; // Length of message minus the obligatory bytes
	arrTransmit[4] = 0x03; // WRITE instruction
	arrTransmit[5] = 0x08; // Address of min angle

	// Translate the angles from degrees into 8-bit numbers
	int normalized_value = (int)(maxAngle / 300 * 1023);

	arrTransmit[6] = normalized_value & 0xFF; // Low byte of CCW angle limit
	arrTransmit[7] = (normalized_value >> 8) & 0xFF; // High byte of CCW angle limit

	// Checksum = 255 - (sum % 256)
	arrTransmit[8] = Dynamixel_ComputeChecksum(arrTransmit, 9);

	// Set data direction
	__DYNAMIXEL_TRANSMIT();

	// Transmit
	HAL_UART_Transmit(hdynamixel -> _UART_Handle, arrTransmit, 9, 100);
}

// UNIMPLEMENTED
void Dynamixel_SetCWComplianceMargin(Dynamixel_HandleTypeDef *hdynamixel, uint8_t CWcomplianceMargin){
	/* Sets the clockwise compliance margin for the current motor.
	 * The compliance margin is the acceptable error between the current position and goal position.
	 * The greater the value, the more error is acceptable.
	 * Register 0x1A in RAM. Default value: 0x01 (smallest possible)
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  CWcomplianceMargin, the acceptable error between the current and goal position. Arguments in range [0, 255]
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_SetCCWComplianceMargin(Dynamixel_HandleTypeDef *hdynamixel, uint8_t CCWcomplianceMargin){
	/* Sets the counter-clockwise compliance margin for the current motor.
	 * The compliance margin is the acceptable error between the current position and goal position.
	 * The greater the value, the more error is acceptable.
	 * Register 0x1B in RAM. Default value: 0x01 (smallest possible)
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  CCWcomplianceMargin, the acceptable error between the current and goal position. Arguments in range [0, 255]
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_SetCWComplianceSlope(Dynamixel_HandleTypeDef *hdynamixel, uint8_t CWcomplianceSlope){
	/* Sets the clockwise compliance slope for the current motor.
	 * The compliance slope sets the level of torque near the goal position.
	 * The higher the value, the more flexibility that is obtained. That is, a high value
	 * means that as the goal position is approached, the torque will be significantly reduced. If a low
	 * value is used, then as the goal position is approached, the torque will not be reduced all that much.
	 * Address is 0x1C in RAM. Default value is 0x20 (tightest possible)
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  CWcomplianceSlope, parameter dictating the torque as the goal position is approached. Arguments in range [0,254]
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_SetCCWComplianceSlope(Dynamixel_HandleTypeDef *hdynamixel, uint8_t CCWcomplianceSlope){
	/* Sets the counter-clockwise compliance slope for the current motor.
	 * The compliance slope sets the level of torque near the goal position.
	 * The higher the value, the more flexibility that is obtained. That is, a high value
	 * means that as the goal position is approached, the torque will be significantly reduced. If a low
	 * value is used, then as the goal position is approached, the torque will not be reduced all that much.
	 * Address is 0x1D in RAM. Default value is 0x20 (tightest possible)
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  CWcomplianceSlope, parameter dictating the torque as the goal position is approached. Arguments in range [0,254]
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_TorqueEnable(Dynamixel_HandleTypeDef *hdynamixel, uint8_t isEnabled){
	/* Sets address 0x18 in the RAM of the current motor. Default value is 0x00
	 *
	 * Arguments: hdynamixel, the motor handle
	 * 			  isEnabled, if 1, then generates torque by impressing power to the motor
	 * 			  			 if 0, then interrupts power to the motor to prevent it from generating torque
	 *
	 * Returns: none
	 */
}


/********** GETTERS **********/
// UNIMPLEMENTED
void Dynamixel_GetPosition(Dynamixel_HandleTypeDef *hdynamixel){
	/* Reads addresses 0x24 and 0x25 in the motors RAM to see what the current position
	 * of the motor is.
	 * Writes the results to hdynamixel -> _lastPosition
	 *
	 * Arguments: hdynamixel, the motor handle
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_GetVelocity(Dynamixel_HandleTypeDef *hdynamixel){
	/* Reads addresses 0x26 and 0x27 in the motor RAM to see what the current velocity
	 * of the motor is.
	 * Writes the results to hdynamixel -> _lastVelocity
	 *
	 * Arguments: hdynamixel, the motor handle
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_GetTemperature(Dynamixel_HandleTypeDef *hdynamixel){
	/* Reads address 0x2B in the motor RAM to see what the current temperature is inside the motor.
	 * Writes the results to hdynamixel -> _lastTemperature
	 *
	 * Arguments: hdynamixel, the motor handle
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_GetLoad(Dynamixel_HandleTypeDef *hdynamixel){
	/* Reads addresses 0x28 and 0x29 in the motor RAM to see what the current
	 * load is.
	 * Writes the results to hdynamixel -> _lastLoad
	 *
	 * Arguments: hdynamixel, the motor handle
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
void Dynamixel_GetVoltage(Dynamixel_HandleTypeDef *hdynamixel){
	/* Reads address 0x2A in the motor RAM to see what the current voltage is.
	 * Writes the results to hdynamixel -> _lastVoltage
	 *
	 * Arguments: hdynamixel, the motor handle
	 *
	 * Returns: none
	 */
}

// UNIMPLEMENTED
uint8_t Dynamixel_IsMoving(Dynamixel_HandleTypeDef *hdynamixel){
	/* Reads the 0x2E address in motor RAM to see if motor is moving.
	 *
	 * Arguments: hdynamixel, the motor handle
	 *
	 * Returns: 0 if not moving
	 * 			1 if moving
	 */

	return 0;
}

// UNIMPLEMENTED
uint8_t Dynamixel_IsJointMode(Dynamixel_HandleTypeDef *hdynamixel){
	/* Reads the CW (addr: 0x06) and CCW (addr: 0x07) angle limits. If both are 0, motor is in wheel mode
	 * and can spin indefinitely. Otherwise, motor is in joint mode and has angle limits set
	 *
	 * Arguments: hdynamixel, the motor handle
	 *
	 * Returns: 0 if in wheel mode
	 * 			1 if in joint mode
	 */

	return 1;
}

/********** Computation **********/
uint8_t Dynamixel_ComputeChecksum(uint8_t *arr, int length){
	/* Compute the checksum for data to be transmitted */

	// Local variable declaration
	uint8_t accumulate = 0;

	// Loop through the array starting from the 2nd element of the array and finishing before the last
	// since the last is where the checksum will be stored
	for(uint8_t i = 2; i < length - 1; i++){
		accumulate += arr[i];
	}

	return 255 - (accumulate % 256); // Lower 8 bits of the logical NOT of the sum
}

/********** Initialization *********/
void Dynamixel_Init(Dynamixel_HandleTypeDef *hdynamixel, uint8_t ID,\
						   double BaudRate, UART_HandleTypeDef *UART_Handle)
{
	hdynamixel -> _ID = ID; // Motor ID (unique or global)
	hdynamixel -> _lastPosition = 0; // In future, initialize this accurately
	hdynamixel -> _lastVelocity = 0; // In future, initialize this accurately
	hdynamixel -> _lastTemperature = 0; // In future, initialize this accurately
	hdynamixel -> _lastVoltage = 0; // In future, initialize this accurately
	hdynamixel -> _lastLoad = 0; // In future, initialize this accurately
	hdynamixel -> _isJointMode = 1; // In future, initialize this accurately
	hdynamixel -> _UART_Handle = UART_Handle; // For UART TX and RX
}

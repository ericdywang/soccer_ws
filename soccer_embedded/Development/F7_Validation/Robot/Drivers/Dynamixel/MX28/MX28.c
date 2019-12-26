/**
  ******************************************************************************
  * @file    MX28.c
  * @author  Tyler
  * @brief   This file implements MX28-specific functions.
  ******************************************************************************
  */




/********************************* Includes ************************************/
#include "../h/MX28.h"




/********************************* Externs *************************************/
extern void Dynamixel_SetCWAngleLimit(Dynamixel_HandleTypeDef* hdynamixel, float minAngle); // (EEPROM)
extern void Dynamixel_SetCCWAngleLimit(Dynamixel_HandleTypeDef* hdynamixel, float maxAngle); // (EEPROM)




/********************************** Macros ***********************************/
// Register addresses
#define MX28_REG_MULTI_TURN_OFFSET              0x14    /**< Register to fine-tune "0" position */
#define MX28_REG_RESOLUTION_DIVIDER             0x16    /**< Register to change how many bits of resolution are used */
#define MX28_REG_D_GAIN                         0x1A    /**< Derivative gain register */
#define MX28_REG_I_GAIN                         0x1B    /**< Integral gain register */
#define MX28_REG_P_GAIN                         0x1C    /**< Proportional gain register */
#define MX28_REG_GOAL_ACCELERATION              0x49    /**< Goal acceleration register */




/********************************* Constants *********************************/
// Value limit definitions
const uint8_t MX28_MAX_VELOCITY = 117; /**< Maximum angular velocity (RPM) */

// Default register value definitions
const uint8_t MX28_DEFAULT_BAUD_RATE              =   0x22;    /**< Default baud rate register setting */
const uint16_t MX28_DEFAULT_CCW_ANGLE_LIMIT       =   0x0FFF;  /**< Default counter-clockwise angle limit */
const uint8_t MX28_DEFAULT_HIGHEST_VOLTAGE_LIMIT  =   0xA0;    /**< Default permitted maximum voltage (0xA0 = 160 -> 16.0 V) */
const uint8_t MX28_DEFAULT_D_GAIN                 =   0x08;    /**< Default derivative gain parameter value */
const uint8_t MX28_DEFAULT_I_GAIN                 =   0x00;    /**< Default integral gain parameter value */
const uint8_t MX28_DEFAULT_P_GAIN                 =   0x08;    /**< Default proportional gain parameter value */
const uint16_t MX28_DEFAULT_PUNCH                 =   0x0000;  /**< Default punch */




/******************************** Functions ************************************/
/**
 * @defgroup MX28 MX28
 * @brief    Globally-accessible functions for interfacing with MX28 actuators.
 *           These functions are specific to MX28s and have no analogue for
 *           other actuators supported by this library. All of these functions
 *           will return and have no effect if the motor structure type field
 *           is not MX28TYPE.
 * @ingroup  Dynamixel
 */




/*****************************************************************************/
/*  Setter functions                                                         */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/**
 * @defgroup MX28_Setters Setters
 * @brief    Register-setting functions
 *
 * # Setter functions #
 *
 * This subsection provides a set of functions which provide interfaces for
 * setting motor register values.
 *
 * @ingroup MX28
 * @{
 */

/**
 * @brief   For an actuator in multi-turn mode, this applies a tunable offset
 *          to all positions. That is it allows you to change where the
 *          actuator considers position 0 to be. For an actuator not in
 *          multi-turn mode, this setting has no effect
 * @param 	hdynamixel pointer to a Dynamixel_HandleTypeDef structure that
 *          contains the configuration information for the motor
 * @param   offset the signed offset argument indicating the offset value to
 *          program
 * @return  None
 */
void MX28_SetMultiTurnOffset(Dynamixel_HandleTypeDef* hdynamixel, int16_t offset){
    if(offset > 28672){
        offset = 28672;
    }
    else if(offset < -28672){
        offset = -28672;
    }

    uint8_t args[3] = {MX28_REG_MULTI_TURN_OFFSET, offset & 0xFF, (offset >> 16) & 0xFF};
    Dynamixel_DataWriter(hdynamixel, args, sizeof(args));
}

// TODO: implement. Challenge with this is it means we need to track resolution
// divider in the handle so that we can compute position properly in the
// position functions
/**
 * @brief   Unimplemented. This function should change the resolution divider,
 *          allowing for more precise or less precise control settings
 *          depending on the value
 * @param 	hdynamixel pointer to a Dynamixel_HandleTypeDef structure that
 *          contains the configuration information for the motor
 * @param   divider the resolution divider setting
 * @return  None
 */
void MX28_SetResolutionDivider(Dynamixel_HandleTypeDef* hdynamixel, uint8_t divider){

}

/**
 * @brief   Sets the value of the derivative gain used in the motor's PID
 *          controller
 * @details kD = DGain / 250
 * @param 	hdynamixel pointer to a Dynamixel_HandleTypeDef structure that
 *          contains the configuration information for the motor
 * @param   DGain the derivative gain parameter
 * @return  None
 */
void MX28_SetDGain(Dynamixel_HandleTypeDef* hdynamixel, uint8_t DGain){
    if(hdynamixel -> _motorType == MX28TYPE){
        uint8_t args[2] = {MX28_REG_D_GAIN, DGain};
        Dynamixel_DataWriter(hdynamixel, args, sizeof(args));
    }
}

/**
 * @brief   Sets the value of the integral gain used in the motor's PID
 *          controller
 * @details kI = IGain * 125/256
 * @param 	hdynamixel pointer to a Dynamixel_HandleTypeDef structure that
 *          contains the configuration information for the motor
 * @param   IGain the integral gain parameter
 * @return  None
 */
void MX28_SetIGain(Dynamixel_HandleTypeDef* hdynamixel, uint8_t IGain){
    if(hdynamixel -> _motorType == MX28TYPE){
        uint8_t args[2] = {MX28_REG_I_GAIN, IGain};
        Dynamixel_DataWriter(hdynamixel, args, sizeof(args));
    }
}

/**
 * @brief   Sets the value of the proportional gain used in the motor's PID
 *          controller
 * @details kP = PGain / 8
 * @param 	hdynamixel pointer to a Dynamixel_HandleTypeDef structure that
 *          contains the configuration information for the motor
 * @param   PGain the proportional gain parameter
 * @return  None
 */
void MX28_SetPGain(Dynamixel_HandleTypeDef* hdynamixel, uint8_t PGain){
    if(hdynamixel -> _motorType == MX28TYPE){
        uint8_t args[2] = {MX28_REG_P_GAIN, PGain};
        Dynamixel_DataWriter(hdynamixel, args, sizeof(args));
    }
}

/**
 * @brief   Sets the goal acceleration. The argument should be in units of
 *          degree/s^2
 * @details Special: goalAcceleration of 0 means no control over accel (uses
 *          max acceleration of motor)
 * @param 	hdynamixel pointer to a Dynamixel_HandleTypeDef structure that
 *          contains the configuration information for the motor
 * @param   goalAcceleration the target acceleration in degree/s^2
 * @return  None
 */
void MX28_SetGoalAcceleration(Dynamixel_HandleTypeDef* hdynamixel, float goalAcceleration){
    if(hdynamixel -> _motorType == MX28TYPE){
        if(goalAcceleration > 2180){
            goalAcceleration = 2180;
        }
        if(goalAcceleration < 0){
            goalAcceleration = 0;
        }

        uint8_t accelArg = (uint8_t)(goalAcceleration / 8.583);

        uint8_t args[2] = {MX28_REG_GOAL_ACCELERATION, accelArg};
        Dynamixel_DataWriter(hdynamixel, args, sizeof(args));
    }
}

/**
 * @}
 */
/* end MX28_Setters */


/*****************************************************************************/
/*  Interfaces for previously-defined functions                              */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/**
 * @defgroup MX28_Interfaces Interfaces for previously-defined functions
 * @brief    Interfaces for previously-defined functions
 *
 * # Interfaces for previously-defined functions #
 *
 * This subsection provides a set of functions which implement functions
 * which call previously-defined functions in order to accomplish specific
 * tasks.
 *
 * @ingroup MX28
 * @{
 */

/**
 * @brief   Activates multi-turn mode, which allows the actuator to have a
 *          range of controllable position values from -28672 to 28672
 * @param 	hdynamixel pointer to a Dynamixel_HandleTypeDef structure that
 *          contains the configuration information for the motor
 * @return  None
 */
void MX28_EnterMultiTurnMode(Dynamixel_HandleTypeDef* hdynamixel){
    if(hdynamixel -> _motorType == MX28TYPE){
        Dynamixel_SetCWAngleLimit(hdynamixel, 4095);
        Dynamixel_SetCCWAngleLimit(hdynamixel, 4095);
    }
}

/**
 * @}
 */
/* end MX28_Interfaces */

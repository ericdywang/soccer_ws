/**
  *****************************************************************************
  * @file    Dynamixel_test.cpp
  * @author  Tyler Gamvrelis
  *
  * @defgroup Test
  * @ingroup  Dynamixel
  * @brief    Dynamixel test driver
  * @{
  *****************************************************************************
  */




/********************************* Includes **********************************/
#include "Dynamixel.h"

#include "MockUartInterface.h"
#include "MockOsInterface.h"
#include "MockGpioInterface.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::Return;
using ::testing::_;

using uart::UartDriver;
using mocks::MockOsInterface;
using mocks::MockUartInterface;
using mocks::MockGpioInterface;
using dynamixel::Motor;
using dynamixel::DaisyChainParams;
using dynamixel::DaisyChain;
using dynamixel::ResolutionDivider;




/******************************** File-local *********************************/
namespace{
// Variables
// ----------------------------------------------------------------------------
MockUartInterface uart;
MockOsInterface os;
MockGpioInterface gpio;
UART_HandleTypeDef UARTx = {0};
UartDriver UARTxDriver(&os, &uart, &UARTx);

GPIO_TypeDef dataDirPort;

DaisyChainParams p;



// Classes & structs
// ----------------------------------------------------------------------------
class DynamixelTest : public ::testing::Test {
protected:
    void SetUp() override {
        p.uartDriver = &UARTxDriver;
        p.gpioif = &gpio;
        p.dataDirPort = &dataDirPort;
        p.dataDirPinNum = 1;
    }
};




// Functions
// ----------------------------------------------------------------------------
TEST_F(DynamixelTest, CanBeCreated){
    DaisyChain chain(p);
    Motor m1(1, &chain, ResolutionDivider::AX12A);
    Motor m2(1, &chain, ResolutionDivider::MX28);
}

TEST_F(DynamixelTest, CanSetGoalPosition){
    DaisyChain chain(p);
    Motor m(1, &chain, ResolutionDivider::AX12A);
    m.setGoalPosition(150.0);
}

TEST_F(DynamixelTest, CanGetGoalPosition){
    DaisyChain chain(p);
    Motor m(1, &chain, ResolutionDivider::AX12A);

    float angle = 0;
    ASSERT_TRUE(m.getPosition(angle));
}



} // end anonymous namespace




/**
 * @}
 */
/* end - module name */

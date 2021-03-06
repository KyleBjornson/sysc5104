/*******************************************************************
*
*  DESCRIPTION: Atomic Model Sender
*
*  AUTHORS: Ben Earle, Kyle Bjornson
*
*  EMAIL: ben.earle@cmail.carleton.ca, kyle.bjornson@cmail.carleton.ca
*
*  DATE: November 2nd, 2018
*
*******************************************************************/

/** include files **/
#include "steerDriver.h"
#include "message.h"    // class ExternalMessage, InternalMessage
#include "mainsimu.h"   // MainSimulator::Instance().getParameter( ... )

/** public functions **/

/*******************************************************************
* Function Name: SteerDriver
********************************************************************/
SteerDriver::SteerDriver( const string &name )
: Atomic( name )
, wheelDirectionIn( addInputPort( "wheelDirectionIn" ) )
, speedIn( addInputPort( "speedIn" ) )
, servoDutyCycle( addOutputPort( "servoDutyCycle" ) )
{
	speed = 0;
	wheelDirection = 0;
	desiredWheelDirection = 0;
}

/*******************************************************************
* Function Name: initFunction
********************************************************************/
Model &SteerDriver::initFunction() {
	this-> passivate();
	return *this ;
}

/*******************************************************************
* Function Name: externalFunction
********************************************************************/
Model &SteerDriver::externalFunction( const ExternalMessage &msg ) {
	if( msg.port() == wheelDirectionIn) {
		int x = int(msg.value()); /*Yikes, I sure hope I can just do this... Sorry :P*/
		if(x < -25) x = -25;
		if(x > 25) x = 25;
		desiredWheelDirection = x;
		if (this->state() == passive) {
			holdIn(active, Time( static_cast<float>((speed * SPEED_TIMEOUT_MULTIPLIER))));
		} else {
			holdIn(active, (msg.time() - lastChange()));
		}
	} else if (msg.port() == speedIn) {
		/*Speed in is in km/hr, this block will store it in m/s to simplify calculations.*/	
		float x = float(msg.value())* MPS_FROM_KMPH; 
		if (this->state() == passive) {
			this->passivate();
		} else {
			holdIn(active, (msg.time() - lastChange()));
		}
		speed = x;
	}
	return *this;
}

/*******************************************************************
* Function Name: internalFunction
********************************************************************/
Model &SteerDriver::internalFunction( const InternalMessage & ){
	/*We just output vout, select timeout related to the current output*/
	if(wheelDirection == desiredWheelDirection) {
		passivate();
	} else {
		if(wheelDirection < desiredWheelDirection) {
			wheelDirection++;		
		} else {
			wheelDirection--;		
		}
		holdIn(active, Time( static_cast<float>(speed * SPEED_TIMEOUT_MULTIPLIER)));
	}
	return *this ;
}

/*******************************************************************
* Function Name: outputFunction
********************************************************************/
Model &SteerDriver::outputFunction( const InternalMessage &msg ){
	sendOutput( msg.time(), servoDutyCycle, (7.5 + (0.1*wheelDirection)));
	return *this ;
}

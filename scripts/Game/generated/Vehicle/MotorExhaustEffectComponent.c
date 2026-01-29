/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class MotorExhaustEffectComponentClass: BaseEffectComponentClass
{
}

class MotorExhaustEffectComponent: BaseEffectComponent
{
	proto external IEntity GetOwner();
	//! Remove every particle associed with this effect
	proto external void TurnOff();
	/*!
	Active particle associed with this effect
	\param owner Owner of this component
	*/
	proto external void TurnOn(IEntity owner);
	proto external EntitySlotInfo GetEffectPosition();
	/*!
	Get current RPM
	\return RPM value in <0, maxRpm>
	*/
	proto external float GetSignalRpm();
	proto external float GetSignalThrust();
	/*!
	Get current scaled RPM
	\return RPM value in <0, 1>
	*/
	proto external float GetRpmScaled();

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	event void OnDelete(IEntity owner);
}

/*!
\}
*/

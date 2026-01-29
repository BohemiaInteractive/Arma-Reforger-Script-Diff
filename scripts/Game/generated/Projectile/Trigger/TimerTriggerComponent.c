/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile_Trigger
\{
*/

class TimerTriggerComponentClass: BaseTriggerComponentClass
{
}

class TimerTriggerComponent: BaseTriggerComponent
{
	/*!
	* Gets the local time left on the timer trigger.
	* \warning The timer variable is not replicated.
	* \return Current time duration in seconds.
	*/
	proto external float GetTimer();
	/*!
	* Sets the local time left on the timer trigger.
	* \warning The timer variable is not replicated.
	* \param newTime The new timer duration in seconds.
	*/
	proto external void SetTimer(float newTime);
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

enum WeatherTransitionRequestResponse
{
	/*!
	Transition request successful. Transition will start immediately.
	*/
	SUCCESS,
	/*!
	Unknown error
	*/
	E_UNKNOWN,
	/*!
	Not authorized to make a transition
	*/
	E_UNAUTHORIZED,
	/*!
	Already transitioning, must wait until current transition is completed. (This doesn't count if immediate transition is requested)
	*/
	E_ALREADY_TRANSITIONING,
	/*!
	Transition state or variant not found within transition list.
	*/
	E_NOT_FOUND,
}

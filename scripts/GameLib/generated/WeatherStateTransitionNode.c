/*
===========================================
Do not modify, this script is generated
===========================================
*/

class WeatherStateTransitionNode
{
	private void WeatherStateTransitionNode();

	proto external void SetLooping(bool looping);
	proto external bool IsLooping();
	/*!
	Sets transition duration in in-game hours.
	*/
	proto external void SetTransitionDurationHours(float transDurationHours);
	/*!
	Sets destination state duration in in-game hours.
	*/
	proto external void SetStateDurationHours(float stateDurationHours);
	/*!
	Gets destination state index.
	*/
	proto external int GetDestinationStateIndex();
	/*!
	Gets transition duration in in-game hours.
	*/
	proto external float GetTransitionDurationHours();
	/*!
	Gets destination state duration in in-game hours.
	*/
	proto external float GetStateDurationHours();
	/*!
	Checks whether this state transition node points to a valid destination state.
	Invalid state transition nodes cannot be enqueued or triggered to transition.

	\return True if valid, False if state transition is pointing to an invalid (not found) destination state.
	*/
	proto external bool IsValid();
	/*!
	Checks whether a transition can be done between this node and the one.
	\param destinationState Destination state after the current one.
	\return true if compatibel, false otherwise.
	*/
	proto external bool IsValidTransitionTo(WeatherStateTransitionNode destinationState);
	/*!
	Checks whether this node is transitioning.
	\return True if transitioning, false if not transitioned yet or if transitioned completely
	*/
	proto external bool IsTransitioning();
}

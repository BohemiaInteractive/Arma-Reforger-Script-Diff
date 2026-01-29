/*
===========================================
Do not modify, this script is generated
===========================================
*/

class BaseWeatherStateTransitionManager
{
	private void BaseWeatherStateTransitionManager();

	/*!
	Check if we are previewing a transition.
	*/
	proto external bool IsPreviewingState();
	proto external bool IsPreviewingDateTime();
	proto external bool IsPreviewingWind();
	/*!
	Previews state locally (without network syncing).
	It is not required that the state is part of the state transition queue.
	I.e. you can preview nodes without inserting them.

	\param preview Enables or disables the preview
	\param stateName The state to preview
	*/
	proto external bool SetStatePreview(bool preview, string stateName = "");
	/*!
	Previews date and time of the day locally (without network syncing).

	\param preview Enables or disables the preview
	*/
	proto external bool SetDateTimePreview(bool preview, int year = -1, int month = -1, int day = -1, float timeOfTheDay01 = -1);
	proto external bool SetWindPreview(bool preview, float windSpeed = -1, float windAngleDegrees = -1);
	/*!
	Requests an immediate transition.
	\param node The transition node we want to transition to, it must be a valid node within the state transition queue
	\return SUCCESS if correct, otherwise see WeatherTransitionRequestResponse for possible responses.
	*/
	proto external WeatherTransitionRequestResponse RequestStateTransitionImmediately(WeatherStateTransitionNode node);
	/*!
	Requests next state transition from the state transition node queue.
	This will start transitioning smoothly according to transition duration.
	Cannot be called if a transition is already on course.
	\return SUCCESS if correct, otherwise see WeatherTransitionRequestResponse for possible responses.
	*/
	proto external WeatherTransitionRequestResponse RequestStateTransition();
	proto external ref WeatherStateTransitionNode CreateStateTransition(string stateName, float transitionDurationHours, float stateDurationHours);
	/*!
	Adds a new state transition to the state transition queue.
	Keep in mind that state transition node cannot be inserted to the first node if a transition is already on course.
	Can only be called by the authority (server, singleplayer...).

	\param transition Transition object to enqueue
	\param checkValidity If true, it will only accept valid transitions (transition must be compatible with previous node)

	\return TRUE if transition is enqueued correctly. FALSE if transition is invalid not enqueued
	*/
	proto external bool EnqueueStateTransition(WeatherStateTransitionNode transition, bool checkValidity);
	/*!
	Inserts a state transition to the state transition queue.
	Keep in mind that state transition node cannot be inserted to the first node if a transition is already on course.
	Can only be called by the authority (server, singleplayer...).

	\param index Correct index, from 0 to length
	\param transition Transition node
	\param checkValidity If true, it will only accept valid transitions (transition must be compatible with previous and next nodes)

	\return TRUE if transition is valid. FALSE if transition is invalid or not authority or out of bounds.
	*/
	proto external bool InsertStateTransition(int index, WeatherStateTransitionNode transition, bool checkValidity);
	/*!
	Retrieves state transition node list.
	*/
	proto external void GetStateTransitionNodeList(out notnull array<ref WeatherStateTransitionNode> outArr);
	proto external WeatherStateTransitionNode GetStateTransitionNode(int index);
	proto external WeatherStateTransitionNode GetCurrentStateTransitionNode();
	proto external WeatherState GetCurrentState();
	proto external WeatherState GetNextState();
	/*!
	Retrieves in-game remaining time until next state is completely set.
	*/
	proto external float GetTimeLeftUntilNextState();
	/*!
	Retrieves in-game remaining time until next state's variant is completely set.
	*/
	proto external float GetTimeLeftUntilNextVariant();
	/*!
	Removes state transition node at supplied index.
	Can only be called by the authority (server, singleplayer...).

	\param index Index of node which will be deleted.
	\param removeInvalidTransitions Perform a sanity check and remove any transition nodes which arent compatible after deleting the selected one.
	\param totalRemoved how many nodes were removed, this will be 0 if supplied index is invalid or caller is not master. 1 if index is valid and removeInvalidTransitions == false, N if index is valid and removeInvalidTransitions == true
	\return true if master and something was removed, false otherwise.
	*/
	proto external bool RemoveStateTransition(int index, bool removeInvalidTransitions, out int totalRemoved);
	/*!
	Get total number of state transitions
	*/
	proto external int GetStateTransitionsCount();
	proto external bool CheckValidStateTransition(int stateIndexFrom, int stateIndexTo);
	proto external void AddTransitionCallbacks(BaseWeatherTransitionCallbacks callbacks);
	proto external void RemoveTransitionCallbacks(BaseWeatherTransitionCallbacks callbacks);
}

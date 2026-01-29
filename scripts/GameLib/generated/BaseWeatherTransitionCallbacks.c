/*
===========================================
Do not modify, this script is generated
===========================================
*/

class BaseWeatherTransitionCallbacks
{
	/*!
	Called ONLY after weather is computed (Mainly probes & cloud shadowmap) after a call to
	RequestStateTransitionImmediately.

	This callback will NOT be called if state is transitioning smoothly.
	*/
	event protected void OnStateSetImmediately() { };
	/*!
	Called when transition finishes and a new node is dequeued from the state transition queue.
	\param oldTransition The state we just transitioned from
	\param newTransition The new state we will transition, after waiting for the state duration of oldTransition's state.
	*/
	event protected void OnNewTransitionSet(WeatherStateTransitionNode oldTransition, WeatherStateTransitionNode newTransition) { };
	/*!
	Called when a transition starts.
	Does not get called if state is transitioned immediately through RequestStateTransitionImmediately.
	*/
	event protected void OnTransitionStarted(WeatherStateTransitionNode transition) { };
	/*!
	Called when a transition ends.
	Does not get called if state is transitioned immediately through RequestStateTransitionImmediately.
	*/
	event protected void OnTransitionEnded(WeatherStateTransitionNode transition) { };
}

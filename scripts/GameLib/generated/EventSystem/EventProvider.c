/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup EventSystem
\{
*/

/*!
\brief Class for broadcasting events to registered receivers.

The \c EventProvider class defines the interface for emitting named events to compatible receiver objects.
Only approved classes (such as \c WorldSystem and \c WorldController) may inherit from this class, depending
on project-specific rules. In scripting environments, the class is sealed to prevent custom extensions.

Event methods must be annotated with \c [EventAttribute] and represent the callable events that the provider exposes.
Receivers can be connected or disconnected using the \c ConnectEvent and \c DisconnectEvent APIs.

Internally, connections are managed through weak references. However, it is good practice to explicitly disconnect
receivers upon destruction to avoid orphaned connections and potential memory issues.

\see EventAttribute
\see \ref Page_EventSystem
*/
sealed class EventProvider: ScriptAndConfig
{
	/*!
	Emit event on provider
	\param eventSender Event method on provider
	*/
	proto external protected void ThrowEvent(func eventSender, void param1 = null, void param2 = null, void param3 = null, void param4 = null, void param5 = null, void param6 = null, void param7 = null, void param8 = null, void param9 = null);
	/*!
	Connects receiver callback to provider event
	\param eventSender Event method on provider
	\param eventReceiver Callback method on receiver
	*/
	static proto bool ConnectEvent(func eventSender, func eventReceiver);
	/*!
	Connects receiver callback to provider event
	\param eventSender Event method on provider
	\param eventReceiver Callback method on receiver
	\param filter Integer or Managed, used to compare with the event's first argument
	*/
	static proto bool ConnectEventFiltered(func eventSender, func eventReceiver, void filter);
	/*!
	Disconnect single event connection
	\param eventSender Event method on provider
	\param eventReceiver Callback method on receiver
	*/
	static proto bool DisconnectEvent(func eventSender, func eventReceiver);
	/*!
	Disconnect single event connection. Only event connection with same filter value is removed.
	\param eventSender Event method on provider
	\param eventReceiver Callback method on receiver
	\param filter Integer or Managed
	*/
	static proto bool DisconnectEventFiltered(func eventSender, func eventReceiver, void filter);
	/*!
	Disconnect all connections between provider and receiver
	*/
	static proto bool DisconnectEvents(EventProvider provider, Managed receiver);
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Events
\{
*/

class EventHandlerManagerComponentClass: GameComponentClass
{
}

class EventHandlerManagerComponent: GameComponent
{
	/*!
	*	Supported and tested argument types when raising event in script with handler in gamecode:
	*	- int, float, bool
	*	- string in script, enf::CStr in code
	*	- vector in script, enf::Vector3* in code
	*	- Instance in script, pointer in code (BaseWeaponComponent -> BaseWeaponComponent*)
	*	- array<float/ int/ string/ vector>, others not tested, but should work, pointer in code (array<float> -> enf::ArrayFloat*)
	*
	*	The execution of the callbacks itself will be delayed, therefore it is necessary that any instances
	*	passed in will still exist at least until the next frame, and their state does not change (or, if
	*	it does change, that this change is expected by the callback, as it will be propagated).
	*	- basic types - int, float, bool are copied by value and thus safe.
	*	- string is copied by value, but contains a pointer to the string that has reference counting.
	*	- vector and arrays have only their pointers copied, so they behave the same as instance.
	*/
	proto void RaiseEvent(string eventName, int argsCount, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	proto void RegisterScriptHandler(string eventName, Managed inst, func callback, bool delayed = true, bool singleUse = false);
	proto void RemoveScriptHandler(string eventName, Managed inst, func callback, bool delayed = true);
	//! Returns a list and count of all event handlers
	proto external int GetEventHandlers(out notnull array<BaseEventHandler> outEventHandlers);
}

/*!
\}
*/

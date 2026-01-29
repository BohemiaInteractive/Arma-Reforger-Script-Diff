/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

//! Simple signal component. Use signals to share data between components without direct dependency on each other.
class SimpleSignalComponent: GenericComponent
{
	//! Convert signal name to its index. Returns -1 if no such signal is known to the component.
	proto external int FindSignalIndex(IEntity owner, string signalName);
	//! Get current signal value.
	//! Local signals are read from this component directly, parent signals are searched in the parent hierarchy of the entity.
	//! Returns zero if no such signal is found.
	proto external float GetSignalValue(int signalIndex);
	//! Set current signal value.
	//! Only local signals can be set, setting value to parent signal silently fails.
	proto external void SetSignalValue(int signalIndex, float signalValue);
	//! Check whether the signal at this index is stored locally.
	//! Only local signals can be set, setting value to parent signal silently fails.
	proto external bool IsLocalSignal(int signalIndex);
}

/*!
\}
*/

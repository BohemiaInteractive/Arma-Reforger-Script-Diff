/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Events
\{
*/

class Event
{
	//! Add callback and returns callback id.
	proto external int AddCallback(func fn);
	proto external bool Remove(int callbackId);
	proto external bool RemoveByOwner(Class owner);
	proto external bool Emit(Managed caller = null);
}

/*!
\}
*/

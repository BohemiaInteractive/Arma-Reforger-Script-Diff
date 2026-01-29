/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

class BaseFuelNode: ScriptAndConfig
{
	//! Set whether or not this node should receive Fixed Frame calls
	proto external void SetShouldSimulate(bool value);
	//! Returns whether or not fixed frame calls are done on this node
	proto external bool ShouldSimulate();
	//! Set current amount of fuel in this fuel node
	proto external void SetFuel(float value);
	//! Get current amount of fuel
	proto external float GetFuel();
	//! Get max amount of fuel
	proto external float GetMaxFuel();
	//! Get type of fuel stored in this node
	proto external EFuelType GetFuelType();
	/*!
	Returns the point info representing the fuel cap or null if none defined.
	\return PointInfo instance or null if none.
	*/
	proto external PointInfo GetFuelCapPointInfo();

	// callbacks

	//! Init call in script
	event protected void OnInit(IEntity owner);
	//! Fixed Frame Call in script
	event protected void OnFixedFrame(IEntity owner, float timeSlice);
	event protected void OnFuelChanged(float newFuel);
}

/*!
\}
*/

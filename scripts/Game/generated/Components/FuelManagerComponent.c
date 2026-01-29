/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class FuelManagerComponentClass: GameComponentClass
{
}

class FuelManagerComponent: GameComponent
{
	proto external int GetFuelNodesList(out notnull array<BaseFuelNode> outNodes);
	//! Returns true if one node has at least some fuel
	proto external bool HasFuel();
	//! Returns true if at least one node can be refueled
	proto external bool CanBeRefueled();
	//! Get the current sum of amount of fuel in all nodes
	proto external float GetTotalFuel();
	//! Get the current sum of the max amount of fuel in all nodes
	proto external float GetTotalMaxFuel();

	// callbacks

	event void OnInit(IEntity owner);
}

/*!
\}
*/

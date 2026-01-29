/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup WorldSystems
\{
*/

//! Structure holding world system meta-information required by the engine.
sealed class WorldSystemInfo: pointer
{
	/*!
	Restore all properties to their default values. Default values for each
	property are following:
	- `Abstract`: `true`
	- `Unique`: `false`
	- `Location`: WorldSystemLocation.Both
	- `Points`: empty set
	- `Controllers`: empty set
	- `ExecuteBefore`: empty set
	- `ExecuteAfter`: empty set
	*/
	proto external WorldSystemInfo RestoreDefaults();
	proto external WorldSystemInfo SetAbstract(bool value);
	proto external WorldSystemInfo SetUnique(bool value);
	proto external WorldSystemInfo SetLocation(WorldSystemLocation value);
	proto external WorldSystemInfo AddPoint(WorldSystemPoint point);
	proto external WorldSystemInfo RemovePoint(WorldSystemPoint point);
	proto external WorldSystemInfo ClearPoints();
	proto external WorldSystemInfo AddController(typename controller);
	proto external WorldSystemInfo RemoveController(typename controller);
	proto external WorldSystemInfo ClearControllers();
	proto external WorldSystemInfo AddExecuteBefore(typename otherSystem, WorldSystemPoint point);
	proto external WorldSystemInfo RemoveExecuteBefore(typename otherSystem, WorldSystemPoint point);
	proto external WorldSystemInfo ClearExecuteBefore();
	proto external WorldSystemInfo AddExecuteAfter(typename otherSystem, WorldSystemPoint point);
	proto external WorldSystemInfo RemoveExecuteAfter(typename otherSystem, WorldSystemPoint point);
	proto external WorldSystemInfo ClearExecuteAfter();
}

/*!
\}
*/

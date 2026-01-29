/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIExtraMoveParams: ScriptAndConfig
{
	//! Resets everything to default values.
	//! You must call this after creation of the object.
	proto external void Reset();
	//! Sets cost modifier which increases cost of graph edges aligned same way as the provided direction.
	//! Default value is 1.0 and thus does not affect anything.
	//! Values above 1.0 will make pathfinding favor non-straight path to destination.
	//! But it's impossible to know beforehand if the chosen path will flank left or right.
	proto external void SetStraightPathCostModifier(float value, vector dir);
}

/*!
\}
*/

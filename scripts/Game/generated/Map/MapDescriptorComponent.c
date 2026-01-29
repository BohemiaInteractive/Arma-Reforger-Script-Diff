/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapDescriptorComponent: GameComponent
{
	/*!
	\brief Determine type of descriptor
	*/
	proto external int GetBaseType();
	/*!
	\brief Determine unit type of descriptor - may not be set if not unit
	*/
	proto external int GetUnitType();
	/*!
	\brief Determine group type of descriptor
	*/
	proto external int GetGroupType();
	//! Search for the item. Use with moderation and ideally not inside tight loops
	proto external MapItem Item();
}

/*!
\}
*/

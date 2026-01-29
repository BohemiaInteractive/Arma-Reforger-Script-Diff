/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class ItemAttributeCollection: ScriptAndConfig
{
	proto external UIInfo GetUIInfo();
	//! Script Attribute getter
	//! Finds first occurance of the coresponding attribute data object.
	proto external BaseItemAttributeData FindAttribute(typename typeName);
	proto external ECommonItemType GetCommonType();

	// callbacks

	event protected void OnInitCollection(IEntityComponentSource src);
}

/*!
\}
*/

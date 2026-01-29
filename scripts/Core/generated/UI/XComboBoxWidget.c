/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class XComboBoxWidget: UIWidget
{
	proto external int GetCurrentItem();
	proto external void SetCurrentItem(int item);
	proto external int GetNumItems();
	//!Adds one item. Returns its index
	proto external int AddItem(string item);
	//!Changes item
	proto external void SetItem(int item, string value);
	//!Clear all items
	proto external void ClearAll();
	//!Remove item by index
	proto external void RemoveItem(int item);
}

/*!
\}
*/

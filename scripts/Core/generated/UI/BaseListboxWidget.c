/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class BaseListboxWidget: UIWidget
{
	proto external void ClearItems();
	proto external void RemoveRow(int row);
	//!Returns number of items(rows)
	proto external int GetNumItems();
	proto external void EnsureVisible(int row);
	proto external int GetSelectedRow();
	proto external void SelectRow(int row);
}

/*!
\}
*/

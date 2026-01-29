/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class TextListboxWidget: SimpleListboxWidget
{
	proto external void SetItemColor(int row, int column, int color);
	proto external void SetItem(int position, string text, Managed userData, int column);
	proto external Managed GetItemData(int row, int column = 0);
	//! Insert new Row, if row = -1, new Row is inserted at the end otherwise at row index.
	proto external int AddItem(string text, Managed userData, int column, int row = -1);
	proto bool GetItemText(int row, int column, out string text);
}

/*!
\}
*/

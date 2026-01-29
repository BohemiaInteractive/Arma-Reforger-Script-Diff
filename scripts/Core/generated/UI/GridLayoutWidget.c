/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

/*!
GridLayoutWidget class
*/
sealed class GridLayoutWidget: Widget
{
	/*!
	Returns weight of row fill at index `rowIndex`
	Indexing "out of bounds" of the grid (at index greater than the maximum row number for any child)
	or at any row which has no weight set is handled and returns 0.
	*/
	proto external float GetRowFillWeight(int rowIndex);
	//! \see GetRowFillWeight
	proto external float GetColumnFillWeight(int columnIndex);
	/*!
	Sets fill weight for `rowIndex` to `weight`
	Indexing "out of bounds" of the grid (at index greater than the maximum row number for any child)
	is handled and the weight is correctly set.
	*/
	proto external void SetRowFillWeight(int rowIndex, float weight);
	//! \see SetRowFillWeight
	proto external void SetColumnFillWeight(int columnIndex, float weight);
}

/*!
\}
*/

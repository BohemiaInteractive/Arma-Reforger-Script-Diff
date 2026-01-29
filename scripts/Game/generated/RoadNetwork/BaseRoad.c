/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup RoadNetwork
\{
*/

class BaseRoad
{
	/*!
	* Get width of road
	* \return width of road or 0 if no road is available
	*/
	proto external float GetWidth();
	//! Returns the points of the road
	proto external int GetPoints(out notnull array<vector> outPoints);
}

/*!
\}
*/

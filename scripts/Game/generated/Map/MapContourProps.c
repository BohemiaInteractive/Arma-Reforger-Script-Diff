/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapContourProps
{
	// Visualisation
	proto external void EnableVisualisation(bool value);
	proto external void EnableLandVisualization(bool value);
	proto external void EnableOceanVisualization(bool value);
	proto external void EnableCoastVisualization(bool value);
	// LOD
	proto external int GetLOD();
	proto external void SetLOD( int lod );
	// Density
	proto external float GetMajorDensity();
	proto external void SetMajorDensity(float density);
	proto external float GetContourDensity();
	proto external void SetContourDensity(float density);
	// Width
	proto external float GetMajorWidth();
	proto external void SetMajorWidth(float width);
	proto external float GetContourWidth();
	proto external void SetContourWidth(float width);
	proto external float GetCostalWidth();
	proto external void SetCostalWidth(float width);
	// World scale
	proto external float GetWorldScale();
	proto external void SetWorldScale(float scale);
	/*!
	Major color
	*/
	proto ref Color GetMajorColor();
	proto external void SetMajorColor(notnull Color color);
	/*!
	Coast color
	*/
	proto ref Color GetCoastColor();
	proto external void SetCoastColor(notnull Color color);
	/*!
	Ocean color
	*/
	proto ref Color GetOceanColor();
	proto external void SetOceanColor(notnull Color color);
	/*!
	Land color
	*/
	proto ref Color GetLandColor();
	proto external void SetLandColor(notnull Color color);
}

/*!
\}
*/

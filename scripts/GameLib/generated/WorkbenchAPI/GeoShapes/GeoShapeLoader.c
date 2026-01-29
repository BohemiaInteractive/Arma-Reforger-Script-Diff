/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_GeoShapes
\{
*/

/*!
*	\brief	Geo shape API entry point - static class for loading various geo shape formats
*					such as ESRI Shapefiles.
*/
sealed class GeoShapeLoader
{
	private void GeoShapeLoader();
	private void ~GeoShapeLoader();

	/*!	\brief	Loads ESRI Shapefile.
	\param	fileName	path to the file
	\return	Shape collection - loaded Shapefile's read-only collection
					or nullptr if loading failed for some reason.

	Note: For successful load, all the Shapefile mandatory files must exist in the directory
	(i.e. the SHP, SHX and DBF).
	*/
	static proto ref GeoShapeCollection LoadShapeFile(string fileName);
}

/*!
\}
*/

#endif // WORKBENCH

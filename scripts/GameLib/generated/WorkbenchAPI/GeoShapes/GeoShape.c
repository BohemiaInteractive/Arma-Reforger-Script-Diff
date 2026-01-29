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

//! Base class for all shapes.
sealed class GeoShape: Managed
{
	private void GeoShape();

	//! Shape's type
	proto external GeoShapeType GetType();
	//! Shape's attributes
	proto external GeoAttribCollection	GetAttributes();
}

/*!
\}
*/

#endif // WORKBENCH

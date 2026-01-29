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

//! Polygon (can have multiple parts).
sealed class GeoPolygon: GeoShape
{
	proto external int PartsCount();
	proto external ref GeoVertexCollection GetPart(int index);
}

/*!
\}
*/

#endif // WORKBENCH

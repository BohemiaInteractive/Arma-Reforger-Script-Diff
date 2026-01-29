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
Collection of shapes, i.e. (loaded) geo shape file interface. Can be treated
like read-only array.
*/
sealed class GeoShapeCollection: Managed
{
	proto external int Count();
	proto external ref GeoShape Get(int index);
}

/*!
\}
*/

#endif // WORKBENCH

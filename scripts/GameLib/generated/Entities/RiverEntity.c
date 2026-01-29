/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class RiverEntityClass: GeneratorBaseEntityClass
{
}

/*!
Entity for a single river.

Must have a shape entity as a parent - central points are taken from the parent.
*/
class RiverEntity: GeneratorBaseEntity
{
	/*!
	Gets all existing RiverEntity instances (across all loaded worlds).
	\param instances      Where the instance pointers will be added.
	*/
	static proto void GetExistingInstances(out notnull array<RiverEntity> instances);
	/*!
	Computes the central polyline (an approximation of the parent spline in world space) and the river width at its points.
	\param dstPositions   Output array for the resulting central points.
	                      NOTE: Any existing content will be removed.
	\param dstWidths      Output array for the resulting widths (will have the same size as dstPositions).
	                      NOTE: Any existing content will be removed.
	\param precision      Maximum allowed distance between the polyline approximation and the original spline.
	                      Higher values yield lower precision and fewer points.
	                      The default value matches the one used in generating the actual river mesh.
	\return               Returns true on success, or false if the parent spline has fewer than two points.
	*/
	proto bool GetCentralPolyline(out notnull array<vector> dstPositions, out notnull array<float> dstWidths, float precision = 0.1);
	/*!
	Computes the river's outline points (in world space).
	\param dst          Output array for the resulting points (left-side and right-side alternating).
	                    NOTE: Any existing content will be removed.
	\param precision    Higher values result in lower precision and fewer points.
	                    The default value matches the one used to generate the actual river mesh.
	                    Details:
	                    First, the spline is approximated by a polyline, which is then stroked to
	                    generate the outline. This value specifies the maximum allowed distance
	                    between the polyline and the original spline.
	*/
	proto void GetOutlinePoints(out notnull array<vector> dst, float precision = 0.1);
}

/*!
\}
*/

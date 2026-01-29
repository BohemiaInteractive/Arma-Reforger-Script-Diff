/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class SplineShapeEntityClass: ShapeEntityClass
{
}

class SplineShapeEntity: ShapeEntity
{
	/*!
	Returns true if tangents at given anchor point were explicitly set by user,
	false if they are computed automatically.
	*/
	proto external bool HasPointExplicitTangents(int pointIdx);
	/*!
	Reconstructs the shape from the points in the points array, or by adding it after first paceFirstAtIdx points.
	points are the points to be added, entSource is the source of the shape entity. tangentLengthIn/Out allow to multiply the length of the tangents instead of the usual 0.5.
	placeFirstAtIdx allows to keep the first (placeFirstAtIdx - 1) existing points of the spline and adds new points after those.
	*/
	proto external void SetPointsSpline(array<vector> points, IEntitySource entSource, float tangentLengthIn, float tangentLengthOut, int placeFirstAtIdx = 0);
	/*!
	Returns tangents at given anchor point
	For point `i` the inTangent is a tangent between points `i - 1` and `i` at point `i`.
	The outTangent is a tangent between points `i` and `i + 1` at point `i`.
	*/
	proto void GetTangents(int pointIdx, out vector inTangent, out vector outTangent);
}

/*!
\}
*/

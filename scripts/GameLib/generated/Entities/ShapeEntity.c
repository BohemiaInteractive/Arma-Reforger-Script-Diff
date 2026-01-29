/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class ShapeEntityClass: GenericEntityClass
{
}

class ShapeEntity: GenericEntity
{
	proto external BaseContainer GetPointContainerOfType(IEntitySource src, int anchorIdx, string type);
	/*!
	Reconstructs the shape from the points in the points array, or by adding it after first paceFirstAtIdx points.
	points are the points to be added, entSource is the source of the shape entity.
	placeFirstAtIdx allows to keep the first (placeFirstAtIdx - 1) existing points of the spline and adds new points after those.
	*/
	proto external void SetPoints(array<vector> points, IEntitySource entSource, int placeFirstAtIdx = 0);
	proto external void GenerateTesselatedShape(notnull array<vector> outPoints);
	/*!
	Returns bboxes for all segments of this shape
	Similar to GeneratorBaseEntity events, ith bbox is given by mins[i] and maxes[i].
	Since the bboxes are done by parts their sum may be smaller than the bbox of the whole entity.
	The bboxes returned may overlap.
	*/
	proto void GetAllInfluenceBBoxes(IEntitySource shapeEntitySrc, out notnull array<vector> mins, out notnull array<vector> maxes);
	/*!
	Returns bboxes for segments of this shape within index range
	Similar to GeneratorBaseEntity events, ith bbox is given by mins[i] and maxes[i].
	Since the bboxes are done by parts their sum may be smaller than the bbox of the whole entity.
	The bboxes returned may overlap.
	The range is including the first and last index.
	*/
	proto void GetInfluenceBBoxesInSegments(IEntitySource shapeEntitySrc, int firstIncl, int lastIncl, out notnull array<vector> mins, out notnull array<vector> maxes);
	proto external int GetPointCount();
	proto external bool IsClosed();
	//! Copies positions of points to given array
	proto external void GetPointsPositions(notnull array<vector> outPoints);
	proto external void AddPointData(IEntitySource entSrc, int idx, ShapePointDataScriptBase pointData);
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class SplineTerrainGeneratorEntityBaseClass: GeneratorBaseEntityClass
{
}

class SplineTerrainGeneratorEntityBase: GeneratorBaseEntity
{
	// Creates a list of BBoxes of tesselated segments that are between each neighboring control points (bBoxes[i] contains bBox for spline between controlPoints i and i+1).
	proto external void GetSegmentBoundBoxes(inout notnull array<vector> mins, inout notnull array<vector> maxs);
	proto external void RegisterGenerator(vector bbMin, vector bbMax, int priority);
	proto external void RemoveGenerator(bool forceUpdate, array<vector> mins, array<vector> maxs);
	proto external void AddRectsToUpdateQueue(array<vector> mins, array<vector> maxs);
	proto external void UpdateRectsFromQueue();
	proto external float GetMaxTerrainResolution();

	// callbacks

	// Returns the extents per segments (divided by control points) with segment 0-1 under index 0, and so on.
	event array<float> GetExtents();
	// Callback that calculates the new terrain height. Return float.MAX to keep the original terrain height.
	event float GenerateTerrain(array<vector> aShapeControlPoints, array<vector> aShapeTesselatedPoints, array<float> aShapeDistancesFromStart, array<int> aTesselatedToOriginalIndices, int heightMapXPos, int heightMapYPos, vector closestPointOnLineWP, vector genPointWS, float oldHeight, int tessPointAIdx, int tessPointBIdx, float ABRatio, float terrainInvScale, float signedDistFromSpline)  /* signedDistFromSpline is negative, if the point is on the left side from the spline.*/;
}

/*!
\}
*/

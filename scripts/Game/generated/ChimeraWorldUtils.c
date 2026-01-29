/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class ChimeraWorldUtils
{
	private void ChimeraWorldUtils();
	private void ~ChimeraWorldUtils();

	// Check if current point is submerged under water, if it is fill in out arguments
	static proto bool TryGetWaterSurface(BaseWorld world, vector inPoint, out vector outWaterSurfacePoint, out EWaterSurfaceType outType, out vector transformWS[4], out vector obbExtents);
	static proto bool TryGetWaterSurfaceSimple(BaseWorld world, vector inPoint);
}

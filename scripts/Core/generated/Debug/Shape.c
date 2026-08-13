/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

//! Instance of created debug visualizer.
class Shape: Managed
{
	//!don't call destructor directly!
	proto void ~Shape();
	private void Shape();

	static proto ref Shape Create(ShapeType type, int color, ShapeFlags flags, vector p1, vector p2);
	/*!
	Create a single line
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param p1 Start point
	\param p2 End point
	\param thickness Optional line thickness in screen pixels
	*/
	static proto ref Shape CreateLine(int color, ShapeFlags flags, vector p1, vector p2, float thickness = 1);
	/*!
	Create line strip
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param p Array of points
	\param num Number of points. Must be at last 2
	\param thickness Optional line thickness in screen pixels
	*/
	static proto ref Shape CreateLines(int color, ShapeFlags flags, vector p[], int num, float thickness = 1);
	/*!
	Create looped line strip (the last and the first point are connected too).
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param p Array of points
	\param num Number of points. Must be at last 2
	\param thickness Optional line thickness in screen pixels
	*/
	static proto ref Shape CreateLinesLoop(int color, ShapeFlags flags, vector p[], int num, float thickness = 1);
	/*!
	Create list of separate lines
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param p Array of points
	\param num Number of points. Must be at last 2
	\param thickness Optional line thickness in screen pixels
	*/
	static proto ref Shape CreateSeparateLines(int color, ShapeFlags flags, vector p[], int num, float thickness = 1);
	/*!
	Create a cross (three lines aligned with axis, perpendicular to each other)
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param origin Position of shape
	\param radius Radius of circle (in meters)
	\param thickness Optional line thickness in screen pixels
	*/
	static proto ref Shape CreateCross(int color, ShapeFlags flags, vector origin, float radius, float thickness = 1);
	/*!
	Create circle aligned with xz-plane
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param origin Position of shape
	\param radius Radius of circle (in meters)
	\param overrideNumSlices if greater than 0, override number of slices by this amount.
	\param thickness Optional line thickness in screen pixels
	*/
	static proto ref Shape CreateCircle(int color, ShapeFlags flags, vector origin, float radius, int overrideNumSlices = 0, float thickness = 1);
	/*!
	Create triangle-list mesh.
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param p Array of vertexes
	\param num Number of triangles
	*/
	static proto ref Shape CreateTris(int color, ShapeFlags flags, vector p[], int num);
	/*!
	Create sphere
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param origin Position of shape
	\param radius Radius of sphere
	*/
	static proto ref Shape CreateSphere(int color, ShapeFlags flags, vector origin, float radius);
	/*!
	Create cylinder aligned with Z axis
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param origin Position of cylinder
	\param radius Radius of cylinder
	\param length Length of cylinder
	*/
	static proto ref Shape CreateCylinder(int color, ShapeFlags flags, vector origin, float radius, float length);
	/*!
	Creates arrow from lines
	\param from Starting position of arrow
	\param to Ending position of arrow
	\param size Size of the arrow head (in meters)
	\param color Color encoded in 0xA8R8G8B8 form
	\param flags Flags, see ShapeFlags
	\param thickness Optional line thickness in screen pixels
	*/
	static proto ref Shape CreateArrow(vector from, vector to, float size, int color, ShapeFlags flags, float thickness = 1);
	proto external void SetMatrix(vector mat[4]);
	proto external void GetMatrix(out vector mat[4]);
	proto external void SetColor(int color);
	proto external void SetFlags(ShapeFlags flags);
	/*!
	Enable/Disable contrast rendering mode which darkens the shape color when rendered over a bright background or lightens the shape color over dark backgrounds
	\param mode Contrast mode
	\param intensity Intensity of darkening/lightening effect. Resulting color of the shape rendered over pure white/black can be
	represented as linear interpolation (when this variable is the interpolation parameter) between the original
	shape color and the shape color with value = 1 in HSV color model.
	\param threshold Threshold when the shape color starts changing. For darkening effect, if the background is brighter than this
	value, the shape will be darkened. For lightening effect, the shape will be lightened when the background is
	darker then this value.
	*/
	proto external void SetContrast(ShapeContrastMode mode, float intensity, float threshold);
	proto external ShapeContrastMode GetContrastMode();
	proto external float GetContrastIntensity();
	proto external float GetContrastThreshold();
}

/*!
\}
*/

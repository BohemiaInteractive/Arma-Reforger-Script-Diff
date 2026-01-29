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
	Create list of lines
	\param color Color encoded in 0xA8R8G8B8 form.
	\param flags Flags, see ShapeFlags
	\param p Array of points
	\param num Number of points. Must be multiply of 2
	*/
	static proto ref Shape CreateLines(int color, ShapeFlags flags, vector p[], int num);
	/*!
	Create triangle-list mesh.
	\param color Color encoded in 0xA8R8G8B8 form.
	\param flags Flags, see ShapeFlags
	\param p Array of vertexes
	\param num Number of triangles
	*/
	static proto ref Shape CreateTris(int color, ShapeFlags flags, vector p[], int num);
	/*!
	Create sphere
	\param color Color encoded in 0xA8R8G8B8 form.
	\param flags Flags, see ShapeFlags
	\param origin Position of shape
	\param radius Radius of sphere
	*/
	static proto ref Shape CreateSphere(int color, ShapeFlags flags, vector origin, float radius);
	/*!
	Create cylinder aligned with Z axis
	\param color Color encoded in 0xA8R8G8B8 form.
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
	\param color Color of arrow
	\param flags Flags, see ShapeFlags
	*/
	static proto ref Shape CreateArrow(vector from, vector to, float size, int color, ShapeFlags flags);
	proto external void SetMatrix(vector mat[4]);
	proto external void GetMatrix(out vector mat[4]);
	proto external void SetColor(int color);
	proto external void SetFlags(ShapeFlags flags);
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Math
\{
*/

sealed class Math3D
{
	private void Math3D();
	private void ~Math3D();

	static proto float IntersectionRayBox(vector start, vector end, vector mins, vector maxs);
	/*!
	Tests whether intersection of ray and sphere exists and returns distance to intersection
	from `raybase` and `raycos` (`I{01} = P + dir*t{01}`).
	\param raybase start point of ray
	\param raycos direction of ray
	\param center sphere center
	\param radius radius of sphere
	*/
	static proto float IntersectionRaySphere(vector raybase, vector raycos, vector center, float radius);
	/*!
	Tests whether sphere is intersecting cone.
	\param origin Origin of sphere
	\param radius Radius of sphere
	\param conepos Position of top of cone
	\param axis Orientation of cone in direction from top to bottom
	\param angle Angle of cone in radians
	\return `true` when intersects
	*/
	static proto bool IntersectionSphereCone(vector origin, float radius, vector conepos, vector axis, float angle);
	/*!
	Tests whether sphere is intersecting cone.
	\param origin Origin of sphere
	\param radius Radius of sphere
	\param conepos Position of top of cone
	\param axis Orientation of cone in direction from top to bottom
	\param angle Angle of cone in radians
	\return `true` when intersects
	*/
	static proto bool IntersectionWholeSphereCone(vector origin, float radius, vector conepos, vector axis, float angle);
	/*!
	Tests whether point is intersecting cylinder.
	\param origin Origin of point
	\param cylinderTip Tip of cylinder
	\param cylinderAxis Axis of cylinder
	\param cylinderRadius Radius of cylinder
	\return `true` when intersects
	*/
	static proto bool IntersectionPointCylinder(vector origin, vector cylinderTip, vector cylinderAxis, float cylinderRadius);
	/*!
	Tests whether sphere is intersecting AABB.
	\param origin Origin of sphere
	\param radius Radius of sphere
	\param mins minimum point of bounding box
	\param maxs maximum point of bounding box
	\return `true` when intersects
	*/
	static proto bool IntersectionSphereAABB(vector origin, float radius, vector mins, vector maxs);
	/*!
	Returns `true`, when bounding boxes intersect.
	\code
		vector mins1 = "1 1 1";
		vector maxs1 = "3 3 3";
		vector mins2 = "2 2 2";
		vector maxs2 = "4 4 4";
		Print( Math3D.IntersectionBoxBox(mins1, maxs1, mins2, maxs2) );

		>> 1
	\endcode
	\param mins1 Minimum point of first bounding box.
	\param maxs1 Maximum point of first bounding box.
	\param mins2 Minimum point of second bounding box.
	\param maxs2 Maximum point of second bounding box.
	\return `true` when bounding boxes intersect, otherwise `false`.
	*/
	static proto bool IntersectionBoxBox(vector mins1, vector maxs1, vector mins2, vector maxs2);
	/*!
	Creates rotation matrix from direction and up vector.
	\code
		vector mat[4];
		vector dir = "1 0 1";
		vector up = "0 1 0";
		DirectionAndUpMatrix( dir, up, mat );
		Print( mat );

		>> <0.707107,0,-0.707107>,<0,1,0>,<0.707107,0,0.707107>,<0,0,0>
	\endcode
	\param dir direction vector
	\param up up vector
	\param[out] mat created rotation matrix
	*/
	static proto void DirectionAndUpMatrix(vector dir, vector up, out vector mat[4]);
	/*!
	Transforms rotation matrix.
	\code
		vector mat0[3] = { "1.5 2.5 0", "0.1 1.3 0", "0 0 1" }; // rotation matrix
		vector mat1[3] = { "1 0.4 0", "0 1 0", "0 1.3 2.7" }; // rotation matrix
		vector res[3];
		Math3D.MatrixMultiply3(mat0, mat1, res);
		Print( res );

		>> <1.54,3.02,0>,<0.1,1.3,0>,<0.13,1.69,2.7>
	\endcode
	\param mat0 first matrix
	\param mat1 second matrix
	\param[out] res result of first and second matrix multiplication
	*/
	static proto void MatrixMultiply3(vector mat0[3], vector mat1[3], out vector res[3]);
	/*!
	Transforms matrix.
	\code
		vector mat0[4] = { "2 0 0", "0 3 0", "0 1 0", "0 0 0" }; // scale matrix
		vector mat1[4] = { "1 0 0", "0 1 0", "0 1 0", "2 4 1" }; // translation matrix
		vector res[4];
		Math3D.MatrixMultiply4(mat0, mat1, res);
		Print( res );

		>> <2,0,0>,<0,3,0>,<0,3,0>,<4,13,0>
	\endcode
	\param mat0 first matrix
	\param mat1 second matrix
	\param[out] res result of first and second matrix multiplication
	*/
	static proto void MatrixMultiply4(vector mat0[4], vector mat1[4], out vector res[4]);
	/*!
	Inverts matrix.
	\code
		vector mat[3] = { "2 0 0", "0 3 0", "0 0 1" }; // rotation matrix
		vector res[3];
		Math3D.MatrixGetInverse3(mat, res);
		Print( res );

		>> <0.5,0,0>,<0,0.333,0>,<0,0,1>
	\endcode
	\param mat input matrix
	\param[out] res inverted matrix
	*/
	static proto void MatrixGetInverse3(vector mat[3], out vector res[3]);
	/*!
	Inverts matrix.
	\code
		vector mat[4] = { "2 0 0 0", "0 3 0 0", "0 0 1 0", "0 0 0 5" }; // rotation matrix
		vector res[4];
		Math3D.MatrixGetInverse4(mat, res);
		Print( res );

		>> <0.5,0,0,0>,<0,0.333,0,0>,<0,0,1,0>,<0,0,0,0.2>
	\endcode
	\param mat input matrix
	\param[out] res inverted matrix
	*/
	static proto void MatrixGetInverse4(vector mat[4], out vector res[4]);
	/*!
	Invert-transforms rotation matrix.
	\code
		vector mat0[3] = { "1.5 2.5 0", "0.1 1.3 0", "0 0 1" }; // rotation matrix
		vector mat1[3] = { "1 0.4 0", "0 1 0", "0 1.3 2.7" }; // rotation matrix
		vector res[3];
		Math3D.MatrixInvMultiply3(mat0, mat1, res);
		Print( res );

		>> <2.5,0.62,0>,<2.5,1.3,0>,<3.25,1.69,2.7>
	\endcode
	\param mat0 first matrix
	\param mat1 second matrix
	\param[out] res result of first and second matrix multiplication
	*/
	static proto void MatrixInvMultiply3(vector mat0[3], vector mat1[3], out vector res[3]);
	/*!
	Invert-transforms matrix.
	\code
		vector mat0[4] = { "2 0 0", "0 3 0", "0 0 1", "0 0 0" }; // scale matrix
		vector mat1[4] = { "1 0 0", "0 1 0", "0 0 1", "2 4 1" }; // translation matrix
		vector res[4];
		Math3D.MatrixInvMultiply4(mat0, mat1, res);
		Print( res );

		>> <2,0,0>,<0,3,1>,<0,3,1>,<4,12,4>
	\endcode
	\param mat0 first matrix
	\param mat1 second matrix
	\param[out] res inverse result of first and second matrix multiplication
	*/
	static proto void MatrixInvMultiply4(vector mat0[4], vector mat1[4], out vector res[4]);
	/*!
	Converts rotation matrix to quaternion.
	\code
		vector mat[3];
		vector rot = "70 15 45";
		Math3D.AnglesToMatrix(rot, mat);
		float d[4];
		Math3D.MatrixToQuat( mat, d );
		Print( d );

		>> {0.241626,0.566299,-0.118838,0.778973}
	\endcode
	\param mat rotation matrix
	\param[out] d created quaternion copy
	*/
	static proto void MatrixToQuat(vector mat[3], out float d[4]);
	/*!
	Returns angles of rotation matrix.
	\code
		vector mat[3];
		Math3D.AnglesToMatrix( "70 15 45", mat );
		vector ang = Math3D.MatrixToAngles( mat );
		Print( ang );

		>> <70,15,45>
	\endcode
	\param mat rotation matrix
	\return yaw, pitch, roll angles in degrees
	*/
	static proto vector MatrixToAngles(vector mat[3]);
	/*!
	Returns angles and scale of rotation matrix.
	\param mat rotation matrix
	\param angles yaw, pitch, roll angles in degrees
	\return scale of matrix
	*/
	static proto float MatrixToAnglesAndScale(vector mat[3], out vector angles);
	static proto void MatrixFromForwardVec(vector forwardVec, out vector mat[3]);
	static proto void MatrixFromUpVec(vector upVec, out vector mat[3]);
	/*!
	Creates rotation matrix from angles (yaw, pitch, roll in degrees).
	\code
		vector mat[3];
		Math3D.AnglesToMatrix( "70 15 45", mat );
		Print( mat );

		>> <0.41382,-0.683013,-0.601869>,<0.069869,0.683013,-0.727057>,<0.907673,0.258819,0.330366>
	\endcode
	\param ang vector which contains angles
	\param[out] mat created rotation matrix
	*/
	static proto void AnglesToMatrix(vector ang, out vector mat[3]);
	/*!
	Creates identity matrix.
	\code
		vector mat[4];
		Math3D.MatrixIdentity4( mat );
		Print( mat );

		>> <1,0,0>,<0,1,0>,<0,0,1>,<0,0,0>
	\endcode
	\param[out] mat created identity matrix
	*/
	static proto void MatrixIdentity4(out vector mat[4]);
	/*!
	Creates identity matrix.
	\code
		vector mat[3];
		Math3D.MatrixIdentity3( mat );
		Print( mat );

		>> <1,0,0>,<0,1,0>,<0,0,1>
	\endcode
	\param[out] mat created identity matrix
	*/
	static proto void MatrixIdentity3(out vector mat[3]);
	/*!
	Copy matrixes.
	\param matSrc `vector[3]` or `vector[4]`
	\param[out] matDst `vector[3]` or `vector[4]`
	*/
	static proto void MatrixCopy(vector matSrc[], out vector matDst[]);
	//! Normalize matrix
	static proto void MatrixNormalize(vector mat[]);
	//! Scale matrix
	static proto void MatrixScale(vector mat[], float scale);
	/*!
	Creates identity quaternion.
	\code
		float q[4];
		Math3D.QuatIdentity( q );
		Print( q );

		>> {0,0,0,1}
	\endcode
	\param[out] q created identity quaternion
	*/
	static proto void QuatIdentity(out float q[4]);
	/*!
	Copies quaternion.
	\code
		float s[4] = { 2, 3, 4, 1 };
		float d[4];
		Math3D.QuatCopy( s, d );
		Print( d );

		>> {2,3,4,1}
	\endcode
	\param s quaternion to copy
	\param[out] d created quaternion copy
	*/
	static proto void QuatCopy(float s[4], out float d[4]);
	//! Converts quaternion to rotation matrix.
	static proto void QuatToMatrix(float q[4], out vector mat[3]);
	/*!
	Normalizes quaternion.
	\code
		float quat[4] = {1, 1, 1, 1};
		float length = QuatNormalize(quat);
		Print(length);
		>> {2}
		Print(quat);
		>> {0.5,0.5,0.5,0.5}
	\endcode
	\param quat in/out quaternion
	\return length
	*/
	static proto float QuatNormalize(out float quat[4]);
	/*!
	Linear interpolation between `q1` and `q2` with weight `frac` (0...1).
	\code
		float q1[4] = { 1, 1, 1, 1 };
		float q2[4] = { 2, 2, 2, 1 };
		float qout[4];
		Math3D.QuatLerp( qout, q1, q2, 0.5 );
		Print( qout );

		>> {1.5,1.5,1.5,1}
	\endcode
	\param[out] qout result quaternion
	\param q1 first quaternion
	\param q2 second quaternion
	\param frac interpolation weight
	*/
	static proto void QuatLerp(out float qout[4], float q1[4], float q2[4], float frac);
	/*!
	Multiplies quaternions.
	\code
		float q1[4] = { 1, 2, 3, 1 };
		float q2[4] = { 2, 2, 2, 1 };
		float qout[4];
		Math3D.QuatMultiply( qout, q1, q2 );
		Print( qout );

		>> {2,4,6,1}
	\endcode
	\param[out] qout result quaternion
	\param q1 first quaternion
	\param q2 second quaternion
	*/
	static proto void QuatMultiply(out float qout[4], float q1[4], float q2[4]);
	//! Returns a float value equal to the angle between two quaternions.
	static proto float QuatAngle(float q1[4], float q2[4]);
	//! Returns a float value equal to the dot product of two quaternions.
	static proto float QuatDot(float q1[4], float q2[4]);
	//! Returns a quaternion `qout` that is rotated between quaternions `q1` and `q2`.
	static proto void QuatRotateTowards(out float qout[4], float q1[4], float q2[4], float maxDegreesDelta);
	//! Returns the norm of a quaternion.
	static proto float QuatNorm(float quat[4]);
	//! Returns the magnitude of a quaternion.
	static proto float QuatLength(float quat[4]);
	//! Inverse quaternion.
	static proto void QuatInverse(out float qout[4], float q[4]);
	//! Multiply each part of quaternion by scalar.
	static proto void QuatScale(out float qout[4], float scale);
	//! Conjugate quaternion.
	static proto void QuatConjugate(out float qout[4], float q[4]);
	//! Returns Angles vector (yaw, pitch, roll) from quaternion.
	static proto vector QuatToAngles(float q[4]);
	/*!
	Obsolete, use CurveFloat/LegacyCurve.Curve
	*/
	[Obsolete("Use CurveFloat/LegacyCurve.Curve instead")]
	static proto vector Curve(ECurveType type, float param, notnull array<vector> points, array<float> knots = null);
	/*!
	Obsolete, use CurveVector/CurveColor/LegacyCurve.Curve3
	*/
	[Obsolete("Use CurveVector/CurveColor/LegacyCurve.Curve3 instead")]
	static proto vector Curve3(ECurveType type, float param, notnull array<float> points, int curveIndex, array<float> knots = null);
	/*!
	Generates tessellation of a cubic, C0 Bezier spline.
	\code
		auto points = new array<vector>();
		points.Insert(Vector(0, 0, -0.5));
		points.Insert(Vector(1, 0, 1.5));

		auto segmentPoints = new array<vector>();
		segmentPoints.Insert(Vector(1, 0, -1));
		segmentPoints.Insert(Vector(1, 0, 1));

		array<vector> outPoints = new array<vector>();
		Math3D.TesselateBezierSpline(points, segmentPoints, 0.05, 20, outPoints);
	\endcode
	\param points End points for each segment.
	\param segmentPoints Points between end points.
	\param maxDistance Maximum distance between two consecutive generated points.
	\param maxPointsPerSegment Maximum number of points to generate for each segment of the spline.
	\param outPoints Points of the resulting rasterization. Size of the array is changed.
	*/
	static proto void TessellateBezierSpline(array<vector> points, array<vector> segmentPoints, float maxDistance, int maxPointsPerSegment, array<vector> outPoints);
	/*!
	Generates tessellation of a cubic, C0 Hermite spline.
	\code
		auto points = new array<vector>();
		points.Insert(Vector(0, 0, -0.5));
		points.Insert(Vector(1, 0, 1.5));

		auto tangents = new array<vector>();
		tangents.Insert(Vector(1, 0, -1));
		tangents.Insert(Vector(1, 0, 1));

		array<vector> outPoints = new array<vector>();
		Math3D.TesselateHermiteSpline(points, tangents, 0.05, 20, outPoints);
	\endcode
	\param points End points for each segment.
	\param tangents Tangent vectors in respective points.
	\param maxDistance Maximum distance between two consecutive generated points.
	\param maxPointsPerSegment Maximum number of points to generate for each segment of the spline.
	\param outPoints Points of the resulting rasterization. Size of the array is changed.
	*/
	static proto void TessellateHermiteSpline(array<vector> points, array<vector> tangents, float maxDistance, int maxPointsPerSegment, array<vector> outPoints);
	/*!
	Convert polygon to triangles.
	\return number of generated vertices
	*/
	static proto int PolyToTriangles(vector in[], int num, out int indices[]);
	//! Calculates squared distance of `point` to a line segment given by points `v0` and `v1`.
	static proto float PointLineSegmentDistanceSqr(vector point, vector v0, vector v1);
	//! Calculates distance of `point` to a line segment given by points `v0` and `v1`.
	static proto float PointLineSegmentDistance(vector point, vector v0, vector v1);
	//! Calculates distance of `point` to a parametric line given by a point on line and direction vector.
	static proto float PointLineDistance(vector point, vector pointOnLine, vector dirVector);
	/*!
	Determines whether two line segments intersect or not.
	\param p11 First point of the first segment
	\param p12 Second point of the first segment
	\param p21 First point of the second segment
	\param p22 Second point of the second segment
	\return `true` if the segments intersect, `false` otherwise.
	*/
	static proto bool IntersectionLineSegments(vector p11, vector p12, vector p21, vector p22);
}

/*!
\}
*/

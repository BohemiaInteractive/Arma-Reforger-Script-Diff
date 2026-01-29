/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class vector
{
	private void vector();
	private void ~vector();

	static const vector Up = "0 1 0";
	static const vector Right = "1 0 0";
	static const vector Forward = "0 0 1";
	static const vector Zero = "0 0 0";
	static const vector One = "1 1 1";

	/*!
	\brief Normalizes vector.
		@code
			vector vec = "1 0 1";
			vec.Normalize();
			Print( vec );

			>> vec = <0.707107,0,0.707107>
		@endcode
	*/
	proto external void Normalize();
	/*!
	\brief Normalizes vector. Returns its length.
	\warning Slower than calling Normalize() because additional calculations are necessary. Use Normalize() if the vector length is not needed.
	\return \p float Length of origin vector
	@code
		vector vec = "1 0 1";
		float length = vec.NormalizeSize();
		Print( vec );
		Print( length );

		>> vec = <0.707107,0,0.707107>
		>> length = 1.41421
	@endcode
	*/
	proto external float NormalizeSize();
	//! return normalized vector (keeps the orginal vector untouched)
	proto external vector Normalized();
	/*!
	\brief Returns length of vector (magnitude)
		\return \p float Length of vector
		@code
			vector vec = "1 0 1";
			float length = vec.Length();
			Print( length );

			>> length = 1.41421
		@endcode
	*/
	proto external float Length();
	/*!
	\brief Returns squared length (magnitudeSqr)
		\return \p float Length of vector
		@code
			vector vec = "1 1 0";
			float length = vec.LengthSq();
			Print( length );

			>> length = 2
		@endcode
	*/
	proto external float LengthSq();
	/*!
	\brief Returns the distance between tips of two 3D vectors.
		\param v1 \p vector 3D Vector 1
		\param v2 \p vector 3D Vector 2
		\return \p float Distance
		@code
			float dist = vector.Distance( "1 2 3", "4 5 6" );
			Print( dist );

			>> dist = 5.19615
		@endcode
	*/
	static proto float Distance(vector v1, vector v2);
	static proto float DistanceXZ(vector v1, vector v2);
	/*!
	\brief Returns the squere distance between tips of two 3D vectors.
		\param v1 \p vector 3D Vector 1
		\param v2 \p vector 3D Vector 2
		\return \p float Squere distance
		@code
			float dist = vector.DistanceSq( "0 0 0", "0 5 0" );
			Print( dist );

			>> dist = 25
		@endcode
	*/
	static proto float DistanceSq(vector v1, vector v2);
	static proto float DistanceSqXZ(vector v1, vector v2);
	/*!
	\brief Returns perpendicular vector. Perpendicular vector is computed as cross product between input vector and up vector (0, 1, 0).
			\return \p vector perpendicular vector
		@code
			vector vec = "1 0 0";
			Print( vec.Perpend() );

			>> <0,0,1>
		@endcode
	*/
	proto external vector Perpend();
	/*!
	\brief Returns direction vector from point p1 to point p2.
		\warning Output vector is NOT normalized.
		\param p1 \p vector point from
		\param p2 \p vector point to
		\return \p vector direction vector
	*/
	static proto vector Direction(vector p1, vector p2);
	/*!
	\brief Returns Dot product of vector v1 and vector v2
			\param v1 \p vector input vector
		\param v2 \p vector input vector
		\return \p float dot product
	*/
	static proto float Dot(vector v1, vector v2);
	static proto float DotXZ(vector v1, vector v2);
	/*!
	\brief Re-map angles from one range to another.
		\return \p float re-maped angles
		@code
			vector angles = "-45 190 160";
			Print( angles.MapAngles(360, -Math.PI, Math.PI) );

			>> <-0.785398,-2.96706,2.79253>
		@endcode
	*/
	proto external vector MapAngles(float fFromRange = 360, float fToLow = -180, float fToHigh = 180);
	/*!
	Returns yaw of vector (use XZ plane).
	\return Yaw of vector in degrees
	\code
		vector v1 = "1 0 0";
		vector v2 = "0.7 0 0.7";
		Print( v1.ToYaw() );
		Print( v2.ToYaw() );

		>> 90
		>> 45
	\endcode
	*/
	proto external float ToYaw();
	/*!
	\brief Returns vector of yaw (in XZ plane)
		\param yaw \p float Value of yaw (in degrees)
		\return \p vector Yaw converted in vector (in XZ plane)
		@code
			Print( vector.FromYaw(90) );
			Print( vector.FromYaw(45) );

			>> <1,0,0>
			>> <0.707107,0,0.707107>
		@endcode
	*/
	static proto vector FromYaw(float yaw);
	/*!
	\brief Converts vector to spherical coordinates with radius = 1
		\return \p vector spherical coordinates (yaw, pitch, roll in degrees)
		@code
			vector v1 = "1 0 1";
			vector v2 = "1 1 1";
			Print( v1.VectorToAngles() );
			Print( v2.VectorToAngles() );

			>> <45,-0,0>
			>> <45,35.2644,0>
		@endcode
	*/
	proto external vector VectorToAngles();
	/*!
	\brief Converts spherical coordinates (yaw, pitch, roll in degrees) to unit length vector
		\return \p normalized direction vector
		@code
			vector v1 = "45 0 0";
			vector v2 = "15 60 0";
			Print( v1.AnglesToVector() );
			Print( v2.AnglesToVector() );

			>> <0.707107,0,0.707107>
			>> <0.12941,0.866025,0.482963>
		@endcode
	*/
	proto external vector AnglesToVector();
	/*!
	\brief Create quaternion from angles (yaw pitch roll in degrees)
		@code
					float q[4];
			vector ang = "70 15 45";
				ang.QuatFromAngles( q );
				Print( q );

			>> {-0.316402,0.484465,-0.241626,0.778973}
		@endcode
	*/
	proto external void QuatFromAngles(out float quat[4]);
	/*!
	Transforms position.
	\param mat transformation matrix
	\return transformed position
	\code
		vector mat[4] = { "1 0 0 0", "0 1 0 0", "0 0 1 1", "3 1 2 1" }; // translation matrix
		vector pos = "1 1 1";
		Print( pos.Multiply4(mat) );

		>> <4,2,3>
	\endcode
	*/
	proto external vector Multiply4(vector mat[4]);
	/*!
	Transforms vector.
	\param mat transformation matrix
	\return transformed vector
	\code
		vector mat[3] = { "2 0 0", "0 3 0", "0 0 1" }; // scale matrix
		vector vec = "1 1 1";
		Print( vec.Multiply3(mat) );

		>> <2,3,1>
	\endcode
	*/
	proto external vector Multiply3(vector mat[3]);
	/*!
	Invert-transforms position.
	\param mat transformation matrix
	\return transformed position
	\code
		vector mat[4] = { "1 0 0 0", "0 1 0 0", "0 0 1 1", "3 1 2 1" }; // translation matrix
		vector pos = "1 1 1";
		Print( pos.InvMultiply4(mat) );

		>> <-2,0,-1>
	\endcode
	*/
	proto external vector InvMultiply4(vector mat[4]);
	/*!
	Invert-transforms vector.
	\param mat transformation matrix
	\return Transformed vector
	\code
		vector mat[3] = { "1.5 2.5 0", "0.1 1.3 0", "0 0 1" }; // rotation matrix
		vector vec = "1 1 1";
		Print( vec.InvMultiply3(mat) );

		>> <4,1.4,1>
	\endcode
	*/
	proto external vector InvMultiply3(vector mat[3]);
	/*!
	\brief Lerp between two vectors
		@code
			vector v1 = Vector(0,0,0);
			vector v2 = Vector(5,6,1);
			Print( vector.Lerp(v1, v2, 0.5) );
		@endcode
	*/
	static proto vector Lerp(vector v1, vector v2, float t);
	proto external string ToString(bool beautify = true);
}

/*!
\}
*/

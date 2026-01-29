/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

class RandomGenerator: Managed
{
	proto external void SetSeed(int seed);
	/*!
	Generates random point in given polygon

	\param polygon Consecutive floats give polygon in 2D (2 floats = Vector2)
	\param bbMin Bounding box minimum corner
	\param bbMax Bounding box maximum corner
	\return Vector3 point in polygon
	*/
	proto external vector GenerateRandomPoint(array<float> polygon, vector bbMin, vector bbMax);
	/*!
	Generates a random point around `center` in range min/max radius

	\param minRadius All generated points will be at least this far from center
	\param maxRadius All generated points will be at most this far from center
	\param center Position around which to generate. Vector2 XZ
	\param uniform If false, has a small bias towards the center which may be desirable in some situations
	\return Vector2 XZ set, Y = 0
	*/
	proto external vector GenerateRandomPointInRadius(float minRadius, float maxRadius, vector center, bool uniform = true);
	//! Generates random float in [0, 1] range.
	proto external float RandFloat01();
	/*! Generates random float in [X, Y] range
	\param x \p float Range starts[inclusive]
	\param y \p float Range ends[inclusive]
	*/
	proto external float RandFloatXY(float x, float y);
	/*!
	\brief Returns random number with Gauss distribution (http://en.wikipedia.org/wiki/File:Normal_Distribution_PDF.svg)
	\param mean
	\param sigma
	*/
	proto external float RandGaussFloat(float sigma, float mean);
	/*!
	\brief Returns a random \p int number between and min [inclusive] and max [exclusive].
		\param min \p int Range starts [inclusive]
		\param max \p int Range ends [exclusive]
		\return \p int - Random number in range
		@code
			RandomGenerator gen = new RandomGenerator();
			Print( gen.RandInt(0, 1) );	// only 0
			Print( gen.RandInt(0, 2) );	// 0 or 1

			>> 0
			>> 1
		@endcode
	*/
	proto external int RandInt(int min, int max);
	/*!
	\brief Returns a random \p int number between and min [inclusive] and max [inclusive].
		\param min \p int Range starts [inclusive]
		\param max \p int Range ends [inclusive]
		\return \p int - Random number in range
		@code
			RandomGenerator gen = new RandomGenerator();
			Print( gen.RandIntInclusive(0, 1) );	// 0 or 1
			Print( gen.RandIntInclusive(0, 2) );	// 0, 1, 2

			>> 1
			>> 2
		@endcode
	*/
	proto external int RandIntInclusive(int min, int max);
}

/*!
\}
*/

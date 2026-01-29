/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Math
\{
*/

sealed class Math
{
	private void Math();
	private void ~Math();

	static const float PI = 3.14159265358979;
	static const float PI2 = 6.28318530717958;
	static const float PI_HALF = 1.570796326794;
	static const float RAD2DEG = 57.2957795130823208768;
	static const float DEG2RAD = 0.01745329251994329577;
	static const float E = 2.71828182845905;

	/*!
	Returns value `f` rounded to the closest whole number.
	\code
		Print( Math.Round(5.3) );
		Print( Math.Round(5.8) );

		>> 5
		>> 6
	\endcode
	*/
	static proto float Round(float f);
	/*!
	Returns value `f` rounded down to the closest whole number.
	\code
		Print( Math.Floor(5.3) );
		Print( Math.Floor(5.8) );

		>> 5
		>> 5
	\endcode
	*/
	static proto float Floor(float f);
	/*!
	Returns value `f` rounded up to the closest whole number.
	\code
		Print( Math.Ceil(5.3) );
		Print( Math.Ceil(5.8) );

		>> 6
		>> 6
	\endcode
	*/
	static proto float Ceil(float f);
	/*!
	Returns lower of two values `x` and `y`.
	\code
		Print( Math.Min(5.3, 2.8) );

		>> 2.8
	\endcode
	*/
	static proto float Min(float x, float y);
	/*!
	Returns larger of two values `x` and `y`.
	\code
		Print( Math.Max(5.3, 2.8) );

		>> 5.3
	\endcode
	*/
	static proto float Max(float x, float y);
	/*!
	Returns sine of an angle specified in radians.
	\code
		Print( Math.Sin(0.785398) ); // (45)

		>> 0.707107
	\endcode
	*/
	static proto float Sin(float angleRad);
	/*!
	Returns cosine of an angle specified in radians.
	\code
		Print( Math.Cos(0.785398) ); // (45)

		>> 0.707107
	\endcode
	*/
	static proto float Cos(float angleRad);
	/*!
	Returns tangent of an angle specified in radians.
	\code
		Print( Math.Tan(0.785398) ); // (45)

		>> 1
	\endcode
	*/
	static proto float Tan(float angleRad);
	/*!
	Returns angle in radians from sine.
	\code
		Print( Math.Asin(0.707107) ); // (sine 45)

		>> 0.785398
	\endcode
	*/
	static proto float Asin(float s);
	/*!
	Returns angle in radians from cosine.
	\code
		Print( Math.Acos(0.707107) ); // (cosine 45)

		>> 0.785398
	\endcode
	*/
	static proto float Acos(float c);
	/*!
	Returns angle in radians from tangent. Tangent is specified as 2D coordinates
	`[x,y]` of a point on circle centered at coordinates `[0,0]`.
	\code
		Print ( Math.Atan2(1, 1) );

		>> 0.785398
	\endcode
	\param y Y coordinate on the circle.
	\param x X coordinate on the circle.
	*/
	static proto float Atan2(float y, float x);
	/*!
	Returns `v` raised to the `power`.
	\code
		Print( Math.Pow(2, 4) ); // (2*2*2*2)=16

		>> 16
	\endcode
	*/
	static proto float Pow(float v, float power);
	/*!
	Returns remainder of division `v/div`.
	\code
		Print( Math.Mod(5.0, 2.0) );

		>> 1.0
	\endcode
	*/
	static proto float Mod(float v, float div);
	/*!
	Returns absolute value of `f`.
	\code
		Print( Math.AbsFloat(-12.5) );

		>> 12.5
	\endcode
	*/
	static proto float AbsFloat(float f);
	/*!
	Returns absolute value of `i`.
	\code
		Print( Math.AbsInt(-12) );

		>> 12
	\endcode
	*/
	static proto int AbsInt(int i);
	/*!
	Returns square root of `val`.
	\code
		Print( Math.Sqrt(25) );

		>> 5
	\endcode
	*/
	static proto float Sqrt(float val);
	/*!
	Returns the binary (base-`2`) logarithm of `x`.
	\code
		Print( Math.Log2(1.0) );

		>> 0.0
	\endcode
	\param x Value whose logarithm is calculated.
	\return Result depends on the value of `x`:
		- If `x` is positive, result is binary logarithm of `x`.
		- If `x` is negative or zero, an error occurs and result is undefined.
	*/
	static proto float Log2(float x);
	/*!
	Returns the common (base-`10`) logarithm of `x`.
	\code
		Print( Math.Log10(100.0) );

		>> 2.0
	\endcode
	\param x Value whose logarithm is calculated.
	\return Result depends on the value of `x`:
	- If `x` is positive, result is common logarithm of `x`.
	- If `x` is negative or zero, an error occurs and result is undefined.
	*/
	static proto float Log10(float x);
	/*!
	Returns the natural (base-`e`) logarithm of `x`.
	\code
		Print( Math.Log2(1.0) );

		>> 0.0
	\endcode
	\param x Value whose logarithm is calculated.
	\return Result depends on the value of `x`:
	- If `x` is positive, result is natural logarithm of `x`.
	- if `x` is negative or zero, an error occurs and result is undefined.
	*/
	static proto float Log(float x);
	/*!
	Does the CD smoothing function - easy in | easy out / S shaped smoothing
	\code
		val = EnfMath.SmoothCD(val, varTarget, valVelocity, 0.3, 1000, dt);
	\endcode
	\param val actual value
	\param target value we are reaching for -> Target
	\param velocity Acceleration/deceleration rate, need to be zeroed when filter is about to be reset.
	\param smoothTime Smoothing parameter, 0.1 .. 0.4 are reasonable values, 0.1 is sharp, 0.4 is very smooth.
	\param maxVelocity Maximum value change when multiplied by `dt`.
	\param dt Time delta.
	\return smoothed/filtered value
	*/
	static proto float SmoothCD(float val, float target, inout float velocity, float smoothTime, float maxVelocity, float dt);
	/*!
	SmoothCD() version that wraps around on `[-PI,PI]` range.
	\code
		val = EnfMath.SmoothCDPI2PI(val, varTarget, valVelocity, 0.3, 1000, dt);
	\endcode
	*/
	static proto float SmoothCDPI2PI(float val, float target, inout float velocity, float smoothTime, float maxVelocity, float dt);
	/*!
	Does spring smoothing function.
	\param val actual value
	\param target value we are reaching for -> Target
	\param velocity kind of memory and actual accel/decel rate, need to be zeroed when filter is about to be reset
	\param spring spring amount 0...1
	\param damping damper amount 0...1
	\param dt delta time
	*/
	static proto float SmoothSpring(float val, float target, inout float velocity, float spring, float damping, float dt);
	/*!
	Returns value (between -1 and 1) of Perlin noise for given parameters x, y, z
	To get 1D or 2D noise simply leave the respective parameters to 0 (y and z or just z)
	*/
	static proto float PerlinNoise(float x, float y = 0, float z = 0);
	//! PerlinNoise() but normalized to (0, 1).
	static proto float PerlinNoise01(float x, float y = 0, float z = 0);
	//! Returns value (between -1 and 1) of Simplex noise for given input `x`.
	static proto float SimplexNoise1D(float x);
	/*!
	Returns a random `int` number between `min` [inclusive] and `max` [exclusive].
	\code
		Print( Math.RandomInt(0, 1) );	// only 0
		Print( Math.RandomInt(0, 2) );	// 0 or 1

		>> 0
		>> 1
	\endcode
	\param min Range starts [inclusive]
	\param max Range ends [exclusive]
	\return Random number in range
	*/
	static proto int RandomInt(int min, int max);
	/*!
	Sets the seed for the random number generator.
	\code
	Print( Math.Randomize(5) );

	>> 5
	\endcode
	\param seed New seed for the random number generator or -1 to use current time as seed.
	\return Returns new seed.
	*/
	[Obsolete("If seed need to be used use RandomGenerator instead")]
	static proto int Randomize(int seed);
	/*!
	Returns a random `float` number between `min` [inclusive] and `max` [exclusive].
	\code
		Print( Math.RandomFloat(0,1) );
		Print( Math.RandomFloat(0,2) );

		>> 0.597561
		>> 1.936456
	\endcode
	\param min Range start [inclusive]
	\param max Range end [exclusive]
	\return Random number in range
	*/
	static proto float RandomFloat(float min, float max);
	//! Returns random number with Gauss distribution (http://en.wikipedia.org/wiki/File:Normal_Distribution_PDF.svg)
	static proto float RandomGaussFloat(float sigma, float mean);
	/*!
	Returns a random `int` number between `min` [inclusive] and `max` [inclusive].
	\code
		Print( Math.RandomIntInclusive(0, 1) );	// 0 or 1
		Print( Math.RandomIntInclusive(0, 2) );	// 0, 1, 2

		>> 1
		>> 2
	\endcode
	\param min Range start [inclusive]
	\param max Range end [inclusive]
	\return Random number in range
	*/
	static proto int RandomIntInclusive(int min, int max);
	/*!
	Returns a random `float` number between `min` [inclusive] and `max` [inclusive].
	\code
		Print( Math.RandomFloatInclusive(0, 1) );	// 0.0 .. 1.0
		Print( Math.RandomFloatInclusive(1, 2) );	// 1.0 .. 2.0

		>> 0.3
		>> 2.0
	\endcode
	\param min Range start [inclusive]
	\param max Range end [inclusive]
	\return Random number in range
	*/
	static proto float RandomFloatInclusive(float min, float max);
	/*!
	Returns a random `float` number in range `[0.0, 1.0]` inclusive on both ends.
	\code
		Print( Math.RandomFloat01() );	// 0.0 .. 1.0

		>> 0.3
		>> 1.0
	\endcode
	\return Random number in range 0.0 .. 1.0
	*/
	static proto float RandomFloat01();
	/*!
	Loop the value in given range (similar to modulo).
	\code
		Print( Math.Repeat(370, 360) );
		Print( Math.Repeat(-20, 360) );

		>> 10
		>> 340
	\endcode
	*/
	static proto float Repeat(float value, float range);
	/*!
	Re-maps a number from one range to another.
	\code
		Print( Math.Map(0, -5, 5, 100, 200) );

		>> 150
	\endcode
	*/
	static proto float Map(float value, float fromLow, float fromHigh, float toLow, float toHigh);
	/*!
	Re-maps angle from one range to another.
	\code
		Print( Math.MapAngle(-45, 360, -Math.PI, Math.PI) );

		>> -0.785398
	\endcode
	*/
	static proto float MapAngle(float value, float fFromRange = 360, float fToLow = -180, float fToHigh = 180);
	/*!
	Returns `true` when `value` is greater than or equal to `min` and less than or
	equal to `max`.
	\code
		Print( Math.IsInRange(-0.1, 0, 1) );
		Print( Math.IsInRange(2, 0, 1) );
		Print( Math.IsInRange(0.5, 0, 1) );
		Print( Math.IsInRange(0, 0, 1) );
		Print( Math.IsInRange(1, 0, 1) );

		>> false
		>> false
		>> true
		>> true
		>> true
	\endcode
	*/
	static proto bool IsInRange(float value, float min, float max);
	/*!
	Returns `true` when `value` is greater than `min` and less than `max`.
	\code
		Print( Math.IsInRange(-0.1, 0, 1) );
		Print( Math.IsInRange(2, 0, 1) );
		Print( Math.IsInRange(0.5, 0, 1) );
		Print( Math.IsInRange(0, 0, 1) );
		Print( Math.IsInRange(1, 0, 1) );

		>> false
		>> false
		>> true
		>> false
		>> false
	\endcode
	*/
	static proto bool IsInRangeSharp(float value, float min, float max);
	/*!
	Clamps `value` to range specified by `[min, max]`.
	- When `value` is less than `min`, returns `min`.
	- When `value` is greater than `max`, returns `max`.
	- In all other cases, returns `value`.

	\code
		Print( Math.Clamp(-0.1, 0, 1) );
		Print( Math.Clamp(2, 0, 1) );
		Print( Math.Clamp(0.5, 0, 1) );

		>> 0
		>> 1
		>> 0.5
	\endcode
	*/
	static proto float Clamp(float value, float min, float max);
	/*!
	Clamps `value` to range specified by `[min, max]`.
	- When `value` is less than `min`, returns `min`.
	- When `value` is greater than `max`, returns `max`.
	- In all other cases, returns `value`.

	\code
		Print( Math.ClampInt(-1, 0, 1) );
		Print( Math.ClampInt(2, 0, 1) );
		Print( Math.ClampInt(1, 0, 2) );

		>> 0
		>> 1
		>> 1
	\endcode
	*/
	static proto int ClampInt(int value, int min, int max);
	/*!
	Does linear interpolation between `a` and `b` using `t`.
	\code
		Print( Math.Lerp(3, 7, 0.5) );
		>> 5
	\endcode
	\param t Interpolation parameter, must be in range 0..1 [inclusive]
	*/
	static proto float Lerp(float a, float b, float t);
	/*!
	Calculates the interpolation parameter that produces the `value` by
	interpolating from `a` to `b`. It's an inverse of Lerp().
	\code
		Print( Math.InverseLerp(3, 7, 5) );
		>> 0.5
	\endcode
	*/
	static proto float InverseLerp(float a, float b, float value);
}

/*!
\}
*/

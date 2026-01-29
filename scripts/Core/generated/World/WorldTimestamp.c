/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

/*!
Type storing timestamps of the world.

When a world is created, its timestamp starts at 0. Internal timestamp format is
opaque and units used are not specified. However, timestamps guarantee enough
precision to provide results in milliseconds, and overflow effects will take at
least 100,000 (hundred thousand) years to become observable.

Note that timestamp precision is much higher than what can be represented
accurately by `float` before rounding occurs. For example, when storing duration
in milliseconds in a `float`, the highest value that can be stored before
rounding occurs is 16,777,216 ms (approximately 4.5 hours).
*/
sealed class WorldTimestamp: handle64
{
	proto external WorldTimestamp PlusSeconds(float seconds);
	proto external WorldTimestamp PlusMilliseconds(float milliseconds);
	proto external float DiffSeconds(WorldTimestamp other);
	proto external float DiffMilliseconds(WorldTimestamp other);
	proto external int Compare(WorldTimestamp other);
	proto external bool Equals(WorldTimestamp other);
	proto external bool Greater(WorldTimestamp other);
	proto external bool GreaterEqual(WorldTimestamp other);
	proto external bool Less(WorldTimestamp other);
	proto external bool LessEqual(WorldTimestamp other);
}

/*!
\}
*/

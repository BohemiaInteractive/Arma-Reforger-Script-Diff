/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class SoundWorldClass: GenericEntityClass
{
}

class SoundWorld: GenericEntity
{
	proto void GetMapValuesAtPos(vector pos, out float sea, out float forest, out float city, out float meadow);
	proto void GetMapValuesAdvAtPos(vector pos, out float river, out float riverSlope, out float lake, out float coast);
	proto void CalculateInterirorAt(vector pos, InteriorRequestCallback callback);
}

/*!
\}
*/

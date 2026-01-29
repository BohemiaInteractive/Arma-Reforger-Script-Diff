/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Sound
\{
*/

class AudioSourceConfig: ScriptAndConfig
{
	static const int AS_Static = 1;
	static const int AS_TerminateOnDestroy = 2;
	static const int AS_Distance = 4;
	static const int AS_Obstruction = 8;
	static const int AS_InteriorValues = 16;
	static const int AS_BoundingVolume = 32;

	proto external void SetResource(string resourceName);
	proto external void SetEvent(string eventName);
	proto external void SetOffset(vector offset);
	proto external void SetFlags(int flags);
}

/*!
\}
*/

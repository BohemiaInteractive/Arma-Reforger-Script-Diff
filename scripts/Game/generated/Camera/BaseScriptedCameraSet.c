/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class BaseScriptedCameraSet
{
	//! inserts camera into set
	proto external void RegisterCameraCreator(int cameraId, typename pCameraType);

	// callbacks

	//! called when camera set is created - used for adding camera items and for transition between cameras settings
	event void Init();
	event protected float OnCameraChange(int pFrom, int pTo) { return 0.3; };
}

/*!
\}
*/

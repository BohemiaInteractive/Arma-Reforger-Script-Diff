/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class PreviewRenderAttributes: BaseItemAttributeData
{
	/*!
	Add the rotation to the current rotation of the camera used for previewing
	\param deltaRotation The rotation that should be added in degrees
	\param limitMin Minimum limit of the X axis of the camera
	\param limitMax Maximum limit of the Y axis of the camera
	*/
	proto external void RotateItemCamera(vector deltaRotation, vector limitMin, vector limitMax);
	/*!
	Reset the camera to it's initial rotation
	*/
	proto external void ResetDeltaRotation();
	/*!
	Add FOV to the current camera
	\param zoomIncrement The increment needed
	\param minFOV Minimum FOV
	\param maxFOV Maximum FOV
	*/
	proto external void ZoomCamera(float zoomIncrement, float minFOV = 25.0, float maxFOV = 120.0);
}

/*!
\}
*/

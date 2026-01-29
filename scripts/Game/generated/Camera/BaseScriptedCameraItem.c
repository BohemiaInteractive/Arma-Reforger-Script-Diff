/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class BaseScriptedCameraItem
{
	//! virtual callback - called when camera is created
	event void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult);
	event void OnDeactivate(ScriptedCameraItem pNextCamera);
	event void OnBlendIn();
	event void OnBlendOut();
	//!	virtual callback - called each frame
	event void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult);
	//! Sets the camera angles in DEG, useful when switching between cameras and changing orientation of the character
	event void SetBaseAngles(out vector angles);
}

/*!
\}
*/

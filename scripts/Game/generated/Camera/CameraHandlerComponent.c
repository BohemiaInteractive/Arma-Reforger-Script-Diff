/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class CameraHandlerComponent: ScriptComponent
{
	proto external ScriptedCameraSet GetCameraSet();
	//! Returns true if the camera is in third person mode
	proto external bool IsInThirdPerson();
	//! Set the current third person camera state
	proto external void SetThirdPerson(bool val);
	//! Returns the current value of the focus mode
	proto external float GetFocusMode();
	//! Set the current value of the focus mode
	proto external void SetFocusMode(float focusMode);
	proto external float GetBlendAlpha(ScriptedCameraItem forCameraItem);
	//! returns current active camera
	proto external ScriptedCameraItem GetCurrentCamera();
	//! returns true if there's more than one processing camera
	proto external bool IsCameraBlending();
	/*!
	Sets camera lens flare set
	\param type type of the lens flare set
	\param userSetName user name of the lens flare set (used only when type is set to User)
	*/
	proto external void SetLensFlareSet(CameraLensFlareSetType type, string lensFlareSetUserName);

	// callbacks

	/*!
	Calculate and return desired fov scalar. 1.0 = normal
	\param mainCamera Main camera used, usually player camera
	\param overlayCamera Overlay camera, usually PiP camera or null
	*/
	event protected float CalculateFovScalar(notnull CameraBase mainCamera, CameraBase overlayCamera) { return 1.0; };
	//! initialization
	event void Init();
	//! activation
	event void OnCameraActivate();
	//! deactivation
	event void OnCameraDeactivate();
	//! selects next camera
	event int CameraSelector();
	//! before camera update
	event void OnBeforeCameraUpdate(float pDt, bool pIsKeyframe);
	//! after camera update
	event void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4], inout vector transformWS[4]);
	//! collision solver
	event void CollisionSolver(float pDt, inout ScriptedCameraItemResult pOutResult, inout vector resCamTM[4], bool isKeyframe);
	//! when we switch between FPP and TPP
	event void OnThirdPersonSwitch(bool isInThirdPerson);
	//! get how long should transition between given cameras be
	event float GetCameraTransitionTime(int pFrom, int pTo);
	/*!
	Weight in the <0, 1> range of how much input sensitivity should be
	modulated if an overlay camera is active. (0 = main camera, 1 = overlay camera).
	*/
	event float GetOverlayCameraFOVScalarWeight();
}

/*!
\}
*/

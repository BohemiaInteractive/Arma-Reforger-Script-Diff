/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class CameraManagerClass: GenericEntityClass
{
}

class CameraManager: GenericEntity
{
	//! Returns the current camera
	proto external CameraBase CurrentCamera();
	/*!
	Set the current camera, it should be alreday registered in this Camera Manager.
	\param cam Camera which will be the new camera used.
	*/
	proto external bool SetCamera(CameraBase pCam);
	//! Set the next camera in the list as current camera.
	proto external void SetNextCamera();
	//! Set the previous camera in the list as current camera.
	proto external void SetPreviousCamera();
	/*!
	Return the list of all cameras
	\param outCameras List where all cameras will be inserted.
	\return The number of camera in the list
	*/
	proto external int GetCamerasList(out notnull array<CameraBase> outCameras);
	/*!
	Set the current overlay camera
	\param pCamera Camera which will be the new overlay camera.
	*/
	proto external void SetOverlayCamera(CameraBase pCam);
	/*!
	Get the current overlay camera
	\return Returns the current overlay canera or null if none.
	*/
	proto external CameraBase GetOverlayCamera();
	/*!
	Sets default first person field of view.
	*/
	proto external void SetFirstPersonFOV(float fieldOfView);
	/*!
	Returns default first person field of view.
	*/
	proto external float GetFirstPersonFOV();
	/*!
	Sets default first person field of view.
	*/
	proto external void SetThirdPersonFOV(float fieldOfView);
	/*!
	Returns default third person field of view.
	*/
	proto external float GetThirdPersonFOV();
	/*!
	Sets default vehicle field of view.
	*/
	proto external void SetVehicleFOV(float fieldOfView);
	/*!
	Returns default vehicle field of view.
	*/
	proto external float GetVehicleFOV();
	/*!
	Returns value clamped between min and max settings.
	*/
	proto external float GetClampedFOV(float fieldOfView);
	/*!
	Returns minimum allowed field of view value.
	*/
	proto external float GetMinFOV();
	/*!
	Returns maximum allowed field of view value.
	*/
	proto external float GetMaxFOV();

	// callbacks

	/*!
	Called on initialization.
	*/
	event protected void OnInit();
}

/*!
\}
*/

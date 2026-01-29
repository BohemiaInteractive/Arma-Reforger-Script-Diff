/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class CameraBaseClass: GenericEntityClass
{
}

class CameraBase: GenericEntity
{
	//! Manually applies transformation. This is usually done in EOnPostFrame automatically.
	proto external void ApplyTransform(float timeslice);
	//! Get current camera world transformation
	proto external void GetWorldCameraTransform(out vector mat[4]);
	/*!
	Performs a raycast from the center of this camera in the forward direction. (result is cached within single frame and reused if called multiple times)
	\return Reference to an entity if it was hit by the trace or null otherwise.
	*/
	proto external IEntity GetCursorTarget();
	/*!
	Performs a raycast from the center of this camera in the forward direction. (result is cached within single frame and reused if called multiple times)
	\param outPosition Position of the ray and target intersect point.
	\return Reference to an entity if it was hit by the trace or null otherwise.
	*/
	proto external IEntity GetCursorTargetWithPosition(out vector outPosition);
	//! Get projection type
	proto external CameraType GetProjType();
	/*!
	Get the vertical FOV.
	\return Returns the vertical FOV in degrees.
	*/
	proto external float GetVerticalFOV();
	/*!
	Get the current near plane clipping value.
	\return Returns the near plane value in meters.
	*/
	proto external float GetNearPlane();
	/*!
	Get the current far plane clipping value.
	\return Returns the far plane value in meters.
	*/
	proto external float GetFarPlane();
	/*!
	Set full symmetrical vertical FOV in degrees.
	\param fovDegrees Vertical FOV in degrees.
	*/
	proto external void SetFOVDegree(float fovDegrees);
	/*!
	Set the vertical FOV.
	\param fov FOV in degrees.
	*/
	proto external void SetVerticalFOV(float fov);
	/*!
	Set the current near plane clipping value.
	\param nearPlane The new near plane value in meters.
	*/
	proto external void SetNearPlane(float nearPlane);
	/*!
	Set the current far plane value.
	\param farPlane The new far plane value in meters.
	*/
	proto external void SetFarPlane(float farPlane);
	/*!
	Set the current camera index.
	\param cameraIndex The new camera index.
	*/
	proto external void SetCameraIndex(int cameraIndex);
	/*!
	Get the current camera index.
	\return Returns the camera index.
	*/
	proto external int GetCameraIndex();
	/*!
	Sets camera lens flare set
	\param type type of the lens flare set
	\param userSetName user name of the lens flare set (used only when type is set to User)
	*/
	proto external void SetLensFlareSet(CameraLensFlareSetType type, string userName);
	//! Test if the AABB is visible against the view frustum of the camera
	proto external bool IsAABBVisible(vector mins, vector maxs);
	//! Test if the sphere is visible against the view frustum of the camera
	proto external bool IsSphereVisible(vector origin, float radius);

	// callbacks

	//! Called the moment this camera is activated by CameraManager.
	event protected void CameraActivate();
	//! Called the moment this camera is deactivated by CameraManager.
	event protected void CameraDeactivate();
}

/*!
\}
*/

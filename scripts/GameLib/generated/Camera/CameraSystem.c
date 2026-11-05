/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class CameraSystem: WorldSystem
{
	// operations on camera (active entity) stack
	proto external IEntity GetCamera();
	proto external bool SetCamera(IEntity entity, float transitionTime, ECameraTransitionCurve transitionCurve = ECameraTransitionCurve.S_CURVE);
	proto external bool SwitchCamera(IEntity entity, float transitionTime, ECameraTransitionCurve transitionCurve = ECameraTransitionCurve.S_CURVE);
	proto external bool PushCamera(IEntity entity, float transitionTime, ECameraTransitionCurve transitionCurve = ECameraTransitionCurve.S_CURVE, bool popMoreRecentIfAlreadyPresent = false);
	// if entity == nullptr top-most entity gets removed
	proto external bool PopCamera(IEntity entity, float transitionTime, ECameraTransitionCurve transitionCurve = ECameraTransitionCurve.S_CURVE, bool popMoreRecent = true);
	proto external bool IsBlending();
	proto external IEntity GetPrioritizedCamera();
	proto external bool SetPrioritizedCamera(IEntity entity);
	proto external float GetCurrentCameraFOV();
	proto external float GetCurrentCameraNearPlane();
	proto external float GetCurrentCameraFarPlane();
	proto external float GetCurrentCameraObjectsFarPlane();
	static proto float GetCameraFOV(IEntity cameraEntity);
	static proto float GetCameraNearPlane(IEntity cameraEntity);
	static proto float GetCameraFarPlane(IEntity cameraEntity);
	static proto float GetCameraObjectsFarPlane(IEntity cameraEntity);
	proto external bool IsPointVisibleForCurrentCamera(vector point);
	proto external int GetCameras(notnull array<IEntity> outCameras);
	proto void GetCurrentCameraTransform(out vector mat[4]);
	proto void GetCurrentCameraLocalTransform(out IEntity parentEntity , out vector mat[4]);
	proto void GetCameraTransform(IEntity cameraEntity, out vector mat[4]);
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class GenericCameraEntityClass: GenericEntityClass
{
}

class GenericCameraEntity: GenericEntity
{
	proto external float GetFarPlane();
	proto external float GetObjectsFarPlane();
	proto external float GetNearPlane();
	proto external float GetFOV();

	// callbacks

	//! Callback for far plane
	event float OnGetFarPlane();
	//! Callback for object far plane
	event float OnGetObjectsFarPlane();
	//! Callback for near plane
	event float OnGetNearPlane();
	//! Callback for FOV
	event float OnGetFOV();
}

/*!
\}
*/

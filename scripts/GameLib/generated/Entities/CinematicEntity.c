/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class CinematicEntityClass: GenericEntityClass
{
}

class CinematicEntity: GenericEntity
{
	proto external void Play();
	proto external void Pause();
	proto external void Stop();
	proto external bool IsPlaying();
	proto external bool Isfinished();
	proto external void AttachCameraToEntity(IEntity ent, int pivotIndex, vector positionOffset, vector anglesOffset);
	proto external void DetachCamera();
	proto external void PlayCameraAnimation(ResourceName animFile, string boneName, float startTimeOffset);
	proto external void PlayCameraAnimationExt(ResourceName animFile, string boneName, float startTimeOffset, vector upVec, vector asideVec);
	proto external void StopCameraAnimation();
	proto external void SetOnFinishedCallback(func fn);
}

/*!
\}
*/

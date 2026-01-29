/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Particles
\{
*/

class ParticleEffectEntityClass: GenericEntityClass
{
}

class ParticleEffectEntity: GenericEntity
{
	/*!
	Spawn a particle effect from an effectPath given as parameter.
	\param effectPath Could be a .ptc path or a prefab path
	*/
	static proto ParticleEffectEntity SpawnParticleEffect(ResourceName effectPath, notnull ParticleEffectEntitySpawnParams spawnParams);
	proto external EParticleEffectState GetState();
	//! It returns the time since it started playing in seconds.
	proto external float GetTotalSimulationTime();
	proto external bool HasActiveParticles();
	/*!
	Set if we should use FRAME instead of the VISIBLE event flag to be updated.
	This means if we should update only when we are rendered, or not.
	*/
	proto external void SetUseFrameEvent(bool useFrameEvent);
	proto external void SetPlayOnHeadlessClient(bool playOnHeadlessClient);
	proto external void SetEffectPath(ResourceName effectPath);
	//! Tells if we should be deleted when it is finished or when the emission is stopped.
	proto external void SetDeleteWhenStopped(bool deleteWhenStopped);
	proto external void SetUseParentAsVelocitySource(bool useParentAsVelocitySource);
	//! Follow the parent, if pivotId != -1, local transform is local to the pivot otherwise local to the entity
	proto external void SetFollowParent(IEntity parent, vector localTransform[4], int pivotId = -1);
	//! Play the current particle. If paused, then it resumes it.
	proto external void Play();
	//! Stop the current particle.
	proto external void Stop();
	//! Pause the current particle.
	proto external void Pause();
	//! Stop the emitters' emission, but update ourselves until it hasn't active particles.
	proto external void StopEmission();

	// callbacks

	event void OnUpdateEffect(float timeSlice);
	event void OnStateChanged(EParticleEffectState oldState, EParticleEffectState newState);
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Particles
\{
*/

sealed class ParticleEffectEntitySpawnParams
{
	//! Entity type to be spawned if a .ptc file is given as an effect path. It must inherit from the ParticleEffectEntity type.
	typename		Type;
	//! Which world where the entity should be spawned? If null, then the current game world is taken.
	BaseWorld		TargetWorld = null;
	ETransformMode	TransformMode = ETransformMode.LOCAL;
	vector			Transform[4];
	//! Which parent the entity should be attached to?
	IEntity			Parent = null;
	//! Which bone should be attached to the parent?
	int				PivotID = -1;
	//! Should the entity follow his parent, when the parent is moved?
	bool			AutoTransform = true;
	/*!
	Which parent the entity should be following to? Difference between Parent and FollowParent is that FollowParent is not directly attached inside the entity hierarchy of the parent.
	Imagine a rocket trace particle effect. It should follow the rocket, but not get deleted as soon as the rocket is deleted. It needs to still simulate until there is no particle anymore.
	*IMPORTANT* Parent and FollowParent are exclusive. Parent takes the priority in case both are provided.
	*/
	IEntity			FollowParent = null;
	/*!
	Should the particle be played instantly when spawned?
	*IMPORTANT* Not used if a prefab is passed as effectPath.
	*/
	bool			PlayOnSpawn = true;
	/*!
	Should the particle be played on dedicated server?
	*IMPORTANT* Not used if a prefab is passed as effectPath.
	*/
	bool			PlayOnHeadlessClient = false;
	/*!
	Set if we should use FRAME instead of the VISIBLE event flag to be updated.
	This means we should update only when we are rendered, or not.
	*IMPORTANT* Not used if a prefab is passed as effectPath.
	*/
	bool			UseFrameEvent = false;
	/*!
	Should the entity be deleted when stopped?
	*IMPORTANT* Not used if a prefab is passed as effectPath.
	*/
	bool			DeleteWhenStopped = true;
	/*!
	Should the parent entity be used as a velocity source?
	*IMPORTANT* Not used if a prefab is passed as effectPath.
	*/
	bool			UseParentAsVelocitySource = true;

	void ParticleEffectEntitySpawnParams()
	{
		Math3D.MatrixIdentity4(Transform);
	}

}

/*!
\}
*/

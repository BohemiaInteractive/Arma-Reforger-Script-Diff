//------------------------------------------------------------------------------------------------
// THIS FILE CONTAINS COMMON FUNCTIONS USED BY THE VARIOUS DESTRUCTION SYSTEMS
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//! Class containing common functions between destructible classes
class SCR_DestructionCommon
{
	static const float PARTICLE_IMPULSE_DEFAULT = 0.5;
	static const float PARTICLE_IMPULSE_INCENDIARY = 1;
	static const float PARTICLE_IMPULSE_EXPLOSION = 1;
	static const float PARTICLE_IMPULSE_COLLISION = 0.5;
	
	//------------------------------------------------------------------------------------------------
	//! Returns particle velocity impulse multiplier for the given damage type
	static float GetPTCImpulseScale(EDamageType type)
	{
		float impulseScale = PARTICLE_IMPULSE_DEFAULT;
		
		switch (type)
		{
			case EDamageType.INCENDIARY:
			{
				impulseScale = PARTICLE_IMPULSE_INCENDIARY;
				break;
			}
			case EDamageType.EXPLOSIVE:
			{
				impulseScale = PARTICLE_IMPULSE_EXPLOSION;
				break;
			}
			case EDamageType.COLLISION:
			{
				impulseScale = PARTICLE_IMPULSE_COLLISION;
				break;
			}
		}
		
		return impulseScale;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect to represent destruction of a fraction (shard/splinter) of an object
	static ParticleEffectEntity PlayParticleEffect_FractionDestruction(IEntity entity, ResourceName particlePath, EDamageType type, vector hitPos, vector hitDir)
	{
		if (!entity)
			return null;
		
		if (particlePath == ResourceName.Empty)
			return null;
		
		vector fw = hitDir * 0.5 + entity.GetWorldTransformAxis(2);
		fw.Normalize();
		vector rt = vector.Up * fw;
		rt.Normalize();
		vector up = fw * rt;
		up.Normalize();
		
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.Transform[0] = rt;
		spawnParams.Transform[1] = up;
		spawnParams.Transform[2] = fw;
		spawnParams.Transform[3] = hitPos;
		spawnParams.UseFrameEvent = true;
		
		ParticleEffectEntity ptc = ParticleEffectEntity.SpawnParticleEffect(particlePath, spawnParams);
		if (!ptc)
			return null;
		
		float velocityScale = GetPTCImpulseScale(type);
		Particles particles = ptc.GetParticles();
		particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		
		return ptc;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect to represent entire object destruction
	static ParticleEffectEntity PlayParticleEffect_CompleteDestruction(IEntity entity, ResourceName particlePath, EDamageType damageType, bool atBoundBoxCenter = true)
	{
		if (!entity)
			return null;
		
		if (particlePath == ResourceName.Empty)
			return null;
		
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		entity.GetWorldTransform(spawnParams.Transform);
		spawnParams.UseFrameEvent = true;
		
		if (atBoundBoxCenter)
		{
			vector mins, maxs;
			entity.GetBounds(mins, maxs);
			vector center = (maxs - mins) * 0.5 + mins;
			spawnParams.Transform[3] = center.Multiply4(spawnParams.Transform);
		}
		
		ParticleEffectEntity ptc = ParticleEffectEntity.SpawnParticleEffect(particlePath, spawnParams);
		if (!ptc)
			return null;
		
		float velocityScale = GetPTCImpulseScale(damageType);
		Particles particles = ptc.GetParticles();
		particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		
		return ptc;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect as child of entity
	static ParticleEffectEntity PlayParticleEffect_Child(ResourceName particlePath, EDamageType damageType, notnull IEntity parent, vector mat[4])
	{
		if (particlePath == ResourceName.Empty)
			return null;
		
		vector parentTransform[4];
		parent.GetWorldTransform(parentTransform);
		
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		Math3D.MatrixInvMultiply4(parentTransform, mat, spawnParams.Transform);
		spawnParams.Parent = parent;
		spawnParams.UseFrameEvent = true;
		
		ParticleEffectEntity ptc = ParticleEffectEntity.SpawnParticleEffect(particlePath, spawnParams);
		if (!ptc)
			return null;
		
		float velocityScale = GetPTCImpulseScale(damageType);
		Particles particles = ptc.GetParticles();
		particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		
		return ptc;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a particle effect at input transformation matrix
	static ParticleEffectEntity PlayParticleEffect_Transform(ResourceName particlePath, EDamageType damageType, vector mat[4])
	{
		if (particlePath == ResourceName.Empty)
			return null;
		
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.Transform = mat;
		spawnParams.UseFrameEvent = true;
		
		ParticleEffectEntity ptc = ParticleEffectEntity.SpawnParticleEffect(particlePath, spawnParams);
		if (!ptc)
			return null;
		
		float velocityScale = GetPTCImpulseScale(damageType);
		Particles particles = ptc.GetParticles();
		if (particles)
		{
			particles.MultParam(-1, EmitterParam.VELOCITY,     velocityScale);
			particles.MultParam(-1, EmitterParam.VELOCITY_RND, velocityScale);
		}
		
		return ptc;
	}
};

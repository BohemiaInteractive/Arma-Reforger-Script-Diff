class VehicleBodyEffectBaseClass: ParticleEffectEntityClass
{
};

class VehicleBodyEffectBase : ParticleEffectEntity
{
	ScriptComponent m_ComponentOwner;
	
	void VehicleBodyEffectBase(IEntitySource src, IEntity parent)
	{}
	
	void UpdateVehicleDustEffect(float speed, float start_speed, float end_speed)
	{
		float size_coef = Math.AbsFloat(  0.2 + ( (speed - start_speed)*0.8)  / end_speed );
		float speed_coef = Math.AbsFloat( 1 + ( (speed - start_speed)*0.5)  / end_speed );
		float gravity_coef = Math.AbsFloat( 0.8 + ( (speed - start_speed)*0.2)  / end_speed );
		
		Particles particles = GetParticles();
		particles.MultParam(-1,	EmitterParam.SIZE,              size_coef);
		particles.MultParam(-1,	EmitterParam.GRAVITY_SCALE_RND, gravity_coef);
		particles.MultParam(-1,	EmitterParam.VELOCITY,          speed_coef);
		particles.MultParam(-1,	EmitterParam.VELOCITY_RND,      speed_coef);
	}
};
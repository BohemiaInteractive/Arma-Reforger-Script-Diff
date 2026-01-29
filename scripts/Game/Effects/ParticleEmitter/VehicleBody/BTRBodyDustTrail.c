class BTRBodyDustTrailClass: VehicleBodyEffectBaseClass
{
};

class BTRBodyDustTrail : VehicleBodyEffectBase
{
	void BTRBodyDustTrail(IEntitySource src, IEntity parent)
	{
		SetEffectPath("{21AB2949F7DAD15A}Particles/Vehicle/Vehicle_dust_car_BTR_wheel_default.ptc");
	}
	
	override void UpdateVehicleDustEffect(float speed, float start_speed, float end_speed)
	{
		float size_coef = Math.AbsFloat(  0.5 + ( (speed - start_speed)*0.5)  / end_speed );
		float speed_coef = Math.AbsFloat(  (speed - start_speed)  / end_speed );
		float gravity_coef = Math.AbsFloat(  (speed - start_speed)  / end_speed );
		
		size_coef = Math.Clamp(size_coef , 0, 1);
		speed_coef = Math.Clamp(speed_coef , 0, 1);
		gravity_coef = Math.Clamp(gravity_coef , 0, 1);
		
		
		Particles particles = GetParticles();
		particles.MultParam(-1, EmitterParam.SIZE,              size_coef);
		particles.MultParam(-1, EmitterParam.GRAVITY_SCALE_RND, gravity_coef);
		particles.MultParam(-1, EmitterParam.VELOCITY,          speed_coef);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND,      speed_coef);
	}
};
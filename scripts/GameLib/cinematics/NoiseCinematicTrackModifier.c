class NoiseCinematicTrackModifier : CinematicTrackModifier
{
	[Attribute(defvalue:"0.1", params:"0 inf", desc:"How big the movement will be")]
	float Strength;
	
	[Attribute(defvalue:"2", params:"0 inf", desc:"How quick the movement will be")]
	float Speed;
	
	[Attribute(defvalue:"0", desc:"Offset for starting time")]
	float Offset;
	
	//------------------------------------------------------------------------------------------------
	override bool OnApplyModifierBool(float time, bool originalValue)
	{
		float t = Offset + time*Speed;
		float noise = Math.PerlinNoise(t);
		
		return noise > 0.0;	
	}
	
	//------------------------------------------------------------------------------------------------
	override int OnApplyModifierInt(float time, int originalValue)
	{
		float t = Offset + time*Speed;
		float noise = Math.PerlinNoise(t);
		
		return originalValue + Math.Round(noise*Strength);
	}
	
	//------------------------------------------------------------------------------------------------
	override float OnApplyModifierFloat(float time, float originalValue)
	{
		float t = Offset + time*Speed;
		float noise = Math.PerlinNoise(t);
		
		return originalValue + noise*Strength;
	}
	
	//------------------------------------------------------------------------------------------------
	override vector OnApplyModifierVector(float time, vector originalValue)
	{
		float t = Offset + time*Speed;
		
		vector noise = Vector(
			Math.PerlinNoise(t, 0, 0),
			Math.PerlinNoise(0, t, 0),
			Math.PerlinNoise(0, 0, t));
		
		return originalValue + noise*Strength;
	}
};

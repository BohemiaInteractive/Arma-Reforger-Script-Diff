class SCR_BayonetComponentClass : ScriptComponentClass
{
}

class SCR_BayonetComponent : ScriptComponent
{	
	//! Blood visibility steps - cannot be 0
	protected const int BLOOD_STEP_SIZE = 10;
	protected const float MAX_BLOOD_LEVEL = 255.0;
	
	protected float m_fBloodStainLevel = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Adds blood texture to bayonet
	void AddBloodToBayonet()
	{
		Rpc_AddBloodToBayonet();
		Rpc(Rpc_AddBloodToBayonet);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddBloodToBayonet()
	{
		if (m_fBloodStainLevel >= MAX_BLOOD_LEVEL)
			return;
		
		ParametricMaterialInstanceComponent materialComponent = ParametricMaterialInstanceComponent.Cast(GetOwner().FindComponent(ParametricMaterialInstanceComponent));
		if (!materialComponent)
			return;
		
		m_fBloodStainLevel += MAX_BLOOD_LEVEL / BLOOD_STEP_SIZE;
		
		materialComponent.SetUserParam2(Math.Clamp(m_fBloodStainLevel, 0.0, MAX_BLOOD_LEVEL));
	}
}

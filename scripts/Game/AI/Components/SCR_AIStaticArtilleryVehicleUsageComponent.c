enum SCR_EAIArtilleryAmmoType
{
	HIGH_EXPLOSIVE,
	SMOKE,
	ILLUMINATION,
	PRACTICE
}

class SCR_AIStaticArtilleryVehicleUsageComponentClass : SCR_AIVehicleUsageComponentClass
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "These prefabs are used by AI to find ammo of specific type.", params: "et", category: "Ammo Prefabs")]
	protected ResourceName m_sAmmoHighExplosive;
	
	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Ammo Prefabs")]
	protected ResourceName m_sAmmoSmoke;
	
	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Ammo Prefabs")]
	protected ResourceName m_sAmmoIllumination;
	
	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Ammo Prefabs")]
	protected ResourceName m_sAmmoPractice;
	
	//----------------------------------------------------------------------------------------
	ResourceName GetAmmoResourceName(SCR_EAIArtilleryAmmoType ammoType)
	{
		switch (ammoType)
		{
			case SCR_EAIArtilleryAmmoType.HIGH_EXPLOSIVE:	return m_sAmmoHighExplosive;
			case SCR_EAIArtilleryAmmoType.SMOKE:			return m_sAmmoSmoke;
			case SCR_EAIArtilleryAmmoType.ILLUMINATION:		return m_sAmmoIllumination;
			case SCR_EAIArtilleryAmmoType.PRACTICE:			return m_sAmmoPractice;
		}
		return string.Empty;
	}
}

class SCR_AIStaticArtilleryVehicleUsageComponent : SCR_AIVehicleUsageComponent
{
	//----------------------------------------------------------------------------------------
	ResourceName GetAmmoResourceName(SCR_EAIArtilleryAmmoType ammoType)
	{
		SCR_AIStaticArtilleryVehicleUsageComponentClass componentData = SCR_AIStaticArtilleryVehicleUsageComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return string.Empty;
		return componentData.GetAmmoResourceName(ammoType);
	}
}
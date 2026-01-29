enum SCR_EQRFGroupType
{
	INFANTRY = 1 << 0,
	MOUNTED_INFANTRY = 1 << 1,
	VEHICLE = 1 << 2,
	AIR = 1 << 3,
}

class SCR_QRFGroupConfig : ScriptAndConfig
{
	[Attribute(desc: "Group/unit prefab")]
	protected ResourceName m_sGroupPrefab;// (f.e. {E552DABF3636C2AD}Prefabs/Groups/OPFOR/Group_USSR_RifleSquad.et),

	[Attribute(SCR_EQRFGroupType.INFANTRY.ToString(), UIWidgets.ComboBox, desc: "Type of this group which will be used to determine if they can spawn for provided spawnpoint", enums: ParamEnumArray.FromEnum(SCR_EQRFGroupType))]
	protected SCR_EQRFGroupType m_eGroupType;// (INFANTRY/VEHICLE/AIR),

	[Attribute("1", UIWidgets.Auto, "Cost of spawning such QRF group")]
	protected int m_iSpawnCost;// (0/1/2/3...),

	[Attribute("-1", UIWidgets.Auto, "How many times this QRF unit can be used until we will run out of them where -1 == unlimited")]
	protected int m_iNumberOfAvailableGroups;// (0 unlimited, 1/2/3...)

	//------------------------------------------------------------------------------------------------
	ResourceName GetGroupPrefabName()
	{
		return m_sGroupPrefab;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EQRFGroupType GetGroupType()
	{
		return m_eGroupType;
	}

	//------------------------------------------------------------------------------------------------
	int GetSpawnCost()
	{
		return m_iSpawnCost;
	}

	//------------------------------------------------------------------------------------------------
	int GetNumberOfAvailableGroups()
	{
		return m_iNumberOfAvailableGroups;
	}

	//------------------------------------------------------------------------------------------------
	void SetNumberOfAvailableGroups(int newNumber)
	{
		m_iNumberOfAvailableGroups = newNumber;
	}
}

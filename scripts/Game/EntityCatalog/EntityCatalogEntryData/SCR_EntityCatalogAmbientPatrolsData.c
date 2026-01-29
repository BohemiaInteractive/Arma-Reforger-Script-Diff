//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "AmbientPatrolsData", "DISABLED - AmbientPatrolsData", 1)]
class SCR_EntityCatalogAmbientPatrolData: SCR_BaseEntityCatalogData
{
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EGroupType))]
	protected SCR_EGroupType m_eGroupType;
	
	[Attribute("0.5", UIWidgets.Slider, "Roll of (0, highestProbability) will be made, a group will be randomly chosen out of those which have lower number than the roll.", "0 1 0.05")]
	protected float m_fProbabilityOfPresence;
	
	//------------------------------------------------------------------------------------------------
	SCR_EGroupType GetGroupType()
	{
		return m_eGroupType;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetProbabilityOfPresence()
	{
		return m_fProbabilityOfPresence;
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_EGroupType
{
	TEAM_SENTRY,
	TEAM_MG,
	TEAM_AT,
	FIRETEAM,
	SQUAD_RIFLE,
	TEAM_SNIPER,
	TEAM_MG_ELITE,
	PARTISAN_SNIPER,
	PARTISAN_TEAM
};
modded class SCR_AdditionalGameModeSettingsComponent
{
	[Attribute(desc: "Race entities names to show on start", category: "Race")]
	protected ref array<string> m_aRaceRaceEntityNames;

	[Attribute(desc: "Vehicles available for races", category: "Race")]
	protected ref array<ResourceName> m_aRaceAvailableVehicles;

	protected SCR_RaceCustomizationPointEntity m_CustomizationEntity;

	//------------------------------------------------------------------------------------------------
	array<ResourceName> GetRaceAvailableVehicles()
	{
		return m_aRaceAvailableVehicles;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetAvailableRaceNames()
	{
		return m_aRaceRaceEntityNames;
	}

	//------------------------------------------------------------------------------------------------
	void SetCustomizationEntity(SCR_RaceCustomizationPointEntity entity)
	{
		m_CustomizationEntity = entity;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RaceCustomizationPointEntity GetCustomizationEntity()
	{
		return m_CustomizationEntity;
	}
}

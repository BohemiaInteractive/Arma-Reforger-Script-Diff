//! Per-entry data for vehicle catalogs used by SCR_AmbientVehicleSpawnPointComponent
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "AmbientVehicleData", "DISABLED - AmbientVehicleData", 1)]
class SCR_EntityCatalogAmbientVehicleData : SCR_BaseEntityCatalogData
{
	[Attribute("1", UIWidgets.EditBox, "Relative chance of this vehicle being picked by ambient vehicle spawn points. Only the ratio between entries matters (2 = twice as likely as 1, 0 = never picked). Entries without this data use 1.", "0 inf 0.05")]
	protected float m_fSpawnWeight;

	[Attribute("", UIWidgets.ComboBox, "Ambient spawn labels of this vehicle. Spawn points whose label is in this list can spawn it. Empty = BASE_GAME.", "", ParamEnumArray.FromEnum(SCR_EAmbientVehicleSpawnLabel))]
	protected ref array<SCR_EAmbientVehicleSpawnLabel> m_aSpawnLabels;

	//------------------------------------------------------------------------------------------------
	float GetSpawnWeight()
	{
		return m_fSpawnWeight;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] label
	//! \return true if this vehicle can be spawned by spawn points with the given label. Empty label list counts as BASE_GAME.
	bool HasSpawnLabel(SCR_EAmbientVehicleSpawnLabel label)
	{
		if (!m_aSpawnLabels || m_aSpawnLabels.IsEmpty())
			return label == SCR_EAmbientVehicleSpawnLabel.BASE_GAME;

		return m_aSpawnLabels.Contains(label);
	}
}

[BaseContainerProps()]
class SCR_InventoryVehicleVisibilityAttribute : BaseItemAttributeData
{
	[Attribute("0", UIWidgets.CheckBox, "If true, this item will appear in inventory only when player is inside the vehicle.")]
	protected bool m_bShowInVehicleOnly;

	[Attribute("0", UIWidgets.CheckBox, "If true, this item will be visible in inventory only when player's and vehicle's factions are the same.")]
	protected bool m_bShowForVehicleFactionOnly;

	//------------------------------------------------------------------------------------------------
	bool GetVisibleInVehicleOnly()
	{
		return m_bShowInVehicleOnly;
	}

	//------------------------------------------------------------------------------------------------
	bool GetVisibleForVehicleFactionOnly()
	{
		return m_bShowForVehicleFactionOnly;
	}
};
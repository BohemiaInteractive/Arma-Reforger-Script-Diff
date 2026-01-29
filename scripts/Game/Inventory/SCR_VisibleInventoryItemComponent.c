[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_VisibleInventoryItemComponentClass : SCR_BaseInventoryItemComponentClass
{
}

class SCR_VisibleInventoryItemComponent : SCR_BaseInventoryItemComponent
{
	[RplProp()]
	protected bool m_bHideInVicinity;

	//------------------------------------------------------------------------------------------------
	//! Setter for property that determines if item will be hidden in vicinity view of the inventory
	void SetHiddenInVicinity(bool hidden)
	{
		m_bHideInVicinity = hidden;
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldHideInVicinity()
	{
		return m_bHideInVicinity;
	}
}

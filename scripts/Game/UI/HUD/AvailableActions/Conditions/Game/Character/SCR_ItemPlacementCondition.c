[BaseContainerProps()]
class SCR_ItemPlacementCondition : SCR_AvailableActionCondition
{
	[Attribute(desc: "Check if currently placed item has multiple preview variants")]
	protected bool m_bCheckVariants;

	[Attribute(desc: "Check if player has more unique placeable items")]
	protected bool m_bCheckItems;

	[Attribute(desc: "Check if placement mode is enabled")]
	protected bool m_bCheckIsPlacementModeEnabled;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		SCR_CampaignBuildingGadgetToolComponent gadgetComponent = SCR_CampaignBuildingGadgetToolComponent.Cast(data.GetHeldGadgetComponent());
		if (!gadgetComponent)
			return GetReturnResult(false);

		if (!gadgetComponent.GetCanEnterPlacementMode())
			return GetReturnResult(false);

		if (m_bCheckIsPlacementModeEnabled && !gadgetComponent.GetIsPlacementModeEnabled())
			return GetReturnResult(false);

		if (m_bCheckItems && !gadgetComponent.HasMoreItemsToPlace())
			return GetReturnResult(false);

		if (m_bCheckVariants && !gadgetComponent.HasMoreVariants())
			return GetReturnResult(false);

		return GetReturnResult(true);
	}
}
[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_DeployablePlaceableItemComponentClass : SCR_PlaceableItemComponentClass
{
}

class SCR_DeployablePlaceableItemComponent : SCR_PlaceableItemComponent
{
	//! Gadget which is used to place this item, this will automatically reset when this item will be placed with that gadget
	protected SCR_CampaignBuildingGadgetToolComponent m_PlacingGadget;

	//------------------------------------------------------------------------------------------------
	//! \param[in] gadget
	void SetPlacingGadget(SCR_CampaignBuildingGadgetToolComponent gadget)
	{
		m_PlacingGadget = gadget;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetForwardAwayFromPlayer()
	{
		if (!m_PlacingGadget)
			return super.GetForwardAwayFromPlayer();

		SCR_MultiPartDeployableItemComponent deployableComp = m_PlacingGadget.GetCurrentlyHandledComponent();
		if (deployableComp)
			return !deployableComp.GetFrontTowardPlayer();

		return super.GetForwardAwayFromPlayer();
	}

	//------------------------------------------------------------------------------------------------
	override float GetMaxAllowedTilt()
	{
		if (!m_PlacingGadget)
			return super.GetMaxAllowedTilt();

		return m_PlacingGadget.GetMaxAllowedTilt();
	}

	//------------------------------------------------------------------------------------------------
	override bool OverrideSpaceValidation(notnull SCR_ItemPlacementComponent caller, inout vector transform[4], out ENotification cantPlaceReason)
	{
		if (!m_PlacingGadget)
			return false;

		return m_PlacingGadget.ValidateSpace(caller, transform, cantPlaceReason);
	}

	//------------------------------------------------------------------------------------------------
	override bool OverrideStartPlaceAction(notnull SCR_ItemPlacementComponent caller, out bool skipItemUsage = false)
	{
		if (!m_PlacingGadget)
			return false;

		m_PlacingGadget.OnItemPlacementStart(caller);
		skipItemUsage = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OverrideOnPlacingEnded(notnull SCR_ItemPlacementComponent caller)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OverrideAfterItemPlaced(notnull SCR_ItemPlacementComponent caller, notnull IEntity item, bool success, bool equipNext)
	{
		if (!m_PlacingGadget)
			return;

		m_PlacingGadget.OnAfterItemPlaced(caller, item, success, equipNext);
		m_PlacingGadget = null;
	}

	//------------------------------------------------------------------------------------------------
	override bool OverrideAutoEquipMechanism(out ScriptedInventoryOperationCallback callBackHolder, notnull SCR_ItemPlacementComponent caller, IEntity placedItem, bool autoEquipNext)
	{
		AfterItemPlacedInvoker invoker = new AfterItemPlacedInvoker();
		invoker.Insert(OverrideAfterItemPlaced);
		callBackHolder = new SCR_PlacementInventoryOperationCallback(invoker, caller, placedItem, autoEquipNext);
		return true;
	}
}

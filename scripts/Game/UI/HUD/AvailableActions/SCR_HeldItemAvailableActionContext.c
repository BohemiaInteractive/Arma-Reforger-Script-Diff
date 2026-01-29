[BaseContainerProps(), SCR_AvailableActionContextTitle()]
class SCR_HeldItemAvailableActionContext : SCR_AvailableActionContext
{
	protected LocalizedString m_sItemName;
	protected IEntity m_LastEntity;

	//------------------------------------------------------------------------------------------------
	override string GetUIName()
	{
		if (m_sItemName.IsEmpty())
			return super.GetUIName();

		return WidgetManager.Translate(m_sName, m_sItemName);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data, float timeSlice)
	{
		IEntity heldItem = data.GetHeldGadget();
		if (!heldItem)
			heldItem = data.GetCurrentWeaponEntity();

		if (!heldItem)
			return false;

		if (m_LastEntity != heldItem)
		{
			if (m_fShowCountdown <= 0)
				m_fShowCountdown = m_iTimeToShow;

			if (m_fHideCountdown <= 0)
				m_fHideCountdown = m_iTimeForHide;
		}

		if (!super.IsAvailable(data, timeSlice))
			return false;

		if (m_LastEntity == heldItem)
			return true;

		InventoryItemComponent itemIIC = InventoryItemComponent.Cast(heldItem.FindComponent(InventoryItemComponent));
		if (!itemIIC)
			return true;

		UIInfo itemUiInfo = itemIIC.GetUIInfo();
		if (!itemUiInfo)
			return true;

		m_LastEntity = heldItem;
		m_sItemName = itemUiInfo.GetName();
		return true;
	}
}
[BaseContainerProps(), SCR_AvailableActionContextTitle()]
class SCR_ItemPlacementActionsDisplay : SCR_AvailableActionContext
{
	protected LocalizedString m_sItemName;
	protected IEntity m_LastEntity;

	//------------------------------------------------------------------------------------------------
	override string GetUIName()
	{
		if (m_sItemName.IsEmpty())
			return string.Empty;

		return WidgetManager.Translate(m_sName, m_sItemName);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data, float timeSlice)
	{
		m_sItemName = data.GetCurrentlyPlacedItemName();
		if (m_sItemName.IsEmpty())
			return false;

		return super.IsAvailable(data, timeSlice);
	}
}
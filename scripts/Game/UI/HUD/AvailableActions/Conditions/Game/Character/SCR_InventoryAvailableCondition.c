[BaseContainerProps()]
class SCR_InventoryAvailableCondition : SCR_AvailableActionCondition
{
	[Attribute("4")]
	protected int m_iMaxShowCount;
	protected static int s_iShowCounter;

	[Attribute("1")]
	protected bool m_bPersistent;

	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (s_iShowCounter >= m_iMaxShowCount)
			return false;

		return GetReturnResult(data.IsInventoryOpen());
	}

	static void IncrementCounter()
	{
		s_iShowCounter++;
	}

	void SCR_InventoryAvailableCondition()
	{
		if (!m_bPersistent)
		{
			s_iShowCounter = 0;
			return;
		}

		UserSettings userSettings = GetGame().GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer container = userSettings.GetModule("SCR_InventoryHintSettings");
		if (container)
			container.Get("m_iInventoryOpenCount", s_iShowCounter)
	}

	void ~SCR_InventoryAvailableCondition()
	{
		if (!m_bPersistent)
			return;

		UserSettings userSettings = GetGame().GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer container = userSettings.GetModule("SCR_InventoryHintSettings");
		if (!container)
			return;

		container.Set("m_iInventoryOpenCount", s_iShowCounter);
		GetGame().UserSettingsChanged();
	}
};

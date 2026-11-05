class SCR_ChangeDataPageAction : ScriptedUserAction
{
	[Attribute("1", UIWidgets.CheckBox, "Value that determines if this action will load next (1) or previous (-1) page", "-1 1 2")]
	protected int m_iStep;

	protected SCR_DataDisplayGadgetComponent m_DataDisplayGadgetComp;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_DataDisplayGadgetComp = SCR_DataDisplayGadgetComponent.Cast(pOwnerEntity.FindComponent(SCR_DataDisplayGadgetComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_DataDisplayGadgetComp.ChangePage(m_iStep);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DataDisplayGadgetComp)
			return false;

		if (m_DataDisplayGadgetComp.GetNumberOfPages() <= 1)
			return false;

		if (m_DataDisplayGadgetComp.GetMode() != EGadgetMode.IN_HAND)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}
}

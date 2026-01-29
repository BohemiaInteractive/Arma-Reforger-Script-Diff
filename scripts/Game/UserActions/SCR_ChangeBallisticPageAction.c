class SCR_ChangeBallisticPageAction : ScriptedUserAction
{
	[Attribute("1", UIWidgets.CheckBox, "Value that determines if this action will load next (1) or previous (-1) page", "-1 1 2")]
	protected int m_iStep;

	protected SCR_BallisticTableComponent m_BallisticTableComp;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_BallisticTableComp = SCR_BallisticTableComponent.Cast(pOwnerEntity.FindComponent(SCR_BallisticTableComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_BallisticTableComp.ChangePage(m_iStep);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_BallisticTableComp)
			return false;

		if (m_BallisticTableComp.GetNumberOfPages() <= 1)
			return false;

		if (m_BallisticTableComp.GetMode() != EGadgetMode.IN_HAND)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}
}

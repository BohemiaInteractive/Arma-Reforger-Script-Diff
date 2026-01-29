[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_SequenceHintCondition : SCR_BaseHintCondition
{
	[Attribute("0", UIWidgets.SearchComboBox, "", enums: ParamEnumArray.FromEnum(EHint))]
	protected EHint m_Type;
	
	//------------------------------------------------------------------------------------------------
	protected void OnHintHide(SCR_HintUIInfo info, bool isSilent)
	{
		if (info && info.GetType() == m_Type)
			Activate();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager)
			hintManager.GetOnHintHide().Insert(OnHintHide);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager)
			hintManager.GetOnHintHide().Remove(OnHintHide);
	}
}

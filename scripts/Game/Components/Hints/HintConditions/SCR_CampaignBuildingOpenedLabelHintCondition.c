[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_CampaignBuildingOpenedLabelHintCondition : SCR_BaseEditorHintCondition
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "A label provider has to have to show a hint.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_eLabel;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		if (!HasLabelSet())
			return;
		
		editorManager.GetOnOpened().Insert(Activate);
		editorManager.GetOnClosed().Insert(Deactivate);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		if (!HasLabelSet())
			return;
		
		editorManager.GetOnOpened().Remove(Activate);
		editorManager.GetOnClosed().Remove(Deactivate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the provider has given label set. If so, the hint is shown.
	//! \return
	bool HasLabelSet()
	{
		SCR_CampaignBuildingEditorComponent CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!CampaignBuildingEditorComponent)
			return false;
		
		SCR_CampaignBuildingProviderComponent providerComponent = CampaignBuildingEditorComponent.GetProviderComponent();
		if (!providerComponent)
			return false;
		
		array<EEditableEntityLabel> providerTraits = providerComponent.GetAvailableTraits();
		return providerTraits.Contains(m_eLabel);
	}
}

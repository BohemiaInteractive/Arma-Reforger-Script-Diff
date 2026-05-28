[BaseContainerProps()]
class SCR_EditorPlacedEntityFlagsCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EEditorPlacingFlags.CHARACTER_PLAYER.ToString(), desc: "Placing Flag", uiwidget: UIWidgets.ComboBox, enumType: EEditorPlacingFlags)]
	protected EEditorPlacingFlags m_ePlacingFlag;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_PlacingEditorComponent placingComponent = data.GetEditorPlacingComponent();
		return GetReturnResult(placingComponent && placingComponent.IsPlacing() && placingComponent.IsPlacingFlagCompatible(m_ePlacingFlag));
	}
}

// Script File
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_PlacingFlagToolbarAction : SCR_BaseToggleToolbarAction
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorPlacingFlags))]
	protected EEditorPlacingFlags m_PlacingFlag;
	
	protected void OnPlacingFlagsChange(EEditorPlacingFlags flag, bool toAdd)
	{
		if (flag == m_PlacingFlag)
			Toggle(toAdd, toAdd);
	}
	
	override void Track()
	{
		SCR_PlacingEditorComponent placingComponent = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (placingComponent)
		{
			placingComponent.GetOnPlacingFlagsChange().Insert(OnPlacingFlagsChange);
			OnPlacingFlagsChange(m_PlacingFlag, placingComponent.HasPlacingFlag(m_PlacingFlag));
		}
	}
	override void Untrack()
	{
		SCR_PlacingEditorComponent placingComponent = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (placingComponent)
		{
			placingComponent.GetOnPlacingFlagsChange().Remove(OnPlacingFlagsChange);
		}
	}
	override bool IsServer()
	{
		return false;
	}
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_PlacingEditorComponent placingComponent = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, false, true));
		return placingComponent && placingComponent.IsPlacing() && placingComponent.IsPlacingFlagCompatible(m_PlacingFlag);
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PlacingEditorComponent placingComponent = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (placingComponent)
			placingComponent.TogglePlacingFlag(m_PlacingFlag);
	}
};
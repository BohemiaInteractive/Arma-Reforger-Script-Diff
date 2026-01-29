[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_CommandPrefab", true)]
class SCR_BaseCommandAction : SCR_BaseToggleToolbarAction
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Objective or waypoint prefab", "et")]
	protected ResourceName m_CommandPrefab;
	
	protected void FilterEntities(notnull set<SCR_EditableEntityComponent> inEntities, out notnull set<SCR_EditableEntityComponent> outEntities)
	{
		outEntities.Copy(inEntities);
	}
	
	protected set<SCR_EditableEntityComponent> GetSelectedEntities(notnull set<SCR_EditableEntityComponent> inEntities)
	{
		set<SCR_EditableEntityComponent> selectedEnities = new set<SCR_EditableEntityComponent>();
		FilterEntities(inEntities, selectedEnities);
		return selectedEnities;
	}
	
	//--- Called from command toolbar, initiates placing mode
	bool StartPlacing(notnull set<SCR_EditableEntityComponent> selectedEntities)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true));
		if (!placingManager)
		{
			return false;
		}
		return placingManager.SetSelectedPrefab(m_CommandPrefab, recipients: GetSelectedEntities(selectedEntities));
	}
	
	//--- Called from radial menu, places the command instantly
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (cursorWorldPosition == vector.Zero)
			return;
		
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (!placingManager)
			return;
		
		bool isQueue = flags & EEditorCommandActionFlags.IS_QUEUE;
		
		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		transform[3] = cursorWorldPosition;
		SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParams(transform);
		
		//++ If the flags is to attach, use the hovered entity as the holder for the spawned entity
		SCR_EditableEntityComponent holder = null;
		if (flags & EEditorCommandActionFlags.ATTACH)
			holder = hoveredEntity;
		
		placingManager.CreateEntity(m_CommandPrefab, params, !isQueue, false, GetSelectedEntities(selectedEntities), holder);
	}
	
	protected void OnCurrentActionChanged()
	{
		SCR_CommandActionsEditorComponent commandActionsManager = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent));
		if (commandActionsManager)
			Toggle(0, commandActionsManager.IsActionCurrent(this))
	}
	override void Track()
	{
		SCR_CommandActionsEditorComponent commandActionsManager = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent));
		if (commandActionsManager)
		{
			commandActionsManager.GetOnCurrentActionChanged().Insert(OnCurrentActionChanged);
			OnCurrentActionChanged();
		}
	}
	override void Untrack()
	{
		SCR_CommandActionsEditorComponent commandActionsManager = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent));
		if (commandActionsManager)
			commandActionsManager.GetOnCurrentActionChanged().Remove(OnCurrentActionChanged);
	}
	override bool IsServer()
	{
		//--- Must be called on client, placing handles server communication itself
		return false;
	}
};
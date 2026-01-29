[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_PlaceEntityContextAction: SCR_DoubleClickAction
{
	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "Placing flags enabled upon creating an entity.", enums: ParamEnumArray.FromEnum(EEditorPlacingFlags))]
	protected EEditorPlacingFlags m_PlacingFlag;
	
	[Attribute(desc: "Active labels and white listed labels and label groups")]
	protected ref SCR_EditorContentBrowserDisplayConfig m_ContentBrowserConfig;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (hoveredEntity || cursorWorldPosition == vector.Zero)
			return false;
		
		//--- Player unit is dead or does not exist
		if (m_PlacingFlag == EEditorPlacingFlags.CHARACTER_PLAYER)
		{
			IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
			if (!controlledEntity)
				return true;
			
			DamageManagerComponent damageManager = DamageManagerComponent.Cast(controlledEntity.FindComponent(DamageManagerComponent));
			if (!damageManager)
				return false;
			
			return damageManager.GetState() == EDamageState.DESTROYED;
		}
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (DidCursorMoveDuringDoubleClick())
			return;
		
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (!placingManager || placingManager.IsPlacing() || GetGame().GetMenuManager().IsAnyDialogOpen())
			return;
		
		vector transform[4];
		transform[3] = cursorWorldPosition;
		
		SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
		if (camera)
			Math3D.AnglesToMatrix(Vector(camera.GetAngles()[0], 0, 0), transform);
		else
			Math3D.MatrixIdentity3(transform);
		
		placingManager.SetPlacingFlag(m_PlacingFlag, true);
		
		if (SCR_ContentBrowserEditorComponent.OpenBrowserLabelConfigInstance(m_ContentBrowserConfig))
		{
			placingManager.SetInstantPlacing(SCR_EditorPreviewParams.CreateParams(transform));
		}
		else
		{
			placingManager.SetPlacingFlag(m_PlacingFlag, false);
		}
	}
	override bool IsServer()
	{
		return false;
	}
};

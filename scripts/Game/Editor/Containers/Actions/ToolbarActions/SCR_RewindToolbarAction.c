[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_RewindToolbarAction: SCR_EditorToolbarAction
{
	[Attribute(desc: "When enabled, the action will delete the rewind point instead of loading it.")]
	protected bool m_bIsDelete;
	
	protected SCR_BaseToolbarEditorUIComponent m_Toolbar;
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
		return rewindManager && rewindManager.HasRewindPoint();
	}

	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (m_bIsDelete)
		{
			SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
			rewindManager.DeleteRewindPoint();
			return;
		}

		new SCR_RewindDialog();
	}

	//---------------------------------------------------------------------------------------------
	override void OnInit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		m_Toolbar = toolbar;
		SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
		if (rewindManager)
			rewindManager.GetOnRewindPointChanged().Insert(OnRewindPointChanged);
	}

	//---------------------------------------------------------------------------------------------
	override void OnExit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
		if (rewindManager)
			rewindManager.GetOnRewindPointChanged().Remove(OnRewindPointChanged);
	}

	//---------------------------------------------------------------------------------------------
	protected void OnRewindPointChanged()
	{
		if (m_Toolbar)
			m_Toolbar.MarkForRefresh();
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_HintSequenceToolbarAction: SCR_BaseToggleToolbarAction
{
	protected SCR_HintSequenceComponent m_Sequence;
	
	protected void OnSequenceChange(bool isActive)
	{
		Toggle(isActive, isActive);
	}
	protected SCR_HintSequenceComponent GetSequence()
	{
		SCR_EditorModeEntity currentMode = SCR_EditorModeEntity.GetInstance();
		if (currentMode)
			return SCR_HintSequenceComponent.Cast(currentMode.FindComponent(SCR_HintSequenceComponent));
		else
			return null;
	}
	
	override bool IsServer()
	{
		return false;
	}
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_HintSequenceComponent sequence = GetSequence();
		return sequence != null;
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_HintSequenceComponent sequence = GetSequence();
		if (sequence)
			sequence.ToggleSequence();
	}
	override void Track()
	{
		m_Sequence= GetSequence();
		if (m_Sequence)
			m_Sequence.GetOnSequenceChange().Insert(OnSequenceChange);
	}
	override void Untrack()
	{
		if (m_Sequence)
		{
			m_Sequence.GetOnSequenceChange().Remove(OnSequenceChange);
			m_Sequence.StopSequence();
		}
	}
};
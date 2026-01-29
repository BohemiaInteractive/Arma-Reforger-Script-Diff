//------------------------------------------------------------------------------------------------
//! An abstract wrapper to provide common interface for items that can be stored in a BaseGroup
class SCR_BaseGroupEntry : ScriptedSelectionMenuEntry
{
	protected ref array<ScriptedSelectionMenuEntry> m_aEntries;
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when this entry should cleanup after itself, aka "deselect" 
	void OnDonePerform(IEntity user, BaseSelectionMenu sourceMenu)
	{	
	}
};

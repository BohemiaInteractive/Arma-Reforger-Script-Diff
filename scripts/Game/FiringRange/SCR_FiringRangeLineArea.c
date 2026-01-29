[EntityEditorProps(category: "GameScripted/Firingrange", description: "Defines area from which player should shoot at target. Needs to be a child of firing line controller. ", color: "0 0 255 255")]
class SCR_FiringRangeLineAreaClass: ScriptedGameTriggerEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_FiringRangeLineArea : ScriptedGameTriggerEntity
{
	//! Firing range controller
	private SCR_FiringRangeController m_LineController;
	private SCR_FiringRangeManager m_FiringRangeManager;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit (IEntity owner)
	{
		// Get its parent - firing line controller
		m_LineController = SCR_FiringRangeController.Cast(owner.GetParent());
		if (!m_LineController)
			return;
		
		// And it's parent firing range manager
		m_FiringRangeManager = SCR_FiringRangeManager.Cast(m_LineController.GetParent());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate(IEntity ent)
	{		
		if (!m_LineController || !m_FiringRangeManager)
			return;
		
		// Get the ID of player who left the trigger 
		int firigLineOwner = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		if (!firigLineOwner)
			return;

		// If those two match, break the targets sequence
		if (m_LineController.GetFiringLineOwnerId() == firigLineOwner) 
			m_LineController.BackToDefaultTargetsFromLineArea();		
	}
	
	//------------------------------------------------------------------------------------------------
	
	void SCR_FiringRangeLineArea(IEntitySource src, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_FiringRangeLineArea()
	{
	
	}
};
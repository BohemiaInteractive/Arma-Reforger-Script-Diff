[EntityEditorProps(category: "GameScripted/Building", description: "Defines a building radius (limits the camera range and draw the visual borders)")]
class SCR_FreeRoamBuildingClientTriggerEntityClass : SCR_FreeRoamBuildingBaseTriggerEntityClass
{
};

class SCR_FreeRoamBuildingClientTriggerEntity : SCR_FreeRoamBuildingBaseTriggerEntity
{
	protected ref ScriptInvokerEntity m_OnEntityEnter;
	protected ref ScriptInvokerEntity m_OnEntityLeave;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity ent)
	{
		GetOnEntityEnterTrigger().Invoke(ent);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Entity leaves the trigger
	override void OnDeactivate(IEntity ent)
	{
		GetOnEntityLeaveTrigger().Invoke(ent); 
	}
		
	//------------------------------------------------------------------------------------------------
	ScriptInvokerEntity GetOnEntityEnterTrigger()
	{
		if (!m_OnEntityEnter)
			m_OnEntityEnter =  new ScriptInvokerEntity();
		
		return m_OnEntityEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerEntity GetOnEntityLeaveTrigger()
	{
		if (!m_OnEntityLeave)
			m_OnEntityLeave =  new ScriptInvokerEntity();
		
		return m_OnEntityLeave;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_FreeRoamBuildingClientTriggerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.NO_TREE);
	}
};

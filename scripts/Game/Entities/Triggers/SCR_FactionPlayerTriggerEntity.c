[EntityEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_FactionPlayerTriggerEntityClass: SCR_BaseFactionTriggerEntityClass
{
};
class SCR_FactionPlayerTriggerEntity: SCR_BaseFactionTriggerEntity
{
	protected SCR_PlayerController m_PlayerController;
	
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (!super.ScriptedEntityFilterForQuery(ent) || !IsAlive(ent))
			return false;
		
		return SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(ent) > 0;
	}
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
	}
};

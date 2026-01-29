#define ENABLE_BASE_DESTRUCTION
[EntityEditorProps(category: "GameScripted/Destruction", description: "Enables destruction.")]
class SCR_DestructionManagerComponentClass : ScriptComponentClass
{
}

class SCR_DestructionManagerComponent : ScriptComponent
{
#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComponent || rplComponent.IsProxy())
			return;
		
		float delay = 10000;
		
		if (RplSession.Mode() != RplMode.Dedicated)
			delay = 500;
		
		GetGame().GetCallqueue().CallLater(SCR_MPDestructionManager.InitializeDestructionManager, delay, false);
	}
#endif
}

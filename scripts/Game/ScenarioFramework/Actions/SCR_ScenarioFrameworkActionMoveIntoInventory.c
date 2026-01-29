[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionMoveIntoInventory : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to move into inventory)")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter; 
	
	[Attribute(desc: "Target entity with inventory)")]
	ref SCR_ScenarioFrameworkGet m_TargetGetter;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;
		
		IEntity targetEntity;
		if (!ValidateInputEntity(object, m_TargetGetter, targetEntity))
			return;
		
		if (entity == targetEntity)
		{
			Print(string.Format("SCR_ScenarioFrameworkActionMoveIntoInventory: Entity is same as target for action %1", this), LogLevel.ERROR);
			return;
		}
		
		ScriptedInventoryStorageManagerComponent storageManager = ScriptedInventoryStorageManagerComponent.Cast(targetEntity.FindComponent(ScriptedInventoryStorageManagerComponent));
		if (!storageManager)
		{
			Print(string.Format("SCR_ScenarioFrameworkActionMoveIntoInventory: Target Entity %2 doesn't have storage StorageManagerComponent for action %1", this, m_TargetGetter), LogLevel.ERROR);
			return;
		}
		
		if (!storageManager.TryInsertItem(entity))
			Print(string.Format("SCR_ScenarioFrameworkActionMoveIntoInventory: Insertion of entity %2 for action %1 failed.", this, m_TargetGetter), LogLevel.WARNING);
	}
}
[BaseContainerProps()]
class SCR_ScenarioFrameworkItemInStorageCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity with storage to check.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Prefabs to search")]
	protected ref array <ResourceName> m_aPrefabResources;
	
	[Attribute(defvalue: "0", desc: "Negation of condition")]
	protected bool m_bNegation;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		if (m_bNegation)
			return !IsInInventory();
		
		return IsInInventory();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsInInventory()
	{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);

			return false;
		}
		
		IEntity storageEntity = entityWrapper.GetValue();
		if (!storageEntity)
			return false;
		
		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(storageEntity.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return false;
		
		//TODO> PrefabFilter would be even better here, to check childrens
		SCR_PrefabNamePredicate predicate = new SCR_PrefabNamePredicate;
		
		foreach (ResourceName resource : m_aPrefabResources)
		{
			predicate.prefabName = resource;
			if (storageManager.FindItem(predicate))
				return true;
		}
		
		return false;
	}
}
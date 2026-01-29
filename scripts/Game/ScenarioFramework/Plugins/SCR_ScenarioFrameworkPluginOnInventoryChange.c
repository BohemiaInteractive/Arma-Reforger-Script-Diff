[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginOnInventoryChange : SCR_ScenarioFrameworkPlugin
{
	[Attribute(UIWidgets.Auto, desc: "What to do once object inventory has changed by item addition")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemAdded;

	[Attribute(UIWidgets.Auto, desc: "What to do once object inventory has changed by item removal")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemRemoved;

	IEntity m_Asset;

	//------------------------------------------------------------------------------------------------
	//! Initializes inventory storage managers for an entity, subscribes to item added and removed events.
	//! \param[in] object of layer base from which entity is further retrieved
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		IEntity entity = object.GetSpawnedEntity();
		if (!entity)
			return;

		m_Asset = entity;

		//Inventory system is a mess and since different entities have different storage managers that don't have this properly inherited, we need to account for that here
		SCR_InventoryStorageManagerComponent storageManager1 = SCR_InventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManager1)
		{
			storageManager1.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager1.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}

		SCR_VehicleInventoryStorageManagerComponent storageManager2 = SCR_VehicleInventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_VehicleInventoryStorageManagerComponent));
		if (storageManager2)
		{
			storageManager2.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager2.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}

		SCR_ArsenalInventoryStorageManagerComponent storageManager3 = SCR_ArsenalInventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		if (storageManager3)
		{
			storageManager3.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager3.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handles inventory item addition, debugs, and triggers actions on item addition.
	//! \param[in] item Adds an item to inventory, triggers actions on item addition.
	//! \param[in] storageOwner Represents the entity owning the inventory where an item is added.
	protected void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		// Here you can debug specific Plugin instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Plugin Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkPluginOnInventoryChange.OnItemAdded] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemAdded)
		{
			action.OnActivate(m_Asset);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handles inventory item removal, debugs, and triggers actions on item removal.
	//! \param[in] item Removes an item from inventory, triggers actions on item removal.
	//! \param[in] storageOwner Represents the entity owning the inventory where item is removed from.
	protected void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		// Here you can debug specific Plugin instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Plugin Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkPluginOnInventoryChange.OnItemRemoved] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemRemoved)
		{
			action.OnActivate(m_Asset);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetActions()
	{
		array<ref SCR_ScenarioFrameworkActionBase>	combinedActions = {};
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemAdded)
	    {
	        combinedActions.Insert(action);
   		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemRemoved)
	    {
	        combinedActions.Insert(action);
   		}
		
		return combinedActions;
	}
}
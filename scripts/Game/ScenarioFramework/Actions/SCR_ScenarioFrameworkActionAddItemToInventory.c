[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAddItemToInventory : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter; 

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCountNoInheritance> m_aPrefabFilter;
	
	[Attribute(desc: "Name of the entity used for identification. If entity with given name exists suffix _numberOfAttemptToNameEntity is added.", category: "Asset")]
	string m_sID;
	
	[Attribute(UIWidgets.CheckBox, desc: "If the entity with the same id exist in the world, it will be deleted and new entity will be give the set name. If this is not set the suffix _x will be added to newly spawned entity.", category: "Asset")]
	bool m_bDeletePreviousInstance;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		entity.GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
		Math3D.AnglesToMatrix(angles, spawnParams.Transform);

		foreach (SCR_ScenarioFrameworkPrefabFilterCountNoInheritance prefabFilter : m_aPrefabFilter)
		{
			int suffixNumber;
			string IDWithSuffix;
			
			for (int i = 0; i < prefabFilter.m_iPrefabCount; i++)
			{
				Resource resource = Resource.Load(prefabFilter.m_sPrefabName);
				if (!resource)
					continue;

				IEntity item = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
				if (!item)
					continue;

				SCR_InvCallBackCheck pInvCallback = new SCR_InvCallBackCheck();
				pInvCallback.m_sNameMaster = entity.GetName();
				pInvCallback.m_sNameItem = item.GetName();

				inventoryComponent.TryInsertItem(item, EStoragePurpose.PURPOSE_ANY, pInvCallback);
				
				if (m_sID.IsEmpty())
                    continue;
				
				if (GetGame().GetWorld().FindEntityByName(m_sID))
				{
					if (m_bDeletePreviousInstance)
					{
						IEntity ent = GetGame().GetWorld().FindEntityByName(m_sID);
						SCR_EntityHelper.DeleteEntityAndChildren(ent);
						item.SetName(m_sID);
						continue;
					}
					
					
					IDWithSuffix = m_sID;
					while (GetGame().GetWorld().FindEntityByName(IDWithSuffix))
	                {
						suffixNumber++;
						Print(string.Format("ScenarioFramework Action: Entity of name %1 was found. The suffix _%2 will be added.", m_sID, suffixNumber), LogLevel.ERROR);
						IDWithSuffix = m_sID + "_" + suffixNumber.ToString();
	                }
					
					item.SetName(IDWithSuffix); 
					continue;
					
				}
				
				item.SetName(m_sID);    
			}
		}
	}
}

class SCR_InvCallBackCheck : SCR_InvCallBack
{
	string m_sNameMaster = "";
	string m_sNameItem = "";
	
	//------------------------------------------------------------------------------------------------
	override void OnFailed()
	{
		PrintFormat("ScenarioFramework: The item %1 could not be inserted into %2! Check name of the entity and be sure the entity has a space in the inventory", m_sNameItem, m_sNameMaster, LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	override void OnComplete()
	{
		//PrintFormat("ScenarioFramework: The item %1 has been correctly inserted into %2 inventory", m_sNameItem, m_sNameMaster);
	}
}
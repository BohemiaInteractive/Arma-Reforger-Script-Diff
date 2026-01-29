/**
Info for Entity Spawner
*/
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "SpawnerData", "DISABLED - SpawnerData", 1)]
class SCR_EntityCatalogSpawnerData : SCR_BaseEntityCatalogData
{				
	[Attribute(desc: "UI info overwrite to overwrite entity name. Will use EditableEntityUI if non is set")]
	protected ref SCR_UIInfo m_UiInfo;
	
	[Attribute("0", desc: "Slot size the entity can be spawned in", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EEntitySpawnerSlotType))]
	protected SCR_EEntitySpawnerSlotType m_eSlotTypes;
	
	[Attribute(defvalue: "1", params: "1 inf", desc: "Prefab entity count. To be used with prefabs like groups, where spawned logic create additional entities")]
	protected int m_iEntityCount;
	
	[Attribute(desc: "Variants prefab data of entity. These are a diffrent variant of the default")]
	protected ref array<ref SCR_SpawnerVariantData> m_aVariantData;
	
	protected ResourceName m_sDefaultPrefab;
	
	protected SCR_ECharacterRank m_eMinimumRequiredRank;
	
	protected int m_iSupplyCost;
	
	//--------------------------------- Enable/Disable in Spawner ---------------------------------\\
	/*!
	Set the enabled state of the entity in the spawner
	Changing this in runtime will allow/disallow certain entities to spawn for factions
	\param enable New Enabled state
	*/
	void SetEnabled(bool enable)
	{
		m_bEnabled = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetEntityCount()
	{
		return m_iEntityCount;
	}
	
	//--------------------------------- Get Overwrite name ---------------------------------\\
	/*!
	Get Localized overwrite name
	Empty if no name is overwritten
	\return Localized overwrite name
	*/
	LocalizedString GetOverwriteName()
	{
		if (!m_UiInfo)
			return string.Empty;
		
		return m_UiInfo.GetName();
	}

	//--------------------------------- Can Spawn in Slot ---------------------------------\\
	/*!
	Check if entity can be spawned in slot of given size
	\param slotSize Slot size to check
	\return Returns true if entity can spawn in given slot size
	*/
	bool CanSpawnInSlot(SCR_EEntitySpawnerSlotType slotSize)
	{
		return SCR_Enum.HasFlag(GetValidSlotSizes(), slotSize);
	}
	
	//--------------------------------- Get Valid slot sizes ---------------------------------\\
	/*!
	Get flags of valid slot sizes
	\return Valid slot sizes flags
	*/
	SCR_EEntitySpawnerSlotType GetValidSlotSizes()
	{
		return m_eSlotTypes;
	}
	
	//--------------------------------- Get has required Rank ---------------------------------\\
	/*!
	Check if entity can be spawned by player with specific rank
	Will check if given rank is equal or greater then minimum required rank
	\param rank Rank of player that request the spawn
	\return Returns true if player has the required rank
	*/
	bool HasRequiredRank(SCR_ECharacterRank rank)
	{
		if (rank == SCR_ECharacterRank.INVALID)
			return false;
		
		return rank >= GetMinimumRequiredRank();
	}
	
	//--------------------------------- Get minimum required Rank ---------------------------------\\
	/*!
	Get the minimum required rank
	\return Returns the minimum required rank
	*/
	SCR_ECharacterRank GetMinimumRequiredRank()
	{
		return m_eMinimumRequiredRank;
	}
	
	//--------------------------------- Get supply cost ---------------------------------\\
	/*!
	Get supply cost of spawning Entity
	\return Supply cost
	*/
	int GetSupplyCost()
	{
		return m_iSupplyCost;
	}
	
	//--------------------------------- Get Variants of Type ---------------------------------\\
	/*!
	Get random variant or default prefab
	\return Random variant or default prefab
	*/
	ResourceName GetRandomDefaultOrVariantPrefab()
	{
		array<ResourceName> prefabs = {};
		
		prefabs.Insert(m_sDefaultPrefab);
		
		foreach (SCR_SpawnerVariantData data: m_aVariantData)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(data.GetVariantPrefabData()))
				continue;
			
			prefabs.Insert(data.GetVariantPrefabData());
		}
		
		return prefabs.GetRandomElement();
	}
	
	//--------------------------------- Get Variant data list ---------------------------------\\
	/*!
	Get list of all variant data
	\param[out] variantData List of variant data
	\return length of variant data array
	*/
	int GetVariantDataList(notnull out array<SCR_SpawnerVariantData> variantData)
	{
		variantData.Clear();
		
		foreach (SCR_SpawnerVariantData data: m_aVariantData)
		{
			variantData.Insert(data);
		}
			
		return variantData.Count();
	}
	
	//--------------------------------- Get Variant Prefab List ---------------------------------\\
	/*!
	Get list of all variant prefabs
	\param[out] variantData List of variant prefabs
	\return length of variant prefab array
	*/
	int GetVariantPrefabList(notnull out array<ResourceName> variantPrefabs)
	{
		variantPrefabs.Clear();
		
		foreach (SCR_SpawnerVariantData data: m_aVariantData)
		{
			variantPrefabs.Insert(data.GetVariantPrefabData());
		}
		
		return variantPrefabs.Count();
	}
	
	//--------------------------------- Get Variants of Type ---------------------------------\\
	/*!
	Get specific variant data of type if it exists
	\param Type of variant to find
	\return Variant data (or null if not found)
	*/
	SCR_SpawnerVariantData GetVariantDataOfType(ESpawnerVariantType variantType)
	{
		foreach (SCR_SpawnerVariantData data: m_aVariantData)
		{
			if (data.GetVariantType() == variantType)
				return data;
		}
		
		return null;
	}
	
	//--------------------------------- Get Variant prefabs of Type ---------------------------------\\
	/*!
	Get specific variant Prefab of type if it exists
	\param Type of variant to find
	\return Variant prefab (or empty if not found)
	*/
	ResourceName GetVariantPrefabOfType(ESpawnerVariantType variantType)
	{
		SCR_SpawnerVariantData variantData = GetVariantDataOfType(variantType);
		
		if (!variantData)
			return string.Empty;
		
		return variantData.GetVariantPrefabData();
	}
	
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		super.InitData(entry);
		
		m_sDefaultPrefab = entry.GetPrefab();
		
		SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(entry.GetEntityUiInfo());
		if (!uiInfo)
			return;
		
		array<ref SCR_EntityBudgetValue> budgets = {};
		uiInfo.GetEntityAndChildrenBudgetCost(budgets);

		m_eMinimumRequiredRank = -1;
		
		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			switch (budget.GetBudgetType())
			{
				//~ Set Supply cost
				case EEditableEntityBudget.CAMPAIGN:
				{
					m_iSupplyCost = budget.GetBudgetValue();
					
					break;
				}
				
				//~ Set ranks
				case EEditableEntityBudget.RANK_RENEGADE:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.RENEGADE;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_PRIVATE:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.PRIVATE;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_CORPORAL:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.CORPORAL;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_SERGEANT:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.SERGEANT;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_LIEUTENANT:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.LIEUTENANT;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_CAPTAIN:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.CAPTAIN;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_MAJOR:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.MAJOR;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_COLONEL:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.COLONEL;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");
				
					break;
				}
				case EEditableEntityBudget.RANK_GENERAL:
				{
					if (m_eMinimumRequiredRank == -1)
						m_eMinimumRequiredRank = SCR_ECharacterRank.GENERAL;
					else 
						Print("'SCR_EntityCatalogSpawnerData' entry " + uiInfo.GetName() + " has rank " + typename.EnumToString(EEditableEntityBudget, m_eMinimumRequiredRank) + " assigned but multiple ranks in the editable UI info which is not supported!");

					break;
				}
			}
		}
		
		//~ No rank set so set minimum rank
		if (m_eMinimumRequiredRank == -1)
			m_eMinimumRequiredRank = SCR_ECharacterRank.PRIVATE;
	}
};

//======================================== VARIANT DATA ========================================\\
/**
Variant Spawnder prefab data. Includes the prefab ResourceName and type of variant
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sVariantPrefab", true)]
class SCR_SpawnerVariantData
{
	[Attribute(desc: "Prefab of Variant entity", params: "et")]
	protected ResourceName m_sVariantPrefab;
	
	[Attribute("0", desc: "Type of variant. Type needs to be unique in the list", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESpawnerVariantType))]
	protected ESpawnerVariantType m_eVehicleVariantType;
	
	/*!
	Get Prefab data
	\return Prefab data
	*/
	ResourceName GetVariantPrefabData()
	{
		return m_sVariantPrefab;
	}
	
	/*!
	Get Vehicle Variant Type
	\return Variant Type
	*/
	ESpawnerVariantType GetVariantType()
	{
		return m_eVehicleVariantType;
	}
	
};

//======================================== ENUMS  ========================================\\
/**
Spawner varient types
*/
enum ESpawnerVariantType
{
	VEHICLE_MERC,
};
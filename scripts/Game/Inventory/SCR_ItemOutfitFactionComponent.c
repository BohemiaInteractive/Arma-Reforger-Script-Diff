[ComponentEditorProps(category: "GameScripted/Inventory", description: "")]
class SCR_ItemOutfitFactionComponentClass : ScriptComponentClass
{
}

[BaseContainerProps(), SCR_BaseContainerTitleFieldWithValue("m_AffiliatedFactionKey", "m_iOutfitFactionValue", "%1: %2", "10")]
class SCR_OutfitFactionData
{
	[Attribute(desc: "Needs to be unique")]
	protected FactionKey m_AffiliatedFactionKey;
	
	[Attribute("10", desc: "0 value means it is ignored", params: "0 inf")]
	protected int m_iOutfitFactionValue;
	
	protected Faction m_AffiliatedFaction;
	
	//------------------------------------------------------------------------------------------------
	//! Init to set the affiliated faction
	bool Init(notnull FactionManager factionManager)
	{		
		m_AffiliatedFaction = factionManager.GetFactionByKey(m_AffiliatedFactionKey);
		if (!m_AffiliatedFaction)
			return false;
		
		return m_iOutfitFactionValue > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Faction key associated with the faction
	FactionKey GetAffiliatedFactionKey()
	{
		return m_AffiliatedFactionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Value associated with the faction
	int GetOutfitFactionValue()
	{
		return m_iOutfitFactionValue;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Faction set on init using the m_AffiliatedFactionKey for quicker getting of the faction
	Faction GetAffiliatedFaction()
	{
		return m_AffiliatedFaction;
	}
}

[BaseContainerProps()]
class SCR_OutfitFactionDataHolder
{
	[Attribute()]
	protected ref array<ref SCR_OutfitFactionData> m_aOutfitFactionData;
	
	//------------------------------------------------------------------------------------------------
	//! Init the faction outfit data array
	void Init(notnull FactionManager factionManager)
	{
		//~ Clear any invalid entries
		for (int i = m_aOutfitFactionData.Count() - 1; i >= 0; i--)
		{
			//~ Init. If failed remove from init
			if (!m_aOutfitFactionData[i].Init(factionManager))
			{
				m_aOutfitFactionData.RemoveOrdered(i);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Count of outfit data array
	int GetOutfitFactionDataArrayCount()
	{
		return m_aOutfitFactionData.Count();
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Get Outfit data array
	//! \param[in] outfitValues Array to add data to (Will be cleared)
	//! \return Array Count
	int GetOutfitFactionDataArray(out notnull array<SCR_OutfitFactionData> outfitDataArray)
	{
		outfitDataArray.Clear();
		
		foreach(SCR_OutfitFactionData data : m_aOutfitFactionData)
		{
			outfitDataArray.Insert(data);
		}
		
		return outfitDataArray.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get specific outfit data of faction (Will return the first valid found)
	//! param[in] faction Faction Key to find data of
	//! return Found data (Can be null)
	SCR_OutfitFactionData GetOutfitFactionData(FactionKey faction)
	{
		if (faction.IsEmpty())
			return null;
		
		foreach (SCR_OutfitFactionData data : m_aOutfitFactionData)
		{
			if (data.GetOutfitFactionValue() <= 0)
				continue;
			
			if (data.GetAffiliatedFactionKey() == faction)
				return data;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get specific outfit data of faction (Will return the first valid found)
	//! param[in] faction Faction to find data of
	//! return Found data (Can be null)
	SCR_OutfitFactionData GetOutfitFactionData(notnull Faction faction)
	{		
		foreach (SCR_OutfitFactionData data : m_aOutfitFactionData)
		{
			if (data.GetOutfitFactionValue() <= 0)
				continue;
			
			if (data.GetAffiliatedFaction() == faction)
				return data;
		}
		
		return null;
	}
}

class SCR_ItemOutfitFactionComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_OutfitFactionDataHolder m_OutfitDataHolder;
	
	protected bool m_bInitCalled;
	
	//------------------------------------------------------------------------------------------------
	//! \return True if data on component is valid
	bool IsValid()
	{
		return m_OutfitDataHolder && m_OutfitDataHolder.GetOutfitFactionDataArrayCount() > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Outfit data array
	//! \param[in] outfitValues Array to add data to (Will be cleared)
	//! \return Array Count
	int GetOutfitFactionDataArray(out notnull array<SCR_OutfitFactionData> outfitValues)
	{
		if (!m_OutfitDataHolder)
			return 0;
		
		array<SCR_OutfitFactionData> outfitDataArray = {};
		m_OutfitDataHolder.GetOutfitFactionDataArray(outfitDataArray);
		
		foreach(SCR_OutfitFactionData data : outfitDataArray)
		{
			outfitValues.Insert(data);
		}
		
		return outfitValues.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get specific outfit value of faction (Will return the first valid found)
	//! param[in] faction Faction to find data of
	//! return Found value (Can be null)
	int GetOutfitValueOfFaction(notnull Faction faction)
	{
		if (!m_OutfitDataHolder)
			return 0;
		
		array<SCR_OutfitFactionData> outfitDataArray = {};
		m_OutfitDataHolder.GetOutfitFactionDataArray(outfitDataArray);
		
		foreach(SCR_OutfitFactionData data : outfitDataArray)
		{
			if (data.GetAffiliatedFactionKey() == faction.GetFactionKey())
				return data.GetOutfitFactionValue();
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when an item with this component is added to a character inventory
	//! param[in] characterFactionAffiliationComponent The Faction component to add itself to
	//! param[in] updateFaction If the faction should be updated and replicated when the item is added (Set false and do a recalculation in the end if it is multiple at one)
	void OnAddedToSlot(notnull SCR_CharacterFactionAffiliationComponent characterFactionAffiliationComponent, bool updateFaction = true)
	{		
		if (!IsValid())
			return;
		
		if (!m_bInitCalled)
			Init();
		
		characterFactionAffiliationComponent.AddFactionOutfitValue(m_OutfitDataHolder, updateFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when an item with this component is removed from a character inventory
	//! param[in] characterFactionAffiliationComponent The Faction component to remove itself from
	//! param[in] updateFaction If the faction should be updated and replicated when the item is removed (Set false and do a recalculation in the end if it is multiple at one)
	void OnRemovedFromSlot(notnull SCR_CharacterFactionAffiliationComponent characterFactionAffiliationComponent, bool updateFaction = true)
	{
		if (!IsValid())
			return;
		
		if (!m_bInitCalled)
			Init();
		
		characterFactionAffiliationComponent.RemoveFactionOutfitValue(m_OutfitDataHolder, updateFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		if (m_bInitCalled)
			return;
		
		m_bInitCalled = true;
		
		if (!m_OutfitDataHolder)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			m_OutfitDataHolder = null;
			return;
		}
			
		m_OutfitDataHolder.Init(factionManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
}



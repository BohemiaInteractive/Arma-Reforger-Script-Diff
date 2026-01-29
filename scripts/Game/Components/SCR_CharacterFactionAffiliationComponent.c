[ComponentEditorProps(category: "GameScripted/Faction", description: "")]
class SCR_CharacterFactionAffiliationComponentClass : FactionAffiliationComponentClass
{
}

class SCR_CharacterFactionAffiliationComponent : FactionAffiliationComponent
{
	protected Faction m_PerceivedFaction;
	
	[RplProp(onRplName: "OnPerceivedFactionChanged")]
	protected int m_iOutfitFactionIndex = -1;
	
	protected ref map<Faction, int> m_mOutfitFactionValues = new map<Faction, int>();
	
	protected ref set<SCR_OutfitFactionDataHolder> m_CurrentOutfitData = new set<SCR_OutfitFactionDataHolder>();
	
	protected ref ScriptInvokerFaction m_OnPerceivedFactionChanged;
	
	protected SCR_PerceivedFactionManagerComponent m_PerceivedManager;
	
	//~ What is the relationship between character faction and the perceived faction
	protected SCR_ECharacterDisguiseType m_eDisguiseType = SCR_ECharacterDisguiseType.DEFAULT_FACTION;
	
	//~ Set by SCR_PerceivedFactionManagerComponent. Only for players to avoid unneccessary replication calls
	protected bool m_bHasPerceivedFaction;
	
	protected PerceivableComponent m_PerceivableComponent;
	
	//------------------------------------------------------------------------------------------------
	//! \return If This character's perceived faction is updated and replicated
	bool HasPerceivedFaction()
	{
		return m_bHasPerceivedFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return What is the relationship between character faction and the perceived faction
	SCR_ECharacterDisguiseType GetCharacterDisguiseType()
	{
		if (HasPerceivedFaction())
			return m_eDisguiseType;
		else 
			return SCR_ECharacterDisguiseType.DEFAULT_FACTION;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get current perceived faction. If HasPerceivedFaction is false it will return default faction instead
	Faction GetPerceivedFaction()
	{
		if (!HasPerceivedFaction())
			return GetAffiliatedFaction();
		
		return m_PerceivedFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] Map of Faction with the value each faction currently has
	//! \return Count of entries in Map
	int GetCharacterOutfitValues(out notnull map<Faction, int> outfitFactionValues)
	{
		outfitFactionValues.Copy(m_mOutfitFactionValues);
		
		return outfitFactionValues.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! paramp[out] Get Set of all outfit data. This data
	int GetCharacterOutfitData(out notnull set<SCR_OutfitFactionDataHolder> outfitFactionData)
	{
		outfitFactionData.Copy(m_CurrentOutfitData);
		
		return outfitFactionData.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change a specific faction outfit value
	//! \param[in] faction Faction to change value of
	//! \param[in] value Value added
	//! \param[in] updateFaction If perceived faction should be recalculated and replicated
	void AddFactionOutfitValue(notnull Faction faction, int value, bool updateFaction = true)
	{
		if (value <= 0)
			return;
		
		ChangeFactionOutfitValue(faction, value, updateFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_ItemOutfitFactionComponent when item was added to a slot
	//! \param[in] outfitFactionHolder Outfit data holder
	//! \param[in] updateFaction If perceived faction should be recalculated and replicated
	void AddFactionOutfitValue(SCR_OutfitFactionDataHolder outfitFactionHolder, bool updateFaction = true)
	{
		//~ Null is given (If not set up) or the outfit is already registered
		if (!outfitFactionHolder || m_CurrentOutfitData.Contains(outfitFactionHolder))
			return;
		
		m_CurrentOutfitData.Insert(outfitFactionHolder);
		
		if (!m_PerceivedManager || !HasPerceivedFaction() || m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.DISABLED)
			return;
		
		if (m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.FULL_OUTFIT)
		{
			//~ If Full outfit and the outfit has the default faction then ignore all other outfit factions
			SCR_OutfitFactionData data = outfitFactionHolder.GetOutfitFactionData(GetAffiliatedFaction());
			if (data)
			{
				AddFactionOutfitValue(data.GetAffiliatedFaction(), data.GetOutfitFactionValue());
				return;
			}
		}
		
		array<SCR_OutfitFactionData> outfitDataArray = {};
		outfitFactionHolder.GetOutfitFactionDataArray(outfitDataArray);
		
		//~ Update faction values
		foreach (SCR_OutfitFactionData data : outfitDataArray)
		{
			if (!data || !data.GetAffiliatedFaction())
				continue;
			
			AddFactionOutfitValue(data.GetAffiliatedFaction(), data.GetOutfitFactionValue(), updateFaction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change a specific faction outfit value
	//! \param[in] faction Faction to change value of
	//! \param[in] value Value removed
	//! \param[in] updateFaction If perceived faction should be recalculated and replicated
	void RemoveFactionOutfitValue(notnull Faction faction, int value, bool updateFaction = true)
	{
		if (value <= 0)
			return;
		
		ChangeFactionOutfitValue(faction, -value, updateFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_ItemOutfitFactionComponent when item was removed from slot
	//! \param[in] outfitFactionHolder Outfit data holder
	//! \param[in] updateFaction If perceived faction should be recalculated and replicated
	void RemoveFactionOutfitValue(SCR_OutfitFactionDataHolder outfitFactionHolder, bool updateFaction = true)
	{			
		//~ Did not find the correct data so recalculate
		if (!outfitFactionHolder || !m_CurrentOutfitData.Contains(outfitFactionHolder))
		{
			if (HasPerceivedFaction())
				RecalculateOutfitFaction();
			return;
		}
		
		m_CurrentOutfitData.RemoveItem(outfitFactionHolder);
		
		if (!HasPerceivedFaction() || !m_PerceivedManager || m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.DISABLED)
			return;
		
		if (m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.FULL_OUTFIT)
		{
			//~ If Full outfit and the outfit has the default faction then ignore all other outfit factions
			SCR_OutfitFactionData data = outfitFactionHolder.GetOutfitFactionData(GetAffiliatedFaction());
			if (data)
			{
				RemoveFactionOutfitValue(data.GetAffiliatedFaction(), data.GetOutfitFactionValue());
				return;
			}
		}
		
		array<SCR_OutfitFactionData> outfitDataArray = {};
		outfitFactionHolder.GetOutfitFactionDataArray(outfitDataArray);
		
		//~ Update faction values
		foreach (SCR_OutfitFactionData data : outfitDataArray)
		{
			if (!data || !data.GetAffiliatedFaction())
				continue;
			
			RemoveFactionOutfitValue(data.GetAffiliatedFaction(), data.GetOutfitFactionValue(), updateFaction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_CharacterInventoryStorageComponent when a slotted item is added/removed. If Perceived faction type is FULL_OUTFIT will check if both jacket and pants are still there
	//! \param[in] addedToSlot If the item was added or removed
	//! \param[in] item Item added or removed
	//! \param[in] updateFaction If perceived faction should be recalculated and replicated
	void OnNoFactionSlottedItemChanged(bool addedToSlot, IEntity item, bool updateFaction = true)
	{
		if (!updateFaction || !m_PerceivedManager || m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() != SCR_EPerceivedFactionOutfitType.FULL_OUTFIT)
			return;
		
		//~ Makes sure that if a clothing piece with no faction is added it checks if the uniform is full or not
		SetPerceivedFaction_S(CalculatePerceivedFactionIndex());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPerceivedFactionOutfitTypeChanged(SCR_EPerceivedFactionOutfitType newType)
	{
		RecalculateOutfitFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Recalculates the perceived faction. Called when the Pervceived faction type is changed and after init. Replicates the faction change if server
	void RecalculateOutfitFaction()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		if (!m_PerceivedManager || m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.DISABLED)
		{
			SetPerceivedFaction_S(factionManager.GetFactionIndex(GetAffiliatedFaction()));
			return;
		}
		
		m_mOutfitFactionValues.Clear();
		array<SCR_OutfitFactionData> outfitDataArray = {};
		
		for (int i = m_CurrentOutfitData.Count() - 1; i >= 0; i--)
		{
			//~ Clear any null data
			if (!m_CurrentOutfitData[i])
			{
				m_CurrentOutfitData.Remove(i);
				continue;
			}
			
			if (m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.FULL_OUTFIT)
			{
				//~ If Full outfit and the outfit has the default faction then ignore all other outfit factions
				SCR_OutfitFactionData data = m_CurrentOutfitData[i].GetOutfitFactionData(GetAffiliatedFaction());
				if (data)
				{
					AddFactionOutfitValue(data.GetAffiliatedFaction(), data.GetOutfitFactionValue(), false);
					continue;
				}
			}
			
			m_CurrentOutfitData[i].GetOutfitFactionDataArray(outfitDataArray);
			
			//~ Update faction values
			foreach (SCR_OutfitFactionData data : outfitDataArray)
			{
				if (!data || !data.GetAffiliatedFaction())
					continue;
				
				AddFactionOutfitValue(data.GetAffiliatedFaction(), data.GetOutfitFactionValue(), false);
			}
		}
		
		//~ Update faction
		SetPerceivedFaction_S(CalculatePerceivedFactionIndex());
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Update the given faction outfit value to calculate perceived faction with
	protected void ChangeFactionOutfitValue(Faction faction, int value, bool updateFaction = true)
	{		
		int foundValue;
		
		if (!m_mOutfitFactionValues.Find(faction, foundValue))
		{
			if (value < 0)
			{
				Debug.Error2("SCR_CharacterFactionAffiliationComponent", "ChangeFactionOutfitValue trying to add a new faction unform value for '" + faction + "' but the given value is less than 0!");
				return;
			}
			
			m_mOutfitFactionValues.Insert(faction, value);
		}
		else
		{
			//~ Update the value
			foundValue += value;
			if (foundValue < 0)
				Debug.Error2("SCR_CharacterFactionAffiliationComponent", "ChangeFactionOutfitValue trying to set a new Outfit value for '" + faction + "' but the total value became less than 0!");
			
			//~ Remove the entry from the map as no need to keep unneccessary data
			if (foundValue <= 0)
				m_mOutfitFactionValues.Remove(faction);
			//~ Set the new value
			else 
				m_mOutfitFactionValues[faction] = foundValue;
		}
		
		//~ Update the percieved faction
		if (updateFaction)
			SetPerceivedFaction_S(CalculatePerceivedFactionIndex());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return True if Character is wearing both a jacket and a pants
	bool IsCharacterWearingFullOutfit()
	{
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterStorage)
			return true;
		
		//~ Get blocked slots
		array<typename> blockedSlots = {};
		characterStorage.GetBlockedSlots(blockedSlots);
		
		//~ No jacket
		if (!characterStorage.GetClothFromArea(LoadoutJacketArea) && !blockedSlots.Contains(LoadoutJacketArea))
			return false;
		
		//~ No pants
		if (!characterStorage.GetClothFromArea(LoadoutPantsArea) && !blockedSlots.Contains(LoadoutPantsArea))
			return false;
		
		//~ No shoes
		//if (!characterStorage.GetClothFromArea(LoadoutBootsArea) && !blockedSlots.Contains(LoadoutBootsArea))
		//	return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes all outfit data and returns the faction index of the perceived faction depending on the Perceived faction type set in the manager.
	//! Can return -1 if Faction is unknown
	protected int CalculatePerceivedFactionIndex()
	{
		if (m_mOutfitFactionValues.IsEmpty())
			return -1;
		
		if (m_PerceivedManager && m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.FULL_OUTFIT)
		{
			//~ Cannot have mixed outfit
			if (m_mOutfitFactionValues.Count() > 1)
				return -1;
			
			//~ Check if both torso and pants on
			if (!IsCharacterWearingFullOutfit())
				return -1;
		}
		
		array<Faction> highestFactionValues = {};
		int highestValue = -1;
		
		//~ Get highest faction(s)
		foreach(Faction faction, int value : m_mOutfitFactionValues)
		{
			//~ No highest is set so set first found entry
			if (highestValue < 0)
			{
				highestFactionValues.Insert(faction);
				highestValue = value;
				continue;
			}
			
			//~ New faction has the highest Outfit value
			if (value > highestValue)
			{
				highestFactionValues.Clear();
				highestFactionValues.Insert(faction);
				highestValue = value;
				continue;
			}
			
			//~ Multiple factions have the same Outfit value
			if (value == highestValue)
			{
				highestFactionValues.Insert(faction);
				continue;
			}
		}
		
		if (highestFactionValues.IsEmpty())
			return -1;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return -1;
		
		//~ Only one entry
		if (highestFactionValues.Count() == 1)		
			return factionManager.GetFactionIndex(highestFactionValues[0]);
		
		Faction defaultFaction = GetDefaultAffiliatedFaction();
		
		//~ One of the highest contains it's own default faction so set that as faction
		if (defaultFaction && highestFactionValues.Contains(defaultFaction))
			return factionManager.GetFactionIndex(defaultFaction);
		
		SCR_Faction scrDefaultFaction = SCR_Faction.Cast(defaultFaction);
		Faction factionCandidate;
		SCR_Faction scrfactionCandidate;
		SCR_Faction scrFaction;
		
		//~ Loop through the highest factions and choose the correct one
		foreach (Faction faction : highestFactionValues)
		{
			if (!factionCandidate)
			{
				factionCandidate = faction;
				scrfactionCandidate = SCR_Faction.Cast(factionCandidate);
				continue;
			}
			
			//~ Always give priority to hostile factions
			if (scrDefaultFaction)
			{
				//~ If current candiate is not hostile to player but another one is a hostile. Set that faction
				if (scrDefaultFaction.IsFactionFriendly(factionCandidate) && scrDefaultFaction.IsFactionEnemy(faction))
				{
					factionCandidate = faction;
					continue;
				}	
			}
			
			//~ If candidate is scripted check if new candidate is military
			if (scrfactionCandidate)
			{
				//~ Get scripted versions of the factions
				scrFaction = SCR_Faction.Cast(faction);
			
				//~ Always give priority to military factions
				if (scrFaction && !scrfactionCandidate.IsMilitary() && scrFaction.IsMilitary())
				{
					factionCandidate = faction;
					continue;
				}	
			}
		}
		
		//~ Set the Outfit faction
		return factionManager.GetFactionIndex(factionCandidate);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPerceivedFaction_S(int OutfitFaction)
	{
		//~ Server only
		if (Replication.IsClient())
			return;
		
		if (m_iOutfitFactionIndex == OutfitFaction)
			return;
		
		
		m_iOutfitFactionIndex = OutfitFaction;
		
		OnPerceivedFactionChanged();
		
		//~ Set the perceived Faction on the m_PerceivableComponent
		if (m_PerceivableComponent && m_PerceivedManager && m_PerceivedManager.DoesPerceivedFactionChangesAffectsAI())
			SetPerceivedFactionForAI();
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPerceivedFactionForAI(bool resetToNull = false)
	{
		if (!m_PerceivableComponent)
			return;
		
		//~ Note that currently having no perceived faction is not supported and it will instead reset the perceived faction to default
		if (!resetToNull)
		{
			Faction faction = GetPerceivedFaction();
			
			//~ Set AI faction to perceived faction
			if (faction)
			{
				m_PerceivableComponent.SetPerceivedFactionOverride(faction);
			}
			//~ Set AI faction to fallback faction if faction is null so AI do not perceive the player as their default faction
			else if (m_PerceivedManager)
			{
				m_PerceivableComponent.SetPerceivedFactionOverride(m_PerceivedManager.GetFallbackFaction());
			}
			//~ Sets perceived faction of AI to default faction
			else 
			{
				m_PerceivableComponent.SetPerceivedFactionOverride(null);
			}
		}
			
		else 
		{
			m_PerceivableComponent.SetPerceivedFactionOverride(null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPerceivedFactionChangesAffectsAIChanged(bool affectsAI)
	{
		SetPerceivedFactionForAI(!affectsAI);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPerceivedFactionChanged()
	{
		//~ Faction is unknown
		if (m_iOutfitFactionIndex < 0)
		{
			if (m_PerceivedManager)
			{
				//~ Set faction to default depending on setting
				if (m_PerceivedManager.DoesNoOutfitSetFactionToDefault() && m_PerceivedManager.GetCharacterPerceivedFactionOutfitType() != SCR_EPerceivedFactionOutfitType.FULL_OUTFIT)
					m_PerceivedFaction = GetDefaultAffiliatedFaction();
				else 
					m_PerceivedFaction = null;
			}
			else 
			{
				m_PerceivedFaction = null;
			}
		}
		//~ Set Faction
		else 
		{
			m_PerceivedFaction = GetGame().GetFactionManager().GetFactionByIndex(m_iOutfitFactionIndex);
		}

		m_bHasPerceivedFaction = m_PerceivedFaction != null;

		if (m_OnPerceivedFactionChanged)
				m_OnPerceivedFactionChanged.Invoke(m_PerceivedFaction);

		Faction affiliatedFaction = GetAffiliatedFaction();
		
		//~ Local player changed perceived faction
		if (m_PerceivedManager && SCR_PlayerController.GetLocalMainEntity() == GetOwner() && m_PerceivedFaction != affiliatedFaction)
			m_PerceivedManager.ShowPerceivedFactionChangedHint(m_PerceivedFaction);
		
		//~ Set Disguise type which is the relationship between the affiliated faction and perceived faction
		if (m_PerceivedFaction)
		{
			if (!affiliatedFaction || affiliatedFaction == m_PerceivedFaction)
				m_eDisguiseType = SCR_ECharacterDisguiseType.DEFAULT_FACTION;
			else 
			{
				SCR_Faction scrPerceivedFaction = SCR_Faction.Cast(m_PerceivedFaction);
				
				//~ Is SCR_Faction
				if (scrPerceivedFaction)
				{
					//~ Check if friendly
					if (scrPerceivedFaction.DoCheckIfFactionFriendly(affiliatedFaction))
						m_eDisguiseType = SCR_ECharacterDisguiseType.FRIENDLY_FACTION;
					else 
						m_eDisguiseType = SCR_ECharacterDisguiseType.HOSTILE_FACTION;
				}
				//~ Not an SCR_Faction
				else 
				{
					//~ Check if friendly
					if (m_PerceivedFaction.IsFactionFriendly(affiliatedFaction))
						m_eDisguiseType = SCR_ECharacterDisguiseType.FRIENDLY_FACTION;
					else 
						m_eDisguiseType = SCR_ECharacterDisguiseType.HOSTILE_FACTION;
				}
			}
		}
		//~ Has no perceived faction so disguise is UNKNOWN type
		else 
		{
			m_eDisguiseType = SCR_ECharacterDisguiseType.UNKNOWN_FACTION;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return On Perceived Faction changed script invoker. Called when players perceived faction changes
	ScriptInvokerFaction GetOnOnPerceivedFactionChanged()
	{
		if (!m_OnPerceivedFactionChanged)
			m_OnPerceivedFactionChanged = new ScriptInvokerFaction();
		
		return m_OnPerceivedFactionChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_PerceivedFactionManagerComponent when player is spawned. Is not called for Non-Players
	void InitPlayerOutfitFaction_S()
	{
		InitPlayerOutfitFaction();		
		Rpc(InitPlayerOutfitFaction);
		
		m_PerceivableComponent = PerceivableComponent.Cast(GetOwner().FindComponent(PerceivableComponent));
		
		if (m_PerceivedManager)
			m_PerceivedManager.GetOnPerceivedFactionChangesAffectsAIChanged().Insert(OnPerceivedFactionChangesAffectsAIChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Go over all inventory items and set the uniform faction
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void InitPlayerOutfitFaction()
	{
		m_bHasPerceivedFaction = true;
		
		m_PerceivedManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		
		if (!m_PerceivedManager)
		{
			Print("SCR_CharacterFactionAffiliationComponent: InitPlayerOutfitFaction, Could not find SCR_PerceivedFactionManagerComponent!", LogLevel.WARNING);
			return;
		}
			
		//~ Listen to Type changed
		m_PerceivedManager.GetPerceivedFactionOutfitTypeChanged().Insert(OnPerceivedFactionOutfitTypeChanged);
		
		//~ Clear any data that was there
		m_CurrentOutfitData.Clear();
		
		SCR_CharacterInventoryStorageComponent characterInventoryComponent = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterInventoryComponent)
		{
			Print("SCR_CharacterFactionAffiliationComponent: InitPlayerOutfitFaction, Could not find SCR_CharacterInventoryStorageComponent!", LogLevel.ERROR);
			return;
		}
		
		//~ Init the character inventory comp
		characterInventoryComponent.InitCharacterPerceivedOutfitData(this);
		
		//~ Recalculate the character's faction and replicate this value if Server
		RecalculateOutfitFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disables perceived faction from updating after the character is destroyed or dead
	void DisableUpdatingPerceivedFaction_S()
	{
		if (!m_bHasPerceivedFaction)
			return;
		
		DisableUpdatingPerceivedFaction();
		Rpc(DisableUpdatingPerceivedFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DisableUpdatingPerceivedFaction()
	{
		m_bHasPerceivedFaction = false;
		
		if (!m_PerceivedManager)
			return;
		
		m_PerceivedManager.GetPerceivedFactionOutfitTypeChanged().Remove(OnPerceivedFactionOutfitTypeChanged);
		m_PerceivedManager = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CharacterFactionAffiliationComponent()
	{		
		if (!m_PerceivedManager)
			return;
		
		if (Replication.IsServer())
			m_PerceivedManager.GetOnPerceivedFactionChangesAffectsAIChanged().Remove(OnPerceivedFactionChangesAffectsAIChanged);
		
		m_PerceivedManager.GetPerceivedFactionOutfitTypeChanged().Remove(OnPerceivedFactionOutfitTypeChanged);
	}
}
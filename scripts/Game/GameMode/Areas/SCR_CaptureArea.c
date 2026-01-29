[EntityEditorProps(category: "GameScripted/GameMode", description: "Area that provides basic events and api to serve as a captureable area.")]
class SCR_CaptureAreaClass : ScriptedGameTriggerEntityClass
{
};

//! Callback delegate used by events regarding character(s) in SCR_CaptureArea.
void CaptureAreaCharacterEventDelegate(SCR_CaptureArea area, Faction affiliatedFaction, IEntity character);
typedef func CaptureAreaCharacterEventDelegate;
typedef ScriptInvokerBase<CaptureAreaCharacterEventDelegate> CaptureAreaEvent;

//! Callback delegate used by events regarding faction ownership in SCR_CaptureArea.
void CaptureAreaOwnerFactionEventDelegate(SCR_CaptureArea area, Faction previousOwner, Faction newOwner);
typedef func CaptureAreaOwnerFactionEventDelegate;
typedef ScriptInvokerBase<CaptureAreaOwnerFactionEventDelegate> CaptureAreaOwnershipEvent;

//------------------------------------------------------------------------------------------------
/*!
	Capture area is a trigger entity that provides callback and API in regards
	to characters entering and/or leaving the area. Characters must be assigned
	to a faction to be recognized by this area. In addition, the area itself
	must have periodic update enabled, otherwise callbacks might not be raised.
*/
class SCR_CaptureArea : ScriptedGameTriggerEntity
{
	/*!
		Map of all occupants of this area.
			key: Faction
			value: Array of characters (must be alive)
	*/
	protected ref map<Faction, ref array<SCR_ChimeraCharacter>> m_mOccupants = new map<Faction, ref array<SCR_ChimeraCharacter>>();

	//! Callback raised when a character enters this area
	protected ref CaptureAreaEvent m_pOnCharacterEnter = new CaptureAreaEvent();

	//------------------------------------------------------------------------------------------------
	//! Returns invoker that is invoked when a character enters this area.
	CaptureAreaEvent GetCharacterEnterInvoker()
	{
		return m_pOnCharacterEnter;
	}

	//! Callback raised when a character leaves this area
	protected ref CaptureAreaEvent m_pOnCharacterExit = new CaptureAreaEvent();

	//------------------------------------------------------------------------------------------------
	//! Returns invoker that is invoked when a character exits this area.
	CaptureAreaEvent GetCharacterExitInvoker()
	{
		return m_pOnCharacterExit;
	}

	//! Callback raised when a new faction claims ownership of this area
	protected ref CaptureAreaOwnershipEvent m_pOnOwnershipChanged = new CaptureAreaOwnershipEvent();

	//------------------------------------------------------------------------------------------------
	//! Returns invoker that is invoked when a faction claims ownership of this area.
	CaptureAreaOwnershipEvent GetOwnershipChangedEvent()
	{
		return m_pOnOwnershipChanged;
	}

	//! The faction that currently owns this area.
	protected Faction m_pOwnerFaction;
	//! Returns the faction that currently owns the area or null if none.
	Faction GetOwningFaction() { return m_pOwnerFaction; }

	//! Replication component of this entity.
	protected RplComponent m_pRplComponent;

	//------------------------------------------------------------------------------------------------
	//! Initializes this area by initializing and preallocating required resources.
	protected override void OnInit(IEntity owner)
	{
		// Do not spam messages outside of playmode,
		// these might not be relevant (yet) also
		// there is no need to initialize the entity (just yet)
		if (!GetGame().InPlayMode())
			return;

		// Mandatory, cannot work without factions
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			Debug.Error("No faction manager present in the world! Capture area will malfunction!");
			return;
		}

		// Neccessary to determine replication logic
		m_pRplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if (!m_pRplComponent)
		{
			Debug.Error("SCR_CaptureArea requires RplComponent to function!");
			return;
		}

		// Enable OnFrame event mask
		SetEventMask(EntityEvent.FRAME);

		array<Faction> availableFactions = {};
		factionManager.GetFactionsList(availableFactions);

		// Fetch available actions and preallocate map arrays
		foreach (Faction faction : availableFactions)
			m_mOccupants.Insert(faction, new array<SCR_ChimeraCharacter>());
	}

	/*!
		Called when Item is initialized from replication stream. Carries the data from Master.
	*/
	protected override bool RplLoad(ScriptBitReader reader)
	{
		int factionIndex = -1;
		reader.ReadInt(factionIndex);

		Faction ownerFaction;
		if (factionIndex != -1)
			ownerFaction = GetGame().GetFactionManager().GetFactionByIndex(factionIndex);

		m_pOwnerFaction = ownerFaction;
		return true;
	}

	/*!
		Called when Item is getting replicated from Master to Slave connection. The data will be
		delivered to Slave using RplInit method.
	*/
	protected override bool RplSave(ScriptBitWriter writer)
	{
		int factionIndex = -1;
		if (m_pOwnerFaction)
			factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_pOwnerFaction);

		writer.WriteInt(factionIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! By default queries only for characters of SCR_ChimeraCharacter type
	protected override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		// Filter for characters only
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return false;

		// That are alive.
		return !character.GetCharacterController().IsDead();
	}

	//------------------------------------------------------------------------------------------------
	//! callback - activation - occurs when and entity which fulfills the filter definitions enters the Trigger
	protected override void OnActivate(IEntity ent)
	{
		if (!m_mOccupants)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return;
		
		Faction faction = character.GetFaction();
		if (!faction)
			return;

		if (faction && !m_mOccupants[faction].Contains(character))
		{
			m_mOccupants[faction].Insert(character);
			OnCharacterEntered(faction, character);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Occurs when a [character] of provided [faction] enters the area.
	protected event void OnCharacterEntered(Faction faction, SCR_ChimeraCharacter character)
	{
		m_pOnCharacterEnter.Invoke(this, faction, character);
	}

	//------------------------------------------------------------------------------------------------
	//! callback - deactivation - occurs when and entity which was activated (OnActivate) leaves the Trigger
	protected override void OnDeactivate(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		Faction faction = character.GetFaction();

		if (faction)
		{
			m_mOccupants[faction].RemoveItem(character);
			OnCharacterExit(faction, character);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Occurs when a [character] of provided [faction] leaves the area.
	protected event void OnCharacterExit(Faction faction, SCR_ChimeraCharacter character)
	{
		m_pOnCharacterExit.Invoke(this, faction, character);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Fills the provided [outCharacters] array with all occupants of provided [faction].
		\param faction Faction to filter for
		\param outCharacters Output array
		\return Number of items output
	*/
	int GetOccupants(Faction faction, notnull array<SCR_ChimeraCharacter> outCharacters)
	{
		array<SCR_ChimeraCharacter> characters = m_mOccupants[faction];
		if (!characters || characters.IsEmpty())
			return 0;

		outCharacters.Copy(characters);
		return characters.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the number of occupants of provided [faction].
		\param Faction The faction to return the count for
		\return Returns number of occupants
	*/
	int GetOccupantsCount(Faction faction)
	{
		return m_mOccupants[faction].Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Evaluate and return the faction that owns this point.
		Can be overridden and implemented for custom logic.

		Returns the faction with most players alive in the point.
	*/
	protected Faction EvaluateOwnerFaction()
	{
		// Fetch all available factions
		array<Faction> availableFactions = {};
		GetGame().GetFactionManager().GetFactionsList(availableFactions);

		int maxCount;
		Faction maxFaction;
		int occupantsCount;
		foreach (Faction faction : availableFactions)
		{
			occupantsCount = GetOccupantsCount(faction);
			if (occupantsCount == 0)
				continue;

			if (occupantsCount > maxCount)
			{
				maxCount = occupantsCount;
				maxFaction = faction;
			}
		}

		// With no alive occupants in the area, no faction can
		// be deemed as the capturing one
		if (maxCount == 0)
			return null;

		return maxFaction;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Updates the internal state of the area and reevaluates the owner.
	*/
	protected override event void OnFrame(IEntity owner, float timeSlice)
	{
		super.OnFrame(owner, timeSlice);

		// Since trigger can be out of sync with character states,
		// filter out dead characters if any are left in the collection
		foreach (Faction faction, array<SCR_ChimeraCharacter> characters : m_mOccupants)
		{
			for (int i = characters.Count() - 1; i >= 0; --i)
			{
				SCR_ChimeraCharacter occupant = characters[i];
				if (!occupant || occupant.GetCharacterController().IsDead())
				{
					characters.Remove(i);
					continue;
				}
			}
		}

		// Only the authority will be updating the state,
		// rpl component is prerequisite for this entity
		if (!m_pRplComponent || !m_pRplComponent.IsMaster())
			return;

		Faction newOwner = EvaluateOwnerFaction();
		if (newOwner != m_pOwnerFaction)
		{
			Faction previousOwner = m_pOwnerFaction;
			SetOwningFactionInternal(previousOwner, newOwner);

			// For the authority, this is fired straight away above,
			// so we only send the change to all clients as broadcast
			SetOwningFactionInternalAuthority(previousOwner, newOwner);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets internal owner faction and raises corresponding callback for clients.
	void SetOwningFactionInternalAuthority(Faction previousOwner, Faction newOwner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		int previousIndex = factionManager.GetFactionIndex(previousOwner);
		int newIndex = factionManager.GetFactionIndex(newOwner);
		Rpc(Rpc_SetOwningFaction_BC, previousIndex, newIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets internal owner faction and raises corresponding callback.
	void SetOwningFactionInternal(Faction previousFaction, Faction newFaction)
	{
		m_pOwnerFaction = newFaction;
		OnOwningFactionChanged(previousFaction, newFaction);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Notifies all clients of the owning faction change.
		Index of faction is -1 if null.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetOwningFaction_BC(int previousFactionIndex, int newFactionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();

		Faction previousFaction;
		if (previousFactionIndex != -1)
			previousFaction = factionManager.GetFactionByIndex(previousFactionIndex);

		Faction newFaction;
		if (newFactionIndex != -1)
			newFaction = factionManager.GetFactionByIndex(newFactionIndex);

		SetOwningFactionInternal(previousFaction, newFaction);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Occurs on change of faction ownership of the area.
	 	\param previousFaction Faction which held the point prior to this change or null if none.
		\param newFaction Faction that holds the point after this change or null if none.
	*/
	protected event void OnOwningFactionChanged(Faction previousFaction, Faction newFaction)
	{
		m_pOnOwnershipChanged.Invoke(this, previousFaction, newFaction);
	}
};

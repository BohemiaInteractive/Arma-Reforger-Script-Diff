[EntityEditorProps(category: "GameScripted/GameMode", description: "Area that provides events and API when an enemy enters it.")]
class SCR_SpawnAreaClass : ScriptedGameTriggerEntityClass
{
};

//! Callback delegate used by events regarding character(s) in SCR_CaptureArea.
void SpawnAreaCharacterEventDelegate(SCR_SpawnArea area, IEntity character, bool isFriendly);
typedef func SpawnAreaCharacterEventDelegate;
typedef ScriptInvokerBase<SpawnAreaCharacterEventDelegate> SpawnAreaEvent;

//! Callback delegate raised when the area is occupied by at least one enemy character.
void SpawnAreaAlertEventDelegate(SCR_SpawnArea area, array<IEntity> enemies);
typedef func SpawnAreaAlertEventDelegate;
typedef ScriptInvokerBase<SpawnAreaAlertEventDelegate> SpawnAreaAlertEvent;

//------------------------------------------------------------------------------------------------
/*!
	Spawn area is a trigger entity that provides callback and API in regards
	to enemy characters entering and/or leaving the area. Characters must be assigned
	to a faction to be recognized by this area. In addition, the area itself
	must have periodic update enabled, otherwise callbacks might not be raised.
*/
class SCR_SpawnArea : ScriptedGameTriggerEntity
{
	//! Key of faction this area belongs to
	[Attribute("", UIWidgets.EditBox, "The key specifying which faction this area belongs to.")]
	protected FactionKey m_sFactionKey;

	/*!
		Map of all occupants of this area.
			key: Faction
			value: Array of characters (must be alive)
	*/
	protected ref set<SCR_ChimeraCharacter>> m_sOccupants = new set<SCR_ChimeraCharacter>();

	//! Callback raised when a character enters this area
	protected ref SpawnAreaEvent m_pOnCharacterEnter = new SpawnAreaEvent();

	//! Buffer of all enemy entities for a single frame
	protected ref array<IEntity> m_aEnemies = {};

	//------------------------------------------------------------------------------------------------
	//! Returns invoker that is invoked when a character enters this area.
	SpawnAreaEvent GetCharacterEnterInvoker()
	{
		return m_pOnCharacterEnter;
	}

	//! Callback raised when a character leaves this area
	protected ref SpawnAreaEvent m_pOnCharacterExit = new SpawnAreaEvent();

	//------------------------------------------------------------------------------------------------
	//! Returns invoker that is invoked when a character leaves this area.
	SpawnAreaEvent GetCharacterExitInvoker()
	{
		return m_pOnCharacterExit;
	}

	//! Callback raised when area is occupied by at least one enemy character.
	protected ref SpawnAreaAlertEvent m_pOnAlert = new SpawnAreaAlertEvent();

	//------------------------------------------------------------------------------------------------
	//! Returns invoker that is invoked when area is occupied by at least one enemy character.
	SpawnAreaAlertEvent GetOnAlertInvoker()
	{
		return m_pOnAlert;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the faction this area is affiliated with or null if none.
	Faction GetAffiliatedFaction()
	{
		return GetGame().GetFactionManager().GetFactionByKey(GetAffiliatedFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the faction key of faction this area is affiliated with or empty if none.
	FactionKey GetAffiliatedFactionKey()
	{
		return m_sFactionKey;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Check whether provided character is in this trigger.
		\return Returns true if character is inside of this trigger, false otherwise.
	*/
	bool IsInside(SCR_ChimeraCharacter character)
	{
		if (!character)
			return false;

		return m_sOccupants.Contains(character);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Check whether provided faction is friendly in relation to this area trigger.
		\return Returns true in case faction is friendly, false otherwise.
	*/
	bool IsFriendly(notnull SCR_ChimeraCharacter character)
	{
		Faction faction = character.GetFaction();
		if (!faction)
		{
			if (m_sFactionKey.IsEmpty())
				return true;

			return false;
		}

		Faction areaFaction = GetAffiliatedFaction();
		if (areaFaction && areaFaction.IsFactionFriendly(faction))
			return true;

		return false;
	}


	//------------------------------------------------------------------------------------------------
	/*!
		Fills the provided array with characters of friendly faction present in this area
		and returns the amount of them.
		\param outCharacters The array to clear and will with data.
		\return Returns number of characters provided.
	*/
	int GetFriendlyCharactersInside(out notnull array<SCR_ChimeraCharacter> outCharacters)
	{
		outCharacters.Clear();
		int count = 0;
		foreach (SCR_ChimeraCharacter character : m_sOccupants)
		{
			if (!character)
				continue;

			if (IsFriendly(character))
			{
				outCharacters.Insert(character);
				count++;
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Fills the provided array with characters of enemy faction present in this area
		and returns the amount of them.
		\param outCharacters The array to clear and will with data.
		\return Returns number of characters provided.
	*/
	int GetEnemyCharactersInside(out notnull array<SCR_ChimeraCharacter> outCharacters)
	{
		outCharacters.Clear();
		int count = 0;
		foreach (SCR_ChimeraCharacter character : m_sOccupants)
		{
			if (!character)
				continue;

			if (!IsFriendly(character))
			{
				outCharacters.Insert(character);
				count++;
			}
		}

		return count;
	}


	//------------------------------------------------------------------------------------------------
	/*!
		Fills the provided array with characters present in this area and returns the amount of them.
		\param outCharacters The array to clear and will with data.
		\return Returns number of characters provided.
	*/
	int GetCharactersInside(out notnull array<SCR_ChimeraCharacter> outCharacters)
	{
		outCharacters.Clear();
		int count = 0;
		foreach (SCR_ChimeraCharacter character : m_sOccupants)
		{
			outCharacters.Insert(character);
			count++;
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return false;
		
		return !character.GetCharacterController().IsDead();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsLocalEntity(notnull IEntity ent)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController && playerController.GetControlledEntity() == ent)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! callback - activation - occurs when and entity which fulfills the filter definitions enters the Trigger
	protected override void OnActivate(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return;

		// Push new character into occupants
		if (!m_sOccupants.Contains(character))
		{
			m_sOccupants.Insert(character);

			// Raise callback
			OnCharacterEnter(character, IsFriendly(character));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected event void OnCharacterEnter(IEntity character, bool isFriendly)
	{
		m_pOnCharacterEnter.Invoke(this, character, isFriendly)
	}

	//------------------------------------------------------------------------------------------------
	//! callback - deactivation - occurs when and entity which was activated (OnActivate) leaves the Trigger
	protected override void OnDeactivate(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return;

		if (m_sOccupants.Contains(character))
		{
			int index = m_sOccupants.Find(character);
			m_sOccupants.Remove(index);

			OnCharacterExit(character, IsFriendly(character));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected event void OnCharacterExit(IEntity character, bool isFriendly)
	{
		m_pOnCharacterExit.Invoke(this, character, isFriendly)
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes this area by initializing and preallocating required resources.
	protected override void OnInit(IEntity owner)
	{
		// Enable OnFrame event mask
		SetEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		super.OnFrame(owner, timeSlice);

		m_aEnemies.Clear();
		// Ignore dead characters and find if there is at least one enemy
		foreach (SCR_ChimeraCharacter character : m_sOccupants)
		{
			if (character.GetCharacterController().IsDead())
				continue;

			if (!IsFriendly(character))
				m_aEnemies.Insert(character);
		}

		// Raise alert event if any
		if (!m_aEnemies.IsEmpty())
			OnAlert(m_aEnemies);
	}

	//------------------------------------------------------------------------------------------------
	protected event void OnAlert(array<IEntity> enemies)
	{
		m_pOnAlert.Invoke(this, enemies);
	}
};

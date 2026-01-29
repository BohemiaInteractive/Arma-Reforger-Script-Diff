[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_ExtendedIdentityComponentClass : ScriptComponentClass
{
};
class SCR_ExtendedIdentityComponent : ScriptComponent
{
	[Attribute(desc: "The type of which Identities are obtained from. NEVER CHANGE ON RUN TIME!", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EIdentityType))]
	protected SCR_EIdentityType m_eIdentityType;

	[Attribute(desc: "Holds all the set identity values of the entity")]
	protected ref SCR_ExtendedIdentity m_ExtendedIdentity;

	[Attribute(desc: "Leave empty to auto assign.")]
	protected ref SCR_IdentityBio m_IdentityBio; //~ Bio data

	[Attribute("18", "When randomizing character age. This is the min age the character can be. Age is randomized between min(included) and max(included)", params: "18 inf 1")]
	protected int m_iMinRandomAge;

	[Attribute("26", "When randomizing character age. This is the max age the character can be. Age is randomized between min(included) and max(included)", params: "18 inf 1")]
	protected int m_iMaxRandomAge;

	[Attribute("1", desc: "If true will randomize any identity varriables that are not yet filled in")]
	protected bool m_bRandomizeIdentityOnInit;

	//~ For replication optimalization these two indexes are combined into one int
	protected int m_iBioGroupIndex = -1; //~ Bio's are divided in groups. Each group has a weight which dictates the rarity
	protected int m_iBioIndex = -1; //~ Index of bio within the group

	//~ Seed assigned in randomization. Same for client as it is for server
	protected int m_RandomizeSeed = -1;
	protected ref RandomGenerator m_Randomizer;

	protected bool m_bDelayedInitCalled;

	//======================================== SEEDED RANDOMIZATION ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Returns the assigned randomize seed of the identity
	//! \return Randomize seed
	int GetRandomizeSeed()
	{
		return m_RandomizeSeed;
	}

	//------------------------------------------------------------------------------------------------
	//! Randomize the identity of the entity. (Server only)
	//! Will create a seed. Note that it will not clear any identity already set by prefab or script
	void RandomizeIdentityServer()
	{
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return;

		//~ Randomize seed
		int seed = Math.RandomInt(0, 0x7FFF);

		#ifdef WORKBENCH
		//~ Seed overwrite, Workbench only
		if (identityManager.m_DebugRandomizeSeedOverwrite > -1)
			seed = identityManager.m_DebugRandomizeSeedOverwrite;
		#endif

		//~ Create randomizer on server
		m_Randomizer = new RandomGenerator();
		m_Randomizer.SetSeed(seed);

		//~ Assign a random bio using randomizer
		AssignRandomBioServer();

		//~ Only set seed as bio is al;ready set
		RandomizeIdentityBroadcast(seed, -1);
		//~ Broadcast seed and combined bio group index and bio index
		Rpc(RandomizeIdentityBroadcast, seed, identityManager.CombineBioIndexes(m_iBioGroupIndex, m_iBioIndex));
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RandomizeIdentityBroadcast(int seed, int combinedBioIndex)
	{
		m_RandomizeSeed = seed;
		RandomizeIdentity(combinedBioIndex);
	}

	//------------------------------------------------------------------------------------------------
	//~ Randomizes the identity variables of entity
	protected void RandomizeIdentity(int combinedBioIndex)
	{
		if (m_RandomizeSeed == -1)
		{
			Print("SCR_ExtendedIdentityComponent cannot randomize Identity as seed was not set!", LogLevel.DEBUG);
			return;
		}

		if (!m_ExtendedIdentity)
		{
			Print("SCR_ExtendedIdentityComponent cannot randomize Identity as m_ExtendedIdentity is null!", LogLevel.DEBUG);
			return;
		}

		//~ Create randomizer (Client only)
		if (!m_Randomizer)
		{
			m_Randomizer = new RandomGenerator();
			m_Randomizer.SetSeed(m_RandomizeSeed);

			//~ Bio was assigned by server. In order to keep all other randomization the same it will execute the randomize twice to sync up with server
			//~ The Randomize Bio will first randomize group using randomizer then randomize index on server. Hence it is called here twice
			m_Randomizer.RandInt(0, 1);
			m_Randomizer.RandInt(0, 1);
		}

		//~ Faction always first
		if (m_ExtendedIdentity.GetFactionOfOriginKey().IsEmpty())
			InitFactionOfOrigin();

		//~ Assign bio if none assigned. Always assign bio just after faction
		if (!m_IdentityBio && combinedBioIndex >= 0)
			AssignBio(combinedBioIndex);

		if (m_ExtendedIdentity.GetAge() <= -1)
			RandomizeAge();

		if (m_ExtendedIdentity.GetMonthOfCreation() <= 0)
			SetRandomMonthOfCreation();

		if (m_ExtendedIdentity.GetDayOfCreation() <= 0)
			SetRandomDayOfCreation(m_ExtendedIdentity.GetMonthOfCreation());

		//~ Any other logic that needs to be executed after all inherented version have executed their Identity logic
		GetGame().GetCallqueue().CallLater(DelayedRandomizeIdentity);
	}

	//------------------------------------------------------------------------------------------------
	//~ Called after all other identiy vars are randomzized
	protected void DelayedRandomizeIdentity()
	{
		//~ For characters Place of origin is only randomized on server. So make sure it is called last
		if (!m_ExtendedIdentity.GetPlaceOfOriginUIInfo())
			RandomizePlaceOfOrigin();

		//~ Delete randomizer next frame after all identities are randomized in class (As well as any inhereted classes)
		DeleteRandomizer();
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteRandomizer()
	{
		if (m_Randomizer)
			delete m_Randomizer;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Extended Identity
	//! Holding information from age to place of creation
	//! \return Extended Identity
	SCR_ExtendedIdentity GetExtendedIdentity()
	{
		return m_ExtendedIdentity;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when a bio is overwriting the extended Identity
	//! Note that only values that where set in the new identity will be overwritten on the entity
	//! \param[in] newExtendedIdentity new Extended Identity to get values from
	void OverwriteExtendedIdentity(SCR_ExtendedIdentity newExtendedIdentity)
	{
		if (!newExtendedIdentity || !m_ExtendedIdentity)
			return;

		//~ Faction first
		if (newExtendedIdentity.GetFactionOfOriginIndex() > -1)
			m_ExtendedIdentity.SetFactionOfOrigin(newExtendedIdentity.GetFactionOfOriginIndex());

		if (newExtendedIdentity.GetAge() > -1)
			m_ExtendedIdentity.SetAge(newExtendedIdentity.GetAge());

		if (newExtendedIdentity.GetMonthOfCreation() > 0)
			m_ExtendedIdentity.SetMonthOfCreation(newExtendedIdentity.GetMonthOfCreation());

		if (newExtendedIdentity.GetDayOfCreation() > 0)
			m_ExtendedIdentity.SetDayOfCreation(newExtendedIdentity.GetDayOfCreation());

		if (newExtendedIdentity.GetPlaceOfOriginIndex() > -1)
			 m_ExtendedIdentity.SetPlaceOfOriginIndex(newExtendedIdentity.GetPlaceOfOriginIndex());

		if (newExtendedIdentity.GetPlaceOfOriginUIInfo())
			m_ExtendedIdentity.SetPlaceOfOrigin(newExtendedIdentity.GetPlaceOfOriginUIInfo(), true);
	}

	//======================================== FACTION OF ORIGIN ========================================\\
	//------------------------------------------------------------------------------------------------
	protected void InitFactionOfOrigin()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation || (!factionAffiliation.GetAffiliatedFaction() && !factionAffiliation.GetDefaultAffiliatedFaction()))
			return;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		if (factionAffiliation.GetDefaultAffiliatedFaction())
			m_ExtendedIdentity.SetFactionOfOrigin(factionManager.GetFactionIndex(factionAffiliation.GetDefaultAffiliatedFaction()));
		else
			m_ExtendedIdentity.SetFactionOfOrigin(factionManager.GetFactionIndex(factionAffiliation.GetAffiliatedFaction()));
	}

	//======================================== BIO ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Get Identity Bio
	//! Bio contains flavor text
	//! \return Identity bio
	SCR_IdentityBio GetIdentityBio()
	{
		return m_IdentityBio;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] bioGroupIndex Get Group index
	//! \param[out] bioIndex Get entry index
	//! \return Bio Indexes
	void GetBioIndexes(out int bioGroupIndex, out int bioIndex)
	{
		bioGroupIndex = m_iBioGroupIndex;
		bioIndex = m_iBioIndex;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign random bio (Server Only)
	protected void AssignRandomBioServer()
	{
		if (!m_Randomizer)
			return;

		//~ Faction always first
		if (m_ExtendedIdentity.GetFactionOfOriginKey().IsEmpty())
			InitFactionOfOrigin();

		SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return;

		//~ Assigns the bio and removes it from the randomize pool
		SCR_IdentityBio identityBio = identityManager.AssignRandomAvailableBio(m_Randomizer, GetOwner(), m_eIdentityType, m_ExtendedIdentity.GetFactionOfOriginIndex(), m_iBioGroupIndex, m_iBioIndex);

		#ifdef WORKBENCH
		//~ Bio overwrite, Workbench only
		if (identityManager.m_DebugBioGroupIndexOverwrite > -1)
			m_iBioGroupIndex = identityManager.m_DebugBioGroupIndexOverwrite;

		if (identityManager.m_DebugBioIndexOverwrite > -1)
			m_iBioIndex = identityManager.m_DebugBioIndexOverwrite;

		if (identityManager.m_DebugBioGroupIndexOverwrite > -1 || identityManager.m_DebugBioIndexOverwrite > -1)
			identityBio = identityManager.GetBioFromIndexes(GetOwner(), m_eIdentityType, m_iBioGroupIndex, m_iBioIndex);
		#endif

		if (!identityBio)
		{
			Print(string.Format("'SCR_ExtendedIdentityComponent' given type (%1) and index bio group: %2 and bio index: %3 are invalid!", typename.EnumToString(SCR_EIdentityType, m_eIdentityType), m_iBioGroupIndex, m_iBioIndex),LogLevel.ERROR);
			return;
		}

		AssignBio(identityBio);
	}

	//------------------------------------------------------------------------------------------------
	//! Assign given bio using combined bio index. (Local only is not replicated)
	//! Will also overwrite any indentity values taken from the bio
	//! \param combinedBioIndex Combined bio group index and bio index
	void AssignBio(int combinedBioIndex)
	{
		SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return;

		identityManager.GetBioIndexesFromCombined(combinedBioIndex, m_iBioGroupIndex, m_iBioIndex);
		AssignBio(identityManager.GetBioFromIndexes(GetOwner(), m_eIdentityType, m_iBioGroupIndex, m_iBioIndex));
	}

	//------------------------------------------------------------------------------------------------
	//! Assign given bio. (Local only is not replicated)
	//! Will also overwrite any indentity values taken from the bio
	//! \param identityBio New bio to assign
	void AssignBio(SCR_IdentityBio identityBio)
	{
		m_IdentityBio = identityBio;

		SCR_IdentityBioExtendedOverwrite advanceOverwrite = SCR_IdentityBioExtendedOverwrite.Cast(m_IdentityBio);
		if (!advanceOverwrite)
			return;

		OverwriteExtendedIdentity(advanceOverwrite.GetOverwriteExtendedIdentity());
	}

	//======================================== AGE ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Set controlled random age of entity in years
	protected void RandomizeAge()
	{
		if (!m_Randomizer)
			return;

		m_ExtendedIdentity.SetAge(m_Randomizer.RandIntInclusive(m_iMinRandomAge, m_iMaxRandomAge));
	}

	//======================================== BIRTH/CREATION DATE ========================================\\
	//---------------------------------------- Month of Birth/Creation ----------------------------------------\\
	//! Set random month of birth of character
	protected void SetRandomMonthOfCreation()
	{
		if (!m_Randomizer)
			return;

		m_ExtendedIdentity.SetMonthOfCreation(m_Randomizer.RandIntInclusive(1, 12));
	}

	//---------------------------------------- Day of Birth/Creation ----------------------------------------\\
	//! Set random day of birth of character
	void SetRandomDayOfCreation(int month = -1)
	{
		if (!m_Randomizer)
			return;

		int maxDay = 28;

		//~ No month given so never set day of birth later then 28
		if (month <= 0)
		{
			m_ExtendedIdentity.SetDayOfCreation(m_Randomizer.RandIntInclusive(1, maxDay));
			return;
		}

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		ChimeraWorld world = ChimeraWorld.CastFrom(gameMode.GetWorld());
		if (!world)
			return;

		//~ No time and weather manager so can never check if date is valid. Never set day of birth later then 28
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
		{
			m_ExtendedIdentity.SetDayOfCreation(m_Randomizer.RandIntInclusive(1, maxDay));
			return;
		}

		//~ Get last day of the month (Can never have day of birth on a leap day as 1990 is not a leap day and year is seperate from age)
		while (timeManager.CheckValidDate(1990, month, maxDay +1))
		{
			maxDay++;
		}

		m_ExtendedIdentity.SetDayOfCreation(m_Randomizer.RandIntInclusive(1, maxDay));
	}

	//======================================== PLACE OF CREATION ========================================\\
	//------------------------------------------------------------------------------------------------
	protected void RandomizePlaceOfOrigin()
	{
		if (!m_ExtendedIdentity)
			return;

		int factionOfOrigin = m_ExtendedIdentity.GetFactionOfOriginIndex();
		if (factionOfOrigin <= -1)
			return;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		SCR_Faction scrFaction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionOfOrigin));
		if (!scrFaction)
			return;

		SCR_FactionHomeTerritoryConfig territory = scrFaction.GetFactionHomeTerritoryConfig();
		if (!territory)
			return;

		m_ExtendedIdentity.SetPlaceOfOriginIndex(territory.GetRandomHomeTerritoryIndex(m_Randomizer));
	}

	//======================================== OTHERS ========================================\\
	//------------------------------------------------------------------------------------------------
	SCR_EIdentityType GetIdentityType()
	{
		return m_eIdentityType;
	}

	//======================================== RPL ========================================\\
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
    {
		SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return false;

		writer.WriteInt(m_RandomizeSeed);
		writer.WriteInt(identityManager.CombineBioIndexes(m_iBioGroupIndex, m_iBioIndex));

        return true;
	}

	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		int seed, combinedBioIndex;
		reader.ReadInt(seed);
		reader.ReadInt(combinedBioIndex);

		RandomizeIdentityBroadcast(seed, combinedBioIndex);
		return true;
    }

	//======================================== INIT/DESTROY ========================================\\
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!m_ExtendedIdentity)
		{
			Print("'SCR_ExtendedIdentityComponent' Identity is null", LogLevel.ERROR);
			return;
		}

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode || !gameMode.IsMaster())
			return;

		//~ If entity is not in the same world as the Game mode it is a preview so do not generate the identity
		if (owner.GetWorld() != gameMode.GetWorld())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//~ Add delay so system knows properly that the character is a player or not and all componenst are initialized
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: owner);
	}

	//------------------------------------------------------------------------------------------------
	void DelayedInit(IEntity owner)
	{
		//corner case where DelayedInit gets queued and entity is already deleted.
		if(!owner)
			return;
		
		m_bDelayedInitCalled = true;

		if (m_bRandomizeIdentityOnInit)
			RandomizeIdentityServer();
	}
};

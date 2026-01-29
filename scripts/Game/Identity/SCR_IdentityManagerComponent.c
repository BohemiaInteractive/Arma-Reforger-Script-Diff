[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Character Identity Manager")]
class SCR_IdentityManagerComponentClass: SCR_BaseGameModeComponentClass
{
};

class SCR_IdentityManagerComponent : SCR_BaseGameModeComponent
{
	protected static SCR_IdentityManagerComponent m_sInstance;

	[Attribute("#AR-CharacterIdentity_DateFormating", desc: "Date formatting %1 = day %2 = month %3 = year %4 = age", uiwidget: UIWidgets.LocaleEditBox, category: "Settings")]
	protected LocalizedString m_sBirthDateFormat;

	[Attribute("#AR-Date_Format_MonthFull", desc: "Date formatting %1 = day %2 = month %3 = year", uiwidget: UIWidgets.LocaleEditBox, category: "Settings")]
	protected LocalizedString m_sDateDeathFormat;

	[Attribute("{6075C04F05460FB4}Configs/Identities/BloodTypes.conf", desc: "Holds an array of all possible bloodtypes with randomization weight. Keep combined randomization weight at 100 for ease of use", params: "conf class=SCR_BloodTypeHolderConfig", category: "Data")]
	protected ResourceName m_sCharacterIdentityBloodTypeConfig;

	[Attribute(desc: "Holds Identity groups for each type. Create one entry for each type. ONLY ONE HOLDER PER INDENTIY TYPE!", params: "conf class=SCR_IdentityBioTypeHolderConfig", category: "Data")]
	protected ref array<ResourceName> m_aIdentityBioTypeHoldersConfigs;

	[Attribute((SCR_EIdentityItemGenerationType.ON_POSSESSION + SCR_EIdentityItemGenerationType.ON_DEATH + SCR_EIdentityItemGenerationType.ON_UNCONSCIOUS).ToString(), desc: "Wether or not the Identity Item is spawned for players and for AI and when it happens. Note that the identity item will only be added once to a character this includes if it is added from an arsenal. Possessing also happens when GM takes control of an character.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EIdentityItemGenerationType), category: "Settings")]
	protected SCR_EIdentityItemGenerationType m_eIdentityItemGenerationType;

	[Attribute("1", desc: "If true the identity item will be added to a special slot. (m_eIdentityItemGenerationType on Possession flags needs to be set to enable the slot). The identity item cannot be removed unless the player dies.", category: "Settings")]
	protected bool m_bEnableIdentityItemSlot;

	[Attribute("0", desc: "If true valuable intel will be generated on identity documentation which can be exchanged at arsenals (Will only generate the intel when taken it from arsenal if m_bAddIdentityItemToInventories is false)", category: "Settings")]
	protected bool m_bGenerateValuableIntel;

	[Attribute("1", desc: "If true will show player name instead of Bio name on the identity item of players", category: "Settings")]
	protected bool m_bShowPlayerNameOnIdentityItem;

	[Attribute(desc: "Hints for the extended identity system", category: "Data")]
	protected ref SCR_GeneralHintStorage m_ExtendedIdentityHints;

	//~ Dev only \/
	[Attribute("-1", desc: "Workbench only. Seed overwrite forces all generated Identities to use the same given seed for debugging purposes. Set to any value greater than -1 to enable overwrite", category: "Debug")]
	int m_DebugRandomizeSeedOverwrite;
	[Attribute("-1", desc: "Workbench only. Forced all assigned bio's to use this value as bio group index for debugging purposes. Set to any value that is greater than -1 to enable overwrite", category: "Debug")]
	int m_DebugBioGroupIndexOverwrite;
	[Attribute("-1", desc: "Workbench only. Forced all assigned bio's to use this value as bio index for debugging purposes. Set to any value that is greater than -1 to enable overwrite", category: "Debug")]
	int m_DebugBioIndexOverwrite;
	//~ Dev only /\

	protected const string HAND_IN_VALUABLE_INTEL_SOUNDEVENT = "SOUND_DEPOSITINTEL";

	protected ref array<ref SCR_IdentityBioTypeHolderConfig> m_aIdentityBioTypeHolders = {};

	protected ref SCR_BloodTypeHolderConfig m_BloodTypeHolder;

	//~ Ref
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManager;
	protected SCR_FactionManager m_FactionManager;

	//~ Together the bio groups indexes and Bio indexes will be one int. This makes sure it will never be greater then int.MAX (Which is extreamly unlikly it ever happens)
	static const int MAX_IDENTITY_GROUPS = 9999; //~ Used to combine bio group indexes and bio indexes. (Bio group * (MAX_BIO_GROUPS +1)
	static const int MAX_IDENTITY_ENTRIES = 99999; //~ Used to combine bio group indexes and bio indexes.

	//======================================== INSTANCE ========================================\\
	//------------------------------------------------------------------------------------------------
	static SCR_IdentityManagerComponent GetInstance()
	{
		return m_sInstance;
	}

	//======================================== BIO ========================================\\
	//------------------------------------------------------------------------------------------------
	protected SCR_IdentityBioTypeHolderConfig GetIdentityBioHolder(SCR_EIdentityType identityType)
	{
		foreach(SCR_IdentityBioTypeHolderConfig bioTypeHolder: m_aIdentityBioTypeHolders)
		{
			if (bioTypeHolder && bioTypeHolder.m_eIdentityType == identityType)
				return bioTypeHolder;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Go over all groups and returns group index with given ID string
	//! \param[in] identityType Identity type to obtain Bio info from
	//! \param[in] groupID
	//! \return Index of group, Returns -1 if not found
	int GetBioGroupIndexFromID(SCR_EIdentityType identityType, string bioGroupID)
	{
		if (bioGroupID.IsEmpty())
			return -1;

		SCR_IdentityBioTypeHolderConfig foundBioTypeHolder = GetIdentityBioHolder(identityType);

		if (!foundBioTypeHolder || foundBioTypeHolder.m_aIdentityBioGroups.IsEmpty())
			return -1;

		foreach (int i, SCR_IdentityBioGroupConfig config : foundBioTypeHolder.m_aIdentityBioGroups)
		{
			if (config.GetBioGroupID() == bioGroupID)
				return i;
		}

		PrintFormat("'SCR_CharacterIdentityManagerComponent' bio group '%1' could not be found in 'm_aIdentityBioGroups' for type '%2'", bioGroupID, typename.EnumToString(SCR_EIdentityType, identityType), level: LogLevel.WARNING);
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Get bio from given indexes
	//! \param[in] entity Entity linked to bio
	//! \param[in] identityType Identity type to obtain Bio info from
	//! \param[in] bioGroupIndex group index
	//! \param[in] bioIndex entry index
	//! \return Bio
	SCR_IdentityBio GetBioFromIndexes(IEntity entity, SCR_EIdentityType identityType, int bioGroupIndex, int bioIndex)
	{
		if (bioGroupIndex < 0)
			return null;

		SCR_IdentityBioTypeHolderConfig foundBioTypeHolder = GetIdentityBioHolder(identityType);
		if (!foundBioTypeHolder)
			return null;

		if (bioGroupIndex >= foundBioTypeHolder.m_aIdentityBioGroups.Count())
			return null;

		return foundBioTypeHolder.m_aIdentityBioGroups[bioGroupIndex].GetIdentityBio(entity, bioIndex);

	}

	//------------------------------------------------------------------------------------------------
	//! Combine Bio group index and Bio index into one int for optimized replication
	//! \param[in] bioGroupIndex group index
	//! \param[in] bioIndex entry index
	//! \return Combined index
	int CombineBioIndexes(int bioGroupIndex, int bioIndex)
	{
		return (bioGroupIndex * (MAX_IDENTITY_GROUPS + 1)) + bioIndex;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the combined bio index and seperate the group and the entry index
	//! \param[in] Combined index
	//! \param[out] bioGroupIndex group index
	//! \param[out] bioIndex entry index
	void GetBioIndexesFromCombined(int combinedIndexes, out int bioGroupIndex, out int bioIndex)
	{
		bioGroupIndex = combinedIndexes / (MAX_IDENTITY_GROUPS + 1);
		bioIndex = combinedIndexes - (bioGroupIndex * (MAX_IDENTITY_GROUPS + 1));
	}

	//------------------------------------------------------------------------------------------------
	//! Get list of all valid bio's for given info (Server only)
	//! \param[in] identityType Identity type to obtain Bio info from
	//! \param[in] factionIndex Checks all groups that allow given faction
	//! \param[out] validBioGroups List of all valid Bio groups
	//! \return Count of all valid bio's
	int GetValidBioGroups(IEntity entity, SCR_EIdentityType identityType, int factionIndex, notnull array<ref SCR_IdentityBioGroupConfig> validBioGroups) //~ Todo: Add gender check here!
	{
		validBioGroups.Clear();

		SCR_IdentityBioTypeHolderConfig foundBioTypeHolder = GetIdentityBioHolder(identityType);
		if (!foundBioTypeHolder)
			return 0;

		//~ Get extended Identity
		SCR_ExtendedIdentityComponent extendedIdentity = SCR_ExtendedIdentityComponent.Cast(entity.FindComponent(SCR_ExtendedIdentityComponent));
		SCR_ExtendedCharacterIdentityComponent extendedCharIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(extendedIdentity);

		//~ If character is a player first check if there are any identities unique for the player UID
		if (extendedCharIdentity && extendedCharIdentity.GetPlayerID() > 0)
		{
			//~ Check if the player ID is in any of the unique identity groups for that uniques player UID
			foreach (SCR_IdentityBioGroupConfig bioGroup: foundBioTypeHolder.m_aIdentityBioGroups)
			{
				if (!SCR_UniquePlayerIdentityBioGroupConfig.Cast(bioGroup))
					continue;

				if (!bioGroup.IsEnabled() || !bioGroup.IsValidForRandomization(entity, extendedIdentity) || !bioGroup.IsValidFaction(factionIndex))
					continue;

				//~ Add to valid group
				validBioGroups.Insert(bioGroup);
			}

			//~ Unique IDs were valid
			if (!validBioGroups.IsEmpty())
				return validBioGroups.Count();
		}

		//~ Get any valid identity
		foreach (SCR_IdentityBioGroupConfig bioGroup: foundBioTypeHolder.m_aIdentityBioGroups)
		{
			if (!bioGroup.IsEnabled() || !bioGroup.IsValidForRandomization(entity, extendedIdentity) || !bioGroup.IsValidFaction(factionIndex))
				continue;

			//~ Add to valid group
			validBioGroups.Insert(bioGroup);
		}

		return validBioGroups.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Assign a random valid bio. (Server Only)
	//! NOTE! Make sure that the randomizer used for server and clients use the same seeds and are called at the same time! Else there will be a missmatch in data
	//! \param[in] useRandomWeighted If false will ignore weights in randomization (Will never randomize groups that have no weight assigned)
	//! \param[in] identityType Identity type to obtain Bio info from
	//! \param[in] factionIndex Checks all groups that allow given faction
	//! \param[out] bioGroupIndex Index of Bio Group
	//! \param[out] bioIndex Index of Bio within group
	//! \param[in] randomizer RandomGenerator to use in randomization. Leave null to not use seeded randomization
	//! \return Character bio that was assigned
	SCR_IdentityBio AssignRandomAvailableBio(RandomGenerator randomizer, IEntity entity, SCR_EIdentityType identityType, int factionIndex, out int bioGroupIndex, out int bioIndex, bool useRandomWeighted = true) //Also add gender check
	{
		if (!randomizer)
			return null;

		array<ref SCR_IdentityBioGroupConfig> validBioGroups = {};
		int count = GetValidBioGroups(entity, identityType, factionIndex, validBioGroups);

		if (validBioGroups.IsEmpty())
			return null;

		int randomValidBioIndex = -1;

		//~ Get weighted randomized
		if (useRandomWeighted)
		{
			int totalWeight = 0;

			//~ Get all weights
			foreach (SCR_IdentityBioGroupConfig bioGroup: validBioGroups)
			{
				totalWeight+= bioGroup.GetWeight();
			}

			//~ Randomize with total weights
			int randomWeight = randomizer.RandIntInclusive(0, totalWeight);
			int checkedWeight = 0;

			//~ Find entry within random Weights
			for(int i = 0; i < count; i++)
	        {
				//~ Last in list no need calculate
				if (i == count -1)
				{
					randomValidBioIndex = i;
					break;
				}

				checkedWeight += validBioGroups[i].GetWeight();
				if (randomWeight <= checkedWeight)
				{
					randomValidBioIndex = i;
					break;
				}
	        }
		}
		//~ Get random
		else
		{
			randomValidBioIndex = randomizer.RandInt(0, count);
		}

		bioGroupIndex = GetBioGroupIndexFromID(identityType, validBioGroups[randomValidBioIndex].GetBioGroupID());

		SCR_IdentityBioTypeHolderConfig foundBioTypeHolder = GetIdentityBioHolder(identityType);
		if (!foundBioTypeHolder)
			return null;

		SCR_IdentityBio bio;
		foundBioTypeHolder.m_aIdentityBioGroups[bioGroupIndex].AssignRandomAvailableBio(randomizer, entity, bioIndex, bio);

		return bio;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign a specific given Bio. Makes sure that it is still removed from Available bio's so it cannot be randomized (if bio group has weight) (Server Only)
	//! This Function allows duplicate bio's even unique once. And allows Bio's that are not valid for randomization to be assigned
	//! NOTE: Gender specific bio's require the given entity to be the specific Gender AND the index to be += SCR_IdentityBioGroupConfig.m_aIdentityList.()!
	//! \param[in] identityType Identity type to obtain Bio info from
	//! \param[in] bioGroupIndex group index to assign bio from
	//! \param[in] bioIndex Bio index within group to assign
	//! \return the Assigned Bio
	SCR_IdentityBio AssignBioManually(IEntity entity, SCR_EIdentityType identityType, int bioGroupIndex, int bioIndex)
	{
		SCR_IdentityBioTypeHolderConfig foundBioTypeHolder = GetIdentityBioHolder(identityType);
		if (!foundBioTypeHolder)
			return null;

		if (bioGroupIndex < 0 || bioGroupIndex >= foundBioTypeHolder.m_aIdentityBioGroups.Count())
			return null;

		return foundBioTypeHolder.m_aIdentityBioGroups[bioGroupIndex].OnCharacterBioAssigned(entity, bioIndex);
	}

	//======================================== BIRTH DATE ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Get date of character's birth.
	//! \param[in] identityComponent Identity Component to get Birthday from
	//! \param[out] format Format of birthday/creationday
	//! \param[out] day Day of birth/creation
	//! \param[out] month Month of of Birth/creation
	//! \param[out] age string
	//! \param[out] year Year of birth/creation using character age and current year
	//! \return False if does not have creation string
	bool GetCreationdayString(SCR_ExtendedIdentityComponent identityComponent, out string format, out string day, out string month, out string year, out string age)
	{
		if (!m_TimeAndWeatherManager || !identityComponent)
			return false;

		SCR_ExtendedIdentity extendedIdentity = identityComponent.GetExtendedIdentity();
		if (!extendedIdentity)
			return false;

		format = m_sBirthDateFormat;
		day = extendedIdentity.GetDayOfCreation().ToString();
		month = SCR_DateTimeHelper.GetAbbreviatedMonthString(extendedIdentity.GetMonthOfCreation());
		year = GetYearOfCreation(identityComponent).ToString();
		age = extendedIdentity.GetAge().ToString();

		return !(day.IsEmpty() && month.IsEmpty() && year.IsEmpty() && age.IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	//! Check if today (ingame) is the characters birthday
	//! \param[in] identityComponent Identity Component to get Birthday/creation day from
	//! \return True if it is currently the characters birthday/Entities creation day
	bool IsCreationDay(SCR_ExtendedIdentityComponent identityComponent)
	{
		if (!m_TimeAndWeatherManager || !identityComponent)
			return false;

		SCR_ExtendedIdentity extendedIdentity = identityComponent.GetExtendedIdentity();
		if (!extendedIdentity)
			return -1;

		return m_TimeAndWeatherManager.GetDay()  == extendedIdentity.GetDayOfCreation() && m_TimeAndWeatherManager.GetMonth()  == extendedIdentity.GetMonthOfCreation();
	}

	//------------------------------------------------------------------------------------------------
	//! Get year of birth using character age and current year
	//! param[in] identityComponent Identity Component to get info from
	//! \return Birth year
	int GetYearOfCreation(SCR_ExtendedIdentityComponent identityComponent)
	{
		if (!m_TimeAndWeatherManager || !identityComponent)
			return -1;

		SCR_ExtendedIdentity extendedIdentity = identityComponent.GetExtendedIdentity();
		if (!extendedIdentity)
			return -1;

		return m_TimeAndWeatherManager.GetYear() - extendedIdentity.GetAge();
	}

	//======================================== DEATH DATE ========================================\\
	//------------------------------------------------------------------------------------------------
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		//~ Server only and only for characters
		if (!GetGameMode().IsMaster() || instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.NOT_A_CHARACTER))
			return;

		//~ Spawn Identity Item for AI
		if (HasIdentityItemGenerationType(SCR_EIdentityItemGenerationType.ON_DEATH))
			SpawnIdentityItemInInventory_S(ChimeraCharacter.Cast(instigatorContextData.GetVictimEntity()));

		SCR_ExtendedCharacterIdentityComponent extendedIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(instigatorContextData.GetVictimEntity().FindComponent(SCR_ExtendedCharacterIdentityComponent));
		if (!extendedIdentity)
			return;

		extendedIdentity.OnCharacterDeath();
	}

	//------------------------------------------------------------------------------------------------
	//! Get date and time of character's death. Will return -1 if character has not died yet.
	//! \param[in] identityComponent Identity Component to get Birthday from
	//! \param[out] formatDate Format of date
	//! \param[out] dayString Day of death
	//! \param[out] monthString Month of of death
	//! \param[out] yearString Year of of death
	//! \param[out] time Time of death (Not included in Format in default format settings)
	//! \return False if did not have a death date or time
	bool GetDeathDateAndTimeString(SCR_ExtendedCharacterIdentityComponent identityComponent, out string formatDate, out string dayString, out string monthString, out string yearString, out string time)
	{
		if (!identityComponent)
			return false;

		SCR_ExtendedCharacterIdentity charExtendedIdentity = SCR_ExtendedCharacterIdentity.Cast(identityComponent.GetExtendedIdentity());
		if (!charExtendedIdentity)
			return false;

		int day, month, year, hour, minute;

		//~ Character death date not set
		if (!charExtendedIdentity.GetDeathDateAndTime(day, month, year, hour, minute))
			return false;

		formatDate = m_sDateDeathFormat;

		dayString = day.ToString();
		monthString = SCR_DateTimeHelper.GetAbbreviatedMonthString(month);
		yearString = year.ToString();

		time = SCR_FormatHelper.GetTimeFormattingHoursMinutes(hour, minute);
		return true;
	}

	//======================================== BLOOD TYPE ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Get bloodtype index, either random or weighted random
	//! \param[in] randomizer If set will use given generator to use sseded randomization
	//! \param[in] useWeightedRandom If true will use weighted random
	//! \return Blood index
	int GetRandomBloodTypeIndex(RandomGenerator randomizer = null, bool useWeightedRandom = true)
	{
		if (!m_BloodTypeHolder)
			return -1;

		array<SCR_CharacterIdentityBloodType> bloodTypes = {};
		int count = m_BloodTypeHolder.GetBloodTypes(bloodTypes);

		if (!useWeightedRandom)
		{
			if (count == 0)
				return -1;

			if (randomizer)
			{
				return randomizer.RandInt(0, count);
			}
			else
			{
				return Math.RandomInt(0, count);
			}
		}

		int totalWeight = 0;

		//~ Get all weights
		foreach (SCR_CharacterIdentityBloodType bloodtype: bloodTypes)
		{
			totalWeight += bloodtype.GetWeight();
		}

		//~ Randomize with total weights
		int randomWeight;

		if (randomizer)
			randomWeight = randomizer.RandIntInclusive(0, totalWeight);
		else
			randomWeight = Math.RandomIntInclusive(0, totalWeight);

		int checkedWeight = 0;

		//~ Find entry within random Weights
		foreach (int i, SCR_CharacterIdentityBloodType bloodtype : bloodTypes)
		{
			//~ Last in list no need calculate
			if (i == count -1)
				return i;

            checkedWeight += bloodtype.GetWeight();
			if (randomWeight <= checkedWeight)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Get bloodtype index Ui Info
	//! \param[in] index Index of bloodtype
	//! \return Ui Info
	SCR_UIInfo GetBloodTypeUIInfo(SCR_EBloodType bloodType)
	{
		if (!m_BloodTypeHolder)
			return null;

		return m_BloodTypeHolder.GetBloodTypeUIInfo(bloodType);
	}

	//======================================== PLAYER SPAWN ========================================\\
	//------------------------------------------------------------------------------------------------
	//~ Sets the character Identity to player as the entity itself does not know yet if the character is a player or not
	override bool PreparePlayerEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		SCR_ExtendedCharacterIdentityComponent extendedIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(entity.FindComponent(SCR_ExtendedCharacterIdentityComponent));
		if (extendedIdentity)
			extendedIdentity.SetPlayerID(requestComponent.GetPlayerId());

		return super.PreparePlayerEntity_S(requestComponent, handlerComponent, data, entity);
	}

	//======================================== ON UNCONSCIOUS ========================================\\
	//------------------------------------------------------------------------------------------------
	//~ Called when a character with an SCR_ExtendedCharacterIdentityComponent falls unconscious
	void OnCharacterBecomeUnconscious(IEntity character)
	{
		if (HasIdentityItemGenerationType(SCR_EIdentityItemGenerationType.ON_UNCONSCIOUS))
			SpawnIdentityItemInInventory_S(ChimeraCharacter.Cast(character));
	}

	//======================================== IDENTITY ITEM ========================================\\
	//------------------------------------------------------------------------------------------------
	protected void SpawnIdentityItemInInventory_S(ChimeraCharacter character, bool checkIfHadSpawnedIdentityItem = true)
	{
		if (!character)
			return;

		//~ Did the character already had a identity item?
		if (checkIfHadSpawnedIdentityItem)
		{
			SCR_ExtendedCharacterIdentityComponent extendedIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(character.FindComponent(SCR_ExtendedCharacterIdentityComponent));
			if (extendedIdentity && extendedIdentity.WasIdentityItemAddedOnce_S())
				return;
		}

		//~ Get inventory manager
		InventoryStorageManagerComponent inventoryManager = InventoryStorageManagerComponent.Cast(character.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryManager)
			return;

		//~ Get catalog
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return;

		//~ Get Identity item
 		ResourceName identityItemPrefab = catalogManager.GetIdentityItemForCharacter(character);
		if (identityItemPrefab.IsEmpty())
			return;

		//~ Get the identity item be added to the inventory
		if (!inventoryManager.CanInsertResource(identityItemPrefab))
			return;

		//~ Spawn the identity item
		EntitySpawnParams params = new EntitySpawnParams();
		character.GetTransform(params.Transform);

		IEntity identityItem = GetGame().SpawnEntityPrefab(Resource.Load(identityItemPrefab), null, params);
		if (!identityItem)
			return;

		//~ Try adding the item to jacket if the slot is not enabled
		if (!IsIdentityItemSlotEnabled())
		{
			SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(character.FindComponent(SCR_CharacterInventoryStorageComponent));
			if (characterStorage)
			{
				IEntity jacket = characterStorage.GetClothFromArea(LoadoutJacketArea);
				if (jacket && inventoryManager.TryInsertItemInStorage(identityItem, BaseInventoryStorageComponent.Cast(jacket.FindComponent(BaseInventoryStorageComponent))))
					return;
			}
		}

		//~ Insert the identity item anywhere in the character's inventory
		if (!inventoryManager.TryInsertItem(identityItem))
			delete identityItem;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type Type to check
	//! \return If identity item generation type is true
	bool HasIdentityItemGenerationType(SCR_EIdentityItemGenerationType type)
	{
		return SCR_Enum.HasFlag(m_eIdentityItemGenerationType, type);
	}

	//------------------------------------------------------------------------------------------------
	//! \return If identity item slot is enabled
	bool IsIdentityItemSlotEnabled()
	{
		return m_bEnableIdentityItemSlot && SCR_Enum.HasFlag(m_eIdentityItemGenerationType, SCR_EIdentityItemGenerationType.ON_POSSESSION);
	}

	//------------------------------------------------------------------------------------------------
	//! \return If identity items will be generated with valuable intel
	bool IsGenerateValuableIntelEnabled()
	{
		return m_bGenerateValuableIntel;
	}

	//------------------------------------------------------------------------------------------------
	//! \return If identity items will show the bio name or player name
	bool ShowPlayerNameOnIdentityItem()
	{
		return m_bShowPlayerNameOnIdentityItem;
	}

	//------------------------------------------------------------------------------------------------
	//! \return If identity items will be generated with valuable intel
	static bool Static_IsGenerateValuableIntelEnabled()
	{
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();

		return identityManager && identityManager.IsGenerateValuableIntelEnabled();
	}

	//------------------------------------------------------------------------------------------------
	//~ When entity becomes possessed it will add an identity item
	protected void OnCharacterPossessed(IEntity entity)
	{
		if (!HasIdentityItemGenerationType(SCR_EIdentityItemGenerationType.ON_POSSESSION))
			return;

		SpawnIdentityItemInInventory_S(ChimeraCharacter.Cast(entity));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		if (!GetGameMode().IsMaster())
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!playerController)
			return;

		playerController.m_OnPossessed.Insert(OnCharacterPossessed);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		if (!GetGameMode().IsMaster())
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!playerController)
			return;

		playerController.m_OnPossessed.Remove(OnCharacterPossessed);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when Valuable intel is handed in
	//! param[in] playerController Player controller of player who handed in the valuable intel
	void OnValuableIntelHandIn_S(notnull PlayerController playerController)
	{
		//~ No audio to play
		if (HAND_IN_VALUABLE_INTEL_SOUNDEVENT.IsEmpty())
			return;

		int playerID = playerController.GetPlayerId();

		OnValuableIntelHandIn_RPL(playerID);
		Rpc(OnValuableIntelHandIn_RPL, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnValuableIntelHandIn_RPL(int playerID)
	{
		if (playerID != SCR_PlayerController.GetLocalPlayerId())
			return;

		SCR_UISoundEntity.SoundEvent(HAND_IN_VALUABLE_INTEL_SOUNDEVENT, true);
	}

	//======================================== HINTS ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Show Extended Identity specific hints
	void ShowHint(EHint hintType)
	{
		//! No hints set
		if (!m_ExtendedIdentityHints)
			return;

		//! No hint manager
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;

		SCR_HintUIInfo hint = m_ExtendedIdentityHints.GetHintByType(hintType);
		if (!hint)
			return;

		//! Show the hint
		hintManager.Show(hint);
	}

	//======================================== INIT/DESTROY ========================================\\
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGameMode().GetWorld());
		if (world)
			m_TimeAndWeatherManager = world.GetTimeAndWeatherManager();

		if (!m_TimeAndWeatherManager)
			Print("'SCR_CharacterIdentityManagerComponent' could not find 'TimeAndWeatherManagerEntity' which it needs to function!", LogLevel.ERROR);

		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!m_FactionManager)
		{
			Print("'SCR_CharacterIdentityManagerComponent' could not find 'SCR_FactionManager' which it needs to function!", LogLevel.ERROR);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		if (m_sInstance)
		{
			Print("Multiple instances of SCR_IdentityManagerComponent detected! Only one is allowed!", LogLevel.WARNING);
			return;
		}

		m_sInstance = this;

		SetEventMask(owner, EntityEvent.INIT);

		m_BloodTypeHolder = SCR_BloodTypeHolderConfig.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sCharacterIdentityBloodTypeConfig, true));

		//~ Load the configs
		SCR_IdentityBioTypeHolderConfig loadedConfig

		foreach(ResourceName config : m_aIdentityBioTypeHoldersConfigs)
		{
			loadedConfig = SCR_IdentityBioTypeHolderConfig.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(config, true));
			if (!loadedConfig)
			{
				Print("'SCR_IdentityManagerComponent' failed to load one of the Identity Configs!", LogLevel.ERROR);
				continue;
			}

			m_aIdentityBioTypeHolders.Insert(loadedConfig);
		}

		//~ Safty checks
		foreach(SCR_IdentityBioTypeHolderConfig holder: m_aIdentityBioTypeHolders)
		{
			foreach(SCR_IdentityBioTypeHolderConfig holderCheck: m_aIdentityBioTypeHolders)
			{
				if ( holder.m_eIdentityType == SCR_EIdentityType.NONE)
				{
					Print(string.Format("'SCR_IdentityManagerComponent' Bio group holder has '%1' entity type! This should not happen!", typename.EnumToString(SCR_EIdentityType, holder.m_eIdentityType)), LogLevel.ERROR);
					continue;
				}

				if (holderCheck != holder && holderCheck.m_eIdentityType == holder.m_eIdentityType)
				{
					Print(string.Format("'SCR_IdentityManagerComponent' one or more bio group holder of the same Identity type: '%1' in array. Each Identity Type can only have one holder!", typename.EnumToString(SCR_EIdentityType, holder.m_eIdentityType)), LogLevel.ERROR);
					break;
				}
			}

			foreach(SCR_IdentityBioGroupConfig identityBioGroup: holder.m_aIdentityBioGroups)
			{
				foreach(SCR_IdentityBioGroupConfig identityBioGroupCheck: holder.m_aIdentityBioGroups)
				{
					if (identityBioGroup.GetBioGroupID() == string.Empty)
					{
						Print(string.Format("'SCR_IdentityManagerComponent' Identity group in type '%1' has empty ID string!", typename.EnumToString(SCR_EIdentityType, holder.m_eIdentityType)), LogLevel.WARNING);
						continue;
					}

					if (identityBioGroupCheck != identityBioGroup && identityBioGroup.GetBioGroupID() == identityBioGroupCheck.GetBioGroupID())
					{
						Print(string.Format("'SCR_IdentityManagerComponent' Identity group in type '%1' has duplicate or empty ID: '%2' in array. Each Identity group needs to have an unique ID!", typename.EnumToString(SCR_EIdentityType, holder.m_eIdentityType), identityBioGroupCheck.GetBioGroupID()), LogLevel.ERROR);
						break;
					}
				}
			}
		}

		super.OnPostInit(owner);
	}
};

//! When will the identity item be generated on the character
enum SCR_EIdentityItemGenerationType
{
	ON_POSSESSION = 1 << 0, //!< When an character is possessed by a player (On player spawned or GM possessing) and it has never had an identity Item one will be added to the inventory
	ON_DEATH = 1 << 1, //!< When an character dies an identity item will be added to his/her inventory. If no Identity item was ever added to the character
	ON_UNCONSCIOUS = 1 << 2, //!< When an character falls unconscious an identity item will be added to his/her inventory. If no Identity item was ever added to the character
}
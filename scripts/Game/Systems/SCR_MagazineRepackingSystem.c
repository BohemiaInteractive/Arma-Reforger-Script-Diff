class SCR_MagazineRepackingSystem : GameSystem
{
	protected ref map<SCR_ChimeraCharacter, ref SCR_MagRepackingData> m_mRepackingData;
	protected ref array<ResourceName> m_aAmmoTypes;
	protected ref array<string> m_aCaliberTypes;
	protected ref set<ref SCR_MagazineComponentClass> m_aInitializationQueue;
	protected bool m_bIsServer;

	private const ref array<SCR_ChimeraCharacter> GARBAGE = {};

	const ResourceName AMMO_LIST = "{DAA31F8A78F30455}Configs/Weapons/Ammo/AmmoList.conf";

	protected static SCR_EMagazineRepackingRulesOverride s_GameModeRepackingRule;

	//------------------------------------------------------------------------------------------------
	//! Returns currently used magazine repacking rule override 
	static SCR_EMagazineRepackingRulesOverride GetGameModeRepackingRule()
	{
		return s_GameModeRepackingRule;
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Both);
		outInfo.AddPoint(WorldSystemPoint.PostFixedFrame);
	}

	//------------------------------------------------------------------------------------------------
	//! \return magazine repacking system instance
	static SCR_MagazineRepackingSystem GetInstance()
	{
		const World world = GetGame().GetWorld();
		if (world)
			return SCR_MagazineRepackingSystem.Cast(world.FindSystem(SCR_MagazineRepackingSystem));

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if system was initialized, otherwise false
	bool IsReady()
	{
		return m_aAmmoTypes != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds magazine data to the initialization queue which will be processed when system is ready
	//! \param[in] data of the magazine that should be initialized
	void AddToTheInitialiationQueue(notnull SCR_MagazineComponentClass data)
	{
		if (IsReady())
		{
			data.SetupAmmoComposition(this);
			return;
		}

		if (!m_aInitializationQueue)
			m_aInitializationQueue = new set<ref SCR_MagazineComponentClass>();

		m_aInitializationQueue.Insert(data);
	}

	//------------------------------------------------------------------------------------------------
	//! Fetches the unique id of provided caliber, and automatically registers it if provided caliber wasnt registered yet
	//! \param[in] caliber name
	//! \return unique id
	int GetCaliberId(string caliber)
	{
		if (!m_aCaliberTypes)
		{
			m_aCaliberTypes = {};
			m_aCaliberTypes.Insert(caliber);
			return 0;
		}

		int id = m_aCaliberTypes.Find(caliber);
		if (id < 0)
		{
			m_aCaliberTypes.Insert(caliber);
			return m_aCaliberTypes.Count() - 1;
		}

		return id;
	}

	//------------------------------------------------------------------------------------------------
	//! Starts repacking procedure for provided character, or in case of a proxy it will start playback of the sound events
	//! \param[in] character who will be repacking the magazines
	//! \param[in] sourceMag magazine from which ammunition will be removed. This is only present for the authority
	//! \param[in] targetMag magazine to which ammunition will be added. This is only present for the authority
	//! \param[in] transferTime used by proxies to determine how often sound event should be played
	void StartRepackingMagazines(notnull SCR_ChimeraCharacter character, SCR_MagazineComponent sourceMag, SCR_MagazineComponent targetMag, float transferTime = -1)
	{
		if (!m_mRepackingData)
			m_mRepackingData = new map<SCR_ChimeraCharacter, ref SCR_MagRepackingData>();

		if (!m_bIsServer)
		{
			if (transferTime <= 0)
				return;

			SCR_MagRepackingData data = m_mRepackingData.Get(character);
			if (!data)
			{
				data = new SCR_MagRepackingData(transferTime);
				m_mRepackingData.Set(character, data);
			}
			else
			{
				data.SetTimePerRound(transferTime);
			}

			Enable(true);
			return;
		}

		if (!sourceMag || !targetMag)
			return; // server is doing the ammo transfer, and he must have source and target

		SCR_MagRepackingData data = m_mRepackingData.Get(character);
		if (data)
			StopRepackingMagazines(character);

		if (!targetMag.CanBeRepacked(sourceMag))
			return;

		IEntity magEntity = sourceMag.GetOwner();
		SCR_MagazineComponentClass sourceMagData = SCR_MagazineComponentClass.Cast(sourceMag.GetComponentData(magEntity));
		transferTime = sourceMagData.GetRoundExtractionTime();
		RequestItemLock(magEntity, character, true);

		magEntity = targetMag.GetOwner();
		SCR_MagazineComponentClass targetMagData = SCR_MagazineComponentClass.Cast(targetMag.GetComponentData(magEntity));
		transferTime += targetMagData.GetRoundLoadingTime();
		transferTime *= 0.001; // switch from amount of ms to s, so its easier to compute via time slice of the update
		RequestItemLock(magEntity, character, true);

		data = new SCR_MagRepackingData(transferTime, sourceMag, targetMag, character.GetOrigin(), character.GetCharacterController().GetStance());
		m_mRepackingData.Set(character, data);
		character.SetRepackingState_S(true, transferTime);
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		controller.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
		Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Stops repacking procedure from transferring ammunition for provided character
	//! \param[in] character
	void StopRepackingMagazines(notnull SCR_ChimeraCharacter character)
	{
		if (!m_mRepackingData)
			return;

		SCR_MagRepackingData data = m_mRepackingData.Get(character);
		if (!data) // if there is no data for this character, then there is nothing to do
			return;

		IEntity magEntity;
		SCR_MagazineComponent magComp = data.GetSourceMagazine();
		if (magComp)
		{
			magEntity = magComp.GetOwner();
			RequestItemLock(magEntity, character, false);
		}

		magComp = data.GetTargetMagazine();
		if (magComp)
		{
			magEntity = magComp.GetOwner();
			RequestItemLock(magEntity, character, false);
		}

		// set to null instead of out right removing to prevent issues if it were to happen while array is processed in the update
		m_mRepackingData.Set(character, null);
		if (!m_bIsServer)
			return;

		character.SetRepackingState_S(false);
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		controller.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Locks provided item from being interacted with
	//! \param[in] item which is being locked
	//! \param[in] lockingCharacter who imposes the lock
	//! \param[in] newLockState true to lock, false to unlock
	protected void RequestItemLock(notnull IEntity item, notnull ChimeraCharacter lockingCharacter, bool newLockState)
	{
		InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (iic)
			iic.RequestUserLock(lockingCharacter, newLockState);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_mRepackingData || m_mRepackingData.IsEmpty())
		{
			Enable(false);
			return;
		}

		float timeSlice = args.GetTimeSliceSeconds();
		CharacterControllerComponent controller;
		CompartmentAccessComponent compartmentAccessComp;
		SCR_MagazineComponent sourceMag;
		SCR_MagazineComponent targetMag;
		const SCR_ChimeraCharacter localCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		foreach (SCR_ChimeraCharacter character, SCR_MagRepackingData data : m_mRepackingData)
		{
			if (!character || !data)
			{
				GARBAGE.InsertAt(character, 0);
				continue;
			}

			if (data.m_fTimeSinceLastTransfer <= data.GetTimePerRound())
			{
				data.m_fTimeSinceLastTransfer += timeSlice;
				continue;
			}

			data.m_fTimeSinceLastTransfer -= data.GetTimePerRound();
			if (!m_bIsServer)
			{
				PlayRepackingSound(character);
				if (character == localCharacter)
					character.UpdateLocalMagazineUi();

				continue;
			}

			controller = character.GetCharacterController();
			if (controller.IsChangingStance() || controller.GetStance() != data.GetLastStance())
			{
				StopRepackingMagazines(character);
				GARBAGE.InsertAt(character, 0);
				continue;
			}

			if (controller.IsWeaponRaised() || controller.IsUsingItem())
			{
				StopRepackingMagazines(character);
				GARBAGE.InsertAt(character, 0);
				continue;
			}

			if (character.IsInVehicle())
			{
				compartmentAccessComp = character.GetCompartmentAccessComponent();
				if (compartmentAccessComp.IsInCompartmentADS() || compartmentAccessComp.IsSwitchingSeatsAnim() || compartmentAccessComp.IsGettingOut() || compartmentAccessComp.IsGettingIn())
				{
					StopRepackingMagazines(character);
					GARBAGE.Insert(character);
					continue;
				}
			}
			else if (!data.CheckPosition(character.GetOrigin()))
			{
				StopRepackingMagazines(character);
				GARBAGE.Insert(character);
				continue;
			}

			sourceMag = data.GetSourceMagazine();
			targetMag = data.GetTargetMagazine();
			if (!sourceMag || !targetMag)
			{
				StopRepackingMagazines(character);
				GARBAGE.Insert(character);
				continue;
			}

			PlayRepackingSound(character);

			int sourceAmmoCount = sourceMag.GetAmmoCount() - 1;
			sourceMag.SetAmmoCount(sourceAmmoCount);

			int targetAmmoCount = targetMag.GetAmmoCount() + 1;
			targetMag.SetAmmoCount(targetAmmoCount);

			if (sourceAmmoCount <= 0)
			{
				StopRepackingMagazines(character);
				GARBAGE.Insert(character);
				if (sourceMag.IsDeletedWhenEmpty())
					RplComponent.DeleteRplEntity(sourceMag.GetOwner(), false);

				continue;
			}

			if (targetAmmoCount >= targetMag.GetMaxAmmoCount())
			{
				StopRepackingMagazines(character);
				GARBAGE.Insert(character);
				continue;
			}

			if (character == localCharacter)
				character.UpdateLocalMagazineUi(sourceMag, targetMag);
		}

		if (GARBAGE.IsEmpty())
			return;

		foreach (SCR_ChimeraCharacter key : GARBAGE)
		{
			m_mRepackingData.Remove(key);
		}

		GARBAGE.Clear();
		if (m_mRepackingData.IsEmpty())
			Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays repacking sound from provided character
	//! \param[in] character
	protected void PlayRepackingSound(notnull SCR_ChimeraCharacter character)
	{
		CharacterSoundComponent sound = CharacterSoundComponent.Cast(character.FindComponent(CharacterSoundComponent));
		if (sound)
			sound.SoundEvent(SCR_SoundEvent.SOUND_CHAR_MAG_REPACK);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	protected void OnPlayerDisconnected(int playerID)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		if (char)
			StopRepackingMagazines(char);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] previousLifeState
	//! \param[in] newLifeState
	//! \param[in] isJIP
	//! \param[in] controller
	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState, bool isJIP, SCR_CharacterControllerComponent controller)
	{
		if (newLifeState == ECharacterLifeState.ALIVE)
			return;

		SCR_ChimeraCharacter char = controller.GetCharacter();
		if (char)
			StopRepackingMagazines(char);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] projectilePrefab
	//! \return unique id of the projectile
	int GetAmmoTypeId(ResourceName projectilePrefab)
	{
		return m_aAmmoTypes.Find(projectilePrefab);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInit()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
			s_GameModeRepackingRule = gameMode.GetMagazineRepackingRulesOverride();
		}

		m_bIsServer = GetNode().GetRole() == RplRole.Authority;

		Resource res = BaseContainerTools.LoadContainer(AMMO_LIST);
		if (!res.IsValid())
			return;

		BaseContainer container = res.GetResource().ToBaseContainer();
		if (!container)
			return;

		SCR_AmmoList ammoConfig = SCR_AmmoList.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!ammoConfig || !ammoConfig.m_aAmmoList || ammoConfig.m_aAmmoList.IsEmpty())
			return;

		m_aAmmoTypes = {};
		foreach (ResourceName projectilePrefab : ammoConfig.m_aAmmoList)
		{
			if (m_aAmmoTypes.Contains(projectilePrefab))
				continue;

			m_aAmmoTypes.Insert(projectilePrefab); // this will ensure that even if multiple mods are adding the same projectile, then it will be unique list
		}

		if (m_aInitializationQueue)
		{
			foreach (SCR_MagazineComponentClass data : m_aInitializationQueue)
			{
				data.SetupAmmoComposition(this);
			}
	
			m_aInitializationQueue = null;
		}

		Enable(false);
	}
}

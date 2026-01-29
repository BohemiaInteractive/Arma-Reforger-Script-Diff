[EntityEditorProps("GameScripted/Gadgets", description: "Mortar shell that can be picked up.")]
class SCR_MortarShellGadgetComponentClass : SCR_GadgetComponentClass
{
	[Attribute(desc: "If this shell is using timed fuze")]
	protected bool m_bIsUsingTimeFuze;

	[Attribute("15", desc: "Minimal time in seconds that this projectile can be set to [s]", params: "1 inf")]
	protected float m_fMinFuzeTime;

	[Attribute("15", desc: "Maximal time in seconds that this projectile can be set to [s]", params: "1 inf")]
	protected float m_fMaxFuzeTime;

	[Attribute("100", desc: "What altitude in meters above the impact point should be used for automatically setting the detonation time [m]", params: "1 inf")]
	protected float m_fDetonationAltitude;

	[Attribute("1", desc: "Fuze time offset applied based on the time that it takes to travel the distance specified in Detonation Altitude attribute [s]", params: "0.01 inf")]
	protected float m_fVerticalImpactTimeOffset;

	[Attribute(desc: "1. Number of charge rings.\n2. Projectile speed coef.\n3. Is this the default amount of charge rings.", params: "0 inf")]
	protected ref array<vector> m_aChargeRingConfig;

	//------------------------------------------------------------------------------------------------
	bool IsUsingTimeFuze()
	{
		return m_bIsUsingTimeFuze;
	}

	//------------------------------------------------------------------------------------------------
	float GetMinFuzeTime()
	{
		return m_fMinFuzeTime;
	}

	//------------------------------------------------------------------------------------------------
	float GetMaxFuzeTime()
	{
		return m_fMaxFuzeTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Overrides configuration of time fuze usage for all entities of this prefab
	//! \param[in] shouldBeUsed is set to true if time fuze should be used
	//! \param[in] min fuze time value
	//! \param[in] max fuze time value
	void OverrideTimeFuzeConfig(bool shouldBeUsed = false, float min = 0, float max = 0)
	{
		if (min == max)
		{
			m_bIsUsingTimeFuze = false;
			return;
		}

		m_bIsUsingTimeFuze = shouldBeUsed;
		if (min > max)
		{
			m_fMinFuzeTime = max;
			m_fMaxFuzeTime = min;
		}
		else
		{
			m_fMinFuzeTime = min;
			m_fMaxFuzeTime = max;
		}
	}

	//------------------------------------------------------------------------------------------------
	float GetDetonationAltitude()
	{
		return m_fDetonationAltitude;
	}

	//------------------------------------------------------------------------------------------------
	float GetVerticalImpactTimeOffset()
	{
		return m_fVerticalImpactTimeOffset;
	}

	//------------------------------------------------------------------------------------------------
	int GetNumberOfChargeRingConfigurations()
	{
		return m_aChargeRingConfig.Count();
	}

	//------------------------------------------------------------------------------------------------
	vector GetChargeRingConfig(int id)
	{
		if (!m_aChargeRingConfig || !m_aChargeRingConfig.IsIndexValid(id))
			return -vector.One;

		return m_aChargeRingConfig[id];
	}

	//------------------------------------------------------------------------------------------------
	//! Searches for the first charge ring config that is marked as default
	//! \param[out] foundConfig or negative vector.One if no default one was found
	//! \return id of the default config or -1 when such is not found
	int FindDefaultChargeRingConfig(out vector foundConfig = -vector.One)
	{
		foundConfig = -vector.One;
		if (!m_aChargeRingConfig || m_aChargeRingConfig.IsEmpty())
		{
			return -1;
		}

		int configId = 0;
		foreach (int i, vector config : m_aChargeRingConfig)
		{
			if (float.AlmostEqual(config[2], 0))
				continue;

			foundConfig = config;
			configId = i;
			break;
		}

		// this is possible when someone has forgotten to set at least one of the configs to be marked as default
		// in such case we treat the first config as the default one
		if (foundConfig == -vector.One)
		{
			foundConfig = m_aChargeRingConfig[configId];
			Print("SCR_MortarShellGadgetComponent.FindDefaultChargeRingConfig: WARNING! There is not default config, as all configs have Z set to value different than 1! First config is going to be used as default for now, but this must be fixed in the prefab!", LogLevel.ERROR);
		}

		return configId;
	}
}

class SCR_MortarShellGadgetComponent : SCR_GadgetComponent
{
	[RplProp(onRplName: "OnChargeRingReplicated")]
	protected int m_iChargeRingConfigIndex;

	protected ref ScriptInvokerVoid m_OnShellUsed;
	protected SCR_MortarMuzzleComponent m_MortarMuzzleComp;
	protected SlotManagerComponent m_SlotManagerComp;
	protected string m_sRegisteredActionName;
	protected float m_fFuzeTime;
	protected bool m_bCustomFuzeTimeSet;
	protected bool m_bCustomChargeRingSet;

	static const protected string CHARGE_RING_CONFIG_MEMBER_NAME = "m_aChargeRingConfig";

	//------------------------------------------------------------------------------------------------
	//! Extracts the information about available init speed coef from resource name
	//! \param[in] resourceName prefab from data will be extracted
	//! \param[out] defaultConfigId id of the entry which is the default init speed coef
	//! \return array containing all available init speed coef for this prefab
	static array<float> GetPrefabInitSpeedCoef(ResourceName resourceName, out int defaultConfigId)
	{
		Resource res = Resource.Load(resourceName);
		if (!res.IsValid())
			return null;

		BaseResourceObject resObject = res.GetResource();
		if (!resObject)
			return null;

		IEntitySource entitySrc = resObject.ToEntitySource();
		if (!entitySrc)
			return null;

		IEntityComponentSource componentSrc;
		array <float> output = {};
		array <vector> chargeRingConfigs = {};
		defaultConfigId = -1;
		for (int i, componentsCount = entitySrc.GetComponentCount(); i < componentsCount; i++)
		{
			componentSrc = entitySrc.GetComponent(i);
			if (!componentSrc)
				continue;

			if (!componentSrc.GetClassName().ToType().IsInherited(SCR_MortarShellGadgetComponent))
				continue;

			componentSrc.Get(CHARGE_RING_CONFIG_MEMBER_NAME, chargeRingConfigs);
			if (!chargeRingConfigs || chargeRingConfigs.IsEmpty())
				return null;

			foreach (vector entry : chargeRingConfigs)
			{
				output.Insert(entry[1]);
				if (defaultConfigId == -1 && !float.AlmostEqual(entry[2], 0))
					defaultConfigId = i;//if there are multiple we only care about first one that was marked as default
			}

			break;
		}

		return output;
	}

	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.NONE;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeRaised()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnShellUsed()
	{
		if (!m_OnShellUsed)
			m_OnShellUsed = new ScriptInvokerVoid();

		return m_OnShellUsed;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsLoaded()
	{
		return m_bActivated;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsUsingTimeFuze()
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.IsUsingTimeFuze();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetFuzeTime()
	{
		return m_fFuzeTime;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMinFuzeTime()
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 1;

		return data.GetMinFuzeTime();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxFuzeTime()
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 1;

		return data.GetMaxFuzeTime();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetDetonationAltitude()
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 1;

		return data.GetDetonationAltitude();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfChargeRingConfigurations()
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		return data.GetNumberOfChargeRingConfigurations();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetCurentChargeRingConfig()
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return -vector.One;

		return data.GetChargeRingConfig(m_iChargeRingConfigIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCurentChargeRingConfigId()
	{
		return m_iChargeRingConfigIndex;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetChargeRingConfig(int id)
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return -vector.One;

		return data.GetChargeRingConfig(id);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method that will be triggered when m_iChargeRingConfigIndex is bumped for replication
	protected void OnChargeRingReplicated()
	{
		SetChargeRingConfig(m_iChargeRingConfigIndex, false, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the number of charge rings that are going to be used to fire this projectile
	//! \param[in] configId id of the config that is going ot be used by the projectile
	//! \param[in] silent determines if audio should be played
	//! \param[in] replicate determines if it should attempt to replicate change to the currently selected charge ring config - this can only be done by the owner
	void SetChargeRingConfig(int configId, bool silent = false, bool replicate = true)
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		vector previousConfig = data.GetChargeRingConfig(m_iChargeRingConfigIndex);
		vector newConfig = data.GetChargeRingConfig(configId);
		m_bCustomChargeRingSet = true;
		if (newConfig == -vector.One)
		{
			configId = data.FindDefaultChargeRingConfig(newConfig);
			if (configId == -1)
				configId = 0;

			m_bCustomChargeRingSet = false;
		}

		int chargeRingDif = newConfig[0] - previousConfig[0];
		m_iChargeRingConfigIndex = configId;
		
		const IEntity owner = GetOwner();
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(owner.FindComponent(ProjectileMoveComponent));
		if (!moveComp)
			return;

		moveComp.SetBulletCoef(newConfig[1]);
		VisualiseChargeRings(newConfig[0]);

		if (!silent)
		{
			if (chargeRingDif > 0)
				SCR_SoundManagerModule.CreateAndPlayAudioSource(owner, SCR_SoundEvent.SOUND_SHELL_CHARGE_RING_ADD);
			else if (chargeRingDif < 0)
				SCR_SoundManagerModule.CreateAndPlayAudioSource(owner, SCR_SoundEvent.SOUND_SHELL_CHARGE_RING_REMOVE);
		}

		if (!replicate)
			return;

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(GetOwner());
		if (!rplComp || !rplComp.IsOwner())
			return;

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Method that is meant to be called when it is firred from a mortar
	//! \param[in] muzzle dfrom which this shell was fired
	void OnShellFired(notnull BaseMuzzleComponent muzzle)
	{
		SCR_VisibleInventoryItemComponent inventoryItemComp = SCR_VisibleInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_VisibleInventoryItemComponent));
		if (inventoryItemComp)
			inventoryItemComp.SetHiddenInVicinity(true);

		//Charge rings burn up in order to propel the projectile and because of that they shouldnt be visible when fired
		VisualiseChargeRings(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows or hides charge ring visualiastion that are defined in prefab slot manager
	//! \param[in] numberOfChargeRings determines how many charge rings should be visible
	protected void VisualiseChargeRings(int numberOfChargeRings)
	{
		array<EntitySlotInfo> outSlotInfos = {};
		if (!m_SlotManagerComp || m_SlotManagerComp.GetSlotInfos(outSlotInfos) < 1)
			return;

		IEntity chargeRing;
		foreach (int i, EntitySlotInfo slot : outSlotInfos)
		{
			if (!slot)
				continue;

			chargeRing = slot.GetAttachedEntity();
			if (!chargeRing)
				continue;

			if (i < numberOfChargeRings)
				chargeRing.SetFlags(EntityFlags.VISIBLE);
			else
				chargeRing.ClearFlags(EntityFlags.VISIBLE);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state if shell should be locked and
	//! \param[in] mortarMuzzle muzzle from which mortar this shell is going to be fired
	//! \param[in] fireActionName name of the input action for which game should wait with transfering the item to mortar inventory
	//! \param[in] user one who loads the shell
	//! \return true if caller should wait for user to activate the gadget
	bool SetLoadedState(bool state, SCR_MortarMuzzleComponent mortarMuzzle = null, string fireActionName = "", IEntity user = null)
	{
		m_bActivated = state;
		m_MortarMuzzleComp = mortarMuzzle;
		if (!state)
		{
			GetGame().GetInputManager().RemoveActionListener(m_sRegisteredActionName, EActionTrigger.DOWN, UseShellCB);
			return false;
		}

		if (!user || fireActionName.IsEmpty())
			return false;

		if (user != SCR_PlayerController.GetLocalControlledEntity())
			return true;

		m_sRegisteredActionName = fireActionName;
		GetGame().GetInputManager().AddActionListener(m_sRegisteredActionName, EActionTrigger.DOWN, UseShellCB);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback used to tell if user wants to fire the shell now
	protected void UseShellCB()
	{
		GetGame().GetInputManager().RemoveActionListener(m_sRegisteredActionName, EActionTrigger.DOWN, UseShellCB);

		ToggleActive(true, SCR_EUseContext.CUSTOM);
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{
		if (state && m_OnShellUsed)
			m_OnShellUsed.Invoke();

		if(m_MortarMuzzleComp)
		{
			m_MortarMuzzleComp.SetLoadingState(false);
			SetLoadedState(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);

		if (mode != EGadgetMode.IN_HAND)
			return;

		if (m_bCustomChargeRingSet)
			return;

		if (!charOwner || charOwner != SCR_PlayerController.GetLocalControlledEntity())
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(charOwner);
		if (!character)
			return;

		SCR_ShellConfig config = GetSavedConfig(character);
		if (!config || config.GetChargeRingConfigId() == m_iChargeRingConfigIndex)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;

		controller.SyncShellChargeRingConfig(this, config.GetChargeRingConfigId());
		SetChargeRingConfig(config.GetChargeRingConfigId(), true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear gadget mode
	//! \param[in] mode that is being cleared
	override protected void ModeClear(EGadgetMode mode)
	{
		if (mode != EGadgetMode.IN_HAND)
			return;

		if (!m_MortarMuzzleComp)
			return;

		m_MortarMuzzleComp.SetLoadingState(false);
		SetLoadedState(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] time
	void SetFuzeTime(float time)
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		if (!data.IsUsingTimeFuze())
			return;

		m_fFuzeTime = Math.Clamp(time, data.GetMinFuzeTime(), data.GetMaxFuzeTime());

		TimerTriggerComponent trigger = TimerTriggerComponent.Cast(GetOwner().FindComponent(TimerTriggerComponent));
		if (!trigger)
			return;

		trigger.SetTimer(m_fFuzeTime);
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves locally saved data about manual adjustment of the fuze that player set for this shell type
	SCR_ShellConfig GetSavedConfig(notnull ChimeraCharacter character)
	{
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return null;

		EntityPrefabData shellPrefab = GetOwner().GetPrefabData();
		if (!shellPrefab)
			return null;

		return controller.GetSavedShellConfig(shellPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves information about the time of flight for this shell prefab
	float GetTimeToDetonation(float angle, bool isDirectFire)
	{
		IEntity owner = GetOwner();
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return 0;

		EntityPrefabData prefabData = owner.GetPrefabData();
		if (!prefabData)
			return 0;

		BaseContainer prefab = prefabData.GetPrefab();
		if (!prefab)
			return 0;

		IEntitySource entitySrc = prefab.ToEntitySource();
		if (!entitySrc)
			return 0;

		vector chargeConfig = data.GetChargeRingConfig(m_iChargeRingConfigIndex);
		angle = angle * Math.DEG2RAD;
		float distance1, distance2, travelTime1, travelTime2;
		distance1 = BallisticTable.GetDistanceOfProjectileSource(angle, travelTime1, entitySrc, chargeConfig[1], isDirectFire);

		float detonationAltitude = data.GetDetonationAltitude();
		distance2 = distance1 + detonationAltitude * 0.5;
		if (isDirectFire)
			distance2 = distance1 - detonationAltitude * 0.5;

		BallisticTable.GetHeightFromProjectileSource(distance1, travelTime1, entitySrc, chargeConfig[1], isDirectFire);
		BallisticTable.GetHeightFromProjectileSource(distance2, travelTime2, entitySrc, chargeConfig[1], isDirectFire);

		if (travelTime2 < 0)
		{
			distance2 = distance1 - detonationAltitude * 0.5;
			if (isDirectFire)
				distance2 = distance1 + detonationAltitude * 0.5;

			BallisticTable.GetHeightFromProjectileSource(distance2, travelTime2, entitySrc, chargeConfig[1], isDirectFire);
			travelTime2 = Math.Lerp(travelTime1, travelTime1 - Math.AbsFloat(travelTime1 - travelTime2), 0.9);
		}

		return travelTime2 - Math.Max(data.GetVerticalImpactTimeOffset() - Math.AbsFloat(travelTime1 - travelTime2), 0);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsUsingTimeFuze())
		{
			if (!owner.FindComponent(TimerTriggerComponent))
			{
				Print("SCR_MortarShellGadgetComponent.EOnInit: WARNING! "+owner.GetPrefabData().GetPrefabName()+" is missing TimerTriggerComponent and because of that fuze adjustment will not be avialable!", LogLevel.ERROR);
				data.OverrideTimeFuzeConfig(false);
			}

			if (data.GetMaxFuzeTime() < data.GetMinFuzeTime())
			{
				Print("SCR_MortarShellGadgetComponent.EOnInit: WARNING! MaxFuzeTime is less than MinFuzeTime for "+owner.GetPrefabData().GetPrefabName()+". Please fix this in the actual prefab data!", LogLevel.WARNING);
				data.OverrideTimeFuzeConfig(data.IsUsingTimeFuze(), data.GetMaxFuzeTime(), data.GetMinFuzeTime());
			}

			if (data.GetMaxFuzeTime() == data.GetMinFuzeTime())
			{
				Print("SCR_MortarShellGadgetComponent.EOnInit: WARNING! Prefab "+owner.GetPrefabData().GetPrefabName()+" has IsUsingTimeFuze set to true, while MIN and MAX times are the same. Because of that fuze time adjustment will not be avialable!.", LogLevel.ERROR); 
				data.OverrideTimeFuzeConfig(false);
			}
			else
			{
				SetFuzeTime(data.GetMinFuzeTime());
			}
		}

		m_SlotManagerComp = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
		m_iChargeRingConfigIndex = data.FindDefaultChargeRingConfig();
		SetChargeRingConfig(m_iChargeRingConfigIndex, true, false);
		m_bCustomChargeRingSet = false;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsUsingTimeFuze())
			writer.WriteFloat(m_fFuzeTime);

		writer.WriteBool(m_bCustomChargeRingSet);

		SCR_VisibleInventoryItemComponent inventoryItemComp = SCR_VisibleInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_VisibleInventoryItemComponent));
		if (inventoryItemComp)
			writer.WriteBool(inventoryItemComp.ShouldHideInVicinity());

		return super.RplSave(writer);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsUsingTimeFuze())
		{
			reader.ReadFloat(m_fFuzeTime);
			SetFuzeTime(m_fFuzeTime);
		}

		reader.ReadBool(m_bCustomChargeRingSet);

		SCR_VisibleInventoryItemComponent inventoryItemComp = SCR_VisibleInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_VisibleInventoryItemComponent));
		if (inventoryItemComp)
		{
			bool hiddenInVicinity;
			reader.ReadBool(hiddenInVicinity);
			if (hiddenInVicinity)//we only hide it when it was fired and in such case charge rings shouldnt be visible
				VisualiseChargeRings(0);
		}

		return super.RplLoad(reader);
	}
}

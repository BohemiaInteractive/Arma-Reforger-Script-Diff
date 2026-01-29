class SCR_MortarMuzzleComponentClass : MortarMuzzleComponentClass
{
	[Attribute(desc: "Name of the action for which user will wait befor transfering the shell to the mortar - f.e. CharacterFire which is LMB")]
	protected string m_sFireActionName;

	[Attribute(desc: "Name of the signal that will be used to adjust the volume of the fired projectile based on the number of used charge rings")]
	protected string m_sFirePowerSignalName;
	
	[Attribute("45 85 0", desc: "Minimum (X) and maximum (Y) vertical angle for the mortar's muzzle")]
	protected vector m_vVerticalAngleLimits;

	//------------------------------------------------------------------------------------------------
	string GetFireActionName()
	{
		return m_sFireActionName;
	}

	//------------------------------------------------------------------------------------------------
	string GetFirePowerSignalName()
	{
		return m_sFirePowerSignalName;
	}

	//------------------------------------------------------------------------------------------------
	vector GetVerticalAngleLimits()
	{
		return m_vVerticalAngleLimits;
	}
}

class SCR_MortarMuzzleComponent : MortarMuzzleComponent
{

	[Attribute(desc: "Position and rotation that will be used to play animation of dropping the shell")]
	protected ref PointInfo m_LoaderPosition;

	[Attribute(desc: "Position and rotation that will be used to play animation of dropping the shell but from the left side")]
	protected ref PointInfo m_LoaderPositionLeft;

	protected SCR_CharacterControllerComponent m_CharController;
	protected SCR_MortarShellGadgetComponent m_ShellComponent;
	protected TurretControllerComponent m_TurretController;
	protected AnimationEventID m_MortarFireEnd = -1;
	protected AnimationEventID m_MortarFireReady = -1;
	protected AnimationEventID m_MortarFireStart = -1;
	protected bool m_bLoadedFromLeftSide;
	protected bool m_bBeingLoaded;

	protected const string ANIM_EVENT_NAME_FIRE = "MortarFireReady";
	protected const string ANIM_EVENT_NAME_START = "MortarFireStart";
	protected const string ANIM_EVENT_NAME_END = "Event_DetachCharacter";
	protected const string ANIM_VARIABLE_FIRING = "Firing";
	protected const string ANIM_VARIABLE_LEFT_SIDE = "IsLeftSide";
	protected const string ANIM_BIND_COMMAND = "CMD_Item_Action";
	protected const string ANIM_ENTRY_POINT = "FireMasterControl";
	protected const string ANIM_BINDING_NAME = "Weapon";

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_WeaponAttachmentsStorageComponent attachmentStorage = SCR_WeaponAttachmentsStorageComponent.Cast(owner.FindComponent(SCR_WeaponAttachmentsStorageComponent));
		if (attachmentStorage)
			attachmentStorage.m_OnItemAddedToSlotInvoker.Insert(OnShellLoaded);

		if (m_LoaderPosition)
			m_LoaderPosition.Init(owner);

		if (m_LoaderPositionLeft)
			m_LoaderPositionLeft.Init(owner);

		SCR_MuzzleEffectComponent muzzleEffectComp = SCR_MuzzleEffectComponent.Cast(FindComponent(SCR_MuzzleEffectComponent));
		if (muzzleEffectComp)
			muzzleEffectComp.GetOnWeaponFired().Insert(OnWeaponFired);

		m_TurretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool IsBeingLoaded()
	{
		return m_bBeingLoaded;
	}

	//------------------------------------------------------------------------------------------------
	void SetLoadingState(bool newState)
	{
		m_bBeingLoaded = newState;
	}

	//------------------------------------------------------------------------------------------------
	vector GetMuzzleDirection()
	{
		BaseWeaponManagerComponent weaponManager = m_TurretController.GetWeaponManager();
		if (!weaponManager)
			return vector.Zero;

		vector transform[4];
		weaponManager.GetCurrentMuzzleTransform(transform);
		return transform[2];
	}

	//------------------------------------------------------------------------------------------------
	float GetMuzzleElevation()
	{
		return GetMuzzleDirection().VectorToAngles()[1];
	}

	//------------------------------------------------------------------------------------------------
	//! Call back method that is triggered on item insertion and is used to play shell loading sound
	//! \param[in] item that was inserted
	//! \param[in] slotID
	void OnShellLoaded(IEntity item, int slotID)
	{
		if (!item)
			return;

		SCR_MortarShellGadgetComponent shellGadget = SCR_MortarShellGadgetComponent.Cast(item.FindComponent(SCR_MortarShellGadgetComponent));
		if (!shellGadget)
			return;

		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComp)
			return;

		soundComp.SoundEvent(SCR_SoundEvent.SOUND_LOAD);
	}

	//------------------------------------------------------------------------------------------------
	//! Starts the animation of dropping the shell in the mortar barrel.
	//! \param[in] shellComp from loaded shell
	//! \param[in] character loader who will be responsible for the damage that shell will cause
	//! \param[in] fuzeTime when set to -1 then new one will be calculated based on curent muzzle elevation
	//! \param[in] fromLeftSide if loading animation should use left point info
	void LoadShell(notnull SCR_MortarShellGadgetComponent shellComp, notnull ChimeraCharacter character, float fuzeTime = -1, bool fromLeftSide = false)
	{
		m_CharController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!m_CharController)
			return;

		SCR_MortarMuzzleComponentClass data = SCR_MortarMuzzleComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		if (!m_CharController.CanPlayItemGesture())
			return;

		if (shellComp.IsUsingTimeFuze())
		{
			if (fuzeTime >= 0)
			{
				shellComp.SetFuzeTime(fuzeTime);
			}
			else
			{
				float elevation = GetMuzzleElevation();
				float timeToDetonation = shellComp.GetTimeToDetonation(elevation, false);
				shellComp.SetFuzeTime(timeToDetonation);
			}
		}

		SCR_WeaponBlastComponent weaponBlastComponent = SCR_WeaponBlastComponent.Cast(GetOwner().FindComponent(SCR_WeaponBlastComponent));
		if (weaponBlastComponent)
			weaponBlastComponent.OverrideInstigator(character);

		SignalsManagerComponent signalsMgr = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		if (!data.GetFirePowerSignalName().IsEmpty() && signalsMgr)
		{
			int signalId = signalsMgr.AddOrFindSignal(data.GetFirePowerSignalName()); // if such signal does not exist then we need to create it for long range sound replication to work properly
			if (signalId > -1)
				signalsMgr.SetSignalValue(signalId, shellComp.GetCurentChargeRingConfig()[0]);
		}

		m_bBeingLoaded = true;

		bool isAi;
		if (!SCR_CharacterHelper.IsPlayerOrAIOwner(character, isAi))
		{
			shellComp.SetLoadedState(true, this);
			m_bLoadedFromLeftSide = fromLeftSide;
			m_MortarFireStart = GameAnimationUtils.RegisterAnimationEvent(ANIM_EVENT_NAME_START);
			m_CharController.GetOnAnimationEvent().Insert(OnAnimationEvent);
			return;//reject those who arent in control of the loader but do it only now to inform them of who did it
		}

		m_MortarFireStart = -1;//clear it to ensure that loader doesnt try to reset the animation attributes
		CharacterAnimationComponent animationComponent = m_CharController.GetAnimationComponent();
		if (!animationComponent)
			return;

		if (isAi)
		{
			RplComponent loaderRplComp = character.GetRplComponent();
			if (!loaderRplComp)
				return;

			RplComponent shellRplComp = SCR_EntityHelper.GetEntityRplComponent(shellComp.GetOwner());
			if (!shellRplComp)
				return;

			//Because AI is not broadcasting their user action usage this has to be done manually for them
			Rpc(SynchronizeLoaderData, loaderRplComp.Id(), shellRplComp.Id(), shellComp.GetFuzeTime(), fromLeftSide);
		}

		m_ShellComponent = shellComp;

		m_MortarFireReady = GameAnimationUtils.RegisterAnimationEvent(ANIM_EVENT_NAME_FIRE);
		m_MortarFireEnd = GameAnimationUtils.RegisterAnimationEvent(ANIM_EVENT_NAME_END);
		int itemActionId = animationComponent.BindCommand(ANIM_BIND_COMMAND);

		ItemUseParameters animParams = ItemUseParameters();
		animParams.SetEntity(GetOwner());
		if (fromLeftSide)
			animParams.SetAlignmentPoint(m_LoaderPositionLeft);
		else
			animParams.SetAlignmentPoint(m_LoaderPosition);

		animParams.SetAllowMovementDuringAction(false);
		animParams.SetKeepInHandAfterSuccess(false);
		animParams.SetKeepGadgetVisible(true);
		animParams.SetIsMainUserOfTheItem(false);
		animParams.SetCommandID(itemActionId);
		animParams.SetItemGraphEntryPoint(ANIM_ENTRY_POINT);
		animParams.SetCharGraphBindingName(ANIM_BINDING_NAME);
		if (m_CharController.TryUseItemOverrideParams(animParams))
		{
			SetAnimationAttributes(m_CharController, fromLeftSide);
			m_CharController.GetOnAnimationEvent().Insert(OnAnimationEvent);
			m_CharController.m_OnItemUseEndedInvoker.Insert(OnAnimationEnded);
		}
		else
		{
			TransferShellToMortar();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to set currently used animation attributes
	//! \param[in] controller of the character that is going to be executing the animation
	//! \param[in] loadedFromLeftSide
	protected void SetAnimationAttributes(notnull CharacterControllerComponent controller, bool loadedFromLeftSide)
	{
		CharacterAnimationComponent animationComponent = controller.GetAnimationComponent();
		if (!animationComponent)
			return;

		SCR_MortarMuzzleComponentClass data = SCR_MortarMuzzleComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		const vector angleLimit = data.GetVerticalAngleLimits();
		float elevation = Math.Clamp(GetMuzzleElevation(), angleLimit[0], angleLimit[1]);
		animationComponent.SetAnimAimY(elevation);

		TAnimGraphVariable varId = animationComponent.BindVariableBool(ANIM_VARIABLE_LEFT_SIDE);
		if (varId >= 0)
			animationComponent.SetSharedVariableBool(varId, loadedFromLeftSide, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to trigger mortar recoil animation
	//! \param[in] effectEntity
	//! \param[in] muzzle
	//! \param[in] projectileEntity
	void OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		m_bBeingLoaded = false;
		MortarAnimationComponent animationComponent = MortarAnimationComponent.Cast(GetOwner().FindComponent(MortarAnimationComponent));
		if (animationComponent)
		{
			TAnimGraphVariable varId = animationComponent.BindBoolVariable(ANIM_VARIABLE_FIRING);
			animationComponent.SetBoolVariable(varId, true);
			GetGame().GetCallqueue().CallLater(ResetRecoilAnimationVariable, 50, param1: varId); //delyaed to allow the animation system to see the change of the value
		}

		if (!projectileEntity)
			return;

		SCR_MortarShellGadgetComponent shellComp = SCR_MortarShellGadgetComponent.Cast(projectileEntity.FindComponent(SCR_MortarShellGadgetComponent));
		if (shellComp)
			shellComp.OnShellFired(this);

		if (!m_CharController)
			return;

		ChimeraCharacter blamedCharacter = FindResponsibleCharacter(m_CharController);
		if (!blamedCharacter)
			return;

		BaseProjectileComponent projectileComp = BaseProjectileComponent.Cast(projectileEntity.FindComponent(BaseProjectileComponent));
		if (!projectileComp)
			return;

		projectileComp.GetInstigator().SetInstigator(blamedCharacter);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used determine who should be blamed for the damage that shell will do
	//! \param[in] controller of the loader
	//! \return character that should be considered a responsible party for the damage that shell will do
	protected ChimeraCharacter FindResponsibleCharacter(notnull SCR_CharacterControllerComponent controller)
	{
		ChimeraCharacter blamedCharacter = ChimeraCharacter.Cast(controller.GetOwner());
		if (!blamedCharacter)
			return null;

		AIControlComponent aiControlComp = controller.GetAIControlComponent();
		if (!aiControlComp)
			return blamedCharacter;

		AIAgent agent = aiControlComp.GetControlAIAgent();
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		if (!group && agent)
			group = SCR_AIGroup.Cast(agent.GetParentGroup());

		if (!group)
			return blamedCharacter;

		SCR_AIWaypointArtillerySupport currentWaypoint = SCR_AIWaypointArtillerySupport.Cast(group.GetCurrentWaypoint());
		if (!currentWaypoint)
			return blamedCharacter;

		int playerId = group.GetFirstPlayerLeaderID();
		if (playerId > 0)//make SL responsible for all kills if he ordered the AI to fire the mortar
			blamedCharacter = ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));

		return blamedCharacter;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to reset the animation variable to its default value
	void ResetRecoilAnimationVariable(TAnimGraphVariable varId)
	{
		MortarAnimationComponent animationComponent = MortarAnimationComponent.Cast(GetOwner().FindComponent(MortarAnimationComponent));
		if (!animationComponent)
			return;

		animationComponent.SetBoolVariable(varId, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback called when animation event happens.
	//! \param[in] animEventType ID of animation event.
	//! \param[in] animUserString String parameter of animation event
	//! \param[in] intParam int parameter of animation event
	//! \param[in] timeFromStart time from Start of animation event
	//! \param[in] timeToEnd time to end of animation event
	//! \param[in] controller which triggered this event
	protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd, SCR_CharacterControllerComponent controller)
	{
		if (m_CharController && animEventType == m_MortarFireStart)
		{
			SetAnimationAttributes(m_CharController, m_bLoadedFromLeftSide);
			m_CharController.GetOnAnimationEvent().Remove(OnAnimationEvent);
		}

		if (animEventType == m_MortarFireEnd && controller)
		{
			CharacterAnimationComponent animationComponent = controller.GetAnimationComponent();
			if (!animationComponent)
				return;

			CharacterCommandHandlerComponent commandHandler = animationComponent.GetCommandHandler();
			if (commandHandler)
				commandHandler.FinishItemUse(false);
			
			if (m_CharController)
			{
				CameraHandlerComponent cameraHandler = m_CharController.GetCameraHandlerComponent();
				if (cameraHandler)
				{
					//! Reset camera override so it does not follow animation
					ScriptedCameraItem camera = cameraHandler.GetCurrentCamera();
					if (camera && camera.GetOverrideDirectBoneMode() != 0)
						camera.OverrideDirectBoneMode(0);
				}
			}

			return;
		}

		if (!m_ShellComponent)
			return;

		if (animEventType != m_MortarFireReady)
			return;

		IEntity owner;
		if (m_CharController)
			owner = m_CharController.GetOwner();

		SCR_MortarMuzzleComponentClass data = SCR_MortarMuzzleComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		if (data && m_ShellComponent.SetLoadedState(true, this, data.GetFireActionName(), owner))
			m_ShellComponent.GetOnShellUsed().Insert(TransferShellToMortar);
		else
			TransferShellToMortar();
		
		//! Override camera to follow animation while firing
		if (m_CharController && !m_CharController.IsInThirdPersonView())
		{
			CameraHandlerComponent cameraHandler = m_CharController.GetCameraHandlerComponent();
			if (cameraHandler)
			{
				ScriptedCameraItem camera = cameraHandler.GetCurrentCamera();
				if (camera && camera.GetOverrideDirectBoneMode() == 0)
				{
					vector aimingAngles = m_CharController.GetInputContext().GetAimingAngles();
					m_CharController.GetInputContext().SetAimingAngles(Vector(aimingAngles[0], 0, 0));
					camera.OverrideDirectBoneMode(EDirectBoneMode.RelativeTransform);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Moves currently loaded shell from loaders inventory to the mortar inventory
	//! It will also attempt to find next shell that should be equipped when current shell is successfully transferred
	protected void TransferShellToMortar()
	{
		if (!m_ShellComponent)
			return;

		m_ShellComponent.GetOnShellUsed().Remove(TransferShellToMortar);
		m_ShellComponent.UpdateVisibility(m_ShellComponent.GetMode());
		m_ShellComponent.SetLoadedState(false);

		if (!m_CharController)
			return;

		SCR_InventoryStorageManagerComponent playerInventory = SCR_InventoryStorageManagerComponent.Cast(m_CharController.GetInventoryStorageManager());
		if (!playerInventory)
			return;

		SCR_WeaponAttachmentsStorageComponent weaponAttachmentStorage = SCR_WeaponAttachmentsStorageComponent.Cast(GetOwner().FindComponent(SCR_WeaponAttachmentsStorageComponent));
		if (!weaponAttachmentStorage)
			return;

		SCR_InvEquipAnyItemCB inventoryCallBack = new SCR_InvEquipAnyItemCB();
		IEntity shellEntity = m_ShellComponent.GetOwner();
		if (shellEntity)
			playerInventory.TryAssigningNextItemToQuickSlot(shellEntity, equipCallback: inventoryCallBack, equipNewItem: true);

		playerInventory.TryMoveItemToStorage(shellEntity, weaponAttachmentStorage, cb: inventoryCallBack);
		m_ShellComponent = null;

		CharacterAnimationComponent animationComponent = m_CharController.GetAnimationComponent();
		if (!animationComponent)
			return;

		TAnimGraphVariable varId = animationComponent.BindVariableBool(ANIM_VARIABLE_FIRING);
		//inform animation graph of used by the loader that charater loaded the shell to the mortar
		animationComponent.SetVariableBool(varId, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback called when animation of using the item ends
	//! \param[in] item animated item
	//! \param[in] successful if animation was able to finish or was interrupted
	//! \param[in] animParams parameters that were used to play this animation
	protected void OnAnimationEnded(IEntity item, bool successful, ItemUseParameters animParams)
	{
		m_bBeingLoaded = false;
		if (!m_CharController)
			return;

		m_CharController.GetOnAnimationEvent().Remove(OnAnimationEvent);
		m_CharController.m_OnItemUseEndedInvoker.Remove(OnAnimationEnded);

		CameraHandlerComponent cameraHandler = m_CharController.GetCameraHandlerComponent();
		if (cameraHandler)
		{
			ScriptedCameraItem camera = cameraHandler.GetCurrentCamera();
			if (camera && camera.GetOverrideDirectBoneMode() != 0)
			{
				camera.OverrideDirectBoneMode(0);
			}
		}
		
		if (!m_ShellComponent)
			return;

		m_ShellComponent.GetOnShellUsed().Remove(TransferShellToMortar);
		m_ShellComponent.UpdateVisibility(m_ShellComponent.GetMode());
		m_ShellComponent.ToggleActive(false, SCR_EUseContext.CUSTOM);
		m_ShellComponent.SetLoadedState(false);
		m_ShellComponent = null;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SynchronizeLoaderData(RplId loaderRplId, RplId shellRplId, float fuseTime, bool loadedFromLeftSide)
	{
		RplComponent loaderRplComp = RplComponent.Cast(Replication.FindItem(loaderRplId));
		if (!loaderRplComp)
			return;

		RplComponent shellRplComp = RplComponent.Cast(Replication.FindItem(shellRplId));
		if (!shellRplComp)
			return;

		ChimeraCharacter loader = ChimeraCharacter.Cast(loaderRplComp.GetEntity());
		if (!loader)
			return;

		IEntity shell = shellRplComp.GetEntity();
		if (!shell)
			return;

		m_CharController = SCR_CharacterControllerComponent.Cast(loader.GetCharacterController());
		if (!m_CharController)
			return;

		SCR_MortarShellGadgetComponent mortarShellComp = SCR_MortarShellGadgetComponent.Cast(shell.FindComponent(SCR_MortarShellGadgetComponent));
		if (!mortarShellComp)
			return;

		mortarShellComp.SetFuzeTime(fuseTime);
		mortarShellComp.SetLoadedState(true, this);
		m_bBeingLoaded = true;
		SetAnimationAttributes(m_CharController, loadedFromLeftSide);
	}
}

[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_VehicleWeaponSupportStationComponentClass : SCR_BaseItemSupportStationComponentClass
{
}

class SCR_VehicleWeaponSupportStationComponent : SCR_BaseItemSupportStationComponent
{	
	protected InventoryStorageManagerComponent m_InventoryStorage;
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;

		m_InventoryStorage = InventoryStorageManagerComponent.Cast(owner.FindComponent(InventoryStorageManagerComponent));
		super.DelayedInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool InitValidSetup()
	{		
		//~ Resupply ammo does not support range as only players can resupply themselves or others at the moment
		if (!UsesRange())
		{
			Print("'SCR_VehicleWeaponSupportStationComponent' requires range. Make sure it is greater than 0", LogLevel.ERROR);
			return false;
		}
		
		return super.InitValidSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyAmount)
	{
		if (!super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyAmount))
			return false;
		
		SCR_BaseItemHolderSupportStationAction baseItemHolderAction = SCR_BaseItemHolderSupportStationAction.Cast(action);
		if (baseItemHolderAction)
		{			
			//~ Item is not in the inventory
			if (m_InventoryStorage && !SCR_InventoryStorageManagerComponent.IsItemInStorage(m_InventoryStorage, baseItemHolderAction.GetItemPrefab(), actionUser))
			{
				reasonInvalid = ESupportStationReasonInvalid.RESUPPLY_NOT_IN_STORAGE;
				return false;
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasSupplyGainInsteadOfCost(SCR_BaseUseSupportStationAction action)
	{
		//~ If refund action it is gain instead of cost
		return SCR_RefundVehicleAmmoSupportStationAction.Cast(action) || SCR_RefundPylonSupportStationAction.Cast(action);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the multiplier when generating supplies (aka refunding)
	protected float GetGeneratorSupplyMultiplier()
	{
		if (!m_ResourceGenerator)
			return 1;
		
		return m_ResourceGenerator.GetResourceMultiplier();
	}
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.VEHICLE_WEAPON;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override int GetSupplyAmountAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{	
		//~ Uses item logic
		SCR_BaseItemHolderSupportStationAction itemHolder = SCR_BaseItemHolderSupportStationAction.Cast(action);
		if (itemHolder)
			return super.GetSupplyAmountAction(actionOwner, actionUser, action);
		
		//~ Supplies are disabled
		if (!AreSuppliesEnabled())
			return 0;
		
		//~ If Pylon refund action: Get pylon cost to calculate pylon refund
		SCR_RefundPylonSupportStationAction refundPylonAction = SCR_RefundPylonSupportStationAction.Cast(action);
		if (refundPylonAction)
			return SCR_ResourceSystemHelper.RoundRefundSupplyAmount(refundPylonAction.GetSupplyRefundAmount() * GetGeneratorSupplyMultiplier());
		
		//~ If ammo refund action: Get Ammo cost to calculate ammo refund
		SCR_RefundVehicleAmmoSupportStationAction refundAmmoAction = SCR_RefundVehicleAmmoSupportStationAction.Cast(action);
		if (refundAmmoAction)
			return SCR_ResourceSystemHelper.RoundRefundSupplyAmount(refundAmmoAction.GetSupplyRefundAmount() * GetGeneratorSupplyMultiplier());
		
		//~ Invalid action
		Print("SCR_VehicleWeaponSupportStationComponent is called by an invalid action not supported by the GetSupplyAmountAction", LogLevel.ERROR);
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		//~ Add ammo in vehicle
		SCR_ResupplyRocketPodSupportStationAction resupplyRocketAction = SCR_ResupplyRocketPodSupportStationAction.Cast(action);
		if (resupplyRocketAction)
		{
			RocketEjectorMuzzleComponent rocketMuzzleComp = resupplyRocketAction.GetRocketMuzzle();
			if (!rocketMuzzleComp || !rocketMuzzleComp.CanReloadNextBarrel())
				return;
			
			Resource resource = Resource.Load(resupplyRocketAction.GetItemPrefab());
			if (!resource.IsValid())
				return;
			
			//~ Consume Supplies
			if (!OnConsumeSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
				return;
			
			IEntity spawnedRocket = GetGame().SpawnEntityPrefab(resource);
			if (!spawnedRocket)
				return;
			
			rocketMuzzleComp.ReloadNextBarrel(spawnedRocket);
			super.OnExecutedServer(actionOwner, actionUser, action);
			
			return;
		}		
		
		//~ Add ammo in vehicle
		SCR_ResupplyTurretBeltSupportStationAction resupplyMagazineAction = SCR_ResupplyTurretBeltSupportStationAction.Cast(action);
		if (resupplyMagazineAction)
		{
			BaseMagazineComponent currentMagazine = resupplyMagazineAction.GetCurrentMagazine();
			if (!currentMagazine)
				return;
			
			int count = currentMagazine.GetAmmoCount();
			int maxCount = currentMagazine.GetMaxAmmoCount();
			int added = resupplyMagazineAction.GetAddedBulletsAmount();
			
			//~ Consume Supplies
			if (!OnConsumeSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
				return;
			
			if (count + added < maxCount)
				currentMagazine.SetAmmoCount(count + added);
			else
				currentMagazine.SetAmmoCount(maxCount);
			
			super.OnExecutedServer(actionOwner, actionUser, action);
			
			return;
		}
		
		//~ Refund the ammo in vehicle
		SCR_RefundVehicleAmmoSupportStationAction refundAmmoAction = SCR_RefundVehicleAmmoSupportStationAction.Cast(action);
		if (refundAmmoAction)
		{
			RocketEjectorMuzzleComponent rocketMuzzleComp = refundAmmoAction.GetRocketMuzzle();
			if (!rocketMuzzleComp)
				return;
			
			//~ Get rockets in reverse order
			int count = rocketMuzzleComp.GetBarrelsCount();		
			for (int i = count - 1; i >= 0; i--)
			{
				if (!rocketMuzzleComp.CanReloadBarrel(i))
				{	
					IEntity projectile = rocketMuzzleComp.GetBarrelProjectile(i);
					if (!projectile)
						return;
					
					rocketMuzzleComp.UnloadBarrel(i);
					
					//~ Generate supplies
					if (!OnGenerateSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
						return;
					
					super.OnExecutedServer(actionOwner, actionUser, action);
					break;
				}
			}

			return;
		}
		
		//~ Attach Pylon action
		SCR_AttachPylonSupportStationAction attachPylonAction = SCR_AttachPylonSupportStationAction.Cast(action);
		if (attachPylonAction)
		{
			TurretControllerComponent turrentController = TurretControllerComponent.Cast(actionOwner.FindComponent(TurretControllerComponent));
			if (turrentController)
			{
				WeaponSlotComponent weaponSlot = attachPylonAction.GetLinkedWeaponSlot();
				if (!weaponSlot || weaponSlot.GetWeaponEntity())
					return;
				
				Resource resource = Resource.Load(attachPylonAction.GetItemPrefab());
				if (!resource.IsValid())
					return;
				
				//~ Consume supplies
				if (!OnConsumeSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
					return;

				const EntitySpawnParams params = new EntitySpawnParams();
				weaponSlot.GetSlotInfo().GetWorldTransform(params.Transform);

				IEntity spawnedWeapon = GetGame().SpawnEntityPrefab(resource, actionOwner.GetWorld(), params);
				if (!spawnedWeapon)
					return;

				turrentController.AddWeapon(actionUser,  weaponSlot.GetWeaponSlotIndex(), spawnedWeapon);
				super.OnExecutedServer(actionOwner, actionUser, action);
			}
			
			return;
		}
		
		//~ Refund Pylon Action
		SCR_RefundPylonSupportStationAction refundPylonAction = SCR_RefundPylonSupportStationAction.Cast(action);
		if (refundPylonAction)
		{
			TurretControllerComponent turrentController = TurretControllerComponent.Cast(actionOwner.FindComponent(TurretControllerComponent));
			if (!turrentController)
				return;

			const WeaponSlotComponent weaponSlot = refundPylonAction.GetManagedWeaponSlot();
			if (!weaponSlot)
				return;

			const IEntity attachedWeapon = weaponSlot.GetWeaponEntity();
			if (!attachedWeapon)
				return;

			const int weaponId = refundPylonAction.GetPylonIndex();
			turrentController.RemoveWeapon(null, weaponId, null);

			//~ Generate supplies
			if (!OnGenerateSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
				return;

			super.OnExecutedServer(actionOwner, actionUser, action);

			RplComponent.DeleteRplEntity(attachedWeapon, false);
			
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Called by OnExecuteBroadcast and is executed both on server and on client
	//~ playerId can be -1 if the user was not a player
	protected override void OnExecute(IEntity actionOwner, IEntity actionUser, int playerId, SCR_BaseUseSupportStationAction action)
	{
		super.OnExecute(actionOwner, actionUser, playerId, action);

		ResourceName soundProject;
		string soundEffectName;
		
		SCR_BaseAudioSupportStationAction baseAudioAction = SCR_BaseAudioSupportStationAction.Cast(action);
		if (!baseAudioAction || !baseAudioAction.GetSoundEffectProjectAndEvent(soundProject, soundEffectName))
			return;

		SCR_AudioSourceConfiguration audioConfig = CreateOnUseAudioConfig(soundProject, soundEffectName);
		if (audioConfig)
			PlaySoundEffect(audioConfig, baseAudioAction.GetSoundSource(), action);
	}
}

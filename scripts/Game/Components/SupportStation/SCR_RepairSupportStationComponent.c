[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_RepairSupportStationComponentClass : SCR_BaseDamageHealSupportStationComponentClass
{	
	[Attribute(ENotification.SUPPORTSTATION_REPAIRED_BY_OTHER_DONE_NOT_FULL.ToString(), desc: "Leave UNKNOWN to not send notification when healing is done but vehicle is not full heal but not done (for players in vehicle that is being repaired)", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ENotification), category: "Heal/Repair Support Station")]
	protected ENotification m_eHealDoneNotFullNotificationInVehicle;
	
	[Attribute("SOUND_VEHICLE_EXTINGUISH_PARTIAL", desc: "Sound effect played when Extinguish is updated and not done. Broadcast to players. Leave empty if no sfx", category: "Heal/Repair Support Station")]
	protected string m_sOnExtinguishUpdateSoundEffectEventName;
	
	[Attribute("SOUND_VEHICLE_EXTINGUISH_DONE", desc: "Sound effect played when Extinguish is done. Broadcast to players. Leave empty if no sfx", category: "Heal/Repair Support Station")]
	protected string m_sOnExtinguishDoneSoundEffectEventName;
	
	protected ref SCR_AudioSourceConfiguration m_OnExtinguishUpdateAudioSourceConfiguration;
	protected ref SCR_AudioSourceConfiguration m_OnExtinguishDoneAudioSourceConfiguration;
	
	//------------------------------------------------------------------------------------------------
	//! \return Notification for Heal done but not full aka: Support Station cannot heal more. This notification is send to players that are in the vehicle that is healed
	ENotification GetHealDoneNotFullInVehicleNotification()
	{
		return m_eHealDoneNotFullNotificationInVehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Sound Config for heal update. Will return null if no audio is assigned
	SCR_AudioSourceConfiguration GetOnExtinguishUpdateAudioConfig()
	{		
		//~ Create Audio source if it does not yet exist
		if (!m_OnExtinguishUpdateAudioSourceConfiguration)
			m_OnExtinguishUpdateAudioSourceConfiguration = CreateSoundAudioConfig(m_sOnExtinguishUpdateSoundEffectEventName);

		return m_OnExtinguishUpdateAudioSourceConfiguration;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return Sound Config for heal update. Will return null if no audio is assigned
	SCR_AudioSourceConfiguration GetOnExtinguishDoneAudioConfig()
	{		
		//~ Create Audio source if it does not yet exist
		if (!m_OnExtinguishDoneAudioSourceConfiguration)
			m_OnExtinguishDoneAudioSourceConfiguration = CreateSoundAudioConfig(m_sOnExtinguishDoneSoundEffectEventName);

		return m_OnExtinguishDoneAudioSourceConfiguration;
	}	
}

class SCR_RepairSupportStationComponent : SCR_BaseDamageHealSupportStationComponent
{			
	[Attribute("40", desc: "Only valid if m_aDoTTypesHealed includes FIRE, Each Execute the fire rate is reduced with a set amount. If fire rate is 0 it will be extinguished", category: "Heal/Repair Support Station")]
	protected float m_fFireRateReductionEachExecute;
	
	[Attribute("1", desc: "Only valid if m_aDoTTypesHealed includes FIRE. The supply cost each time fire rate is reduced (no matter the reduction amount as fire rate is not replicated)", category: "Heal/Repair Support Station")]
	protected float m_iSupplyCostPerFireRateReduction;
	
	protected static ref ScriptInvokerEntity s_OnFireExtinguished;

	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.REPAIR;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override float GetDamageOrStateToHeal(IEntity actionOwner, IEntity actionUser, notnull SCR_BaseDamageHealSupportStationAction action, out EDamageType activeDoT, out notnull array<HitZone> hitZones)
	{			
		if (!m_aDoTTypesHealed.Contains(EDamageType.FIRE))
			return super.GetDamageOrStateToHeal(actionOwner, actionUser, action, activeDoT, hitZones);
		
		hitZones.Clear();
		action.GetHitZonesToHeal(hitZones);
		SCR_DamageManagerComponent dmgManager = action.GetActionDamageManager();
		if (!dmgManager)
			return 0;
		
		float fireRateHealAmount;
		
		SCR_FlammableHitZone flammableHitZone;
		foreach (HitZone hitZone : hitZones)
		{
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (!flammableHitZone)
				continue;

			if (!dmgManager.IsOnFire(flammableHitZone))
				continue;

			activeDoT = EDamageType.FIRE;
			
			fireRateHealAmount += Math.Max(1, flammableHitZone.GetFireRate());
			
			if (fireRateHealAmount >= m_fFireRateReductionEachExecute)
				return m_fFireRateReductionEachExecute;
		}
		
		if (activeDoT == EDamageType.FIRE && fireRateHealAmount > 0)
			return fireRateHealAmount;
		
		//~ Not on fire so return the normal logic
		return super.GetDamageOrStateToHeal(actionOwner, actionUser, action, activeDoT, hitZones);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override int GetSupplyAmountAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!AreSuppliesEnabled())
			return 0;
		
		if (!m_aDoTTypesHealed.Contains(EDamageType.FIRE))
			return super.GetSupplyAmountAction(actionOwner, actionUser, action);
		
		EDamageType activeDoT;
		array<HitZone> hitZones = {};
		
		float damageToHeal = GetDamageOrStateToHeal(actionOwner, actionUser, SCR_BaseDamageHealSupportStationAction.Cast(action), activeDoT, hitZones);
		
		//~ On fire so use fire supply cost
		if (activeDoT == EDamageType.FIRE)
			return m_iSupplyCostPerFireRateReduction + m_iBaseSupplyCostOnUse;
		
		//~ Not on fire so return normal supply cost action
		return super.GetSupplyAmountAction(actionOwner, actionUser, action);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		EDamageType activeDoT;
		array<HitZone> hitZones = {};
		float damageToHeal = GetDamageOrStateToHeal(actionOwner, actionUser, SCR_BaseDamageHealSupportStationAction.Cast(action), activeDoT, hitZones);
			
		if (activeDoT == EDamageType.FIRE)
		{
			//~ Consume supplies
			if (AreSuppliesEnabled())
			{
				//~ Failed to consume supplies, meaning there weren't enough supplies for the action
				if (!OnConsumeSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
					return;
			}
			
			SCR_FlammableHitZone flammableHitZone;
			
			float removeFireAmount = m_fFireRateReductionEachExecute;
			float fireRate;
			bool stillBurning = false;
			
			foreach (HitZone hitZone : hitZones)
			{
				flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
				if (!flammableHitZone)
					continue;
				
				//~ Get fire rate of hitzone
				fireRate = flammableHitZone.GetFireRate();
				if (fireRate <= 0)
					continue;
				
				//~ Can no longer remove fire rate this execute but the hitzones are still burning
				if (removeFireAmount <= 0)
				{
					stillBurning = true;
					break;
				}
				
				if (fireRate > removeFireAmount)
				{
					stillBurning = true;
					flammableHitZone.SetFireRate(fireRate - removeFireAmount);
					break;
				}
				else 
				{
					flammableHitZone.SetFireRate(0);
					removeFireAmount -= fireRate;
				}
				
				//~ Make sure it doesn't ignite again
				if (flammableHitZone.GetFireRate() <= 0 && flammableHitZone.GetFireState() == SCR_EBurningState.SMOKING_IGNITING)
					flammableHitZone.SetFireState(SCR_EBurningState.SMOKING_HEAVY);
			}
			
			SCR_EDamageSupportStationHealState healState;
			if (stillBurning)
				healState = SCR_EDamageSupportStationHealState.FIRE_EXTINGUISH_UPDATE;
			else 
				healState = SCR_EDamageSupportStationHealState.FIRE_EXTINGUISH_DONE;
				
			//~ Get broadcast ids
			RplId ownerId;
			RplId userId;
			int playerId;
			
			FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
			OnExecuteDamageSystem(actionOwner, actionUser, healState, SCR_BaseDamageHealSupportStationAction.Cast(action), 0);
			Rpc(OnExecuteDamageSystemBroadcast, ownerId, userId, healState, action.GetActionID(), 0);
			return;
		}	
		
		//~ Execute default logics if not on fire
		super.OnExecutedServer(actionOwner, actionUser, action);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnExecuteDamageSystem(IEntity actionOwner, IEntity actionUser, SCR_EDamageSupportStationHealState healState, SCR_BaseDamageHealSupportStationAction action, float healthScaled)
	{
		if (!actionOwner)
		{
			//~ Action was still succesfully executed
			OnSuccessfullyExecuted(actionOwner, actionUser, action);
			return;
		}	
		
		bool isFireStateHealed = false;
		
		//~ Play sound effect
		SCR_RepairSupportStationComponentClass classData = SCR_RepairSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
		if (classData)
		{
			switch (healState)
			{
				case SCR_EDamageSupportStationHealState.FIRE_EXTINGUISH_UPDATE :
				{
					PlaySoundEffect(classData.GetOnExtinguishUpdateAudioConfig(), actionOwner, action);
					isFireStateHealed = true;
					break;
				}
				case SCR_EDamageSupportStationHealState.FIRE_EXTINGUISH_DONE :
				{
					PlaySoundEffect(classData.GetOnExtinguishDoneAudioConfig(), actionOwner, action);
					isFireStateHealed = true;
					if (s_OnFireExtinguished)
						s_OnFireExtinguished.Invoke(actionUser);
					break;
				}
			}
		}
		
		//~ Fire state not healed, use default logic
		if (!isFireStateHealed)
		{
			super.OnExecuteDamageSystem(actionOwner, actionUser, healState, action, healthScaled);
			return;
		}

		//~ On succesfully executed
		OnSuccessfullyExecuted(actionOwner, actionUser, action);
		
		//~ Do not send notification
		if (!GetSendNotificationOnUse())
			return;
		
		SendDamageSupportStationNotification(actionOwner, actionUser, action, healState, healthScaled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void SendDamageSupportStationNotification(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, SCR_EDamageSupportStationHealState healState, float healthScaled)
	{				
		//~ Editable entity not found
		SCR_EditableEntityComponent userEditableEntity = SCR_EditableEntityComponent.Cast(actionUser.FindComponent(SCR_EditableEntityComponent));
		if (!userEditableEntity)
			return;

		ENotification inVehicleNotification = ENotification.UNKNOWN;
		
		//~ Get correct notitfication
		switch (healState)
		{
			case SCR_EDamageSupportStationHealState.HEAL_UPDATE :
			{
				inVehicleNotification = ENotification.SUPPORTSTATION_REPAIRED_BY_OTHER_UPDATE;
				break;
			}
			case SCR_EDamageSupportStationHealState.HEAL_DONE_NOT_FULL_HEAL :
			{
				SCR_RepairSupportStationComponentClass classData = SCR_RepairSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
				if (!classData)
					return;
					
				inVehicleNotification = classData.GetHealDoneNotFullInVehicleNotification();
				break;
			}
			case SCR_EDamageSupportStationHealState.HEAL_DONE :
			{
				inVehicleNotification = ENotification.SUPPORTSTATION_REPAIRED_BY_OTHER_DONE;
				break;
			}
			case SCR_EDamageSupportStationHealState.FIRE_EXTINGUISH_UPDATE :
			{
				inVehicleNotification = ENotification.SUPPORTSTATION_FIRE_EXTINGUISHED_VEHICLE_BY_OTHER_UPDATE;
				break;
			}
			case SCR_EDamageSupportStationHealState.FIRE_EXTINGUISH_DONE :
			{
				inVehicleNotification = ENotification.SUPPORTSTATION_FIRE_EXTINGUISHED_VEHICLE_BY_OTHER_DONE;
				break;
			}
		}
				
		//~ No in vehicle notification
		if (inVehicleNotification == ENotification.UNKNOWN)
			return;
			
		RplId userRplId = Replication.FindId(userEditableEntity);
		
		//~ Get players in vehicle
		array<int> playersInVehicle = {};
		GetPlayerIdsInVehicle(actionOwner, playersInVehicle);
		
		if (!playersInVehicle.Contains(SCR_PlayerController.GetLocalPlayerId()))
			return;

		EVehicleHitZoneGroup notificationHitZoneGroup = EVehicleHitZoneGroup.VIRTUAL;
		if (healState != ENotification.SUPPORTSTATION_FIRE_EXTINGUISHED_VEHICLE_BY_OTHER_UPDATE && healState != ENotification.SUPPORTSTATION_FIRE_EXTINGUISHED_VEHICLE_BY_OTHER_DONE)
		{
			SCR_RepairAtSupportStationAction repairAction = SCR_RepairAtSupportStationAction.Cast(action);
			if (repairAction)
			{
				array<EVehicleHitZoneGroup> hitZoneGroup = {};
				repairAction.GetHitZoneGroups(hitZoneGroup);
				
				if (!hitZoneGroup.IsEmpty())
					notificationHitZoneGroup = hitZoneGroup[0];
			}
		}
		
		//~ Send in vehicle notification if player is in vehicle
		SCR_NotificationsComponent.SendLocal(inVehicleNotification, userRplId, notificationHitZoneGroup, healthScaled * 1000);
	}

	//------------------------------------------------------------------------------------------------
	//! Get event called when player extinguish fire.
	//! Invoker params are: Entity playerEntity
	//! \return ScriptInvokerEntity
	static ScriptInvokerEntity GetOnFireExtinguished()
	{
		if (!s_OnFireExtinguished)
			s_OnFireExtinguished = new ScriptInvokerEntity();

		return s_OnFireExtinguished;
	}
}
	
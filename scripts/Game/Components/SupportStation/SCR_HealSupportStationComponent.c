[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_HealSupportStationComponentClass : SCR_BaseDamageHealSupportStationComponentClass
{
	[Attribute("SOUND_HEAL_SELF", desc: "Sound effect played when heal is updated and not done. Broadcast to players. Leave empty if no sfx", category: "Heal/Repair Support Station")]
	protected string m_sOnHealBloodUpdateSoundEffectEventName;

	protected ref SCR_AudioSourceConfiguration m_OnHealBloodUpdateAudioSourceConfiguration;
	
	//------------------------------------------------------------------------------------------------
	//! \return Sound Config for heal blood update. Will return null if no audio is assigned
	SCR_AudioSourceConfiguration GetOnHealBloodUpdateAudioConfig()
	{		
		//~ Create Audio source if it does not yet exist
		if (!m_OnHealBloodUpdateAudioSourceConfiguration)
			m_OnHealBloodUpdateAudioSourceConfiguration = CreateSoundAudioConfig(m_sOnHealBloodUpdateSoundEffectEventName);

		return m_OnHealBloodUpdateAudioSourceConfiguration;
	}	
}

class SCR_HealSupportStationComponent : SCR_BaseDamageHealSupportStationComponent
{			
	[Attribute("1000", desc: "Max blood amount healed each execute. If you hold the action it will heal each time the action ends", category: "Heal/Repair Support Station")]
	protected float m_iBloodHealedEachExecute;
	
	[Attribute("100", desc: "Supply cost for ever 'm_iBloodHealedEachExecute' damage healed. If the left over blood amount is less then it will of course be cheaper with a min value of 1", category: "Heal/Repair Support Station")]
	protected int m_iSupplyCostBloodHealed;
	
	[Attribute("1", desc: "The max blood scaled that this support station can heal. 1 == 100%", params: "0.01 1", category: "Heal/Repair Support Station")]
	protected float m_fMaxBloodScaled;
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.HEAL;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override float GetDamageOrStateToHeal(IEntity actionOwner, IEntity actionUser, notnull SCR_BaseDamageHealSupportStationAction action, out EDamageType activeDoT, out notnull array<HitZone> hitZones)
	{			
		float damageToHeal = super.GetDamageOrStateToHeal(actionOwner, actionUser, action, activeDoT, hitZones);
 		if (damageToHeal > 0)
 			return damageToHeal;
 		
		SCR_HealSupportStationAction healAction = SCR_HealSupportStationAction.Cast(action);
		if (!healAction)
 			return damageToHeal;
 		
		HitZone bloodHitZone = healAction.GetBloodHitZoneToHeal();
 		if (!bloodHitZone)
 			return damageToHeal;
 		
		hitZones.Clear();
		hitZones.Insert(bloodHitZone);
 		
		return Math.Clamp(action.GetActionDamageManager().GetHitZonesDamage(m_fMaxBloodScaled, hitZones), 0, m_iBloodHealedEachExecute);
 	}
	
	//------------------------------------------------------------------------------------------------
	protected override int GetSupplyAmountAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!AreSuppliesEnabled())
			return 0;
			
		SCR_BaseDamageHealSupportStationAction damageHealAction = SCR_BaseDamageHealSupportStationAction.Cast(action);
		
		array<HitZone> hitZones = {};
		EDamageType activeDoT;
		float damageToHeal = GetDamageOrStateToHeal(actionOwner, actionUser, damageHealAction, activeDoT, hitZones);
		if (activeDoT != -1)
			return super.GetSupplyAmountAction(actionOwner, actionUser, action);
		
		//~ Not a character
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(damageHealAction.GetActionDamageManager());	
		if (!characterDamageManager)
			return super.GetSupplyAmountAction(actionOwner, actionUser, action);
		
		HitZone bloodHitZone = characterDamageManager.GetBloodHitZone();
		
		//~ No blood hitZone or bloodhitZone full health, use default logic				
		if (hitZones.Count() != 1 || hitZones[0] != bloodHitZone)
			return super.GetSupplyAmountAction(actionOwner, actionUser, action);
			
		//~ Calculate the supply cost
		float bloodHealCost = (damageToHeal / m_iBloodHealedEachExecute) * m_iSupplyCostBloodHealed;

		return Math.Ceil(bloodHealCost + m_iBaseSupplyCostOnUse);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{				
		SCR_BaseDamageHealSupportStationAction damageHealAction = SCR_BaseDamageHealSupportStationAction.Cast(action);
		array<HitZone> hitZones = {};
		EDamageType activeDoT;
		float damageToHeal = GetDamageOrStateToHeal(actionOwner, actionUser, damageHealAction, activeDoT, hitZones);
		if (activeDoT != -1)
		{
			super.OnExecutedServer(actionOwner, actionUser, action);
			return;		
		}
		
		//~ Not a character
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(SCR_BaseDamageHealSupportStationAction.Cast(action).GetActionDamageManager());	
		if (!characterDamageManager)
		{
			super.OnExecutedServer(actionOwner, actionUser, action);
			return;	
		}
		HitZone bloodHitZone = characterDamageManager.GetBloodHitZone();
		
		//~ No blood hitZone or bloodhitZone full health, use default logic				
		if (hitZones.Count() != 1 || hitZones[0] != bloodHitZone)
		{
			super.OnExecutedServer(actionOwner, actionUser, action);
			return;
		}
		
		//~ Consume supplies
		if (AreSuppliesEnabled())
		{
			//~ Failed to consume supplies, meaning there weren't enough supplies for the action
			if (!OnConsumeSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
				return;
		}
					
		//~ Heal the blood hitZone
		characterDamageManager.HealHitZones(damageToHeal, true, m_fMaxBloodScaled, hitZones);
		
		//~ Get healstate
		SCR_EDamageSupportStationHealState healState;	
		if (bloodHitZone.GetHealthScaled() < m_fMaxBloodScaled)
			healState = SCR_EDamageSupportStationHealState.BLOOD_UPDATE;
		else if (m_fMaxBloodScaled < 1 && bloodHitZone.GetHealthScaled() == m_fMaxBloodScaled)
			healState = SCR_EDamageSupportStationHealState.BLOOD_DONE_NOT_FULL_HEAL;
		else 
			healState = SCR_EDamageSupportStationHealState.BLOOD_DONE;
				
		RplId ownerId;
		RplId userId;
		int playerId;
		FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
			
		OnExecuteDamageSystem(actionOwner, actionUser, healState, SCR_BaseDamageHealSupportStationAction.Cast(action), bloodHitZone.GetHealthScaled());
		Rpc(OnExecuteDamageSystemBroadcast, ownerId, userId, healState, action.GetActionID(), bloodHitZone.GetHealthScaled());
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnExecuteDamageSystem(IEntity actionOwner, IEntity actionUser, SCR_EDamageSupportStationHealState healState, SCR_BaseDamageHealSupportStationAction action, float healthScaled)
	{
		//~ Could not send notification as unknown owner or user
		if (!actionOwner || !actionUser)
		{
			//~ Action was still succesfully executed
			OnSuccessfullyExecuted(actionOwner, actionUser, action);
			return;
		}
		
		bool isBloodHealState = false;
		
		//~ Play sound effect
		SCR_HealSupportStationComponentClass classData = SCR_HealSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
		if (classData)
		{
			switch (healState)
			{
				case SCR_EDamageSupportStationHealState.BLOOD_UPDATE :
				{
					isBloodHealState = true;
					PlaySoundEffect(classData.GetOnHealBloodUpdateAudioConfig(), actionOwner, action);
					break;
				}
				case SCR_EDamageSupportStationHealState.BLOOD_DONE_NOT_FULL_HEAL :
				{
					isBloodHealState = true;
					PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
					break;
				}
				case SCR_EDamageSupportStationHealState.BLOOD_DONE :
				{
					isBloodHealState = true;
					PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
					
					//~ Play done voice event (if any)
					PlayCharacterVoiceEvent(actionUser);
					break;
				}
			}
		}
		
		//~ Blood not healed, use default logic
		if (!isBloodHealState)
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
		
		int ownerPlayerID;
		
		SCR_PossessingManagerComponent possesionManager = SCR_PossessingManagerComponent.GetInstance();
		if (possesionManager)
		{
			ownerPlayerID = possesionManager.GetPlayerIdFromControlledEntity(actionOwner);
		}
		else 
		{
			SCR_EditableEntityComponent ownerEditableEntity = SCR_EditableEntityComponent.Cast(actionOwner.FindComponent(SCR_EditableEntityComponent));
			
			if (ownerEditableEntity)
				ownerPlayerID = ownerEditableEntity.GetPlayerID();
		}
	
		//~ If owner is AI do not send notification
		if (ownerPlayerID <= 0)
			return;
		
		RplId userRplId = Replication.FindId(userEditableEntity);
		
		ECharacterHitZoneGroup notificationHitZoneGroup = ECharacterHitZoneGroup.VIRTUAL;
		SCR_HealSupportStationAction healAction = SCR_HealSupportStationAction.Cast(action);
		if (healAction)
			notificationHitZoneGroup = healAction.GetHitZoneGroup();
		
		//~ Send correct notification
		switch (healState)
		{
			case SCR_EDamageSupportStationHealState.HEAL_UPDATE :
			{
				if (SCR_PlayerController.GetLocalPlayerId() == ownerPlayerID)
					SCR_NotificationsComponent.SendLocal(ENotification.SUPPORTSTATION_HEALED_BY_OTHER_UPDATE, userRplId, notificationHitZoneGroup, healthScaled * 1000);
				return;
			}
			case SCR_EDamageSupportStationHealState.HEAL_DONE_NOT_FULL_HEAL :
			{
				if (SCR_PlayerController.GetLocalPlayerId() == ownerPlayerID)
					SCR_NotificationsComponent.SendLocal(ENotification.SUPPORTSTATION_HEALED_BY_OTHER_DONE_NOT_FULL, userRplId, notificationHitZoneGroup, healthScaled * 1000);				
				return;
			}
			case SCR_EDamageSupportStationHealState.HEAL_DONE :
			{
				 if (SCR_PlayerController.GetLocalPlayerId() == ownerPlayerID)
					SCR_NotificationsComponent.SendLocal(ENotification.SUPPORTSTATION_HEALED_BY_OTHER_DONE, userRplId, notificationHitZoneGroup, 1);

				return;
			}
		}
	}
}

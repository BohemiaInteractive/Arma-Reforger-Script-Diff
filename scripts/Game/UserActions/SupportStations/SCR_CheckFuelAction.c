class SCR_CheckFuelAction : SCR_BaseAudioScriptedUserAction
{			
	protected ref SCR_RefuelAtSupportStationAction m_RefuelActionPair;
	
	FuelManagerComponent m_FuelManager;	
	SCR_DamageManagerComponent m_DamageComponent;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_FuelManager = FuelManagerComponent.Cast(pOwnerEntity.FindComponent(FuelManagerComponent));	
		m_DamageComponent = SCR_DamageManagerComponent.GetDamageManager(pOwnerEntity);
		
		//~ Delay init so action manager has all action initialized
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: pOwnerEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		ActionsManagerComponent actionManager = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		if (!actionManager)
			return;
		
		array<BaseUserAction> userActions = {};
		actionManager.GetActionsList(userActions);
		
		foreach(BaseUserAction action : userActions)
		{
			m_RefuelActionPair = SCR_RefuelAtSupportStationAction.Cast(action);
			if (m_RefuelActionPair)
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;
		
		if (!m_FuelManager)
			return false;

		//~ Don't show if damage manager destroyed
		if (m_DamageComponent && m_DamageComponent.GetState() == EDamageState.DESTROYED)
			return false;
		
		ESupportStationReasonInvalid cannotPerformReason;
		if (m_RefuelActionPair)
		{
			//~ Do not show action if the normal repair action can be performed
			if (m_RefuelActionPair.CanPerform(cannotPerformReason) || cannotPerformReason == ESupportStationReasonInvalid.FUEL_TANK_FULL)
				return false;
		}
		
		//~ No damage system present
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		//~ Execute Audio broadcast
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		//~ Only show notification for player that used it
		if (SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(pUserEntity) != SCR_PlayerController.GetLocalPlayerId())
			return;
		
		SCR_NotificationsComponent.SendLocal(ENotification.CHECK_FUEL, (m_FuelManager.GetTotalFuel() / m_FuelManager.GetTotalMaxFuel()) * 100);
	}
};
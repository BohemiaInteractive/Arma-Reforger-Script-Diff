//! Allows players to drain an entity with a fuel manager. Use with care as it might create a lot of grieving opertunity
class SCR_DrainFuelUserAction : SCR_BaseAudioScriptedUserAction
{		
	[Attribute("#AR-FuelCanister_Drain_Action_Invalid_Empty")]
	protected LocalizedString m_sCannotPerformNoFuel;
	
	protected IEntity m_ActionUser; //~ The player that uses is currently using the action
	protected bool m_bIsMaster;
	
	protected bool m_bExecutingAction;
	
	protected SCR_FuelManagerComponent m_FuelManager;	
	protected SCR_DamageManagerComponent m_DamageComponent;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_FuelManager = SCR_FuelManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_FuelManagerComponent));	
		m_DamageComponent = SCR_DamageManagerComponent.GetDamageManager(pOwnerEntity);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			m_bIsMaster = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetCannotPerformReason()
	{
		//~ Another user is using it
		if (m_ActionUser)
			return m_sCannotPerformReason;
		//~ Is empty
		else 
			return m_sCannotPerformNoFuel;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;
		
		if (!m_FuelManager)
			return false;

		array<SCR_FuelNode> fuelNodes = {};
		if (m_FuelManager.GetScriptedFuelNodesList(fuelNodes) <= 0)
			return false;
		
		//~ Don't show if damage manager destroyed
		if (m_DamageComponent)
			return m_DamageComponent.GetState() != EDamageState.DESTROYED;
		
		//~ No damage system present
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{	
		if (m_ActionUser && m_ActionUser != user)
			return false;
		
		float totalFuel, totalMaxFuel, totalFuelPercentage
		m_FuelManager.GetTotalValuesOfFuelNodes(totalFuel, totalMaxFuel, totalFuelPercentage, SCR_EFuelNodeTypeFlag.CAN_BE_DRAINED);
		
		return !float.AlmostEqual(totalFuel, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ If continues action it will only execute everytime the duration is done
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{			
		if (!LoopActionUpdate(timeSlice))
			return; 
		
		PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		if (!m_FuelManager)
			return;
		
		//~ Execute Audio broadcast
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		if (!m_bIsMaster)
			return;
		
		float totalFuel, totalMaxFuel, totalFuelPercentage
		
		m_FuelManager.GetTotalValuesOfFuelNodes(totalFuel, totalMaxFuel, totalFuelPercentage, SCR_EFuelNodeTypeFlag.CAN_BE_DRAINED);
		
		array<SCR_FuelNode> fuelNodes = {};
		m_FuelManager.GetScriptedFuelNodesList(fuelNodes);
		
		float actionDuration = GetActionDuration();
		if (actionDuration < 0)
			actionDuration *= -1;
		else if (actionDuration == 0)
			actionDuration = 1;
		
		//~ Todo: Replace with SCR_FuelManager with functions Support Manager fuel
		float fuelRemovePercentage = Math.Clamp((m_FuelManager.GetTotalFuel() - ((fuelNodes[0].GetMaxFlowCapacityOut() / 60) * actionDuration)) / m_FuelManager.GetTotalMaxFuel(), 0, 1);
		
		m_FuelManager.SetTotalFuelPercentage(fuelRemovePercentage, SCR_EFuelNodeTypeFlag.CAN_BE_DRAINED);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{		
		super.OnActionStart(pUserEntity);
		
		m_ActionUser = pUserEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_ActionUser = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{	
		UIInfo uiInfo = GetUIInfo();
		if (!uiInfo)
			return false;
		
		string percentageString = SCR_FormatHelper.FloatToStringNoZeroDecimalEndings((m_FuelManager.GetTotalFuel() / m_FuelManager.GetTotalMaxFuel()) * 100, 1);
		outName = WidgetManager.Translate(uiInfo.GetName(), percentageString);
		return true;
	}
}

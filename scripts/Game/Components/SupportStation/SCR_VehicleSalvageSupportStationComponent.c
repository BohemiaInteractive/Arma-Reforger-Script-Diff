[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_VehicleSalvageSupportStationComponentClass : SCR_BaseSupportStationComponentClass
{
}

class SCR_VehicleSalvageSupportStationComponent : SCR_BaseSupportStationComponent
{	
	[Attribute(defvalue: "0.2", uiwidget: UIWidgets.SpinBox, params: "0 inf", desc: "The refund multiplier for salvging wrecks. 20% of original cost by default.", category: "General Settings")]
	protected float m_fRefundMultiplier;
	
	//------------------------------------------------------------------------------------------------
	// Check if the user action is valid
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyAmount)
	{	
		SCR_VehicleSalvageSupportStationUserAction VehicleSalvageAction = SCR_VehicleSalvageSupportStationUserAction.Cast(action);
		if (!VehicleSalvageAction)
		{
			Debug.Error2("SCR_VehicleSalvageSupportStationComponent", "This user action is not of Vehicle Salvage type.");
			return false;
		}
		
		return super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.LOAD_WRECK;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasSupplyGainInsteadOfCost(SCR_BaseUseSupportStationAction action)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get final amount of supplies that will get refunded for salvaging
	protected override int GetSupplyAmountAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!m_ResourceGenerator)
			return 0;
		
		SCR_VehicleSalvageSupportStationUserAction vehicleSalvageAction = SCR_VehicleSalvageSupportStationUserAction.Cast(action);
		if (!vehicleSalvageAction)
			return 0;
		
		return vehicleSalvageAction.GetSupplyCost() * m_fRefundMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if the vehicle exists and is destroyed, then generate supplies in return for executing the action
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		Vehicle vehicle = Vehicle.Cast(actionOwner.GetParent());
		if (!vehicle)
			return;
		
		SCR_DamageManagerComponent damageManager = vehicle.GetDamageManager();
		if (!damageManager || damageManager.GetState() != EDamageState.DESTROYED)
			return;
		
		super.OnExecutedServer(actionOwner, actionUser, action);
		
		int supplyAmount = GetSupplyAmountAction(actionOwner, actionUser, action);
		 
		delete vehicle;
		
		if (vehicle && !vehicle.IsDeleted())
			return;
		
		OnGenerateSuppliesServer(supplyAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecute(IEntity actionOwner, IEntity actionUser, int playerId, SCR_BaseUseSupportStationAction action)
	{
		super.OnExecute(actionOwner, actionUser, playerId, action);
		
		// Play sound effect
		SCR_AudioSourceConfiguration audioConfig = GetOnUseAudioConfig();
		if (!audioConfig)
			return;
		
		PlaySoundEffect(audioConfig, actionUser, action);
	}
}

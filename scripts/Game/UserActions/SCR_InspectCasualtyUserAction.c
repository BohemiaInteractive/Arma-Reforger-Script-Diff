class SCR_InspectCasualtyUserAction : ScriptedUserAction
{
	[Attribute(defvalue: "4", desc: "Value by which action progress is going to be multiplied to speed it up when character is a medic", params: "0.1 inf 0.01")]
	protected float m_fQualifiedPersonnelBonusUseSpeedFactor;

	protected bool m_bIsQualified;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		ChimeraCharacter userChar = ChimeraCharacter.Cast(user);
		ChimeraCharacter ownerChar = ChimeraCharacter.Cast(GetOwner());
		if (!userChar || !ownerChar)
			return false;
		
		if (userChar == ownerChar)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(ownerChar.GetDamageManager());
		if (!damageMan)
			return false;
		
		HitZone hz = damageMan.GetDefaultHitZone();
		if (hz.GetDamageState() == EDamageState.UNDAMAGED)
		{		
			SCR_CharacterBloodHitZone bloodHZ = damageMan.GetBloodHitZone();
			if (!bloodHZ)
				return false;
			
			if (bloodHZ.GetDamageState() == EDamageState.UNDAMAGED)
				return false;
		}
		
		CompartmentAccessComponent userCompAccessComp = userChar.GetCompartmentAccessComponent();
		CompartmentAccessComponent targetCompAccessComp = ownerChar.GetCompartmentAccessComponent();
		if (!userCompAccessComp || !targetCompAccessComp)
			return false;
		
		IEntity userVeh = userCompAccessComp.GetVehicleIn(userChar);
		IEntity targetVeh = targetCompAccessComp.GetVehicleIn(ownerChar);
		
		// If inspector and target are not in vehicle return true
		if (!userVeh && !targetVeh)
			return CanBePerformedScript(user);

		// If inspector and target are both in vehicle, check if they're in the same one, and if so, return true
		if (userVeh && targetVeh && targetVeh == userVeh)
			return CanBePerformedScript(user);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return false;

		// Don't inspect dead people
		CharacterControllerComponent controller = char.GetCharacterController();
		if (controller.GetLifeState() == ECharacterLifeState.DEAD)
			return false;
		
		// Don't show inspect userAction when already showing SCR_InspectCasualtyWidget
		SCR_InspectCasualtyWidget casualtyInspectDisplay;
		array<BaseInfoDisplay> infoDisplays = {};
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		
		HUDManagerComponent hudMan = playerController.GetHUDManagerComponent();
		if (!hudMan)
			return false;
		
		hudMan.GetInfoDisplays(infoDisplays);
		
		foreach (BaseInfoDisplay baseInfoDisplays : infoDisplays)
		{
			casualtyInspectDisplay = SCR_InspectCasualtyWidget.Cast(baseInfoDisplays);
			if (!casualtyInspectDisplay)
				continue;
			
			if (casualtyInspectDisplay.IsActive() && casualtyInspectDisplay.m_Target == GetOwner())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		array<BaseInfoDisplay> infoDisplays = {};
		GetGame().GetPlayerController().GetHUDManagerComponent().GetInfoDisplays(infoDisplays);
		foreach (BaseInfoDisplay baseInfoDisplays : infoDisplays)
		{
			SCR_InspectCasualtyWidget casualtyInspectDisplay = SCR_InspectCasualtyWidget.Cast(baseInfoDisplays);
			if (!casualtyInspectDisplay)
				continue;

			casualtyInspectDisplay.SetTarget(pOwnerEntity);
			casualtyInspectDisplay.ShowInspectCasualtyWidget(pOwnerEntity);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent contr = character.GetCharacterController();
		if(!contr)
			return;

		m_bIsQualified = character.HasRole(GetQualifiedRoles()) || character.HasLabel(GetQualifiedLabels());
		
		if (contr.CanPartialLower() && !contr.IsPartiallyLowered())
			contr.SetPartialLower(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Override this if you would want to change which squad roles should make a character qualified
	protected array<SCR_EGroupRole> GetQualifiedRoles()
	{
		return {SCR_EGroupRole.MEDIC};
	}

	//------------------------------------------------------------------------------------------------
	//! Override this if you would want to change which character label should make a character qualified
	protected array<EEditableEntityLabel> GetQualifiedLabels()
	{
		return {EEditableEntityLabel.ROLE_MEDIC};
	}

	//------------------------------------------------------------------------------------------------
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		if (m_bIsQualified)
			timeSlice *= m_fQualifiedPersonnelBonusUseSpeedFactor;

		return fProgress + timeSlice;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can this action be shown in the UI for the user?
	override bool HasLocalEffectOnlyScript() 
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! If HasLocalEffectOnly() is true this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript()
	{
		return false;
	};
}

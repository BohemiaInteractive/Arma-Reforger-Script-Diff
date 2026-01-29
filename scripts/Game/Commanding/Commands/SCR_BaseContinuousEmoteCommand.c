[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_ContinuousLoiterCommand : SCR_BaseRadialCommand
{
	protected ref SCR_ScriptedCharacterInputContext m_pScrInputContext;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Loiter animation", enums: ParamEnumArray.FromEnum(ELoiteringType))]
	protected ELoiteringType m_eLoiterType;
	
	[Attribute(defvalue: "1", desc: "Unequip items in hand before doing emote?")]
	protected bool m_bUnequipItems;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Available stances. All stances allowed if empty", enums: ParamEnumArray.FromEnum(ECharacterStance))]
	protected ref array<ref ECharacterStance> m_aAllowedStances;
	
	protected ResourceName m_sHint = "{3D2B20A0A9C3248A}Configs/Hints/GeneralHints.conf";
	
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		//TODO: Implement optional local-only commands to safe networking for commands of this style
		if (SCR_PlayerController.GetLocalPlayerId() != playerID)
			return true;
		
		IEntity playerControlledEntity = GetGame().GetPlayerController().GetControlledEntity();		
		if (!playerControlledEntity)
			return false;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(playerControlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return false;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(playerControlledEntity.FindComponent(SCR_GadgetManagerComponent));
		if (!gadgetManager)
			return false;
		
		if (characterController.IsLoitering())
		{
			if (m_pScrInputContext.m_bLoiteringDisablePlayerInput)
				return false;

			characterController.StopLoitering(false);
			return true;
		}
		
		if (m_bUnequipItems && (gadgetManager.GetHeldGadget() || characterController.GetWeaponManagerComponent().GetCurrentWeapon()))
		{
			if (characterController.IsGadgetInHands())
				characterController.RemoveGadgetFromHand();		
			
			characterController.SelectWeapon(null);
			
			//Call the animation later when the unequipping of the items is done.
			GetGame().GetCallqueue().CallLater(StartLoiter, 1500, false);
			return true;
		}
		
		StartLoiter();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(notnull SCR_ChimeraCharacter user)
	{
		if (!user)
			return false;
		
		SCR_CharacterControllerComponent characterComponent = SCR_CharacterControllerComponent.Cast(user.GetCharacterController());
		if (!characterComponent)
			return false;
		
		if (m_aAllowedStances.IsEmpty())
			return true;
		
		return (m_aAllowedStances.Contains(characterComponent.GetStance()));
	}	
	
	//------------------------------------------------------------------------------------------------
	bool StartLoiter()
	{
		IEntity playerControlledEntity = GetGame().GetPlayerController().GetControlledEntity();		
		if (!playerControlledEntity)
			return false;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(playerControlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return false;
		
		vector emptyVector[4];
		characterController.StartLoitering(m_eLoiterType, false, true, false, emptyVector);
		
		Resource holder = BaseContainerTools.LoadContainer(m_sHint);
		if (!holder.IsValid())
			return true;
		
		//show hint explaining continuous emote
		BaseContainer container = holder.GetResource().ToBaseContainer();
		SCR_GeneralHintStorage hintStorage = SCR_GeneralHintStorage.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		SCR_HintManagerComponent.ShowHint(hintStorage.GetHintByType(EHint.EMOTES_INTRO));
		
		return true;
	}
}
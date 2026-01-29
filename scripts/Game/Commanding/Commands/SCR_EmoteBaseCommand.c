//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_EmoteBaseCommand : SCR_BaseRadialCommand
{
	[Attribute(defvalue: "1", desc: "Unequip items in hand before doing emote?")]
	protected bool m_bUnequipItems;
	
	[Attribute(defvalue: "0", desc: "ID of the gesture in gesture state machine to be played, 0 = no gesture")]
	protected int m_iGestureID;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Available stances. All stances allowed if empty", enums: ParamEnumArray.FromEnum(ECharacterStance))]
	protected ref array<ref ECharacterStance> m_aAllowedStances;
	
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		//TODO: Implement optional local-only commands to safe networking for commands of this style
		if (SCR_PlayerController.GetLocalPlayerId() != playerID)
			return true;
		
		IEntity playerControlledEntity = GetGame().GetPlayerController().GetControlledEntity();
		
		if (!playerControlledEntity)
			return false;
		
		SCR_CharacterControllerComponent characterComponent = SCR_CharacterControllerComponent.Cast(playerControlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterComponent)
			return false;
				
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(playerControlledEntity.FindComponent(SCR_GadgetManagerComponent));
		if (!gadgetManager)
			return false;
		
		if (m_bUnequipItems && (gadgetManager.GetHeldGadget() || characterComponent.GetWeaponManagerComponent().GetCurrentWeapon()))
		{
			if (characterComponent.IsGadgetInHands())
				characterComponent.RemoveGadgetFromHand();		
			characterComponent.SelectWeapon(null);
			
			//Call the animation later when the unequipping of the items is done.
			GetGame().GetCallqueue().CallLater(characterComponent.TryStartCharacterGesture, 1500, false, m_iGestureID, 2000 );
			
			return true;
		}
		
		characterComponent.TryStartCharacterGesture(m_iGestureID, 2000);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(notnull SCR_ChimeraCharacter user)
	{
		IEntity playerControlledEntity = GetGame().GetPlayerController().GetControlledEntity();
		
		if (!playerControlledEntity)
			return false;
		
		SCR_CharacterControllerComponent characterComponent = SCR_CharacterControllerComponent.Cast(playerControlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterComponent)
			return false;
		
		if (m_aAllowedStances.IsEmpty())
			return true;
		
		return (m_aAllowedStances.Contains(characterComponent.GetStance()));
	}
}
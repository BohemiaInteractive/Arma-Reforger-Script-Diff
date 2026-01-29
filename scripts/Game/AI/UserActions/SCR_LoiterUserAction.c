enum ELoiteringType
{
	NONE, 	  
	SIT,	  
	LEAN_LEFT,
	LEAN_RIGHT,
	SMOKING,
	LOITERING,
	PUSHUPS,
	CUSTOM
}

class SCR_LoiterUserAction: SCR_UserActionWithOccupancy
{
	[Attribute( defvalue: "0", uiwidget: UIWidgets.EditBox, desc: "Provide index of related SmartActionSentinel" )]
	protected int m_iSmartActionId;
	
	protected bool m_bHolsterWeapon;
	protected ELoiteringType m_eLoiteringType;
	
	//-------------------------------------------------------------------------
	//! Calculates the position of loitering
	//\param[out] vector with the position
	protected void GetLoiteringPosition(out vector loiteringPosition[4])
	{
		IEntity owner = GetOwner();
		array<Managed> aComponents = {};
		owner.FindComponents(SCR_AISmartActionSentinelComponent, aComponents);
		
		if (!aComponents.IsIndexValid(m_iSmartActionId))
			Print("Invalid SmartAction id!",LogLevel.ERROR);
		
		SCR_AISmartActionSentinelComponent smartAction = SCR_AISmartActionSentinelComponent.Cast(aComponents[m_iSmartActionId]);
		if (!smartAction)
		{
			owner.GetWorldTransform(loiteringPosition);
		}
		else
		{
			vector mat[4];
			owner.GetWorldTransform(mat); 
				
			//--- Position of the action
			vector desiredPos = mat[3] + smartAction.GetActionOffset().Multiply3(mat);
				
			//--- Position to rotate to
			vector desiredLookPos = mat[3] + smartAction.GetLookPosition().Multiply3(mat);
				
			//--- Up vector of entity that owns the action
			vector actionEntityUp = mat[1];	
				
			//--- Now I have rotation of entity to look towards desiredLookPos
			Math3D.DirectionAndUpMatrix(desiredLookPos - desiredPos, actionEntityUp, loiteringPosition);
				
			//--- Standing at desired desiredPos
			loiteringPosition[3] = desiredPos;
				
			m_eLoiteringType = smartAction.GetLoiterAnimation();
			m_bHolsterWeapon = smartAction.GetHolsterWeapon();
		}
	}
	
	//-------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!GetGame().InPlayMode())
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	override void StartAction(IEntity pUserEntity)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;
		
		vector actionTransform[4];
		GetLoiteringPosition(actionTransform);
		
		controller.StartLoitering(m_eLoiteringType, m_bHolsterWeapon, true, true, actionTransform);
	}
	
	//------------------------------------------------------------------------------------------------
	override void StopAction(IEntity pUserEntity)
	{
		bool stopFast = false;
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;
		
		AIControlComponent aiController = controller.GetAIControlComponent();
		if (aiController)
		{
			SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(aiController.GetAIAgent());
			if (!agent)
				return;
			stopFast = agent.ShouldAbortLoiterFast();
		}
		controller.StopLoitering(stopFast);	
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return !m_Occupant || m_Occupant == user;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = ("#AR-UserAction_Loiter");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
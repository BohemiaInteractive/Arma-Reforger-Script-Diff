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
	
	protected bool m_isValid;
	protected bool m_bHolsterWeapon;
	protected ELoiteringType m_eLoiteringType;
	protected vector m_targetPosition[4]
	
	//-------------------------------------------------------------------------
	//! Calculates the position of loitering
	//\param[out] vector with the position
	protected void GetLoiteringPosition(SCR_AISmartActionSentinelComponent smartAction, out vector loiteringPosition[4])
	{
		IEntity owner = GetOwner();
		
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
	}
	
	//-------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		//if (!GetGame().InPlayMode())
		//	return;
		// ...
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
		
		if (!m_isValid)
			return;
		m_isValid = false; // consume validity of this message
		
		controller.StartLoitering(GetOwner(), m_eLoiteringType, m_bHolsterWeapon, true, true, m_targetPosition);
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
			if (agent)
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
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CheckOnServerFirstScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		// Technically this should not be a broadcast action as it requires the rpl owner to initiate and complete it.
		// However, to keep things simple and because this is such a rare action, we allow this to be true for the sake
		// of simplicity.
		// Basically what happens is following:
		// 1) Owner sends a request to the server.
		// 2) The server fills the data which only it has.
		// 2a) if the owner is also the serverit will start loitering now
		// 3) The server broadcasts the action with the data.
		// 4) If (1) was not the server but a client, it will reach out to the server and start loitering now.
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		// Smart actions exist only on the server. Therefore, the server needs to indentify if the action setup is correct
		// and store necessary values in the buffer. When this buffer is replicated clients read it and do stuff based on the result.

		IEntity owner = GetOwner();
		array<Managed> aComponents = {};
		owner.FindComponents(SCR_AISmartActionSentinelComponent, aComponents);
		
		m_isValid = aComponents.IsIndexValid(m_iSmartActionId);
		if (!m_isValid)
		{
			Print(string.Format("Invalid SmartAction id at SCR_LoiterUserAction! Components=%1, id=%2", aComponents.Count(), m_iSmartActionId), LogLevel.WARNING);
			return false;
		}		
		
		SCR_AISmartActionSentinelComponent smartAction = SCR_AISmartActionSentinelComponent.Cast(aComponents[m_iSmartActionId]);
		if (!smartAction)
			return false;
		
		if (m_Occupant)
		{
			SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(m_Occupant.FindComponent(SCR_CharacterControllerComponent));
			if (!charCtrl || !charCtrl.IsLoiteringOnEntity(owner)) 
				m_Occupant = null; // the character is not loitering anymore but he didn't free the action?
		}
		
		bool hasOccupant = SaveOccupant(writer);
		if (hasOccupant)
			return true;
		
		GetLoiteringPosition(smartAction, m_targetPosition);
		
		writer.WriteInt((int)smartAction.GetLoiterAnimation());
		writer.WriteBool(smartAction.GetHolsterWeapon());
				
		writer.WriteVector(m_targetPosition[0]);
		writer.WriteVector(m_targetPosition[1]);
		writer.WriteVector(m_targetPosition[2]);
		writer.WriteVector(m_targetPosition[3]);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		bool hasOccupant = LoadOccupant(reader);
		if (hasOccupant)
		{
			m_isValid = false;
			return true;
		}

		reader.ReadInt(m_eLoiteringType);
		reader.ReadBool(m_bHolsterWeapon);
		
		for (int i = 0; i < 4; i++)
		{
			vector value;
			reader.ReadVector(value);
			m_targetPosition[i] = value;
		}
		
		m_isValid = true;
		return true;
	}
}
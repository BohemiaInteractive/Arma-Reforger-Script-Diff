enum ELoiterCommandState // TODO: SCR_
{
	LOITERING = 0,
	EXITING = 1,
	DONE = 2
}

class SCR_CharacterCommandLoiter : CharacterCommandScripted
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CharacterCommandLoiter(BaseAnimPhysComponent pAnimPhysComponent, ChimeraCharacter pCharacter, CharacterInputContext pBaseInputCtx, 
		SCR_ScriptedCharacterInputContext pScrInputCtx, SCR_ScriptedCommandsStaticTable pStaticTable, SCR_CharacterCommandHandlerComponent pScrCommandHandler,
		SCR_LoiterCustomAnimData customAnimData)
	{
		m_pCharAnimComponent = CharacterAnimationComponent.Cast(pAnimPhysComponent);
		m_pCharacter = pCharacter;
		m_pBaseInputCtx = pBaseInputCtx;
		m_pScrInputCtx = pScrInputCtx;
		m_pStaticTable = pStaticTable;
		m_pCommandHandler = pScrCommandHandler;
		m_customAnimData = customAnimData;
	}
	//------------------------------------------------------------------------------------------------
	SCR_ScriptedCharacterInputContext GetScriptedInputContext()
	{
		return m_pScrInputCtx;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_bWasTag = false;
		
		if (m_pScrInputCtx.m_iLoiteringType < 0)
		{
			SetFlagFinished(true);
			return;
		}
		
		if (m_customAnimData.m_GraphBindingName != string.Empty && m_customAnimData.m_GraphName != string.Empty)
		{
			PreAnim_SetAttachment(m_customAnimData.m_GraphBindingName, m_customAnimData.m_GraphName, m_customAnimData.m_GraphInstanceName, "MasterControl");
		}
		
		SwitchState(ELoiterCommandState.LOITERING);
	}

	//------------------------------------------------------------------------------------------------
	void FreeOccupiedActions()
	{
		if (!m_pScrInputCtx)
			return;
		
		IEntity loiterEnt = m_pScrInputCtx.GetLoiterEntity();
		if (!loiterEnt)
			return;
		
		BaseActionsManagerComponent actMgr = BaseActionsManagerComponent.Cast(loiterEnt.FindComponent(BaseActionsManagerComponent));
		if (!actMgr)
			return;
		
		array<BaseUserAction> actions = {};
		actMgr.GetActionsList(actions);
		
		foreach (BaseUserAction act : actions)
		{
			SCR_LoiterUserAction loiterAct = SCR_LoiterUserAction.Cast(act);
			if (loiterAct && loiterAct.IsOccupiedBy(m_pCharacter))
				loiterAct.ReleaseOccupant();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate()
	{
		// If loitering was terminated without properly releasing the ownership of the loiter action, do it now
		FreeOccupiedActions();
		
		if (m_customAnimData && m_customAnimData.m_CustomCommand != -1 && m_customAnimData.m_GraphName != string.Empty)
		{
			PreAnim_SetAttachment(m_customAnimData.m_GraphBindingName, string.Empty, string.Empty, string.Empty);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPossess()
	{
		StopLoitering(true);
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldForceFreeLook()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsRootMotionControlled()
	{
		return m_pScrInputCtx.m_bLoiteringRootMotion;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PrePhysUpdate(float pDt)
	{
		switch (m_eState)
		{
			case ELoiterCommandState.LOITERING:
			{
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag);
				m_bWasTag = isTag || m_bWasTag;
				
				bool cancelLoiterInputs = false;
				
				PlayerController playerController = GetGame().GetPlayerController();
				if (playerController && playerController.GetControlledEntity() == m_pCharacter)
				{
					InputManager iManager = GetGame().GetInputManager();
					cancelLoiterInputs = iManager.GetActionValue("CharacterFire") || iManager.GetActionValue("CharacterSprint") || iManager.GetActionValue("CharacterRaiseWeapon") || iManager.GetActionValue("CharacterWeaponADS") || iManager.GetActionValue("CharacterReload");
				}
				
				if ((m_bWasTag && !isTag) || (cancelLoiterInputs && !m_pScrInputCtx.m_bLoiteringDisablePlayerInput))
				{
					m_pCommandHandler.StopLoitering(false);
				}
			}
			break;
			case ELoiterCommandState.EXITING:
			{
				m_pCharAnimComponent.CallCommand(m_pStaticTable.m_CommandGesture, -1, 0.0); // -1 is soft exit.
				// @TODO(ivanickyjak) We cannot use tags on server.
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag) || m_pCharAnimComponent.IsSecondaryTag(m_pStaticTable.m_IsLoiteringTag);
				if (!isTag)
				{
					FinishLoiter();
					m_pCommandHandler.BroadCastLoiterFinish();
				}
			}
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param newState
	void SwitchState(ELoiterCommandState newState)
	{
		TAnimGraphCommand customCommand = -1;
		if (m_customAnimData)
			customCommand = m_customAnimData.m_CustomCommand;
		
		switch (newState)
		{
			case ELoiterCommandState.LOITERING:
			{
				m_bWasTag = false;
				if (customCommand != -1)
					m_pCharAnimComponent.CallCommand4I(customCommand, 0, m_pScrInputCtx.m_iLoiteringType, 0, 0, 0.0);
				else
					m_pCharAnimComponent.CallCommand4I(m_pStaticTable.m_CommandGesture, 0, m_pScrInputCtx.m_iLoiteringType, 0, 0, 0.0);
			}
			break;
			case ELoiterCommandState.EXITING:
			{
				if (customCommand == -1)
					m_pCharAnimComponent.CallCommand(m_pStaticTable.m_CommandGesture, -1, 0.0); // -1 is soft exit.
			}
			break;
		}

		m_eState = newState;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param terminateFast should be true when going into alerted or combat state.
	void StopLoitering(bool terminateFast)
	{
		SwitchState(ELoiterCommandState.EXITING);
	}

	//------------------------------------------------------------------------------------------------
	void FinishLoiter()
	{
		SetFlagFinished(true);
		m_pScrInputCtx.m_iLoiteringType = -1;
	}

	protected CharacterAnimationComponent m_pCharAnimComponent;
	protected ChimeraCharacter m_pCharacter;
	protected CharacterInputContext m_pBaseInputCtx;
	protected SCR_ScriptedCharacterInputContext m_pScrInputCtx;
	protected SCR_ScriptedCommandsStaticTable m_pStaticTable;
	protected SCR_CharacterCommandHandlerComponent m_pCommandHandler;
	protected SCR_LoiterCustomAnimData m_customAnimData;
		
	protected ELoiterCommandState m_eState;
	protected bool m_bWasTag;
}

class SCR_LoiterCustomAnimData
{
	const static string BINDING_NAME_NPC = "NPC";
	TAnimGraphCommand m_CustomCommand = -1; 
	string m_GraphName = string.Empty;
	string m_GraphInstanceName = string.Empty;	
	string m_GraphBindingName = BINDING_NAME_NPC;
	
	const static ref SCR_LoiterCustomAnimData Default = new SCR_LoiterCustomAnimData();
	
	//------------------------------------------------------------------------------------------------
	void SCR_LoiterCustomAnimData()
	{}
	
	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return m_CustomCommand == -1 && m_GraphName.IsEmpty() && m_GraphInstanceName.IsEmpty() && m_GraphBindingName == BINDING_NAME_NPC;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRplLoad(ScriptBitReader r)
	{
		int cmdInt;
		r.ReadInt(cmdInt);
		m_CustomCommand = cmdInt;
		r.ReadString(m_GraphName);
		r.ReadString(m_GraphInstanceName);
		r.ReadString(m_GraphBindingName);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRplSave(ScriptBitWriter w)
	{
		w.WriteInt(m_CustomCommand);
		w.WriteString(m_GraphName);
		w.WriteString(m_GraphInstanceName);
		w.WriteString(m_GraphBindingName);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_LoiterCustomAnimData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_CustomCommand);
		snapshot.SerializeString(instance.m_GraphName);
		snapshot.SerializeString(instance.m_GraphInstanceName);
		snapshot.SerializeString(instance.m_GraphBindingName);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_LoiterCustomAnimData instance)
	{
		snapshot.SerializeInt(instance.m_CustomCommand);
		snapshot.SerializeString(instance.m_GraphName);
		snapshot.SerializeString(instance.m_GraphInstanceName);
		snapshot.SerializeString(instance.m_GraphBindingName);		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);
		snapshot.EncodeString(packet);
		snapshot.EncodeString(packet);
		snapshot.EncodeString(packet);
	}

	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		snapshot.DecodeString(packet);
		snapshot.DecodeString(packet);
		snapshot.DecodeString(packet);
		return true;
	}
	
		//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4)	
			&& lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareStringSnapshots(rhs);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_LoiterCustomAnimData instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_CustomCommand)
			&& snapshot.CompareString(instance.m_GraphName)
			&& snapshot.CompareString(instance.m_GraphInstanceName)
			&& snapshot.CompareString(instance.m_GraphBindingName);
	}
}
[ComponentEditorProps(category: "GameScripted/Character", description: "Scripted character command handler", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterCommandHandlerComponentClass : CharacterCommandHandlerComponentClass
{
}

class SCR_CharacterCommandHandlerComponent : CharacterCommandHandlerComponent
{
	//------------------------------------------------------------------------------------------------
	override bool HandleMelee(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if (pInputCtx.GetMeleeAttack())
		{
			m_MeleeComponent.PerformAttack();
			GetCommandModifier_Melee().Attack();
			pInputCtx.SetMeleeAttack(false);		
			return true;
		}

		if (m_MeleeComponent)
			m_MeleeComponent.Update(pDt);

		return GetCommandModifier_Melee().IsMeleeAttackTag();
	}
	
	//------------------------------------------------------------------------------------------------
	override event int SelectMeleeAnimationType()
	{
		return m_MeleeComponent.HasBayonet();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerInt GetOnCommandActivate()
	{
		if (!m_OnCommandActivate)
			m_OnCommandActivate = new ScriptInvokerInt();

		return m_OnCommandActivate;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandActivate(int pCmdId)
	{
		if (m_OnCommandActivate)
			m_OnCommandActivate.Invoke(pCmdId);
		
		super.OnCommandActivate(pCmdId);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void StartCommandLoitering(SCR_LoiterCustomAnimData customAnimData)
	{
		if (IsLoitering())
			return;
		
		SCR_CharacterControllerComponent scrCharCtrl = SCR_CharacterControllerComponent.Cast(GetControllerComponent());
		if (!scrCharCtrl)
			return;

		m_CmdLoiter = new SCR_CharacterCommandLoiter(m_CharacterAnimComp, m_OwnerEntity, GetControllerComponent().GetInputContext(), scrCharCtrl.GetScrInputContext(), m_ScrStaticTable, this, customAnimData);
		m_CharacterAnimComp.SetCurrentCommand(m_CmdLoiter);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterCommandLoiter GetLoiterCommand()
	{
		AnimPhysCommandScripted currentCmdScripted = m_CharacterAnimComp.GetCommandScripted();
		if (!currentCmdScripted)
			return null;
		
		return SCR_CharacterCommandLoiter.Cast(currentCmdScripted);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] terminateFast
	//terminateFast should be true when we are going into alerted or combat state.
	void StopLoitering(bool terminateFast)
	{
		if (m_CmdLoiter)
			m_CmdLoiter.StopLoitering(terminateFast);
		
		Rpc(Rpc_StopLoiter_S, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] terminateFast
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_StopLoiter_S(bool terminateFast)
	{
		if (m_CmdLoiter)
			m_CmdLoiter.StopLoitering(terminateFast);
	}

	//------------------------------------------------------------------------------------------------
	void BroadCastLoiterFinish()
	{
		Rpc(RPC_FinishLoitering_BCNO);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast, RplCondition.NoOwner)]
	void RPC_FinishLoitering_BCNO()
	{		
		auto loiterCommand = GetLoiterCommand();
		if (!loiterCommand)
			return;
		
		loiterCommand.FinishLoiter();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScriptedCommandsStaticTable GetScriptedStaticTable()
	{
		return m_ScrStaticTable;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsLoitering()
	{
		return GetLoiterCommand() != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		m_MovementState 		= new CharacterMovementState();
		m_OwnerEntity 			= ChimeraCharacter.Cast(owner);
		m_CharacterAnimComp 	= CharacterAnimationComponent.Cast(m_OwnerEntity.FindComponent(CharacterAnimationComponent));
		m_MeleeComponent 		= SCR_MeleeComponent.Cast(m_OwnerEntity.FindComponent(SCR_MeleeComponent));
		m_WeaponManager 		= BaseWeaponManagerComponent.Cast(m_OwnerEntity.FindComponent(BaseWeaponManagerComponent));
		m_ScrStaticTable		= new SCR_ScriptedCommandsStaticTable();
		
		if (!m_ScrStaticTable.Bind(m_CharacterAnimComp))
			Print("Failed to bind scripted static table (see class SCR_ScriptedCommandsStaticTable). This can be caused by missing animation commands, tags, or events in the animation graph.");
	}
	
	protected ChimeraCharacter 				m_OwnerEntity;
	protected ref CharacterMovementState 	m_MovementState;
	protected CharacterAnimationComponent	m_CharacterAnimComp;
	protected CharacterControllerComponent  m_CharacterControllerComp;
	protected SCR_MeleeComponent			m_MeleeComponent;
	protected BaseWeaponManagerComponent 	m_WeaponManager;
	protected ref SCR_ScriptedCommandsStaticTable	m_ScrStaticTable;
	
	protected ref SCR_CharacterCommandLoiter m_CmdLoiter;
	
	protected ref ScriptInvokerInt m_OnCommandActivate;
}

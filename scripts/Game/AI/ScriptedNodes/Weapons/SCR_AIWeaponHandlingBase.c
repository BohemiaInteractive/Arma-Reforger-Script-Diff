//! Base class for nodes which handle magazine switching
class SCR_AIWeaponHandlingBase : AITaskScripted
{
	protected CharacterControllerComponent m_ControlComp;
	protected BaseWeaponManagerComponent m_WeaponMgrComp;
	protected SCR_InventoryStorageManagerComponent m_InventoryMgr;
	protected CompartmentAccessComponent m_CompartmentAccessComp;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity controlledEnt = owner.GetControlledEntity();
		
		m_WeaponMgrComp = BaseWeaponManagerComponent.Cast(controlledEnt.FindComponent(BaseWeaponManagerComponent));
		m_ControlComp = CharacterControllerComponent.Cast(controlledEnt.FindComponent(CharacterControllerComponent));
		m_InventoryMgr = SCR_InventoryStorageManagerComponent.Cast(controlledEnt.FindComponent(SCR_InventoryStorageManagerComponent));
		m_CompartmentAccessComp = CompartmentAccessComponent.Cast(controlledEnt.FindComponent(CompartmentAccessComponent));
		
		if (!m_WeaponMgrComp || !m_ControlComp || !m_InventoryMgr)
		{
			NodeError(this, owner, "SCR_AIWeaponHandlingBase didn't find necessary components!");
		}
	}
	
	
	#ifdef AI_DEBUG
	//--------------------------------------------------------------------------------------------
	protected SCR_AIInfoBaseComponent m_InfoComp; // Needed only for debugging
	protected void AddDebugMessage(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		if (!m_InfoComp)
		{
			IEntity owner = m_ControlComp.GetOwner();
			AIControlComponent controlComp = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
			AIAgent agent = controlComp.GetAIAgent();
			SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(agent.FindComponent(SCR_AIInfoBaseComponent));
			m_InfoComp = infoComp;
		}
		
		m_InfoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.WEAPON, logLevel);
	}
	#endif
}
class SCR_AIInspectWeapon: AITaskScripted
{
	[Attribute("0", UIWidgets.EditBox, desc: "State of inspecting the weapon", "")]
	protected float m_fInspectionState;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Stop inspecting the weapon", "")]
	protected bool m_bStopInspecting;

	protected SCR_CharacterControllerComponent m_Controller;
	protected bool m_bAborted;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_Controller = SCR_CharacterControllerComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_CharacterControllerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{				
		if (!m_Controller)
			return ENodeResult.FAIL;
		
		BaseWeaponManagerComponent pWpnManager = m_Controller.GetWeaponManagerComponent();
		if (!pWpnManager)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent pWpn = pWpnManager.GetCurrent();
		if (!pWpn)
			return ENodeResult.FAIL;
		
		IEntity pWpnEnt = pWpn.GetOwner();
		if (!m_Controller.CanInspect(pWpnEnt))
			return ENodeResult.FAIL;
		
		if (!m_bStopInspecting)
		{
			m_Controller.SetInspect(pWpnEnt);
			m_Controller.SetInspectState(m_fInspectionState);
		}
		else
		{
			m_Controller.SetInspect(null);
		}
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (!m_bStopInspecting)
			return "Weapon inspect type: " + m_fInspectionState;
		else
			return "Stop inspecting weapon";
	}
	
	// -----------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		// WARNING: m_bAborted is never set to true, thus the node cannot be restarted in a parralel and expected to abort properly.
		if (m_bAborted)
			return;
		
		m_bAborted = true;
		
		if (!m_Controller || !m_Controller.GetInspect())
			return;
		
		m_Controller.SetInspect(null);
	}
};
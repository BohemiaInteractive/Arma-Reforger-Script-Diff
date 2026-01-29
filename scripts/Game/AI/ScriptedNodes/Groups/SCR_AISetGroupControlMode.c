class SCR_AISetGroupControlMode: AITaskScripted
{
	[Attribute("0", UIWidgets.ComboBox, "Control mode to set", "", ParamEnumArray.FromEnum(EGroupControlMode) )]
	private EGroupControlMode m_ControlMode;
	
	private SCR_AIGroupInfoComponent m_GroupInfo;
	
	override void OnInit(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
			NodeError(this,owner,"SCR_AISetGroupControlMode must be run on group AIAgent!");
		m_GroupInfo = SCR_AIGroupInfoComponent.Cast(group.FindComponent(SCR_AIGroupInfoComponent));
		if (!m_GroupInfo)
			NodeError(this,owner,"Cannot find Group Info Component!");
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (m_GroupInfo)
			m_GroupInfo.SetGroupControlMode(m_ControlMode);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
    static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription() {return "Node sets control mode manually. Should be used only when Waypoint cannot do it by SendGoalMessage!";}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText() 
	{
		return "Control mode: "+ typename.EnumToString(EGroupControlMode,m_ControlMode);
	}
};
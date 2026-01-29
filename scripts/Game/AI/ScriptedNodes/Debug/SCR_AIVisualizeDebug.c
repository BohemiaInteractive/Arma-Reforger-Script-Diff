// BT node for visualizing strings over AIs
class SCR_AIVisualizeDebug : SCR_AIDebugTask
{
	AIAgent m_AgentForVisualize;
	
	[Attribute("0", UIWidgets.ComboBox, "Debug category", "", ParamEnumArray.FromEnum(EAIDebugCategory) )]
	private EAIDebugCategory m_eAICategory;
	
	[Attribute("10.0", UIWidgets.EditBox, "Show time")]
	private float m_fShowTime;
	
	[Attribute("20.0", UIWidgets.EditBox, "FontSize")]
	private float m_fFontSize;

	//------------------------------------------------------------------------------------------------
	protected override void PrintInternal(string in)
	{
		if (m_AgentForVisualize)
			SCR_AIDebugVisualization.VisualizeMessage(m_AgentForVisualize.GetControlledEntity(), in, m_eAICategory, m_fShowTime, Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()), m_fFontSize);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_AgentForVisualize = group.GetLeaderAgent();
		else
			m_AgentForVisualize = owner;
	}

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "BT node for vizualize debug messsages in workbench. String can be in In port or as parameter";
	}
};
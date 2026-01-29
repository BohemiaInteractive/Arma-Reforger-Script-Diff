//! Server kick message specific dialog 

//---------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_KickDialogCutomTitle()]
class SCR_KickDialogUiPreset : SCR_ConfigurableDialogUiPreset
{
	[Attribute("true", UIWidgets.CheckBox, "If true server browser will attempt to find last server to reconnect")]
	bool m_bCanBeReconnected;
	
	[Attribute("10")]
	int m_iAutomaticReconnectTime;
};

//------------------------------------------------------------------------------------------------
class SCR_KickDialogCutomTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sTag");
		if (index == -1)
			return false;
		
		// Tag string 
		string tag = "";
		source.Get("m_sTag", tag);
		
		source.Get("m_sAction", title);
		
		// Enabled string 		
		bool canReconnect;
		source.Get("m_bCanBeReconnected", canReconnect);
		
		string typeStr = "show";
		if (canReconnect)
			typeStr = "reconnect";
		
		// Setup title string 
		title = string.Format("%1 (type: %2)", tag, typeStr);
		return true;
	}
};
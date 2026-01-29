//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Channels", description: "Channel Manager", color: "0 0 255 255")]

//TODO: Probably remove this class for now
class SCR_ChatChannel : BaseChatChannel
{
	[Attribute("", uiwidget: UIWidgets.EditBox, desc :"Id")]
	int m_iId;
	[Attribute("#AR-Chat_CannotAccessChannel", uiwidget: UIWidgets.EditBox, desc :"Default message for channel warning")]
	string m_sWarningMessage;
	[Attribute("1", UIWidgets.CheckBox, "Enables/disables channel for script functionality")]
	bool m_bChannelEnabled;	
	
	//------------------------------------------------------------------------------------------------
	// TODO: Remove this. Channel is not responsible for this.
	bool IsChannelEnabled()
	{
		return m_bChannelEnabled;
	}
	
	
	// TODO: Remove this. Channels are all enabled by default now. Disabling them on player side
	// could be done by the channel itself
	//------------------------------------------------------------------------------------------------
	int GetID()
	{
		return m_iId;
	}
	
	//------------------------------------------------------------------------------------------------
	// If there is some warning needed, just hardcode it in SCR_Chat for now.
	// With some comment to make it more generic in next iteration
	string GetWarningMessage()
	{ 
		return m_sWarningMessage;
	}

};

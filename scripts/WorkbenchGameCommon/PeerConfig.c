[BaseContainerProps(), PeerConfigViewTitle()]
class PeerConfig
{
	[Attribute("Peer Config", UIWidgets.EditBox, "Title string for the peer config.")]
	string Title;
	
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;
	
	[Attribute("-1", UIWidgets.EditBox, "Horizontal coordinate on the screen if running in window mode. Default if -1.")]
	int	X;

	[Attribute("-1", UIWidgets.EditBox, "Vertical coordinate on the screen if running in window mode. Default if -1.")]
	int	Y;

	[Attribute("-1", UIWidgets.EditBox, "Width of the peer window if running in window mode. Default if -1.")]
	int	Width;

	[Attribute("-1", UIWidgets.EditBox, "Height of the peer window if running in window mode. Default if -1.")]
	int	Height;

	[Attribute("true", UIWidgets.CheckBox, "If true the peer will run in window mode.")]
	bool Windowed;

	[Attribute("true", UIWidgets.CheckBox, "If true the peer will run at full speed even when its window doesn't have focus.")]
	bool ForceUpdate;

	[Attribute("true", UIWidgets.CheckBox, "If true the peer won't steal focus from the current active window.")]
	bool NoFocus;

	[Attribute("-1", UIWidgets.EditBox, "Maximum allowed FPS. No limit if -1.")]
	int MaxFPS;

	[Attribute("true", UIWidgets.CheckBox, "If true, the peer joins the running server immediately.")]
	bool RplAutoJoin;
	[Attribute("true", UIWidgets.CheckBox, "If true, any time the connection is droppped an attempt to reconnect is made.")]
	bool RplAutoReconnect;	
	[Attribute("true", UIWidgets.CheckBox, "If true, no network-related timeout will result in disconnect. Necessary for debugging. Don't use for anything else.")]
	bool RplDisableTimeout;
	
	[Attribute("PeerPlugin", UIWidgets.EditBox, "Profile name used for the peer. A suffix of <PeerIndex> is added to the name.")]
	string Profile;
	
	[Attribute("", UIWidgets.EditBox, "Custom parameters.")]
	string Params;

	ProcessHandle	Handle = null;
}

//------------------------------------------------------------------------------------------------
//! Custom names for defined zones to make config setup easier
class PeerConfigViewTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string titleVar;
		source.Get("Title", titleVar);
		title = titleVar;

		bool enabled;	
		source.Get("Enabled", enabled);
		if (enabled)
			title += " (Enabled)";

		return true;
	}
};
class SCR_ServerConfigAdvancedComponent : SCR_ConfigListComponent
{
	protected const string HOST_LOCALY = "lan";
	protected const string BIND_IP = "bindAddress";
	protected const string BIND_PORT = "bindPort";
	protected const string PUBLIC_IP = "publicAddress";
	protected const string PUBLIC_PORT = "publicPort";
	
	protected SCR_WidgetListEntrySpinBox m_HostLocalyCheck;
	protected SCR_WidgetListEntryEditBox m_PublicAddress;
	protected SCR_WidgetListEntryEditBox m_PublicPortEdit;
	protected SCR_WidgetListEntryEditBox m_BindPortEdit;
	
	protected bool m_bWasPortEdited = false;
	
	// Invokers
	protected ref ScriptInvokerString m_OnPortChanged;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerString GetOnPortChanged()
	{
		if (!m_OnPortChanged)
			m_OnPortChanged = new ScriptInvokerString();

		return m_OnPortChanged;
	}
	
	//-------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Find widges 
		m_PublicAddress = SCR_WidgetListEntryEditBox.Cast(FindEntry(PUBLIC_IP));
		
		m_HostLocalyCheck = SCR_WidgetListEntrySpinBox.Cast(FindEntry(HOST_LOCALY));
		if (m_HostLocalyCheck)
			m_HostLocalyCheck.GetSpinBox().m_OnChanged.Insert(OnHostLocalyChanged);
		
		m_PublicPortEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(PUBLIC_PORT));
		if (m_PublicPortEdit)
			m_PublicPortEdit.GetEditBoxComponent().m_OnWriteModeLeave.Insert(OnPublicPortChanged);
		
		m_BindPortEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(BIND_PORT));
		if (m_BindPortEdit)
			m_BindPortEdit.GetEditBoxComponent().m_OnChanged.Insert(OnBindPortChanged);
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnHostLocalyChanged(SCR_SpinBoxComponent spinBox, int option)
	{
		bool selected = option == 1;
		
		// Set public ip to local 
		m_PublicAddress.SetInteractive(!selected);
		
		if (selected) // True
		{
			// Set public ip to local 
			m_PublicAddress.SetValue(SCR_WidgetEditFormatIP.LAN_VALUE);
		}
		else
		{
			// Clear public ip edit 
			m_PublicAddress.SetValue("");
		}
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnBindPortChanged(SCR_EditBoxComponent edit, string text)
	{
		// Registed port edited
		if (!m_bWasPortEdited && !text.IsEmpty())
		{
			m_bWasPortEdited = true;
		}
		else if (m_bWasPortEdited && text.IsEmpty())
		{
			m_bWasPortEdited = false;
		}
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnPublicPortChanged(string text)
	{
		// Edit bind by public port
		if (!m_bWasPortEdited && m_BindPortEdit)
			m_BindPortEdit.ClearInvalidInput();
		
		if (m_OnPortChanged)
			m_OnPortChanged.Invoke(m_PublicPortEdit.ValueAsString());
	}
	
	//-------------------------------------------------------------------------------------------
	//! Fill all entries with values from given DS config 
	void FillFromDSConfig(notnull SCR_DSConfig config)
	{
		FindEntry(HOST_LOCALY).SetValue(SCR_JsonApiStructHandler.BoolToString(config.publicAddress == SCR_WidgetEditFormatIP.LAN_VALUE));
		FindEntry(PUBLIC_IP).SetValue(config.publicAddress);
		FindEntry(PUBLIC_PORT).SetValue(config.publicPort.ToString());
		FindEntry(BIND_IP).SetValue(config.bindAddress);
		FindEntry(BIND_PORT).SetValue(config.bindPort.ToString());
	}
	
	//-------------------------------------------------------------------------------------------
	void SetIPPort(DSConfig config)
	{
		FindEntry(PUBLIC_IP).SetValue(config.publicAddress);
		FindEntry(PUBLIC_PORT).SetValue(config.publicPort.ToString());
		FindEntry(BIND_IP).SetValue(config.bindAddress);
		FindEntry(BIND_PORT).SetValue(config.bindPort.ToString());
	}
	
	//-------------------------------------------------------------------------------------------
	void SetPorts(string port)
	{
		m_PublicPortEdit.SetValue(port);
		m_BindPortEdit.SetValue(port);
	}
}
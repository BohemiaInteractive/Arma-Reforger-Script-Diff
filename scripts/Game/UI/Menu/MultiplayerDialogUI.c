//------------------------------------------------------------------------------------------------
class MultiplayerDialogUI: SCR_ConfigurableDialogUi //DialogUI
{
	const string WIDGET_EDIT_IP = "EditIP";
	const string WIDGET_EDIT_PORT = "EditPort";
	const string WIDGET_CHECK_NETWORK = "CheckboxNetwork";
	
	const string DEFAULT_ADDRESS = "127.0.0.1";
	const string DEFAULT_PORT = "2001";
	const int DIRECT_JOIN_CODE_LENGTH = 10;
	
	protected SCR_EditBoxComponent m_EditIP;
	protected SCR_EditBoxComponent m_EditPort;
	protected SCR_CheckboxComponent m_CheckNetwork;
	
	protected ServerBrowserMenuUI m_ServerBrowserUI;
	
	/*ref ScriptInvoker m_OnJoinCodeEntered = new ScriptInvoker;
	ref ScriptInvoker m_OnIpEntered = new ScriptInvoker;*/
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		string address = GetAddress();
		EDirectJoinFormats format = IsInputValid(m_EditIP);
		bool publicNetwork = !m_CheckNetwork.IsChecked();
		
		m_OnConfirm.Invoke(address, format, publicNetwork);
		
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetAddress()
	{
		// Theres no other way to recover the default values other than setting "UseDefaultValues".
		// This prevents the user from setting the value directly. Instead it forces him to delete
		// the default value glyph by glyph which is tedious.
		
		if (!m_EditIP || !m_EditPort)
			return string.Empty;
		
		// Address 
		string sAddr = m_EditIP.GetValue();
		if (sAddr.Length() == 0)
			return string.Empty;
		
		// Set client address as fallback - todo: this will be removed once lan detection is available
		if (sAddr == DEFAULT_ADDRESS)
			sAddr = GetGame().GetBackendApi().GetClientLobby().GetMyIP();
		
		// Port
		string sPort = string.Empty;
		
		if (IsAddressValid(sAddr))
		{
			sPort = m_EditPort.GetValue();
			if (sPort.Length() == 0)
				return string.Empty;
		}
		
		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		
		// Check whether address matches ipv4 format of x.x.x.x, or x.x.x.x:port where x = [0, 255]
		//REFACTOR NOTE
		//This is wrong, should check for either ip address OR invite code. Switche it to the corret function call.
		if(IsInputValid(m_EditPort))
		{
			if (gameplaySettings)
			{
				gameplaySettings.Set("m_sLastIP", sAddr);
				GetGame().UserSettingsChanged();
			}
		}
		
		if (gameplaySettings)
		{
			gameplaySettings.Set("m_sLastPort", sPort);
			GetGame().UserSettingsChanged();
		}
		
		// Results 
		if (!sPort.IsEmpty())
			return sAddr + ":" + sPort;
		else
			return sAddr;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Old OnMenuOpen()
		super.OnMenuOpen(preset);
		Widget root = GetRootWidget();
		
		
		// IP and join code edit 
		m_EditIP = SCR_EditBoxComponent.GetEditBoxComponent(WIDGET_EDIT_IP, root);
		
		if (m_EditIP)
		{
			m_EditIP.m_OnChanged.Insert(CheckValidInput);
			
			Widget focus = m_EditIP.GetRootWidget();
			if (focus)
				GetGame().GetWorkspace().SetFocusedWidget(focus);
		}
		
		// Port edit
		m_EditPort = SCR_EditBoxComponent.GetEditBoxComponent(WIDGET_EDIT_PORT, root);
		
		if (m_EditPort)
		{
			m_EditPort.m_OnChanged.Insert(CheckValidInput);
		}
		
		// Checkbox network 
		m_CheckNetwork = SCR_CheckboxComponent.GetCheckboxComponent(WIDGET_CHECK_NETWORK, root);
		
		//SetDialogType(EDialogType.ACTION);
		//---
		
		// Try to load the last valid address
		string text;
		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (gameplaySettings)
			gameplaySettings.Get("m_sLastIP", text);
		
		if (text != string.Empty && m_EditIP)
			m_EditIP.SetValue(text);
		
		// Port 
		string port;
		if (gameplaySettings)
			gameplaySettings.Get("m_sLastPort", port);
			
		if (m_EditPort)
		{
			if (!port.IsEmpty())
				m_EditPort.SetValue(port);
			else 
				m_EditPort.SetValue(DEFAULT_PORT);
		}

		// Set the confirm button enabled or disabled
		CheckValidInput(m_EditIP, string.Empty);																				   
	}
	
	//------------------------------------------------------------------------------------------------
	// Checks whether specified ipv4 address matches the 'x.x.x.x' format, where x = [0, 255]
	// if address is passed along with port, port is ignored
	bool IsAddressValid(string address)
	{
		// ""
		if (address == string.Empty)
			return false;
		
		
		// cut off port or whatever follows the :
		if (address.Contains(":"))
		{
			int colonIndex = address.IndexOf(":");
			address = address.Substring(0, colonIndex);
		}
		
		array<string> values = new array<string>();
		address.Split(".", values, true);
		int valuesCount = values.Count();
		
		// 0.1.2.3 = 4 values
		if (valuesCount != 4)
			return false;
		
		int zeroElements = 0;
		// check if values are ok
		for (int i = 0; i < 4; i++)
		{
			// Iterate through all tokens parsed between '.'
			// if all of the characters are digit (ascii 48-57)
			// it then checks if each value is within 0-255 range
			// if anything fails, returns false
			int strLen = values[i].Length();
			for (int j = 0; j < strLen; j++)
			{
				// Check if it's a valid digit
				string char = values[i].Get(j);
				int asciiValue = char.ToAscii();
				if (asciiValue >= 48 && asciiValue <= 57)
					continue;
				else
					return false;
			}
			
			int value = values[i].ToInt();
			if (value == 0)
				zeroElements++;
			
			if (value >= 0 && value <= 255)
				continue;
			else
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check it string is in join code format - 10 size, digits are check in editbox
	protected bool IsJoinCodeValid(string text)
	{
		// Text size 
		if (text.Length() != DIRECT_JOIN_CODE_LENGTH)
			return false;
		
		for (int i = 0, len = text.Length(); i < len; i++)
		{
			// fail if is not a number  
			if (text[i].ToInt() == 0 && text[i] != "0")
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckValidInput(SCR_EditBoxComponent comp, string text)
	{
		if (!m_EditIP)
			return;
		
		bool validInput = IsInputValid(m_EditIP) != EDirectJoinFormats.INVALID;
		bool validAddress = IsInputValid(m_EditIP) == EDirectJoinFormats.IP_PORT;

		// Enable input 
		m_EditPort.SetEnabled(validAddress);
		//m_Confirm.SetEnabled(validInput);
		FindButton("confirm").SetEnabled(validInput);
	}
	
	//------------------------------------------------------------------------------------------------
	protected EDirectJoinFormats IsInputValid(SCR_EditBoxComponent comp)
	{
		// IP:port
		if (IsAddressValid(m_EditIP.GetValue()))
			return EDirectJoinFormats.IP_PORT;
		
		// Join code
		if (IsJoinCodeValid(m_EditIP.GetValue()))
			return EDirectJoinFormats.JOIN_CODE;
		
		return EDirectJoinFormats.INVALID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetServerBrowserUI(ServerBrowserMenuUI serverBrowserUI) { m_ServerBrowserUI = m_ServerBrowserUI; }
};

//------------------------------------------------------------------------------------------------
enum EDirectJoinFormats
{
	INVALID,
	IP_PORT,
	JOIN_CODE,
	ROOM_ID,
};


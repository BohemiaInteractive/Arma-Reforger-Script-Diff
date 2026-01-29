class SCR_InterfaceSettingsSubMenu : SCR_SettingsSubMenuBase
{
	protected const string CHAT_SPINBOX_WIDGET_NAME = "Chat";
	protected const string NOTIFICATIONS_SPINBOX_WIDGET_NAME = "Notifications";
	protected const string VON_SPINBOX_WIDGET_NAME = "VoN";
	protected const string CONTROLHINTS_SPINBOX_WIDGET_NAME = "ControlHints";
	protected const string HINTS_SPINBOX_WIDGET_NAME = "Hints";
	protected const string NEARBYINTERACTION_SPINBOX_WIDGET_NAME = "NearbyInteractions";
	protected const string NAMETAGS_SPINBOX_WIDGET_NAME = "NameTags";
	protected const string FPS_SPINBOX_WIDGET_NAME = "FPS";
	protected const string SERVER_FPS_SPINBOX_WIDGET_NAME = "ServerFPS";
	protected const string PACKETLOSS_SPINBOX_WIDGET_NAME = "PacketLoss";
	protected const string LATENCY_SPINBOX_WIDGET_NAME = "Latency";
	protected const string GAMEVERSION_SPINBOX_WIDGET_NAME = "GameVersion";
	protected const string WEAPONINFO_SPINBOX_WIDGET_NAME = "WeaponInfo";
	protected const string VEHICLEINFO_SPINBOX_WIDGET_NAME = "VehicleInfo";
	protected const string MASTERSWITCH_SPINBOX_WIDGET_NAME = "MasterSwitch";
	protected const string SERVERNAME_SPINBOX_WIDGET_NAME = "ServerName";
	protected const string INTERACTION_HINT_SPINBOX_WIDGET_NAME = "InteractionHint";

	protected const string MASTER_SWITCH_HIDE_STATE_NAME = "#AR-Settings_Hide_All";
	protected const string MASTER_SWITCH_SHOW_STATE_NAME = "#AR-Settings_Show_All";
	protected const string MASTER_SWITCH_CUSTOM_STATE_NAME = "#AR-Settings_Interface_Custom";
	
	protected const int MASTER_SWITCH_CUSTOM_STATE_SPINBOX_INDEX = 2;

	protected BaseContainer m_InterfaceSettings;
	protected SCR_SpinBoxComponent m_SpinnerBoxComp;
	protected SCR_SpinBoxComponent m_MasterSpinBoxComp;

	protected bool m_bIsAllShown;
	protected bool m_bIsCustom;
	
	protected ref array<ref SCR_SpinBoxComponent> m_aAllSpinBoxes = {};

	//------------------------------------------------------------------------------------------------
	//! Get the correct Spinnerbox widget for the setting and set it's value based on the stored setting
	//! \param[in] name of the Spinnerbox widget
	//! \param[in] variable name of the setting from SCR_InterfaceSettings
	protected void SetupSpinboxes(string widgetName, string settingName)
	{
		m_SpinnerBoxComp = SCR_SpinBoxComponent.GetSpinBoxComponent(widgetName, m_wRoot);
		if (m_SpinnerBoxComp)
		{
			SCR_SettingSpinBoxUserData data = new SCR_SettingSpinBoxUserData();
			data.SetSettingName(settingName);
			
			bool state;
			m_InterfaceSettings.Get(settingName, state);
			m_SpinnerBoxComp.SetCurrentItem(state);

			
			m_SpinnerBoxComp.SetData(data);
			m_aAllSpinBoxes.Insert(m_SpinnerBoxComp);
		}
		else
		{
			PrintFormat("Interface setting widget '%1' not found", widgetName, level: LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupMasterSwitch()
	{
		m_MasterSpinBoxComp = SCR_SpinBoxComponent.GetSpinBoxComponent(MASTERSWITCH_SPINBOX_WIDGET_NAME, m_wRoot);
		if (!m_MasterSpinBoxComp)
			return;
		
		// Add the 2 items that will always be in the SpinBox
		m_MasterSpinBoxComp.AddItem(MASTER_SWITCH_HIDE_STATE_NAME);
		m_MasterSpinBoxComp.AddItem(MASTER_SWITCH_SHOW_STATE_NAME);

		ChangeMasterSwitchState();
	}

	//------------------------------------------------------------------------------------------------
	protected void ChangeMasterSwitchState()
	{
		if (!m_MasterSpinBoxComp)
			return;

		// Remove invoker to prevent it from triggering the function when Item is removed
		m_MasterSpinBoxComp.m_OnChanged.Remove(OnMasterChange);

		int state;
		m_bIsCustom = false;
		
		foreach (int index, SCR_SpinBoxComponent spinBoxComponent : m_aAllSpinBoxes)
		{
			state = spinBoxComponent.GetCurrentIndex();

			// Check the first index to define what is expected of the others
			// In the end this defines what the MasterSwitch should show.
			if (index == 0)
				m_bIsAllShown = state;

			if (m_bIsAllShown != state)
			{
				// If one SpinBox has not the state we expect, the MasterSwitch needs to show a custom state
				m_bIsCustom = true;
				break;
			}
		}

		if (m_bIsCustom && m_MasterSpinBoxComp.GetCurrentItem() != MASTER_SWITCH_CUSTOM_STATE_NAME)
		{
			SetCustomState();
		}
		else if (!m_bIsCustom)
		{
			if (m_MasterSpinBoxComp.GetCurrentItem() == MASTER_SWITCH_CUSTOM_STATE_NAME)
				m_MasterSpinBoxComp.RemoveItem(m_MasterSpinBoxComp.GetCurrentIndex());

			m_MasterSpinBoxComp.SetCurrentItem(m_bIsAllShown);
		}

		// Add the invoker again once everything is done
		m_MasterSpinBoxComp.m_OnChanged.Insert(OnMasterChange);

	}

	//------------------------------------------------------------------------------------------------
	protected void SetCustomState()
	{
		if (m_MasterSpinBoxComp)
			m_MasterSpinBoxComp.SetCurrentItem(m_MasterSpinBoxComp.AddItem(MASTER_SWITCH_CUSTOM_STATE_NAME));
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_InterfaceSettings = GetGame().GetGameUserSettings().GetModule("SCR_InterfaceSettings");

		if (!m_InterfaceSettings)
			return;

		// Setup Chat
		SetupSpinboxes(CHAT_SPINBOX_WIDGET_NAME, "m_bShowChat");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup Notifications
		SetupSpinboxes(NOTIFICATIONS_SPINBOX_WIDGET_NAME, "m_bShowNotifications");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup VoN
		SetupSpinboxes(VON_SPINBOX_WIDGET_NAME, "m_bShowVoN");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup Control hints
		SetupSpinboxes(CONTROLHINTS_SPINBOX_WIDGET_NAME, "m_bShowControlHints");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup hints
		SetupSpinboxes(HINTS_SPINBOX_WIDGET_NAME, "m_bShowHints");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup nearby interactions
		SetupSpinboxes(NEARBYINTERACTION_SPINBOX_WIDGET_NAME, "m_bShowNearbyInteractions");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup Nametags
		SetupSpinboxes(NAMETAGS_SPINBOX_WIDGET_NAME, "m_bShowNameTags");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup FPS
		SetupSpinboxes(FPS_SPINBOX_WIDGET_NAME, "m_bShowFPS");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup ServerFPS
		SetupSpinboxes(SERVER_FPS_SPINBOX_WIDGET_NAME, "m_bShowServerFPS");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup PacketLoss
		SetupSpinboxes(PACKETLOSS_SPINBOX_WIDGET_NAME, "m_bShowPacketLoss");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup Latency
		SetupSpinboxes(LATENCY_SPINBOX_WIDGET_NAME, "m_bShowLatency");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup GameVersion
		SetupSpinboxes(GAMEVERSION_SPINBOX_WIDGET_NAME, "m_bShowGameVersion");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup WeaponInfo
		SetupSpinboxes(WEAPONINFO_SPINBOX_WIDGET_NAME, "m_bShowWeaponInfo");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);

		// Setup VehicleInfo
		SetupSpinboxes(VEHICLEINFO_SPINBOX_WIDGET_NAME, "m_bShowVehicleInfo");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);
		
		// Setup ServerName
		SetupSpinboxes(SERVERNAME_SPINBOX_WIDGET_NAME, "m_bShowServerName");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);
		
		// Setup InteractionNames
		SetupSpinboxes(INTERACTION_HINT_SPINBOX_WIDGET_NAME, "m_bShowInteractionHint");
		if (m_SpinnerBoxComp)
			m_SpinnerBoxComp.m_OnChanged.Insert(OnSpinBoxChange);
		
		// Setup Master switch
		SetupMasterSwitch();
	}

	//------------------------------------------------------------------------------------------------
	void OnSpinBoxChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		SCR_SettingSpinBoxUserData data = SCR_SettingSpinBoxUserData.Cast(spinBox.GetData());
		if (!data)
			return;
		
		string settingsName = data.GetSettingName();
		if (settingsName.IsEmpty())
			return;
		
		m_InterfaceSettings.Set(settingsName, currentItem);

		if (settingsName == "m_bShowHints")
		{
			BaseContainer hintSetings = GetGame().GetGameUserSettings().GetModule("SCR_HintSettings");
			if (hintSetings)
				hintSetings.Set("m_bHintsEnabled", currentItem);
		}

		GetGame().UserSettingsChanged();
		ChangeMasterSwitchState();
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting("Interface", settingsName);
	}

	//------------------------------------------------------------------------------------------------
	void OnMasterChange(SCR_SpinBoxComponent spinBox, int currentItem)
	{
		// Remove invoker to prevent it from triggering this function again when Item is removed
		m_MasterSpinBoxComp.m_OnChanged.Remove(OnMasterChange);

		string settingName;
		SCR_SettingSpinBoxUserData data;
		
		foreach (SCR_SpinBoxComponent spinBoxComponent : m_aAllSpinBoxes)
		{
			data = SCR_SettingSpinBoxUserData.Cast(spinBoxComponent.GetData());
			if (!data)
				continue;
			
			settingName = data.GetSettingName();
			if (settingName.IsEmpty())
				continue;
			
			m_InterfaceSettings.Set(settingName, currentItem);
			
			spinBoxComponent.m_OnChanged.Remove(OnSpinBoxChange);
			spinBoxComponent.SetCurrentItem(currentItem);
			spinBoxComponent.m_OnChanged.Insert(OnSpinBoxChange);
		}

		GetGame().UserSettingsChanged();

		if (m_MasterSpinBoxComp.GetCurrentItem() != MASTER_SWITCH_CUSTOM_STATE_NAME)
		{
			m_MasterSpinBoxComp.RemoveItem(MASTER_SWITCH_CUSTOM_STATE_SPINBOX_INDEX);
			m_MasterSpinBoxComp.SetCurrentItem(currentItem);
		}

		// Add the invoker again once everything is done
		m_MasterSpinBoxComp.m_OnChanged.Insert(OnMasterChange);
		
		SCR_AnalyticsApplication.GetInstance().SetHudVisibilitySetting(currentItem);
	}
}

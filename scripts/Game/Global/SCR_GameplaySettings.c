enum ECollectiveModeForSettings
{
	Default,
	Manual,
	Sticky,
	Direct
}

class SCR_GameplaySettings : ModuleGameSettings
{
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Allow controls hints on screen.")]
	bool m_bControlHints;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Allow controls hints on screen.")]
	bool m_b2DScopes;
	
	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Show nametag platform icon.")]
	bool m_bPlatformIconNametag;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable Logitech LED support.")]
	bool m_bLogitechSupport;

	[Attribute(defvalue: "127.0.0.1", uiwidget: UIWidgets.EditBox, desc: "Last IP used for server connection.")]
	string m_sLastIP;
	
	[Attribute(defvalue: "2001", uiwidget: UIWidgets.EditBox, desc: "Last port used for server connection.")]
	string m_sLastPort;

	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Player's local profile name.")]
	string m_sProfileName;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Show radio protocol subtitles in chat.")]
	bool m_bShowRadioProtocolText;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Preserve selected gadget after performing actions like sprinting.")]
	bool m_bStickyGadgets;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Preserve aim down sights after performing actions like sprinting.")]
	bool m_bStickyADS;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Whether or not the game should automatically equip next placeable item when previous one was placed")]
	bool m_bAutoEquipNextPlaceableItem;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Use mouse input for steering instead of for freelook when piloting aircrafts.")]
	bool m_bMouseControlAircraft;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Use gamepad input for freelook instead of for steering when piloting aircrafts.")]
	bool m_bGamepadFreelookInAircraft;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(ECollectiveModeForSettings.Default), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECollectiveModeForSettings), desc: ".")]
	ECollectiveModeForSettings m_eKeyboardCollective;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(ECollectiveModeForSettings.Direct), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECollectiveModeForSettings), desc: ".")]
	ECollectiveModeForSettings m_eGamepadCollective;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(ECollectiveModeForSettings.Direct), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECollectiveModeForSettings), desc: ".")]
	ECollectiveModeForSettings m_eHotasCollective;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(EVehicleDrivingAssistanceMode.FULL), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleDrivingAssistanceMode), desc: "Player's vehicle driving assistance mode. Controls gearbox, engine and persistent handbrake automation.")]
	EVehicleDrivingAssistanceMode m_eDrivingAssistance;
};

class SCR_FieldOfViewSettings : ModuleGameSettings
{
	[Attribute(defvalue: "74", uiwidget: UIWidgets.Slider, params: "40 90 1", desc: "Field of view in first person camera")]
	float m_fFirstPersonFOV;

	[Attribute(defvalue: "74", uiwidget: UIWidgets.Slider, params: "40 90 1", desc: "Field of view in third person camera.")]
	float m_fThirdPersonFOV;

	[Attribute(defvalue: "74", uiwidget: UIWidgets.Slider, params: "40 90 1", desc: "Field of view in vehicle camera.")]
	float m_fVehicleFOV;

	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "Aiming down sights focus intensity.")]
	float m_fFocusInADS;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "Scale of aiming down sight focus intensity for PIP scopes.")]
	float m_fFocusInPIP;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Use focus mode by holding right mouse button.")]
	bool m_bEnableFocus;
};

class SCR_AudioSettings : ModuleGameSettings
{
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "0 200 5", desc: "Audio dynamic range.")]
	float m_fDynamicRange;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Tinnitus Sound Playback")]
	bool m_bGTinnitus;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable HQ Announcer")]
	bool m_bHQAnnouncer;
};

class SCR_VideoSettings : ModuleGameSettings
{
	// Simple dof as default, using enum DepthOfFieldTypes
	[Attribute("1")]
	float m_iDofType;

	// Enable or disable nearby DepthOfField
 	[Attribute("1")]
 	bool m_bNearDofEffect;
	
	[Attribute("2500")]
	int m_iViewDistance;
	
	[Attribute("-1")]
	int m_iLastUsedPreset;
};

class SCR_HintSettings : ModuleGameSettings
{
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Allow context hints on screen")]
	protected bool m_bHintsEnabled;

	[Attribute(desc: "Types of hints which were already shown and should not be displayed again.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EHint))]
	protected ref array<EHint> m_aShownHints;

	[Attribute(desc: "How many times were hints in Shown Hints attribute displayed. Order of array items is the same.")]
	protected ref array<int> m_aShownHintCounts;

	/*!
	\return True if context hints are enabled
	*/
	bool AreHintsEnabled()
	{
		return m_bHintsEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHintsEnabled(bool enabled)
	{
		m_bHintsEnabled = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check how many times has specific hint been shown previously.
	\param type Hint type
	\return Number of time the hint has been shown
	*/
	int GetCount(EHint type)
	{
		if (!m_aShownHints)
			return 0;

		int index = m_aShownHints.Find(type);
		if (index == -1)
			return 0;
		else
			return m_aShownHintCounts[index];
	}
	/*!
	Adjust number of times specific hint has been shown.
	\param type Hint type
	\param count Delta value by which the count will be modified
	\return Number of time the hint has been shown after the change
	*/
	int AddCount(EHint type, int delta = 1)
	{
		if (!m_aShownHints)
			return 0;

		int index = m_aShownHints.Find(type);
		if (index == -1)
		{
			index = m_aShownHints.Insert(type);
			m_aShownHintCounts.Insert(delta);
		}
		else
		{
			m_aShownHintCounts[index] = m_aShownHintCounts[index] + 1;
		}
		
		return m_aShownHintCounts[index];
	}

	//------------------------------------------------------------------------------------------------
	void DontShowAgain(EHint type, BaseContainer container)
	{
		if (!m_aShownHints)
			return;

		int index = m_aShownHints.Find(type);
		if (!m_aShownHintCounts.IsIndexValid(index))
			return;

		m_aShownHintCounts[index] = -1;

		SaveShownHints(container);
	}

	/*!
	Load shown hints to this settings class and validate if existing values are correct. If not, reset them.
	\param container Settings container
	*/
	void LoadShownHints(out BaseContainer container)
	{
		BaseContainerTools.WriteToInstance(this, container);

		//--- Arrays don't match, assume that data are corrupted and erase them completely
		if (m_aShownHints.Count() != m_aShownHintCounts.Count())
		{
			Debug.Error2("SCR_HintSettings.LoadShownHints()", "Error when loading persistent hints, number of hint IDs does not match the number of repetitions. Memory of shown hints was erased to prevent issues.");
			m_aShownHints.Clear();
			m_aShownHintCounts.Clear();
			SaveShownHints(container);
		}

		//--- When hints are disabled, clear the memory. All hints will be shown again upon re-enabling.
		if (!m_bHintsEnabled && !m_aShownHints.IsEmpty())
		{
			m_aShownHints.Clear();
			m_aShownHintCounts.Clear();
			SaveShownHints(container);
			Print("Persistent state of hints cleared!", LogLevel.VERBOSE);
		}
	}
	/*!
	Persistently save shown hints from this settings class.
	\param container Settings container
	*/
	void SaveShownHints(BaseContainer container)
	{
		container.Set("m_aShownHints", m_aShownHints);
		container.Set("m_aShownHintCounts", m_aShownHintCounts);
		GetGame().UserSettingsChanged();
	}
};

class SCR_RecentGames : ModuleGameSettings
{
	[Attribute("", UIWidgets.ResourceAssignArray, "Recent game headers", "conf")]
	ref array<ResourceName> m_aRecentMissions;

	[Attribute("3")]
	int m_iMaxRecentEntries;

	[Attribute("false")]
	bool m_bEAScreenShown;
	
	[Attribute("false")]
	bool m_bTutorialPlayed;

	[Attribute("true")]
	bool m_bFirstTimePlay;
	
	[Attribute("0")]
	int m_iPlayTutorialShowCount;
	
	[Attribute("3")]
	int m_iPlayTutorialShowMax;	
};

class SCR_LoadingHints : ModuleGameSettings
{
	[Attribute("", UIWidgets.ResourceAssignArray, "Already read hints", "conf")]
	ref array<LocalizedString> m_aReadHints;
};


class SCR_InventoryHintSettings : ModuleGameSettings
{
	[Attribute()]
	protected int m_iInventoryOpenCount;

	[Attribute()]
	protected int m_iQuickSlotShowCount;
};

class SCR_DeployMenuSettings : ModuleGameSettings
{
	[Attribute("1")]
	protected bool m_bShowPersistentFactionWarning;
};

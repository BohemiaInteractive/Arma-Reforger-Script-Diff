//! Base Attribute Script for other attributes to inherent from to get and set varriables in Editor Attribute window
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseEditorAttribute
{
	[Attribute()]
	protected ref SCR_EditorAttributeUIInfo m_UIInfo;
	
	[Attribute(defvalue: "1", desc: "True to get the value from server.")]
	private bool m_bIsServer;
	
	[Attribute(params: "conf")]
	protected ResourceName m_CategoryConfig;
	
	[Attribute(params: "layout")]
	protected ResourceName m_Layout;
	
	[Attribute(desc: "Holds a list of dynamic descriptions from attributes. When hovered and when changing the attribute the system will loop through all entries and display the dynamic description for the first valid. If none are valid than the default description is shown")]
	protected ref array<ref SCR_BaseAttributeDynamicDescription> m_aAttributeDynamicDescriptions;
	
	//State
	protected bool m_bAttributeEnabled = true;
	protected bool m_bIsSubAttribute;
	protected bool m_bInitCalled;
	protected bool m_bConflictingAttributeWasReset;
	protected BaseGameMode m_GameMode;
	
	//Conflicting attributes
	protected bool m_bIsMultiSelect = false;
	protected bool m_bHasConflictingValues = false;
	protected bool m_bOverridingValues = false;
	
	//Vars
	protected ref SCR_BaseEditorAttributeVar m_Var;
	protected ref SCR_BaseEditorAttributeVar m_CopyVar; //Used when has conflicting values. m_Var is deleted and a copy is saved if the var is disabled
	
	protected ref SSnapshot m_Snapshot;
	protected ref ScriptInvoker Event_OnExternalChange;
	protected ref ScriptInvoker Event_OnVarChanged = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnToggleEnable = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnToggleButtonSelected = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnSetAsSubAttribute = new ScriptInvoker;
		
	//------------------------------------------------------------------------------------------------
	//! Reset the attribute
	//! Called in SCR_AttributesManagerEditorComponent when it loops through valid attributes
	void ResetAttribute()
	{
		SetInitCalled(false);
		ClearCopyVar();
		SetIsOverridingValues(false);
		SetConflictingAttributeWasReset(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get UIinfo, Description is attribute description and Icon is used when the attribute description is overridden
	//! \return m_UIInfo
	SCR_EditorAttributeUIInfo GetUIInfo()
	{
		return m_UIInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get array of dynamic descriptions
	//! \param[out] Dynamic descriptions array
	//! \return count of dynamic description array
	int GetDynamicDescriptionArray(notnull out array<SCR_BaseAttributeDynamicDescription> dynamicDescriptionArray)
	{
		foreach (SCR_BaseAttributeDynamicDescription description : m_aAttributeDynamicDescriptions)
		{
			dynamicDescriptionArray.Insert(description);
		}
		
		return dynamicDescriptionArray.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the server should be extracted and applied on server.
	//! \return True if the attribute is controlled by server.
	bool IsServer()
	{
		return m_bIsServer;
	}

	//------------------------------------------------------------------------------------------------
	//! Get category config file.
	//! \return Path to *.conf file
	ResourceName GetCategoryConfig()
	{
		return m_CategoryConfig;
	}

	//------------------------------------------------------------------------------------------------
	//! Get UI layout.
	//! \return Path to *.layout file
	ResourceName GetLayout()
	{
		return m_Layout;
	}

	//------------------------------------------------------------------------------------------------
	//! Return True if attribute is allowed to be duplicate.
	//! Only Global attributes can be duplicates. Entity Attributes that can have confliciting attributes (eg two entities are opened and both have diffrent values. Can never be duplicate!
	//! \return True if allow dupplicate
	bool BoolAllowDuplicate()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return True if the attribute value is serialized in session save file
	bool IsSerializable()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set a variable to this attribute.
	//! \param var Variable
	//! \return if attribute valid
	sealed bool SetVariable(SCR_BaseEditorAttributeVar var)
	{
		if (!var)
			return false;
		
		if (m_Var)
		{
			if (var.Type() == m_Var.Type())
			{
				m_Var = var;
				Event_OnVarChanged.Invoke(var);
				return true;
			}
			else 
			{
				PrintFormat("Trying to set attribute var to type '%1' but is type '%2'", var.Type().ToString(), m_Var.Type().ToString(), LogLevel.WARNING);
				return false;
			}
		}
		else 
		{
			m_Var = var;
			Event_OnVarChanged.Invoke(var);
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set var null
	sealed void ClearVar()
	{
		m_Var = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Set copy var null
	sealed void ClearCopyVar()
	{
		m_CopyVar = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get On Var changed script invoker
	//! \return changed variable invoker
	sealed ScriptInvoker GetOnVarChanged()
	{
		return Event_OnVarChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sends out an event which UI listens to to enable/disable UI component
	//! \param bool enable
	sealed void Enable(bool enabled)
	{
		m_bAttributeEnabled = enabled;
		Event_OnToggleEnable.Invoke(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sends out an event which UI listens if is selected
	//! \param bool enable
	sealed void ToggleSelected(bool selected, int index)
	{
		Event_OnToggleButtonSelected.Invoke(selected, index, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sends set bool UI listens to to enable/disable UI component
	//! \param bool enable
	sealed void SetAsSubAttribute()
	{
		m_bIsSubAttribute = true;
		Event_OnSetAsSubAttribute.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if init is called. If it is a conflicting attribute it will set values to conflicting default if not yet called.
	//! \return if is init Called
	bool GetInitCalled()
	{
		return m_bInitCalled;
	}

	//------------------------------------------------------------------------------------------------
	//! Set Init called
	//! \param initCalled if Init is called
	void SetInitCalled(bool initCalled)
	{
		m_bInitCalled = initCalled;
	}

	//------------------------------------------------------------------------------------------------
	//! Get is multi select var
	//! Multi select means that multiple entities where selected when editing attributes and those entities share the same attributes
	//! \return is Multi Select
	bool GetIsMultiSelect()
	{
		return m_bIsMultiSelect;
	}	

	//------------------------------------------------------------------------------------------------
	//! Set if variable is multiselect
	//! Multi select means that multiple entities were selected when editing attributes and those entities share the same attributes
	//! \param isMultiSelect if is Multi select
	void SetIsMultiSelect(bool isMultiSelect)
	{
		m_bIsMultiSelect = isMultiSelect;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get has conflicting values.
	//! This can only be true when m_bIsMultiSelect is ture and means that the shared attributes have one ore more conflicting values
	//! \return true if it has conflicting values
	bool GetHasConflictingValues()
	{
		return m_bHasConflictingValues;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set if varriable is has conflicting values.
	//! This can only be true when m_bIsMultiSelect is ture and means that the shared attributes have one ore more conflicting values
	//! \param hasConflictingValues if has conflicting values
	void SetHasConflictingValues(bool hasConflictingValues)
	{
		m_bHasConflictingValues = hasConflictingValues;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set is overriding values. If m_bHasConflictingValues is true
	//! \param bool is overriding values
	void SetIsOverridingValues(bool isOverridingValues)
	{
		m_bOverridingValues = isOverridingValues;
	}

	//------------------------------------------------------------------------------------------------
	//! Get is overriding values.
	//! \return true if overriding values
	bool GetIsOverridingValues()
	{
		return m_bOverridingValues;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if The Attribute is Enabled
	//! \return bool enabled
	bool IsEnabled()
	{
		return m_bAttributeEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if The Attribute is Enabled
	//! \return bool enabled
	bool GetIsSubAttribute()
	{
		return m_bIsSubAttribute;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get On ToggleEnable script invoker
	//! \return ScriptInvoker
	sealed ScriptInvoker GetOnToggleEnable()
	{
		return Event_OnToggleEnable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get OnToggleSelected script invoker
	//! \return ScriptInvoker
	sealed ScriptInvoker GetOnToggleButtonSelected()
	{
		return Event_OnToggleButtonSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get On ToggleEnable script invoker
	//! \return ScriptInvoker
	sealed ScriptInvoker GetOnSetAsSubAttribute()
	{
		return Event_OnSetAsSubAttribute;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get variable of this attribute.
	//! \param createWhenNull True to create the variable when none is defined
	//! \return Variable
	sealed SCR_BaseEditorAttributeVar GetVariable(bool createWhenNull = false)
	{
		if (!m_Var && createWhenNull) 
			m_Var = CreateDefaultVariable();
		
		return m_Var;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Copy variable of this attribute.
	//! \return Copy Variable
	sealed SCR_BaseEditorAttributeVar GetCopyVariable()
	{
		return m_CopyVar;
	}

	//------------------------------------------------------------------------------------------------
	//! Get variable or copy variable of this attribute.
	//! \return Variable or Copy
	sealed SCR_BaseEditorAttributeVar GetVariableOrCopy()
	{
		if (!m_Var)
			return m_CopyVar;
		
		return m_Var;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Copy variable of this attribute.
	//! \return Copy Variable
	sealed void CreateCopyVariable()
	{
		if (m_Var)
			m_CopyVar = SCR_BaseEditorAttributeVar.CreateVector(m_Var.GetVector());
		else 
			m_CopyVar = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if conflicting attribute was reset so UI knows to set default values if it was not active on reset
	//! \return if attribute was reset
	sealed bool GetConflictingAttributeWasReset()
	{
		return m_bConflictingAttributeWasReset;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets conflicting attribute was reset so UI knows to set default values if it was not active on reset
	//! \param bool if attribute was reset
	sealed void SetConflictingAttributeWasReset(bool wasReset)
	{
		m_bConflictingAttributeWasReset = wasReset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get snapshot which holds original attribute's value.
	//! \return Snapshot
	sealed SSnapshot GetSnapshot()
	{
		return m_Snapshot;
	}

	//------------------------------------------------------------------------------------------------
	//! Get event called when attribute var is changed from outside.
	//! \return Script invoker
	sealed ScriptInvoker GetOnExternalnChange()
	{
		return Event_OnExternalChange;
	}

	//------------------------------------------------------------------------------------------------
	//! Initialise editing variables
	sealed void StartEditing(SCR_BaseEditorAttributeVar var, SSnapshot snapshot)
	{
		m_Var = var;
		m_Snapshot = snapshot;
		Event_OnExternalChange = new ScriptInvoker;
	}

	//------------------------------------------------------------------------------------------------
	//! Reset editing variables
	sealed void StopEditing()
	{
		m_Var = null;
		m_Snapshot = null;
		Event_OnExternalChange = null;
		
		//Stop preview
		PreviewVariable(false, null);
	}

	//------------------------------------------------------------------------------------------------
	//! \param isReset
	sealed void TelegraphChange(bool isReset)
	{
		//If has conflicting values on reset
		if (isReset)
		{			
			if (GetHasConflictingValues())
			{
				SetConflictingAttributeWasReset(true);
				SetIsOverridingValues(false);
			}
		}
		
		Event_OnExternalChange.Invoke(m_Var, isReset);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create a default bool (false) variable
	protected SCR_BaseEditorAttributeVar CreateDefaultVariable()
	{
		return SCR_BaseEditorAttributeVar.CreateBool(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Function to fill attribute's variable.
	//! It should connect to API which controls what the attribute represents (e.g., getting fuel from fuel system).
	//! To be overridden by child classes.
	//! \param item Item from which the value is extracted from
	//! \param manager Attribute manager which is currently managing the attribute. Will be null when the attribute is used for session saving!
	//! \return Variable or null to ignore the attribute
	SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager);

	//------------------------------------------------------------------------------------------------
	//! Function to apply attribute's variable.
	//! It should connect to API which controls what the attribute represents (e.g., setting fuel to fuel system).
	//! To be overridden by child classes.
	//! \param item Item on which the value is applied to.
	//! \var Variable
	//! \param manager Attribute manager which is currently managing the attribute. Will be null when the attribute is used for session saving!
	//! \return Variable
	void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID);

	//------------------------------------------------------------------------------------------------
	//! Preview the attribute while editing.
	//! \param manager Attribute manager which is currently managing the attribute. Will be null when the attribute is used for session saving!
	void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager);

	//------------------------------------------------------------------------------------------------
	//! Updates the interlinked variables
	//! \param var
	//! \param manager
	//! \param isInit
	void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false);

	//------------------------------------------------------------------------------------------------
	//! Get entries to be shown in GUI lists.
	//! To be overridden by child classes.
	//! \outEntries Entries
	//! \return Number of entries
	int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param item
	//! \return true if the provided item is the current gamemode
	protected bool IsGameMode(Managed item)
	{
		return item == m_GameMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method for initialization of base editor attribute
	void Initialize() 
	{
		m_GameMode = GetGame().GetGameMode();
	}
}

class SCR_BaseEditorAttributeEntry
{
}

class SCR_BaseEditorAttributeEntryText: SCR_BaseEditorAttributeEntry
{
	private string m_sText;
	
	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BaseEditorAttributeEntryText(string text)
	{
		m_sText = text;
	}
}

class SCR_BaseEditorAttributeEntryUIInfo : SCR_BaseEditorAttributeEntry
{
	private ref SCR_UIInfo m_Info;
	private int m_iValue;
	
	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}

	//------------------------------------------------------------------------------------------------
	int GetValue()
	{
		return m_iValue;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BaseEditorAttributeEntryUIInfo(SCR_UIInfo info, int value = 0)
	{
		m_Info = info;
		m_iValue = value;
	}
}

class SCR_BaseEditorAttributeEntrySlider: SCR_BaseEditorAttributeEntry
{
	protected float m_fMin;
	protected float m_fMax;
	protected float m_fStep;
	protected int m_sDecimals;
	protected string m_sSliderValueFormating;
	protected string m_sSliderLabel;
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] min
	//! \param[out] max
	//! \param[out] step
	void GetSliderMinMaxStep(out float min, out float max, out float step)
	{
		min = m_fMin;
		max = m_fMax;
		step = m_fStep;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSliderLabel(string label)
	{
		m_sSliderLabel = label;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSliderlabel()
	{
		 return m_sSliderLabel;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param value
	//! \return
	string GetText(float value)
	{
		//--- ToDo: Use native fixed length conversion once it's implemented
		float coef = Math.Pow(10, m_sDecimals);
		value = Math.Round(value * coef);
		string valueText = value.ToString();
		if (m_sDecimals > 0)
		{
			for (int i = 0, count = m_sDecimals - valueText.Length() + 1; i < count; i++)
			{
				valueText = "0" + valueText;
			}
			int length = valueText.Length();
			valueText = valueText.Substring(0, length - m_sDecimals) + "." + valueText.Substring(length - m_sDecimals, m_sDecimals);
		}

		return valueText;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the secondary Label value
	// Formatt*ing
	string GetSliderValueFormating()
	{
		return m_sSliderValueFormating;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BaseEditorAttributeEntrySlider(SCR_EditorAttributeBaseValues sliderValues)
	{
		string sliderValueFormating;
		float min, max, step, decimals;
		
		sliderValues.GetSliderValues(sliderValueFormating, min, max, step, decimals);
		
		m_fMin = min;
		m_fMax = max;
		m_fStep = step;
		m_sDecimals = decimals;
		m_sSliderValueFormating = sliderValueFormating;
	}
}

class SCR_BaseEditorAttributeEntryTimeSlider: SCR_BaseEditorAttributeEntry
{
	protected ETimeFormatParam m_eHideIfZero;
	protected bool m_bAlwaysHideSeconds;
	
	//------------------------------------------------------------------------------------------------
	//! Get values to set time slider
	//! \param[out] hideIfZeroWill hide the given time values if fully 0
	//! \param[out] alwaysHideSeconds Will always hide the seconds if true
	void GetTimeSliderValues(out ETimeFormatParam hideIfZero, out bool alwaysHideSeconds)
	{
		hideIfZero = m_eHideIfZero;
		alwaysHideSeconds = m_bAlwaysHideSeconds;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BaseEditorAttributeEntryTimeSlider(ETimeFormatParam hideIfZero, bool alwaysHideSeconds)
	{
		m_eHideIfZero = hideIfZero;
		m_bAlwaysHideSeconds = alwaysHideSeconds;
	}
}

class SCR_BaseEditorAttributeFloatStringValues : SCR_BaseEditorAttributeEntry
{
	protected array<ref SCR_EditorAttributeFloatStringValueHolder> m_aValues;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BaseEditorAttributeFloatStringValues(array<ref SCR_EditorAttributeFloatStringValueHolder> values)
	{
		m_aValues = values;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetValueCount()
	{
		return m_aValues.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param index wanted value's index
	//! \return the requested entry or null if the provided index is invalid
	SCR_EditorAttributeFloatStringValueHolder GetValuesEntry(int index)
	{
		if (!m_aValues.IsIndexValid(index))
			return null;
		
		return m_aValues[index];
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return index entry's float value or 0 if the provided index is invalid
	float GetEntryFloatValue(int index)
	{
		if (!m_aValues.IsIndexValid(index))
			return 0;
		
		return m_aValues[index].GetFloatValue();
	}
}

class SCR_EditorAttributeEntryBool : SCR_BaseEditorAttributeEntry
{
	protected bool m_bBool;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_EditorAttributeEntryBool(bool newBool)
	{
		SetBool(newBool);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBool(bool newBool)
	{
		m_bBool = newBool;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetBool()
	{
		return m_bBool;
	}
}

class SCR_EditorAttributeEntryStringArray : SCR_BaseEditorAttributeEntry
{
	protected array<ref LocalizedString> m_aValues;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_EditorAttributeEntryStringArray(array<ref LocalizedString> values)
	{
		m_aValues = values;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCount()
	{
		return m_aValues.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param index the wanted value's index - will fail if the index is incorrect!
	//! \return the index's value
	string GetEntry(int index)
	{
		return m_aValues[index];
	}
}

class SCR_EditorAttributeEntryIntArray : SCR_BaseEditorAttributeEntry
{
	protected array<int> m_aValues;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_EditorAttributeEntryIntArray(array<int> values)
	{
		m_aValues = values;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCount()
	{
		return m_aValues.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetEntry(int index)
	{
		if (index < 0 || index >= m_aValues.Count())
			return 0;
		
		return m_aValues[index];
	}
}

class SCR_EditorAttributeEntryInt : SCR_BaseEditorAttributeEntry
{
	protected int m_iValue;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_EditorAttributeEntryInt(int value)
	{
		m_iValue = value;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetInt()
	{
		return m_iValue;
	}
}


class SCR_EditorAttributePresetEntry : SCR_BaseEditorAttributeEntry
{
	protected int m_iButtonsOnRow;
	protected bool m_bHasRandomizeButton;
	protected ResourceName m_sIconOfRandomizeButton;
	protected bool m_bHasIcon;
	protected bool m_bHasButtonDescription;
	protected string m_sButtonDescription;
	protected float m_iButtonHeight;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param buttonsOnRow
	//! \param hasRandomizeButton
	//! \param iconOfRandomizeButton
	//! \param hasIcon
	//! \param hasButtonDescription
	//! \param buttonDescription
	//! \param buttonHeight
	void SCR_EditorAttributePresetEntry(int buttonsOnRow, bool hasRandomizeButton, ResourceName iconOfRandomizeButton = string.Empty, bool hasIcon = false, bool hasButtonDescription = false, string buttonDescription = string.Empty, int buttonHeight = -1)
	{
		m_iButtonsOnRow = buttonsOnRow;
		m_bHasRandomizeButton = hasRandomizeButton;
		m_sIconOfRandomizeButton = iconOfRandomizeButton;
		m_bHasIcon = hasIcon;
		m_bHasButtonDescription = hasButtonDescription;
		m_sButtonDescription = buttonDescription;
		m_iButtonHeight = buttonHeight;

	}
	
	//------------------------------------------------------------------------------------------------
	void GetPresetValues(out int buttonsOnRow, out bool hasRandomizeButton, out ResourceName iconOfRandomizeButton, out bool hasIcon, out bool hasButtonDescription, out string buttonDescription, out int buttonHeight)
	{
		buttonsOnRow = m_iButtonsOnRow;
		hasRandomizeButton = m_bHasRandomizeButton;
		iconOfRandomizeButton = m_sIconOfRandomizeButton;
		hasIcon = m_bHasIcon;
		buttonHeight = m_iButtonHeight;
		buttonDescription = m_sButtonDescription;
		hasButtonDescription = m_bHasButtonDescription;
	}
}

class SCR_EditorAttributePresetMultiSelectEntry : SCR_EditorAttributePresetEntry
{
	protected bool m_bCustomFlags;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param buttonsOnRow
	//! \param hasRandomizeButton
	//! \param iconOfRandomizeButton
	//! \param hasIcon
	//! \param hasButtonDescription
	//! \param buttonDescription
	//! \param buttonHeight
	//! \param customFlags
	void SCR_EditorAttributePresetMultiSelectEntry(int buttonsOnRow, bool hasRandomizeButton, ResourceName iconOfRandomizeButton = string.Empty, bool hasIcon = false, bool hasButtonDescription = false, string buttonDescription = string.Empty, int buttonHeight = -1, int customFlags = false)
	{
		SCR_EditorAttributePresetEntry(buttonsOnRow, hasRandomizeButton, iconOfRandomizeButton, hasIcon, hasButtonDescription, buttonDescription, buttonHeight);
		m_bCustomFlags = customFlags;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetUsesCustomFlags()
	{
		return m_bCustomFlags;
	}
}

// A bool that makes sure that has override visuals are
class SCR_EditorAttributeEntryOverride : SCR_BaseEditorAttributeEntry
{
	protected bool m_bOverrideToggled;
	protected typename m_LinkedOverrideAttributeType;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_EditorAttributeEntryOverride(bool overrideToggled, typename linkedOverrideAttributeType)
	{
		m_bOverrideToggled = overrideToggled;
		m_LinkedOverrideAttributeType = linkedOverrideAttributeType;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetToggleStateAndTypename(out bool overrideToggleState, out typename linkedOverrideAttributeType)
	{
		overrideToggleState = m_bOverrideToggled;
		linkedOverrideAttributeType = m_LinkedOverrideAttributeType;
	}
}

class SCR_BaseEditorAttributeDefaultFloatValue : SCR_BaseEditorAttributeEntry
{
	protected int m_fDefaultFloatValue;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BaseEditorAttributeDefaultFloatValue(float floatValue)
	{
		m_fDefaultFloatValue = floatValue;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDefaultFloatValue()
	{
		return m_fDefaultFloatValue;
	}
}

class SCR_BaseEditorAttributeCustomTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_", "");
		title.Replace("EditorAttribute", "");
		
		bool isServer;
		source.Get("m_bIsServer", isServer);
		if (!isServer)
			title += " (Local)";
		
		return true;
	}
}

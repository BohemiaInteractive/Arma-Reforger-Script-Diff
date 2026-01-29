/*!
Classes used for simply configurable widget from and to config files.
This should make more streamlined creation of configurable list.
*/

//-------------------------------------------------------------------------------------------
// Custom titles
//-------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
class SCR_WidgetListEntryCustomTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Group tag string 
		string groupTag = "";
		source.Get("m_sGroupTag", groupTag);
		
		// Name string 
		string name = "";
		source.Get("m_sPropertyName", name);
		
		// Class tag 
		string classTag = "";
		source.Get("m_sClassTag", classTag);
		
		// Setup title string 
		title = "";
		
		if (!groupTag.IsEmpty())
			title += groupTag + ": ";
		
		if (!name.IsEmpty())
			title += name;
		else 
			title += "!!!";
		
		if (!classTag.IsEmpty())
			title += string.Format("(%1)", classTag);
		
		return true;
	}
};

//------------------------------------------------------------------------------------------------
//! Displays label and value name for variable with localization
class SCR_LocalizedPropertyTitle : BaseContainerCustomTitle
{
	protected string m_Label;
	protected string m_PropertyName;
	
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Group tag string 
		string label = "";
		source.Get("m_Label", label);
		
		// Name string 
		string property = "";
		source.Get("m_PropertyName", property);
		
		// Setup title string 
		title = label + ": " + property;
		
		return true;
	}
};

//-------------------------------------------------------------------------------------------
//! Variable that is using localized label
[BaseContainerProps(configRoot: true), SCR_LocalizedPropertyTitle()]
class SCR_LocalizedProperty
{
	[Attribute("")]
	string m_sLabel;
	
	[Attribute("")]
	string m_sPropertyName;
	
	void SCR_LocalizedProperty(string label, string propertyName)
	{
		if (m_sLabel.IsEmpty())
			m_sLabel = label;
		
		if (m_sPropertyName.IsEmpty())
			m_sPropertyName = propertyName;
	}
};

//-------------------------------------------------------------------------------------------
//! Localied property with extra array info
[BaseContainerProps(configRoot: true), SCR_LocalizedPropertyTitle()]
class SCR_LocalizedPropertyExtended : SCR_LocalizedProperty
{
	[Attribute()]
	ref array<string> m_aInfo;
	
	void SCR_LocalizedPropertyExtended(string label, string propertyName, array<string> info)
	{
		m_sLabel = label;
		m_sPropertyName = propertyName;
		m_aInfo = info;
	}
};

//-------------------------------------------------------------------------------------------
// Widget entries
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
//! Configurable class for widget 
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntry
{
	protected const ResourceName ENTRY_DEFINITIONS = "{A6EFD45A3A38965C}Configs/JsonConfigList/JsonConfigListDefinition.conf";
	
	const string INVALID_VALUE = "-invalid-";
	
	[Attribute("{0022F0B45ADBC5AC}UI/layouts/WidgetLibrary/EditBox/WLib_EditBox.layout", UIWidgets.ResourceNamePicker, "What kind of widget should be generated for this value", "layout")]
	protected ResourceName m_sEntryLayout;
	
	[Attribute("1")]
	protected bool m_bShow;
	
	[Attribute("", UIWidgets.EditBox, "Text for label")]
	protected string m_sLabel;
	
	[Attribute("", UIWidgets.EditBox, "Name of property that in json file")]
	protected string m_sPropertyName;
	
	[Attribute("", UIWidgets.EditBox, "Name of property that in json file")]
	protected string m_sGroupTag;
	
	[Attribute("", UIWidgets.EditBox, "Fallback value that will be prefilled at start")]
	protected string m_sDefaultValue;
	
	[Attribute("1")]
	protected bool m_bRequired;
	
	[Attribute("1")]
	protected bool m_bInteractive;

	protected SCR_ChangeableComponentBase m_ChangeableComponent;
	protected EJsonApiStructValueType m_iType;
	protected string m_sClassTag;
	
	protected Widget m_EntryRoot;
	
	protected bool m_bValidInput = true;
	
	//-------------------------------------------------------------------------------------------
	void CreateWidget(Widget parent)
	{
		// Create widget 
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, parent);
		m_EntryRoot = w;
		
		// Setup component
		m_ChangeableComponent = SCR_ChangeableComponentBase.Cast(w.FindHandler(SCR_ChangeableComponentBase));
		if (!m_ChangeableComponent)
			return;
		
		m_ChangeableComponent.SetLabel(m_sLabel);
		m_ChangeableComponent.UseLabel(!m_sLabel.IsEmpty());
		
		SetupHandlers();
		SetInteractive(m_bInteractive);
		
		w.SetVisible(m_bShow);
	}
	
	//-------------------------------------------------------------------------------------------
	void RemoveWidget();
	
	//-------------------------------------------------------------------------------------------
	//! Empty function to override to assing custom handlers
	protected void SetupHandlers();
	
	//-------------------------------------------------------------------------------------------
	//! Return value from widget in string format
	string ValueAsString();
	
	//-------------------------------------------------------------------------------------------
	void SetValue(string str){}
	
	//-------------------------------------------------------------------------------------------
	//! Default validity for majority of inputs will be true
	bool CheckValidity()
	{
		if (CanSkip())
			return true;
		
		m_bValidInput = true;
		
		return m_bValidInput;
	} 
	
	//-------------------------------------------------------------------------------------------
	//! Can skip return true if input is empty and value is not required
	protected bool CanSkip()
	{
		return ValueAsString().IsEmpty() && !m_bRequired;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Can value be interacted - set handler enabled
	void SetInteractive(bool interactive)
	{
		if (!m_ChangeableComponent)
			return;
		
		m_ChangeableComponent.SetEnabled(interactive, false);
	}
	
	//-------------------------------------------------------------------------------------------
	// Get and Set 
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	ResourceName GetEntryLayout()
	{
		return m_sEntryLayout;
	}
	
	//-------------------------------------------------------------------------------------------	
	void SetEntryLayout(ResourceName layout)
	{
		m_sEntryLayout = layout;
	}
	
	//-------------------------------------------------------------------------------------------
	string GetLabel()
	{
		return m_sLabel;
	}
	
	//-------------------------------------------------------------------------------------------
	void SetLabel(string label)
	{
		m_sLabel = label;
	}
	
	//-------------------------------------------------------------------------------------------
	string GetPropertyName()
	{
		return m_sPropertyName;
	}
	
	//-------------------------------------------------------------------------------------------
	void SetPropertyName(string propertyName)
	{
		m_sPropertyName = propertyName;
	}
	
	//-------------------------------------------------------------------------------------------
	string GetGroupTag()
	{
		return m_sGroupTag;
	}
	
	//-------------------------------------------------------------------------------------------
	void SetGroupTag(string tag)
	{
		m_sGroupTag = tag;
	}
	
	//-------------------------------------------------------------------------------------------
	EJsonApiStructValueType GetType()
	{
		return m_iType;
	}
	
	//-------------------------------------------------------------------------------------------
	Widget GetEntryRoot()
	{
		return m_EntryRoot;
	}
	
	//-------------------------------------------------------------------------------------------
	bool IsInputValid()
	{
		return m_bValidInput;
	}
	
	//-------------------------------------------------------------------------------------------
	bool GetVisible()
	{
		return m_bShow;
	}
	
	//-------------------------------------------------------------------------------------------
	void SetVisible(bool show)
	{
		m_bShow = show;
		m_EntryRoot.SetVisible(m_bShow);
	}
	
	//-------------------------------------------------------------------------------------------
	static SCR_ConfigListEntries GetEntriesDefinitions(ResourceName presetsResourceName)
	{
		Resource rsc = BaseContainerTools.LoadContainer(presetsResourceName);
		if (!rsc)
			return null;
		
		BaseContainer container = rsc.GetResource().ToBaseContainer();
		
		return SCR_ConfigListEntries.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
	}
};

//-------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_WidgetListEntryGroup
{
	[Attribute("", UIWidgets.EditBox, "Group tag for fast recognition")]
	protected string m_sTag;
	
	[Attribute()]
	protected ref array<ref SCR_WidgetListEntry> m_aEntries;
	
	//-------------------------------------------------------------------------------------------
	string GetTag()
	{
		return m_sTag;
	}
};


//-------------------------------------------------------------------------------------------
//! Used for entry that just show text or message. This widget is not editable
//! Example: Uploaded GM save has a version which is set automatically, therefore: Version - 1.0.0
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntryMessage : SCR_WidgetListEntry
{ 
	[Attribute("", UIWidgets.EditBox, "Group tag for fast recognition")]
	protected string m_sMessage;
	
	protected SCR_SimpleEntryComponent m_SimpleEntry;
	
	//-------------------------------------------------------------------------------------------
	override void CreateWidget(Widget parent)
	{
		// Create widget 
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, parent);
	
		// Show 
		w.SetVisible(m_bShow);
		if (!m_bShow)
			return;
		
		m_EntryRoot = w;
		
		m_SimpleEntry = SCR_SimpleEntryComponent.Cast(m_EntryRoot.FindHandler(SCR_SimpleEntryComponent));
		m_SimpleEntry.SetMessages(m_sLabel, m_sMessage);
	}
	
	//-------------------------------------------------------------------------------------------
	//! Apply value to message
	override void SetValue(string str)
	{
		m_sMessage = str;
		m_SimpleEntry.SetMessages(m_sLabel, m_sMessage);
	}
	
	//-------------------------------------------------------------------------------------------
	//! Return message
	override string ValueAsString()
	{
		return m_sMessage;
	}
}

//-------------------------------------------------------------------------------------------
//! Configurable widget list entry for list label
[BaseContainerProps(configRoot: true)]
class SCR_WidgetListEntryLabel : SCR_WidgetListEntry
{
	[Attribute("8")]
	protected float m_iOffsetTop;
	
	[Attribute("8")]
	protected float m_iOffsetBottom;
	
	[Attribute("8")]
	protected float m_iLeftBottom;
	
	//-------------------------------------------------------------------------------------------
	override void CreateWidget(Widget parent)
	{
		
		TextWidget wText = TextWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, parent));
		if (!wText)
			return;
		
		wText.SetText(m_sLabel);
		VerticalLayoutSlot.SetPadding(wText, m_iLeftBottom, m_iOffsetTop, 0, m_iOffsetBottom);
		
		m_sClassTag = "label";
		
		m_EntryRoot = wText;
		
		// Show 
		wText.SetVisible(m_bShow);
	}
};

//-------------------------------------------------------------------------------------------
//! Configurable widget list entry for various value that can be typed - numbers, strings, references
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntryEditBox : SCR_WidgetListEntry
{
	[Attribute(EJsonApiStructValueType.TYPE_STRING.ToString(), uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(EJsonApiStructValueType))]
	protected EJsonApiStructValueType m_iEditType;
	
	[Attribute("100")]
	protected int m_CharLimit;
	
	[Attribute("5", UIWidgets.EditBox, "In how many seconds should editbox error should be cleared after focus")]
	protected int m_fClearErrorTimes;
	
	[Attribute()]
	protected ref SCR_WidgetEditFormat m_FormatCheck;
	
	[Attribute("...")]
	protected string m_sPlaceholderText;
	
	[Attribute("0", desc: "When false - there will be used warning text below as default. Otherwise there might be hardcoded strings in format messages.")]
	protected bool m_bUseFormatWarning;
	
	[Attribute("")]
	protected string m_sWarningText;
	
	[Attribute("")]
	protected string m_sObfuscation;
	
	[Attribute("")]
	protected string m_sCharBlackList;
	
	[Attribute("0")]
	protected bool m_bBlacklistIsWhiteList;
	
	[Attribute("0")]
	protected bool m_bShowWriteIcon;
	
	[Attribute("-1", desc: "Height -1 keep editbox single line. Changing height will override current height")]
	protected float m_fCustomHeight;
	
	protected SCR_EditBoxComponent m_EditBox;
	protected EditBoxFilterComponent m_Filter;
	
	// It's a bad idea to store this on the Widget itself!
	// Also, we need to be able to freely change it without triggering the Edit Box's OnChange invoker by calling SetText()
	protected string m_sCurrentValue;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		if (m_ChangeableComponent)
			m_EditBox = SCR_EditBoxComponent.Cast(m_ChangeableComponent);
		
		if (!m_EditBox)
			return;
		
		m_Filter = EditBoxFilterComponent.Cast(m_EditBox.m_wEditBox.FindHandler(EditBoxFilterComponent));
		
		m_iType = m_iEditType;
		m_sClassTag = "edit";
		
		// Editbox setup
		SetValue(m_sDefaultValue);
		m_EditBox.m_OnTextChange.Insert(OnEditBoxTextChange);
		
		m_EditBox.SetPlaceholderText(m_sPlaceholderText);
		m_EditBox.ShowWriteIcon(m_bShowWriteIcon);
		
		// Label and size setup
		//m_EditBox.SetLabel(m_sLabel);
		bool useLabel = !m_sLabel.IsEmpty();
		
		if (m_fCustomHeight > 0)
		{
			if (useLabel)
				m_EditBox.SetSizeWithLabel(m_fCustomHeight);
			else
				m_EditBox.SetSizeWithoutLabel(m_fCustomHeight);
		}
		
		//m_EditBox.UseLabel(useLabel);
		
		// Hint setup
		if (m_EditBox.GetHint())
			m_EditBox.GetHint().SetMessage(m_sWarningText);
		
		EditBoxWidget editBox = EditBoxWidget.Cast(m_EditBox.GetEditBoxWidget());
		if (editBox)
			editBox.SetObfuscationChar(m_sObfuscation);
		
		if (m_FormatCheck)
		{
			m_EditBox.m_OnWriteModeLeave.Insert(OnEditModeLeave);
			
			if (m_Filter)
			{
				m_FormatCheck.SetEditBoxFilter(m_Filter);
				
				// Call character list settings next frame to ensure list is properly override default values
				GetGame().GetCallqueue().CallLater(SetCharLists);
			}
		}
		
		// Number specific setup 
		if (m_iType == EJsonApiStructValueType.TYPE_INT || m_iType == EJsonApiStructValueType.TYPE_FLOAT)
		{
			if (m_Filter)
			{
				m_Filter.SetNumbers(true);
				m_Filter.SetPunctuation(m_iType == EJsonApiStructValueType.TYPE_FLOAT);
				m_Filter.SetASCIIchars(false);
				m_Filter.SetUTFMultibyte(false);
			}
		}
		
		// Limit
		if (m_Filter)
		{
			m_Filter.SetCharacterLimit(m_CharLimit);
			m_Filter.m_OnInvalidInput.Insert(OnInvalidInput);
			m_Filter.m_OnTextTooLong.Insert(OnInvalidInput);
			
			if (m_bBlacklistIsWhiteList)
			{
				m_Filter.SetNumbers(false);
				m_Filter.SetPunctuation(false);
				m_Filter.SetASCIIchars(false);
				m_Filter.SetUTFMultibyte(false);
				m_Filter.SetCharWhiteList(m_sCharBlackList);
			}
			else
				m_Filter.SetCharBlacklist(m_sCharBlackList);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	override void RemoveWidget()
	{
		if (m_EditBox)
			m_EditBox.m_OnWriteModeLeave.Remove(OnEditModeLeave);
	}
	
	//-------------------------------------------------------------------------------------------
	protected void SetCharLists()
	{
		if (m_bBlacklistIsWhiteList)
			m_Filter.SetCharWhiteList(m_sCharBlackList);
		else
			m_Filter.SetCharBlacklist(m_sCharBlackList);
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnEditBoxTextChange(string text)
	{
		m_sCurrentValue = text;
	}
	
	//-------------------------------------------------------------------------------------------
	override string ValueAsString()
	{
		if (!m_EditBox)
			return INVALID_VALUE;
		
		return m_sCurrentValue;
	}
	
	//-------------------------------------------------------------------------------------------
	override void SetValue(string str)
	{
		m_sCurrentValue = str;
		UpdateEditBoxText();
	}
	
	//-------------------------------------------------------------------------------------------
	protected void UpdateEditBoxText()
	{
		if (m_EditBox)
			m_EditBox.SetValue(m_sCurrentValue);
	}
	
	//-------------------------------------------------------------------------------------------
	override bool CheckValidity()
	{
		if (CanSkip())
			return true;
		
		CheckEditBoxValidity(m_EditBox, m_sCurrentValue);
		return m_bValidInput;
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnEditModeLeave(string text)
	{
		CheckValidity();
	}
	
	//-------------------------------------------------------------------------------------------
	//! Check if input is right, otherwise show warning 
	void CheckEditBoxValidity(SCR_EditBoxComponent editBox, string value)
	{
		if (!m_FormatCheck)
		{
			m_bValidInput = true;
			return;
		}
		
		m_bValidInput = m_FormatCheck.IsFormatValid(value);
		
		// Invalid
		if (m_Filter && !m_bValidInput)
		{	
			// Call in next frame to make sure this invoke happens after editbox change
			// Change clears invalid input
			GetGame().GetCallqueue().CallLater(InvokeInvalidInput);
		}
		
		// Setup error 
		if (m_EditBox && m_bUseFormatWarning)
		{
			SCR_WidgetHintComponent hint = m_EditBox.GetHint();
			
			if (hint)
				hint.SetMessage(m_FormatCheck.GetFormatMessage());
		}
	}
	
	//-------------------------------------------------------------------------------------------
	protected void InvokeInvalidInput()
	{
		m_Filter.m_OnInvalidInput.Invoke();
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnInvalidInput()
	{
		// Call later to clear out invalid input 
		if (m_bValidInput)
			GetGame().GetCallqueue().CallLater(ClearInvalidInput, m_fClearErrorTimes * 1000);
	}
	
	//-------------------------------------------------------------------------------------------
	void ClearInvalidInput()
	{
		if (m_EditBox)
			m_EditBox.ClearInvalidInput();
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_EditBoxComponent GetEditBoxComponent()
	{
		return m_EditBox;
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_WidgetEditFormat GetFormatCheck()
	{
		return m_FormatCheck;
	}
	
	//-------------------------------------------------------------------------------------------
	void SetWarningText(string warning)
	{
		if (!m_EditBox)
			return;
		
		if (m_EditBox.GetHint())
			m_EditBox.GetHint().SetMessage(warning);
	}
};

//-------------------------------------------------------------------------------------------
//! IP specific entry
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntryEditBoxIP : SCR_WidgetListEntryEditBox
{
	protected SCR_WidgetEditFormatIP m_IPCheck;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		super.SetupHandlers();
		m_IPCheck = SCR_WidgetEditFormatIP.Cast(m_FormatCheck);
	}
	
	//-------------------------------------------------------------------------------------------
	override bool CheckValidity()
	{
		super.CheckValidity();
		
		// Private IP 
		if (m_IPCheck)
		{
			if (m_IPCheck.IsPrivate())
				m_EditBox.ShowHint(true);
		}
		
		return m_bValidInput;
	}
}

//-------------------------------------------------------------------------------------------
//! Configurable widget list entry for boolean values - yes/no
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntryCheckBox : SCR_WidgetListEntry
{
	[Attribute("")]
	protected string m_sPositive;
	
	[Attribute("")]
	protected string m_sNegative;
	
	protected SCR_CheckboxComponent m_Checkbox;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		if (m_ChangeableComponent)
			m_Checkbox = SCR_CheckboxComponent.Cast(m_ChangeableComponent);
		
		m_iType = EJsonApiStructValueType.TYPE_BOOL;
		m_sClassTag = "check box";
		
		// Checkbox value setup
		if (!m_sNegative.IsEmpty() && m_Checkbox.m_aSelectionElements[0])
		{
			SCR_ButtonTextComponent textComp = SCR_ButtonTextComponent.Cast(m_Checkbox.m_aSelectionElements[0]);
			if (textComp)
				textComp.SetText(m_sNegative);
		}
		
		if (!m_sPositive.IsEmpty() && m_Checkbox.m_aSelectionElements[1])
		{
			SCR_ButtonTextComponent textComp = SCR_ButtonTextComponent.Cast(m_Checkbox.m_aSelectionElements[1]);
			if (textComp)
				textComp.SetText(m_sPositive);
		}
		
		m_Checkbox.SetChecked(SCR_JsonApiStructHandler.StringToBool(m_sDefaultValue), false, false);
	}
	
	//-------------------------------------------------------------------------------------------
	override void SetValue(string str)
	{
		if (!m_Checkbox)
			return;
		
		bool checked = SCR_JsonApiStructHandler.StringToBool(str);
		m_Checkbox.SetChecked(checked);
	}
	
	//-------------------------------------------------------------------------------------------
	override string ValueAsString()
	{
		if (!m_Checkbox)
			return INVALID_VALUE;
		
		return m_Checkbox.IsChecked().ToString();
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_CheckboxComponent GetCheckbox()
	{
		return m_Checkbox;
	}
};

//-------------------------------------------------------------------------------------------
//! Configurable widget list entry list selection
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntrySelection : SCR_WidgetListEntry
{
	[Attribute()]
	protected ref array<ref SCR_LocalizedProperty> m_aOptions;
	
	protected SCR_SelectionWidgetComponent m_Selection;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		if (m_ChangeableComponent)
			m_Selection = SCR_SelectionWidgetComponent.Cast(m_ChangeableComponent);
		
		if (!m_Selection)
			return;
		
		// Fill values
		for (int i = 0, count = m_aOptions.Count(); i < count; i++)
		{
			m_Selection.AddItem(m_aOptions[i].m_sLabel);
		}
		
		// Select default value 
		if (!m_sDefaultValue.IsEmpty())
			SelectOption(m_sDefaultValue.ToInt());
	}
	
	//-------------------------------------------------------------------------------------------
	override string ValueAsString()
	{
		if (!m_Selection)
			return INVALID_VALUE;
		
		int id = m_Selection.m_iSelectedItem;
		
		#ifdef SB_DEBUG
		Print("option Id: " + id + "/" + m_aOptions.Count());
		#endif
		
		if (id == -1 || id >= m_aOptions.Count())
			return INVALID_VALUE;
		
		if (m_aOptions[id] == null)
			return INVALID_VALUE;
		
		return m_aOptions[id].m_sPropertyName;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Get selected value as id
	int ValueId()
	{
		if (!m_Selection)
			return INVALID_VALUE;
		
		return m_Selection.m_iSelectedItem;
	}
	
	//-------------------------------------------------------------------------------------------
	// API
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------\
	void SelectOption(int id)
	{
		if (m_Selection)
			m_Selection.SetCurrentItem(id);
	}
	
	//-------------------------------------------------------------------------------------------
	//! Return current selected option as full localized property
	SCR_LocalizedProperty GetSelectedOption()
	{
		int id = -1;
		
		if (m_Selection)
			id = m_Selection.GetCurrentIndex();
		
		if (id == -1)
			return null;
		
		return m_aOptions[id];
	}
	
	//-------------------------------------------------------------------------------------------
	//! Fill list with options 
	void SetOptions(array<ref SCR_LocalizedProperty> options)
	{
		m_aOptions = options;
		
		// Update values 
		if (!m_Selection)
			return;
		
		m_Selection.ClearAll();

		for (int i = 0, count = m_aOptions.Count(); i < count; i++)
		{
			m_Selection.AddItem(m_aOptions[i].m_sLabel);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_SelectionWidgetComponent GetSelectionComponent()
	{
		return m_Selection;
	}
	
	//-------------------------------------------------------------------------------------------
	void GetOptions(out array<ref SCR_LocalizedProperty> options)
	{
		for (int i = 0, count = m_aOptions.Count(); i < count; i++)
		{
			options.Insert(m_aOptions[i]);
		}
	}
};

//-------------------------------------------------------------------------------------------
//! Configurable widget list entry selection specific for spinbox 
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntrySpinBox : SCR_WidgetListEntrySelection
{
	[Attribute("1")]
	protected bool m_bCycle;
	
	protected SCR_SpinBoxComponent m_SpinBox;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		super.SetupHandlers();
		m_SpinBox = SCR_SpinBoxComponent.Cast(m_Selection);
		
		if (!m_SpinBox)
			return;
		
		m_SpinBox.SetCycleMode(m_bCycle);
	}
	
	//-------------------------------------------------------------------------------------------
	override void SetValue(string str)
	{
		if (m_SpinBox)
			m_SpinBox.SetCurrentItem(str.ToInt());
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_SpinBoxComponent GetSpinBox()
	{
		return m_SpinBox;
	}
}

//-------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntryBoolSpinBox : SCR_WidgetListEntrySpinBox
{
	[Attribute("0", desc: "Revert item reading order*")]
	protected bool m_bReversed;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		if (m_ChangeableComponent)
			m_Selection = SCR_SelectionWidgetComponent.Cast(m_ChangeableComponent);
		
		if (!m_Selection)
			return;
		
		// Fill values
		if (m_bReversed)
		{
			for (int i = m_aOptions.Count() - 1; i >= 0; i--)
			{
				m_Selection.AddItem(m_aOptions[i].m_sLabel);
			}
		}
		else 
		{
			for (int i = 0, count = m_aOptions.Count(); i < count; i++)
			{
				m_Selection.AddItem(m_aOptions[i].m_sLabel);
			}
		}
		
		// Select default value 
		if (!m_sDefaultValue.IsEmpty())
			SelectOption(m_sDefaultValue.ToInt());
		
		// Spinbox 
		m_SpinBox = SCR_SpinBoxComponent.Cast(m_Selection);
		
		if (!m_SpinBox)
			return;
		
		m_SpinBox.SetCycleMode(m_bCycle);
	}
	
	//-------------------------------------------------------------------------------------------
	override void SetValue(string str)
	{
		int value = str.ToInt();
		if (m_bReversed)
		{
			if (value == 0)
				value = 1;
			else if (value == 1)
				value = 0;
		}
		
		if (m_SpinBox)
			m_SpinBox.SetCurrentItem(value);
	}
}

//-------------------------------------------------------------------------------------------
//! Configurable widget list entry for slider with ranged values
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntrySlider : SCR_WidgetListEntry
{
	[Attribute()]
	protected float m_fMin;
	
	[Attribute()]
	protected float m_fMax;
	
	[Attribute("0.05")]
	protected float m_fStep;

	[Attribute("%1")]
	protected string m_sFormatText;
	
	protected SCR_SliderComponent m_Slider;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		if (m_ChangeableComponent)
			m_Slider = SCR_SliderComponent.Cast(m_ChangeableComponent);
		
		m_Slider.SetMin(m_fMin);
		m_Slider.SetMax(m_fMax);
		m_Slider.SetStep(m_fStep);
		m_Slider.SetFormatText(m_sFormatText);
		
		m_Slider.SetValue(m_sDefaultValue.ToFloat());
	}
	
	//-------------------------------------------------------------------------------------------
	override void SetValue(string str)
	{
		if (!m_Slider)
			return;
		
		m_Slider.SetValue(str.ToFloat());
	}
		
	//-------------------------------------------------------------------------------------------
	override string ValueAsString()
	{
		if (!m_Slider)
			return INVALID_VALUE;
		
		return m_Slider.GetValue().ToString();
	}
	
	//-------------------------------------------------------------------------------------------
	void SetRange(float min, float max)
	{
		m_Slider.SetMin(min);
		m_Slider.SetMax(max);
	}
};
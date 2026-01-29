/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_ButtonBoxAttributeUIComponent: SCR_BaseEditorAttributeUIComponent
{
	[Attribute("0", UIWidgets.ComboBox, "Attribute Toolbox type", "", ParamEnumArray.FromEnum(EAttributeToolBoxType))]
	protected EAttributeToolBoxType m_ToolboxType;
	
	[Attribute("#AR-Editor_Attribute_Randomize_Name")]
	protected LocalizedString m_sRandomizeButtonLabel;
	
	[Attribute("{0C6A9655C33FB75D}UI/layouts/Editor/Attributes/Base/ButtonImageUnderlined.layout")]
	protected ResourceName m_sButtonWithIcon;
	
	[Attribute("{B98EA1D6487A6F45}UI/layouts/Editor/Attributes/Base/AttributeEmptyButton.layout")]
	protected ResourceName m_sEmptyButton;
	
	[Attribute("0", desc: "At which int the randomizer starts. This is when you want the first few buttons to be ignored by the randomize button")]
	protected int m_iRandomizerButtonStart;
	
	[Attribute("true", desc: "Shows border when hovering over button.")]
	protected bool m_bShowBorderOnHover;

	protected SCR_ToolboxComponent m_ToolBoxComponent;

	protected ref SCR_BaseEditorAttributeFloatStringValues m_ButtonBoxData;
	
	protected bool m_bButtonValueInitCalled;
	
	//Button specific
	protected bool m_bHasRandomizeButton;
	
	//~ If at least one selection button is selected (Only works with EAttributeToolBoxType.SELECTABLE_BUTTON)
	protected bool m_bSelectionButtonIsSelected = true;
	
	
	//---- REFACTOR NOTE START: Init is bloated 
	/*
	Searching for widgets with names hardcoded in method 
	Working with various button types (m_ToolboxType) could be simpler
	Use of DelayedInit() to setup empty buttons  
	*/
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		Widget toolBox = w.FindAnyWidget(m_sUiComponentName);
		if (!toolBox) return;
		
		m_ToolBoxComponent = SCR_ToolboxComponent.Cast(toolBox.FindHandler(SCR_ToolboxComponent));
		if (!m_ToolBoxComponent) return;
		
		if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
			m_ToolBoxComponent.m_bAllowMultiselection = false;
		else
			m_ToolBoxComponent.m_bAllowMultiselection = true;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var) return;

		array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
		attribute.GetEntries(entries);
		
		bool hasIcon, hasButtonDescription;
		ResourceName iconOfRandomizeButton;
		int buttonsOnRow;
		
		SCR_ToolboxMultilineComponent multiLine = SCR_ToolboxMultilineComponent.Cast(m_ToolBoxComponent);
		
		foreach (SCR_BaseEditorAttributeEntry entry: entries)
		{	
			SCR_EditorAttributePresetEntry presetEntry = SCR_EditorAttributePresetEntry.Cast(entry);
			if (presetEntry)
			{
				float buttonHeight;
				string buttonDescription;
				presetEntry.GetPresetValues(buttonsOnRow, m_bHasRandomizeButton, iconOfRandomizeButton, hasIcon, hasButtonDescription, buttonDescription, buttonHeight);
				
				if (hasButtonDescription)
				{
					SCR_EditorAttributeUIInfo newButtonDescription = new SCR_EditorAttributeUIInfo();
					newButtonDescription.CopyFromEditorAttributeUIInfo(m_ButtonDescriptionUIInfo, buttonDescription);
					m_ButtonDescriptionUIInfo = newButtonDescription;
				}
				
				if (multiLine)
				{
					multiLine.SetMaxItemsInRow(buttonsOnRow);
					
					if (buttonHeight != -1)
						multiLine.m_fElementHeight = buttonHeight;
				}
				
				if (hasIcon)
					m_ToolBoxComponent.m_ElementLayout = m_sButtonWithIcon;
				
				continue;
			}
			
			SCR_BaseEditorAttributeFloatStringValues data = SCR_BaseEditorAttributeFloatStringValues.Cast(entry);
			if (data)
			{
				m_ButtonBoxData = data;
				int count = m_ButtonBoxData.GetValueCount();
			
				for (int i = 0; i < count; i++)
				{
					m_ToolBoxComponent.AddItem(m_ButtonBoxData.GetValuesEntry(i).GetName());	
				}
				
				Widget buttonRow = w.FindAnyWidget("ButtonRow");
				
				//Fill with empty buttons
				if (buttonRow && !m_sEmptyButton.IsEmpty())
				{
					while (count < buttonsOnRow)
					{
						Widget emptyButton = GetGame().GetWorkspace().CreateWidgets(m_sEmptyButton, buttonRow);
						UniformGridSlot.SetRow(emptyButton, 0);
						UniformGridSlot.SetColumn(emptyButton, count);
						count++;
					}
				}
				continue;
			}
		}
		
		//Buttons with Icon or description delayed init
		if (hasIcon || hasButtonDescription)
			GetGame().GetCallqueue().CallLater(DelayedInit, 1, false, hasIcon, hasButtonDescription);		
		
		if (m_ToolboxType == EAttributeToolBoxType.SINGLE_BUTTON)
			m_ToolBoxComponent.m_OnChanged.Insert(OnSingleButton);
		else if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
			m_ToolBoxComponent.m_OnChanged.Insert(OnSelectableButton);
		else if (m_ToolboxType == EAttributeToolBoxType.MULTI_SELECTABLE_BUTTON)
			m_ToolBoxComponent.m_OnChanged.Insert(OnMultiSelectButton);
		
		//Set button selected
		if (m_ToolboxType == EAttributeToolBoxType.SINGLE_BUTTON)
		{
			int value = var.GetInt();
			if (value >= 0 && value < m_ButtonBoxData.GetValueCount())
				m_ToolBoxComponent.SetItemSelected(value, false);
		}
		
		//Add random button
		if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON && m_bHasRandomizeButton)
		{
			m_ToolBoxComponent.AddItem(m_sRandomizeButtonLabel);
			
			if (hasIcon && !iconOfRandomizeButton.IsEmpty())
			{
				array<SCR_ButtonBaseComponent> elements = new array<SCR_ButtonBaseComponent>;
				int count = m_ToolBoxComponent.GetItems(elements);
				
				SCR_ButtonBaseComponent randomizeButton = elements[count -1];
				
				if (randomizeButton)
				{
					ImageWidget imageWidget = ImageWidget.Cast(randomizeButton.GetRootWidget().FindAnyWidget("Image"));
					
					if (imageWidget)
						imageWidget.LoadImageTexture(0, iconOfRandomizeButton);
					
					//Note randomize description is currently not supported as randomize is not used.
					/*
					SCR_AttributeButtonUIComponent button = SCR_AttributeButtonUIComponent.Cast(randomizeButton.GetRootWidget().FindHandler(SCR_AttributeButtonUIComponent));
					if (!button)
					{
						button = new SCR_AttributeButtonUIComponent();
						randomizeButton.GetRootWidget().AddHandler(button);
					}
					
					button.ButtonDescriptionInit(this, randomizeButton, randomizedescription);*/
				}
			}	
		}
		
		if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
		{
			int value = var.GetInt();
			m_ToolBoxComponent.SetFocusedItem(value);
		}
		
		super.Init(w, attribute);
	}	
	
	//Set icon and set button hover description
	protected void DelayedInit(bool hasIcon, bool hasDescription)
	{
		array<SCR_ButtonBaseComponent> elements = new array<SCR_ButtonBaseComponent>;
		int count = m_ToolBoxComponent.GetItems(elements);
		
		if (m_bHasRandomizeButton)
			count -1;
		
		for (int i = 0; i < count; i++)
		{
			SCR_EditorAttributeFloatStringValueHolder value = m_ButtonBoxData.GetValuesEntry(i);
			if(!value)
				continue;
			
			elements[i].m_bShowBorderOnHover = m_bShowBorderOnHover;
			
			if (hasIcon)
			{
				ResourceName icon = value.GetIcon();						
				ImageWidget imageWidget = ImageWidget.Cast(elements[i].GetRootWidget().FindAnyWidget("Image"));
				
				if (imageWidget)
				{
					if (!icon.IsEmpty())
					{
						if (!value.IsImageset())
							imageWidget.LoadImageTexture(0, icon);
						else
							imageWidget.LoadImageFromSet(0, icon, value.GetIconName());
					}
					else 
					{
						imageWidget.SetVisible(false);
						Widget textWidget = elements[i].GetRootWidget().FindAnyWidget("Text");
						if (textWidget)
							textWidget.SetVisible(true);
					}
				}
			}
			
			if (hasDescription)
			{
				SCR_AttributeButtonUIComponent button = SCR_AttributeButtonUIComponent.Cast(elements[i].GetRootWidget().FindHandler(SCR_AttributeButtonUIComponent));
				if (!button)
				{
					button = new SCR_AttributeButtonUIComponent();
					elements[i].GetRootWidget().AddHandler(button);
				}
				
				button.ButtonDescriptionInit(this, elements[i], value.GetDescription(), value.GetName());
			}
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{		
		if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
			ToggleButtonSelected(true, 0);
		else if (m_ToolboxType == EAttributeToolBoxType.MULTI_SELECTABLE_BUTTON)
			ToggleButtonSelected(false, -1);
		
		if (var && m_ToolboxType != EAttributeToolBoxType.SINGLE_BUTTON)
		{
			if (var)
				var.SetInt(0);		
		}	
		
		SetFromVarOrDefault();
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{							
		if (!var)
			return;
		
		if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
		{
			if (!m_bButtonValueInitCalled)
			{
				m_ToolBoxComponent.SetItemSelected(var.GetInt(), true, false);
				m_bButtonValueInitCalled = true;
			}
			else 
			{
				ToggleButtonSelected(true, var.GetInt());
				
				if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
					m_bSelectionButtonIsSelected = true;
			}
		}
		
		SetFromVarOrDefault();
	}
		
	/*!
	Get toolbox component of the attribute UI
	\return Toolbox component
	*/
	SCR_ToolboxComponent GetToolboxComponent()
	{
		return m_ToolBoxComponent;
	}		
	
	/*!
	Get toolbox component of the attribute UI
	\return Toolbox component
	*/
	SCR_BaseEditorAttributeFloatStringValues GetButtonBoxData()
	{
		return m_ButtonBoxData;
	}
	
	//-1 means all are set to selected value
	override void ToggleButtonSelected(bool selected, int index, bool animated = true)
	{
		if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON && index == -1)
		{
			if (m_bSelectionButtonIsSelected == selected)
				return;
			else 
				m_bSelectionButtonIsSelected = selected;
		}
		
		int count =  m_ToolBoxComponent.m_aSelectionElements.Count();

		if (index > count)
			return;
		
		//Set all selected/deselected
		if (index < 0)
		{
			for(int i = 0; i < count; ++i)
			{
				m_ToolBoxComponent.SetItemSelected(i, selected);
				m_ToolBoxComponent.GetItem(i).ShowBorder(selected, animated);
			}
			
		}
		//Deselect current and select new
		else if (selected && m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
		{
			for(int i = 0; i < count; ++i)
			{
				if (m_ToolBoxComponent.m_aSelectionElements[i].IsToggled())
				{
					if (i != index)
					{
						m_ToolBoxComponent.SetItemSelected(i, false, animated);
						m_ToolBoxComponent.SetItemSelected(index, true, animated);
						m_ToolBoxComponent.GetItem(i).ShowBorder(false, animated);
						m_ToolBoxComponent.GetItem(index).ShowBorder(true, animated);
					}
					return;
				}
			}		
		}
		//Set item selected/deslected
		else {
			m_ToolBoxComponent.SetItemSelected(index, selected);
			m_ToolBoxComponent.GetItem(index).ShowBorder(selected, animated);
		}
	}
	
	protected void OnSingleButton(SCR_ToolboxComponent toolbox, int index, bool state)
	{
		if (!state)
			return;
		
		OnChangeInternal(null, index, state, false);
		m_ToolBoxComponent.SetItemSelected(index, false);
	}
	
	protected void OnSelectableButton(SCR_ToolboxComponent toolbox, int index)
	{	
		m_bSelectionButtonIsSelected = true;
		OnChangeInternal(null, index, 0, false);
	}

	protected void OnMultiSelectButton(SCR_ToolboxComponent toolbox, int index, bool state)
	{				
		//This logic is done via flags and needs custom script
	}
	
	protected void UpdateButtonBorder(int selectedIndex)
	{
		array<SCR_ButtonBaseComponent> elements = new array<SCR_ButtonBaseComponent>;
		
		int count = m_ToolBoxComponent.GetItems(elements);
		
		for(int i = 0; i < count; i++)
        {
			//~ Show Border
			if (i == selectedIndex)
				elements[i].ShowBorder(true);
			//~ Hide border
			else
				elements[i].ShowBorder(false);
        }
	}
	
	//---- REFACTOR NOTE START: Hardcoded overflow value
	// Call later for button randomizer
	
	//protected ref SCR_BaseEditorAttributeEntryToolbox m_ToolboxData;
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{			
		if (m_bHasRandomizeButton && x == m_ButtonBoxData.GetValueCount())
		{
			int currentIndex = m_ToolBoxComponent.GetCurrentIndex();
			int count =  m_ToolBoxComponent.m_aSelectionElements.Count();
			
			for(int i = 0; i < count; ++i)
			{
				m_ToolBoxComponent.SetItemSelected(i, false);
			}
			GetGame().GetCallqueue().CallLater(DelayedButtonRandomizer, 100, false, currentIndex);
			
			return false;
		}
		
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute) return false;
		SCR_BaseEditorAttributeVar var = attribute.GetVariable(true);
	
		var.SetInt(x);
		super.OnChangeInternal(w, x, y, finished);
		
		GetGame().GetCallqueue().CallLater(UpdateButtonBorder, 1, false, x);
		
		return false;
	}
	
	protected void DelayedButtonRandomizer(int currentIndex)
	{
		int index = currentIndex;
		int overflow = 10;
		while (index == currentIndex && overflow > 0)
		{
			index = Math.RandomInt(m_iRandomizerButtonStart, m_ButtonBoxData.GetValueCount());
			overflow--;
		}
		
		m_ToolBoxComponent.SetCurrentItem(index, false, true);
		UpdateButtonBorder(index);
	}
	
	//---- REFACTOR NOTE END ----
	
	/*
	//Set focus of current selected element
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_ToolboxType != EAttributeToolBoxType.SELECTABLE_BUTTON || !m_ToolBoxComponent)
			return super.OnFocus(w, x, y);
		
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute)
			return super.OnFocus(w, x, y);
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var) 
			return super.OnFocus(w, x, y);
		
		m_ToolBoxComponent.SetFocusedItem(var.GetInt(), false);
		return super.OnFocus(w, x, y);
	}
	*/
	
	override void HandlerDeattached(Widget w)
	{
		if (m_ToolBoxComponent)
		{
			if (m_ToolboxType == EAttributeToolBoxType.SINGLE_BUTTON)
				m_ToolBoxComponent.m_OnChanged.Remove(OnSingleButton);
			else if (m_ToolboxType == EAttributeToolBoxType.SELECTABLE_BUTTON)
				m_ToolBoxComponent.m_OnChanged.Remove(OnSelectableButton);
			else if (m_ToolboxType == EAttributeToolBoxType.MULTI_SELECTABLE_BUTTON)
				m_ToolBoxComponent.m_OnChanged.Remove(OnMultiSelectButton);
		}
	}
};

enum EAttributeToolBoxType
{
	SINGLE_BUTTON,
	SELECTABLE_BUTTON,
	MULTI_SELECTABLE_BUTTON
};
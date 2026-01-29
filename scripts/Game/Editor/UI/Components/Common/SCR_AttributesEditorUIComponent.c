/** @ingroup Editor_UI Editor_UI_Components
*/
class SCR_AttributesEditorUIComponent: MenuRootSubComponent
{		
	[Attribute()]
	protected ref SCR_ArsenalItemTypeUIConfig m_ArsenalItemTypeConfig;
	
	[Attribute(defvalue: "4", desc: "If more then this amount tabs are shown then the text will be hidden and only icon will be shown")]
	protected int m_iMaxTabsUntilImageOnly;
	
	[Attribute("200")]
	protected int m_iTabTextAndImageWidth;
	
	[Attribute("64")]
	protected int m_iTabImageOnlyWidth;
	
	[Attribute()]
	protected string m_sTabWidgetName;
	
	[Attribute()]
	protected string m_sContentHolderWidgetName;
	
	[Attribute("NoAttributeNote")]
	protected string m_sNoAttributeNoteName;
	
	[Attribute("ButtonClose")]
	protected string m_sButtonCloseButtonName;
	
	[Attribute("ButtonCloseAndSave")]
	protected string m_sButtonCloseAndSaveButtonName;
	
	[Attribute()]
	protected string m_sButtonResetWidgetName;
	
	[Attribute()]
	protected string m_sOutsideWindowCloseButtonName;
	
	[Attribute()]
	protected string m_sButtonEndGameCloseName;
	
	[Attribute()]
	protected string m_sButtonEndGameAcceptName;

	[Attribute("ButtonLockUnlock")]
	protected string m_sGamepadToggleAttributeButtonName;
	
	[Attribute("CustomTitlePage")]
	protected string m_sTitleHeaderWidgetName;
	
	[Attribute("#AR-AttributesDialog_TitlePage_Photo_Text")]
	protected LocalizedString m_sPhotoModeTitle;
	
	[Attribute("#AR-AttributesDialog_TitlePage_Admin_Text")]
	protected LocalizedString m_sAdminModeTitle;
	
	[Attribute("#AR-AttributesDialog_TitlePage_FactionOnly_Text")]
	protected LocalizedString m_sFactionOnlyTitle;
	
	[Attribute("#AR-Tasks_Objective")]
	protected LocalizedString m_sTaskTitle;
	
	[Attribute("")]
	protected LocalizedString m_sEndGameTitle;
	
	[Attribute("#AR-AttributesDialog_TitlePage_Entity_Text")]
	protected LocalizedString m_sMultiEntitiesTitle;
	
	[Attribute("#AR-AttributesDialog_TitlePage_Editing_Text")]
	protected LocalizedString m_sEditingTitlePart;
	
	[Attribute("PlatformImage")]
	protected string m_sPlatformImageWidgetName;
	
	[Attribute("PlayerName")]
	protected string m_sPlayerNameWidgetName;
	
	[Attribute("2", "If more types then given type, m_sMultiEntitiesTitle will be used as text")]
	protected int m_iEntityTypesUntilMulti;
	
	[Attribute(desc: "A List of translated names for entity types which is also used to display the order of entity types (Eg if multiple which one to show first)")]
	protected ref array<ref SCR_EntityAttributeTitleType> m_aEntityTypeNames;
	
	[Attribute("{E2CBA6F76AAA42AF}UI/Fonts/Roboto/Roboto_Regular.fnt")]
	protected string m_sFontForHeaderEntityAmount;
	
	//End game
	[Attribute()]
	protected LocalizedString m_sEndGamePopUpTitle;
	
	[Attribute()]
	protected LocalizedString m_sEndGamePopUpMessage;
	
	[Attribute()]
	protected LocalizedString m_sEndGamePopUpConfirm;
	
	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSfxClickedOutsideWindow_Comfirm;
	
	[Attribute(SCR_SoundEvent.CLICK_CANCEL, UIWidgets.EditBox, "")]
	protected string m_sSfxClickedOutsideWindow_Cancel;
	
	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSfxOnOpenDialog;
	
	//~ Set to comfirm or cancel depending if a setting is changed
	protected string m_sSfxOnCloseDialog;
	
	protected bool m_bIsEndGame;
	
	//Tab saving
	protected bool m_bIsGlobalAttribute;
	protected EEditorMode m_ActiveEditorMode;
	
	//Widget Refs
	protected SCR_TabViewComponent m_TabViewComponent;
	protected Widget m_ContentHolder;
	protected Widget m_ButtonCloseAndSaveButton;
	protected Widget m_ButtonCloseButton;
	protected SCR_InputButtonComponent m_ResetButton;
	protected SCR_InputButtonComponent m_GamepadToggleEnableButton;
	protected SCR_BaseEditorAttributeUIComponent m_GamepadFocusAttribute;
	protected ImageWidget m_PlatformImageWidget;
	protected TextWidget m_PlayerNameWidget;
	
	//Ref
	protected SCR_AttributesManagerEditorComponent m_AttributesManager;
	
	protected int m_iCurrentTab = -1;
	protected ref array<ResourceName> m_CategoryConfigs;
	protected ResourceName m_currentCategory;
	protected bool m_bTabImageOnly;
	
	//State
	protected bool m_bHasConflictingAttributes;
	
	protected void ButtonClose()
	{		
		EditorAttributesDialogUI menu = EditorAttributesDialogUI.Cast(GetMenu());
		if (menu)
		{
			menu.RemoveAutoClose();
			menu.CloseSelf();
		}
		
		if (m_AttributesManager) 
			m_AttributesManager.ConfirmEditing();
	}
	
	protected void OnCancelEndGame()
	{		
		EditorAttributesDialogUI menu = EditorAttributesDialogUI.Cast(GetMenu());
		if (menu)
		{
			menu.RemoveAutoClose();
			menu.CloseSelf();
		}
		
		if (m_AttributesManager) 
			m_AttributesManager.CancelEditing();
	}
	
	protected void EndGamePopupComfirm()
	{
		if (m_AttributesManager) 
			m_AttributesManager.ConfirmEditing();
	}
	
	//---- REFACTOR NOTE START: Hard coded dialog name which can be changed easily in config
	
	protected void OnAcceptEndGame()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("session_end");
		if (!dlg) 
			return;
		
		dlg.m_OnConfirm.Insert(EndGamePopupComfirm);
		
	}
	
	//---- REFACTOR NOTE END ----
	
	protected void ButtonReset()
	{
		if (m_AttributesManager) 
			m_AttributesManager.Reset(true);
	}
	
	protected void OnReset()
	{
		if (m_ResetButton)
			m_ResetButton.SetEnabled(false);
		
		if (m_ButtonCloseAndSaveButton && m_ButtonCloseButton)
		{
			m_ButtonCloseButton.SetVisible(true);
			m_ButtonCloseAndSaveButton.SetVisible(false);
			m_sSfxOnCloseDialog = m_sSfxClickedOutsideWindow_Cancel;
		}		
	}
	
	//To toggle the enable/disable of a conflicted attribute
	protected void ButtonAttributeEnableToggle()
	{
		if (m_GamepadFocusAttribute)
			m_GamepadFocusAttribute.ToggleEnableAttributeTickbox();
	}
	
	
	//! Sets Reset button enabled and close to Save and Close
	void OnAttributeChanged()
	{
		if (m_ResetButton)
			m_ResetButton.SetEnabled(true);
		
		if (m_ButtonCloseAndSaveButton && m_ButtonCloseButton)
		{
			m_ButtonCloseButton.SetVisible(false);
			m_ButtonCloseAndSaveButton.SetVisible(true);
			m_sSfxOnCloseDialog = m_sSfxClickedOutsideWindow_Comfirm;
		}
	}
	
	protected void RefreshAttributes()
	{
		SetHasConflictingAttribute(false);
		
		Widget widget = GetWidget();
		if (!widget) return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) return;
		
		if (!m_ContentHolder) return;
		
		if (!m_AttributesManager) return;
		
		//--- Delete current attributes
		while (m_ContentHolder.GetChildren())
		{
			delete m_ContentHolder.GetChildren();
		}
		
		if (m_currentCategory != string.Empty)
		{		
			//--- Add new attributes
			array<SCR_BaseEditorAttribute> attributes = new array<SCR_BaseEditorAttribute>;
			m_AttributesManager.GetEditedAttributes(attributes);
			
			foreach (SCR_BaseEditorAttribute attribute: attributes)
			{
				if (attribute.GetCategoryConfig() != m_currentCategory) continue;
				
				ResourceName layout = attribute.GetLayout();
				if (layout.IsEmpty()) continue;
				
				Widget attributeWidget = workspace.CreateWidgets(layout, m_ContentHolder); //~Todo: Spawn attribute base and add attribute to AttributeHolder
				if (!attributeWidget) continue;
				
				SCR_BaseEditorAttributeUIComponent attributesUI = SCR_BaseEditorAttributeUIComponent.Cast(attributeWidget.FindHandler(SCR_BaseEditorAttributeUIComponent));
				if (attributesUI)
				{
					attributesUI.Init(attributeWidget, attribute);
					attributesUI.SetFromVar(attribute.GetVariable());
					
					//Subscribe to events
					if (attribute.GetHasConflictingValues())
						attributesUI.GetOnAttributeUIFocusChanged().Insert(SetGamepadAttributeFocus);
					
					attributesUI.GetOnAttributeChanged().Insert(OnAttributeChanged);
				}
				
				//So UI knows that values are conflicted and adds indents to all
				if (!m_bHasConflictingAttributes && attribute.GetHasConflictingValues())
					SetHasConflictingAttribute(true);
			}
		}
	}	
	
	protected void OnCurrentTabChanged(SCR_TabViewComponent tabView, Widget w, int tabIndex)
	{
		//Set text active of selected
		if (m_bTabImageOnly && m_iCurrentTab > -1)
		{
			m_TabViewComponent.ShowTabText(m_iCurrentTab, false, m_iTabImageOnlyWidth);
			m_TabViewComponent.ShowTabText(tabIndex, true, m_iTabTextAndImageWidth);
		}
			
		if (m_CategoryConfigs.Count() == 0) 
			return;
		
		if (m_iCurrentTab != tabIndex)
		{
			m_iCurrentTab = tabIndex;
			
			m_currentCategory = m_CategoryConfigs[tabIndex];
			RefreshAttributes();
			
			if (m_bIsGlobalAttribute)
				m_AttributesManager.SetCurrentCategory(m_currentCategory);
		}
	}	
	
	ResourceName GetCurrentCategory()
	{
		return m_currentCategory;
	}
	void SetWidgetVisible(bool visible)
	{
		Widget widget = GetWidget();
		if (!widget) return;
		
		if (visible) RefreshAttributes();
		
		widget.SetVisible(visible);
		widget.SetEnabled(visible);
	}
	
	//Set which attribute is on focus
	protected void SetGamepadAttributeFocus(SCR_BaseEditorAttributeUIComponent attributeUI)
	{
		m_GamepadFocusAttribute = attributeUI;
		m_GamepadToggleEnableButton.SetEnabled(m_GamepadFocusAttribute != null && m_GamepadFocusAttribute.GetTickboxEnabled());
	}
	
	protected void SetHasConflictingAttribute(bool hasConflicting)
	{
		m_bHasConflictingAttributes = hasConflicting;
		
		if (m_GamepadToggleEnableButton)
			m_GamepadToggleEnableButton.GetRootWidget().SetVisible(m_bHasConflictingAttributes && !GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}
	
	//---- REFACTOR NOTE START: End game logic is hard to read and condition to define behavior are too specific and hard to mod
	
	void InitWidget(array<ResourceName> categoryConfigs, array<ref SCR_EditorAttributeCategory> categories)
	{		
		SCR_UISoundEntity.SoundEvent(m_sSfxOnOpenDialog, true);
		
		m_AttributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (!m_AttributesManager) return;
				
		Widget widget = GetWidget();
		if (!widget) return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) return;
		
		m_PlatformImageWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sPlatformImageWidgetName));
		m_PlayerNameWidget = TextWidget.Cast(widget.FindAnyWidget(m_sPlayerNameWidgetName));
		
		Widget tabWidget = widget.FindAnyWidget(m_sTabWidgetName);
		if (!tabWidget) return;
		
		m_TabViewComponent = SCR_TabViewComponent.Cast(tabWidget.FindHandler(SCR_TabViewComponent));
		if (!m_TabViewComponent) return;
		m_TabViewComponent.GetOnChanged().Insert(OnCurrentTabChanged);
		
		m_ContentHolder = widget.FindAnyWidget(m_sContentHolderWidgetName);
		if (!m_ContentHolder) return;
		
		ScriptInvoker onReset = ButtonActionComponent.GetOnAction(widget, m_sButtonResetWidgetName);
		if (onReset) onReset.Insert(ButtonReset);
		
		Widget resetButton =  widget.FindAnyWidget(m_sButtonResetWidgetName);
		if (resetButton)
			m_ResetButton = SCR_InputButtonComponent.Cast(resetButton.FindHandler(SCR_InputButtonComponent));
		if (m_ResetButton)
			m_ResetButton.SetEnabled(false);
		
		//Toggle attribute
		ScriptInvoker onToggle = ButtonActionComponent.GetOnAction(widget, m_sGamepadToggleAttributeButtonName);
		if (onToggle) onToggle.Insert(ButtonAttributeEnableToggle);	
		
		Widget gamePadToggle =  widget.FindAnyWidget(m_sGamepadToggleAttributeButtonName);
		if (gamePadToggle)
		{
			m_GamepadToggleEnableButton = SCR_InputButtonComponent.Cast(gamePadToggle.FindHandler(SCR_InputButtonComponent));
			
			if (m_GamepadToggleEnableButton)
				m_GamepadToggleEnableButton.SetEnabled(false);
		}
			
		//Show toggle lock button
		SetHasConflictingAttribute(m_bHasConflictingAttributes);
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);

		m_CategoryConfigs = categoryConfigs;
		
		//--- Get current category (must be done before tabs are added, they'll rewrite it)
		ResourceName currentCategory = m_AttributesManager.GetCurrentCategory();
		
		if (categoryConfigs.IsEmpty())
		{
			Widget noAttributes = widget.FindAnyWidget(m_sNoAttributeNoteName);
			if (noAttributes)
				noAttributes.SetVisible(true);
		}
		
		bool globalAttributesSet = false;
		
		//Add tabs
		foreach (int i, ResourceName categoryConfig: categoryConfigs)
		{			
			m_TabViewComponent.AddTab(string.Empty, categories[i].GetInfo().GetName(), true, categories[i].GetInfo().GetIconPath());
			
			if (!globalAttributesSet)
			{
				globalAttributesSet = true;
				m_bIsGlobalAttribute = categories[i].GetIsGlobalAttributeCategory();
			}
		}
		
		array<Managed> editedItems = new array<Managed>;
		int count = m_AttributesManager.GetEditedItems(editedItems);
		
		InputManager inputManager = GetGame().GetInputManager();
		
		//~ Default sfx is cancel unless a setting is changed
		m_sSfxOnCloseDialog = m_sSfxClickedOutsideWindow_Cancel;
		
		//End game
		if (count == 1 && SCR_RespawnComponent.Cast(editedItems[0]))
		{
			m_bIsEndGame = true;
			
			Widget closeButton = widget.FindAnyWidget(m_sButtonEndGameCloseName);
			Widget acceptButton = widget.FindAnyWidget(m_sButtonEndGameAcceptName);
			
			if (closeButton)
			{
				closeButton.SetVisible(true);
				
				ScriptInvoker onClose = ButtonActionComponent.GetOnAction(widget, m_sButtonEndGameCloseName);
				if (onClose) 
					onClose.Insert(OnCancelEndGame);	
			}
			if (acceptButton)
			{
				acceptButton.SetVisible(true);
				
				ScriptInvoker onAccept = ButtonActionComponent.GetOnAction(widget, m_sButtonEndGameAcceptName);
				if (onAccept) 
					onAccept.Insert(OnAcceptEndGame);	
			}
			
			ScriptInvoker onWindowClose = ButtonActionComponent.GetOnAction(widget, m_sOutsideWindowCloseButtonName);
			if (onWindowClose) 
				onWindowClose.Insert(OnCancelEndGame);
			
			if (m_ResetButton)
				m_ResetButton.SetVisible(false);
			
			inputManager.AddActionListener("EditorAttributes", EActionTrigger.DOWN, OnCancelEndGame);
		}
		else 
		{
			m_ButtonCloseButton = widget.FindAnyWidget(m_sButtonCloseButtonName);
			m_ButtonCloseAndSaveButton = widget.FindAnyWidget(m_sButtonCloseAndSaveButtonName);
			
			if (m_ButtonCloseButton)
			{
				m_ButtonCloseButton.SetVisible(true);
				
				ScriptInvoker onClose = ButtonActionComponent.GetOnAction(widget, m_sButtonCloseButtonName);
				if (onClose) 
					onClose.Insert(ButtonClose);	
			}
			if (m_ButtonCloseAndSaveButton)
			{
				ScriptInvoker onClose = ButtonActionComponent.GetOnAction(widget, m_sButtonCloseAndSaveButtonName);
				if (onClose) 
					onClose.Insert(ButtonClose);	
				
				m_ButtonCloseAndSaveButton.SetVisible(false);
			}
			
			ScriptInvoker onWindowClose = ButtonActionComponent.GetOnAction(widget, m_sOutsideWindowCloseButtonName);
			if (onWindowClose) 
				onWindowClose.Insert(ButtonClose);
			
			inputManager.AddActionListener("EditorAttributes", EActionTrigger.DOWN, ButtonClose);
		}
		
		//Init
		m_TabViewComponent.Init();
		
		//Check if only show images
		if (m_TabViewComponent.GetTabCount() > m_iMaxTabsUntilImageOnly)
		{
			m_bTabImageOnly = true;
			m_TabViewComponent.ShowAllTabsText(false, m_iTabImageOnlyWidth);
		}
		
		//Set active tab
		if (m_bIsGlobalAttribute)
		{
			int currentIndex = Math.Max(m_CategoryConfigs.Find(currentCategory), 0);
			m_TabViewComponent.ShowTab(currentIndex, true, false);
		}

		m_AttributesManager.GetOnResetAttributes().Insert(OnReset);
		
		SetHeaderText(RichTextWidget.Cast(widget.FindAnyWidget(m_sTitleHeaderWidgetName)));
		
		if (m_bIsEndGame)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnGameModeEnd().Insert(OnCancelEndGame);
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	//---- REFACTOR NOTE START: Complex and hard to read logic of loops and conditions
	// Conditions are uncessary specific and this approach is not modular
	
	//~Todo: Clean up set header script
	protected void SetHeaderText(RichTextWidget header)
	{		
		if (!header)
			return;

		array<Managed> editedItems = {};
		int count = m_AttributesManager.GetEditedItems(editedItems);
		
		//Check if multiple entites are selected
		if (count > 1)
		{			
			header.SetTextFormat("%1", m_sEditingTitlePart);
			
			SCR_EditableEntityComponent editableEntity;
			array<EEditableEntityType> types = {};
			array<int> typeCounts = {};
			SCR_EditableEntityComponent groupEntity;
			
			bool showTypes = true;
			bool sameNames = true;
			bool oneGroup = true;
			string name = string.Empty;
			
			//Add multiple players
			
			foreach (Managed item: editedItems)
			{
				editableEntity = SCR_EditableEntityComponent.Cast(item);
				
				if (sameNames)
				{
					//If player never display name
					if (SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(editableEntity.GetOwner()) > 0)
					{
						sameNames = false;
					}
					else 
					{
						if (name == string.Empty)
						{
							if (editableEntity.GetEntityType() != EEditableEntityType.TASK)
								name = editableEntity.GetDisplayName();
							else 
								name = m_sTaskTitle;	
						}	
						else if ((name != editableEntity.GetDisplayName() && editableEntity.GetEntityType() != EEditableEntityType.TASK) || (editableEntity.GetEntityType() == EEditableEntityType.TASK && name != m_sTaskTitle))
						{
							sameNames = false;
							
							if (!showTypes)
								break;
						}
					}
				}
				
				//Logic to show types of entity types instead of names when editing
				if (showTypes)
				{
					EEditableEntityType entityType = editableEntity.GetEntityType();
					
					if (entityType == EEditableEntityType.CHARACTER)
					{
						//Is Player so set entity type -1
						if (SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(editableEntity.GetOwner()) > 0)
						{
							oneGroup = false;
							entityType = -1;
						}
						else if (oneGroup)
						{
							if (groupEntity == null)
								groupEntity = editableEntity.GetParentEntity();
							else if (groupEntity != null && groupEntity != editableEntity.GetParentEntity())
								oneGroup = false;
						}
					}
					else if (entityType == EEditableEntityType.GROUP && oneGroup)
					{
						if (groupEntity != null && groupEntity != editableEntity)
							oneGroup = false;
						else
							groupEntity = editableEntity;
					}
					else 
					{
						oneGroup = false;
					}
					
					int index = types.Find(entityType);
					
					if(index < 0)
					{						
						types.Insert(entityType);
						typeCounts.Insert(1);
						
						if (types.Count() > m_iEntityTypesUntilMulti)
						{
							showTypes = false;
							
							if (!sameNames)
								break;
						}
					}
					else 
					{
						typeCounts[index] = typeCounts[index] +1;
					}
				}
			}

			//If all entities have the same name, so display that name
			if (sameNames)
			{
				SetHeaderPart(header, name, count, false);
				HidePlayerInfo();
			}
			//If single group with all characters of same parent then show group name and character amount
			else if (oneGroup)
			{
				int characterIndex = types.Find(EEditableEntityType.CHARACTER);
				
				SetHeaderPart(header, groupEntity.GetDisplayName(), 0 , false);
				SetHeaderPart(header, TranslateEntityType(types[characterIndex]), typeCounts[characterIndex], true);
				HidePlayerInfo();
			}
			//If multiple selected of a type so display those types
			else if (showTypes)
			{
				bool firstShown = false;
				int indexHighestPriority;
				int typeNameCount = m_aEntityTypeNames.Count();
				
				while (types.Count() > 0)
				{
					int typesCount = types.Count();
					int currentPriority = int.MAX;
					
					for(int i = 0; i < typesCount; i++)
					{
						//Is player so always show first
						if (types[i] == -1)
						{
							currentPriority = -1;
							indexHighestPriority = i;
							break;
						}
						
						//Check the priority of the type
						for(int t = 0; t < typeNameCount; t++)
						{
							if (m_aEntityTypeNames[t].GetType() == types[i])
							{
								if (t < currentPriority)
								{
									currentPriority = t;
									indexHighestPriority = i;
								}
								
								break;
							}
						}
					}
					
					HidePlayerInfo();
					SetHeaderPart(header, TranslateEntityType(types[indexHighestPriority]), typeCounts[indexHighestPriority], firstShown);
					types.Remove(indexHighestPriority);
					typeCounts.Remove(indexHighestPriority);
					firstShown = true;
				}
			}
			//If too many types selected show just "entity"
			else 
			{
				HidePlayerInfo();
				SetHeaderPart(header, m_sMultiEntitiesTitle, count, false);
			}				
		}
		//If editing only 1
		else if (count == 1)
		{	
			//Check if gamemode that is being edited			
			if (SCR_GameModeEditor.Cast(editedItems[0]))
			{
				SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
				if (!editorManager) 
					return;
				
				SCR_EditorModeEntity modeEntity = editorManager.GetCurrentModeEntity();
				if (!modeEntity)
					return;
				
				if (modeEntity.GetModeType() == EEditorMode.ADMIN)
				{
					header.SetText(m_sAdminModeTitle);
					return;
				}
			
				//Not admin
				return;
			}
				
			//Factions only being edited
			SCR_TaskSystem taskSystem = SCR_TaskSystem.Cast(editedItems[0]);
			if (taskSystem)
			{
				header.SetText(m_sFactionOnlyTitle);
				return;
			}

			if (m_bIsEndGame)
			{
				header.SetText(m_sEndGameTitle);
				return;
			}
				
			//Camera attributes (Check if Armavision)
			SCR_ManualCamera camera = SCR_ManualCamera.Cast(editedItems[0]);
			if (camera)
			{
				SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
				if (!editorManager) 
					return;		
			
				SCR_EditorModeEntity modeEntity = editorManager.GetCurrentModeEntity();
				if (!modeEntity)
					return;
			
				//Is arma vision
				if (modeEntity.GetModeType() == EEditorMode.PHOTO)
				{
					header.SetText(m_sPhotoModeTitle);
					return;
				}
			}
			
			//If player delegate show player name
			SCR_EditablePlayerDelegateComponent playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(editedItems[0]);
			if (playerDelegate)
			{
				header.SetTextFormat("%1", m_sEditingTitlePart);
				//SetHeaderPart(header, SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerDelegate.GetPlayerID()), 0, false);
				SetHeaderPart(header, "", 0, false);
				SetPlayerInfo(playerDelegate.GetPlayerID());
				return;
			}
			
			//Single entity
			SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(editedItems[0]);
			if (editableEntity)
			{
				header.SetTextFormat("%1", m_sEditingTitlePart);
				
				//Check if player, if true display player name
				if (editableEntity.GetEntityType() == EEditableEntityType.CHARACTER)
				{
					//Check is player and set name
					int playerID = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(editableEntity.GetOwner());						
					if (playerID > 0)
					{
						///SetHeaderPart(header, SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID), 0, false);
						SetHeaderPart(header, "", 0, false);
						SetPlayerInfo(playerID);
						
						return;
					}
				}
				//One task is being edited
				else if (editableEntity.GetEntityType() == EEditableEntityType.TASK)
				{
					HidePlayerInfo();
					SetHeaderPart(header, m_sTaskTitle, 1, false);
					return;
				}
				else if (editableEntity.GetEntityType() == EEditableEntityType.FACTION)
				{
					HidePlayerInfo();
					SetHeaderPart(header, editableEntity.GetDisplayName(), 0, false);
					return;
				}
				HidePlayerInfo();
				SetHeaderPart(header, editableEntity.GetDisplayName(), 1, false);
			}	
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	protected void SetPlayerInfo(int playerId){
		SCR_PlayerController contr = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		m_PlayerNameWidget.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId));
		contr.SetPlatformImageTo(playerId, m_PlatformImageWidget);
		
		m_PlayerNameWidget.SetVisible(true);
		m_PlatformImageWidget.SetVisible(true);
	}
	
	protected void HidePlayerInfo(){
		m_PlayerNameWidget.SetVisible(false);
		m_PlatformImageWidget.SetVisible(false);
	}
	
	protected void SetHeaderPart(RichTextWidget textWidget, string name, int count, bool showSeperator)
	{
		if (showSeperator)
			textWidget.SetTextFormat("%1 -", textWidget.GetText());
		
		textWidget.SetTextFormat("%1 %2", textWidget.GetText(), name);
		
		if (count != 0)
			textWidget.SetTextFormat("%2 <font name=\"%3\"><color rgba=\"226,168,79,255\">#AR-ValueUnit_Short_Times</color></font>", count.ToString(), textWidget.GetText(), m_sFontForHeaderEntityAmount);
	}
	
	protected LocalizedString TranslateEntityType(EEditableEntityType type)
	{
		if (type == -1)
		{
			return "#AR-AttributesDialog_TitlePage_Player_Text";
		}
		else 	
		{
			foreach (SCR_EntityAttributeTitleType titleType: m_aEntityTypeNames)
			{
				if (titleType.GetType() == type)
					return titleType.GetTypeName();
			}
		}
		
		Print(string.Format("Couldn't find type: %1 in m_aEntityTypeNames", typename.EnumToString(EEditableEntityType, type)), LogLevel.ERROR);
		return typename.EnumToString(EEditableEntityType, type);
	}
	
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{			
		//Show toggle lock button
		SetHasConflictingAttribute(m_bHasConflictingAttributes);
	}	
	
	override void HandlerDeattached(Widget w)
	{	
		if (m_AttributesManager)
		{
			m_AttributesManager.GetOnResetAttributes().Remove(OnReset);
		}
		
		if (m_TabViewComponent)
			m_TabViewComponent.GetOnChanged().Remove(OnCurrentTabChanged);
		
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			if (m_bIsEndGame)
				inputManager.RemoveActionListener("EditorAttributes", EActionTrigger.DOWN, OnCancelEndGame);
			else 
				inputManager.RemoveActionListener("EditorAttributes", EActionTrigger.DOWN, ButtonClose);
		}
		
		if (m_bIsEndGame)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnGameModeEnd().Remove(OnCancelEndGame);
		}
		
		SCR_UISoundEntity.SoundEvent(m_sSfxOnCloseDialog, true);
		
	}
};


[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_type")]
class SCR_EntityAttributeTitleType
{
	[Attribute("0", UIWidgets.ComboBox, "Type", "", ParamEnumArray.FromEnum(EEditableEntityType))]
	protected EEditableEntityType m_type;
	
	[Attribute("")]
	protected ref SCR_UIInfo m_UiInfo;
	
	EEditableEntityType GetType()
	{
		return m_type;
	}
	
	string GetTypeName()
	{
		return m_UiInfo.GetName();
	}
}


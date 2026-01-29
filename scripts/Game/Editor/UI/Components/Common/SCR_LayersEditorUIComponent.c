//! Creates the elements for the layer Ui menu
//! If a Layer is selected this menu will show which layer is selected, which layers are before the selected layer and if the selected layer has any children.
//! The player is able to use the menu to switch between layers
class SCR_LayersEditorUIComponent : SCR_BaseEditorUIComponent
{	
	//SFX
	[Attribute(SCR_SoundEvent.SOUND_E_LAYER_EDIT_START, UIWidgets.EditBox, "")]
	protected string m_sSfxStartLayerEditing;
	
	[Attribute(SCR_SoundEvent.SOUND_E_LAYER_EDIT_END, UIWidgets.EditBox, "")]
	protected string m_sSfxStopLayerEditing;
	
	[Attribute(SCR_SoundEvent.SOUND_E_LAYER_DEEPER, UIWidgets.EditBox, "")]
	protected string m_sSfxGoLayerDeeper;
	
	[Attribute(SCR_SoundEvent.SOUND_E_LAYER_BACK, UIWidgets.EditBox, "")]
	protected string m_sSfxGoLayerBack;
	
	//Widget References
	[Attribute()]
	protected string m_sLayerElementHolder;

	protected Widget m_LayerElementHolder;

	[Attribute()]
	protected string m_sExitLayerButton;

	[Attribute()]
	protected string m_sLayerEditorOverlayName;

	protected Widget m_LayerEditorOverlay;
	
	[Attribute("LayerEditorOverlay")]
	protected string m_sParentWidgetName;

	protected Widget m_parentWidget;
	
	protected SCR_EditableEntitySlotManagerUIComponent m_SlotManager;
	
	//References
	protected SCR_EditableEntityComponent m_SelectedLayer;
	protected ref array<SCR_EditableEntityComponent> m_aCurrentSelectedLayers = {};
	protected SCR_LayersEditorComponent m_LayersManager;
	protected InputManager m_InputManager;
	protected SCR_MenuEditorComponent m_EditorMenuManager;
	protected SCR_StatesEditorComponent m_StatesManager;
	protected SCR_HoverEditableEntityFilter m_HoverFilter;

	//Settings
	[Attribute(defvalue: "0", desc: "Layers shown after WorldRoot but before Dropdown")]
	protected int m_iLayersShownAfterRoot; 

	[Attribute(defvalue: "1", desc: "Layers shown before current but after Dropdown")]
	protected int m_iLayersShownBeforeCurrent;

	//Prefabs
	[Attribute()]
	protected ResourceName m_LayerElementPrefab;

	[Attribute()]
	protected ResourceName m_LayerElementButtonPrefab;

	[Attribute()]
	protected ResourceName m_LayerWorldPrefab;

	[Attribute()]
	protected ResourceName m_LayerWorldButtonPrefab;

	[Attribute()]
	protected ResourceName m_NextLayerIndicatorPrefab;

	[Attribute()]
	protected ResourceName m_MoreLayersIndicatorPrefab;
	
	//State
	protected bool m_bUsesGamepad;
	
	protected ref map<SCR_ButtonBaseComponent, SCR_EditableEntityComponent> m_mButtonReferences = new map<SCR_ButtonBaseComponent, SCR_EditableEntityComponent>();
	
	//======================== ON MENU UPDATE ========================\\

	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		bool mapShown = SCR_MapEntity.GetMapInstance() && SCR_MapEntity.GetMapInstance().IsOpen();
		if (m_InputManager && !mapShown)
			m_InputManager.ActivateContext("EditorEditModeContext");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLayerButtonPressed(SCR_ButtonBaseComponent button)
	{
		SCR_EditableEntityComponent layer;
		
		if (m_mButtonReferences.Find(button, layer))
			m_LayersManager.SetCurrentLayer(layer);
	}
	
	//~ToDo: Check is gamepad is used and show controlles for it: GetGame().OnInputDeviceIsGamepadInvoker(). Hide L1 + Right button hint if no children. Check if input can be displayed dynamicly
	//======================== CREATE LAYER UI ========================\\

	//------------------------------------------------------------------------------------------------
	//On Layer changed recreate layer menu
	protected void OnCompositionLayerChanged(SCR_EditableEntityComponent currentLayer, SCR_EditableEntityComponent prevLayer)
	{			
		m_SelectedLayer = currentLayer;
		
		//If currentLayer is not null then menu should be shown
		if (m_SelectedLayer) 
		{
			RefreshLayerUI();
		}
		//No current Layer so set UI invisible
		else 
		{
			//Clear UI
			ClearLayerElements();
			SetVisible(false);
		}
		
		if (!prevLayer && !currentLayer) //~ Safty, No layer thus return
			return;
		else if (!prevLayer && currentLayer) //~ Start Layer editing
			SCR_UISoundEntity.SoundEvent(m_sSfxStartLayerEditing, true);
		else if (prevLayer && !currentLayer) //~ Stop Layer editing
			SCR_UISoundEntity.SoundEvent(m_sSfxStopLayerEditing, true);
		else if (currentLayer.GetParentEntity() == prevLayer) //~ Go layer deeper
			SCR_UISoundEntity.SoundEvent(m_sSfxGoLayerDeeper, true);
		else //~ Go layer back
			SCR_UISoundEntity.SoundEvent(m_sSfxGoLayerBack, true);
	} 
	
	//On input change

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_bUsesGamepad = isGamepad;
		
		RefreshLayerUI();
	}
	
	//======================== PARENT ENTITY CHANGED ========================\\

	//---- REFACTOR NOTE START: Confusing use of hasMoreAfterRootIndicator
	
	//------------------------------------------------------------------------------------------------
	protected void RefreshLayerUI()
	{
		if (m_SelectedLayer == null)
			return;
		
		//~Todo: Figure out what is setting the UI disabled
		m_parentWidget.SetEnabled(true);
		
		//Clear UI
		ClearLayerElements();
		
		//Add the world root
		CreateWorldRootLayer();
		
		//Get Layer List
		m_aCurrentSelectedLayers = {};
		m_SelectedLayer.GetParentEntities(m_aCurrentSelectedLayers);
		m_aCurrentSelectedLayers.InsertAt(m_SelectedLayer, 0);
		int totalLayerCount = m_aCurrentSelectedLayers.Count();
		bool hasMoreAfterRootIndicator = false;

		//Go over each layer and display them properly (In reverse order)
		for (int i = totalLayerCount -1; i > -1; i--)
		{
			//Place layers after World Root or after More indicator
			if (totalLayerCount - i <= m_iLayersShownAfterRoot || i <= m_iLayersShownBeforeCurrent)
			{
				//Create UI elements
				CreateNextLayerIndicator();
				CreateLayerElement(m_aCurrentSelectedLayers[i], i == 0);
			}
			//Layers are hidden
			else if (!hasMoreAfterRootIndicator)
			{
				hasMoreAfterRootIndicator = true;
				CreateNextLayerIndicator();
				CreateMoreLayersIndicator();
			}
		}
		
		//Set UI visible
		SetVisible(true);
	}
	
	//---- REFACTOR NOTE END ----
	
	//======================== CHECK IF CHANGED ========================\\

	//------------------------------------------------------------------------------------------------
	//! Check an entity that was changed was within the current selected Hierarchy
	//! \param[in] entity
	//! \return
	protected bool CheckIfSelectedLayerHierarchyChanged(SCR_EditableEntityComponent entity)
	{
		//If given entity is not null
		if (entity) 
		{
			//Go through array and check if given entity changed
			for (int i = 0, count = m_aCurrentSelectedLayers.Count(); i < count; i++)
	        {
	        	if (m_aCurrentSelectedLayers[i] == entity)
					return true;
			}
		}
		
		return false;
	}
	
	//======================== PARENT ENTITY CHANGED ========================\\

	//------------------------------------------------------------------------------------------------
	//! Check if in Hierarchy, if true: Refresh UI
	//! \param[in] entity
	//! \param[in] parentEntity
	//! \param[in] parentEntityPrev
	protected void OnParentEntityVisibleChanged(SCR_EditableEntityComponent entity, SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev)
	{
		if (CheckIfSelectedLayerHierarchyChanged(entity))
		{
			RefreshLayerUI();
			return;
		}

		if (CheckIfSelectedLayerHierarchyChanged(parentEntity))
		{
			RefreshLayerUI();
			return;
		}

		if (CheckIfSelectedLayerHierarchyChanged(parentEntityPrev))
		{
			RefreshLayerUI();
			return;
		}
	}
	
	//======================== ENTITY STATE CHANGED ========================\\

	//------------------------------------------------------------------------------------------------
	protected void OnEntityStateChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		//Inserted Entities state changed, check if in Hierarchy, if true: Refresh UI
		if (entitiesInsert)
		{
			for (int i = 0, count = entitiesInsert.Count(); i < count; i++)
	        {
				if (CheckIfSelectedLayerHierarchyChanged(entitiesInsert[i]))
				{
					RefreshLayerUI();
					return;
				}
			}
		}
		
		//Inserted Entities state changed, check if in Hierarchy, if true: Refresh UI
		if (entitiesRemove)
		{
			for (int i = 0, count = entitiesRemove.Count(); i < count; i++)
	        {
				if (CheckIfSelectedLayerHierarchyChanged(entitiesRemove[i]))
				{
					RefreshLayerUI();
					return;
				}
			}
		}
	}
	
	//======================== CREATE LAYER ELEMENTS ========================\\
	//------------------------ Layer Element in ------------------------\\

	//---- REFACTOR NOTE START: Hardcoded search for button widget
	
	//------------------------------------------------------------------------------------------------
	protected void CreateLayerElement(SCR_EditableEntityComponent layerEntity, bool isLast)
	{	
		Widget elementWidget;
		
		//Spawn Widget
		if (m_bUsesGamepad || isLast)
		{
			elementWidget = GetGame().GetWorkspace().CreateWidgets(m_LayerElementPrefab, m_LayerElementHolder);
			if (!elementWidget)
				return;
		}
		else 
		{
			elementWidget = GetGame().GetWorkspace().CreateWidgets(m_LayerElementButtonPrefab, m_LayerElementHolder);
			
			if (!elementWidget)
				return;
			
			Widget button = elementWidget.FindAnyWidget("Button");
			
			if (!button)
				return;
			
			SCR_ButtonBaseComponent buttonComponent = SCR_ButtonBaseComponent.Cast(button.FindHandler(SCR_ButtonBaseComponent));
			
			if (!buttonComponent)
				return;
			
			m_mButtonReferences.Insert(buttonComponent, layerEntity);
			buttonComponent.m_OnClicked.Insert(OnLayerButtonPressed);
		}
		
		SCR_BaseToolbarItemEditorUIComponent toolbarItem = SCR_BaseToolbarItemEditorUIComponent.Cast(elementWidget.FindHandler(SCR_BaseToolbarItemEditorUIComponent));
		if (!toolbarItem) 
		{
			elementWidget.RemoveFromHierarchy();
			return;
		}
			
		toolbarItem.InitToolbarItem(elementWidget);
		
		SCR_EditableEntityLinkUIComponent linkedEntityComponent = SCR_EditableEntityLinkUIComponent.Cast(elementWidget.FindHandler(SCR_EditableEntityLinkUIComponent));
		
		if (linkedEntityComponent)
			linkedEntityComponent.SetLinkedEntity(layerEntity);
		
		SCR_EntityToolbarItemEditorUIComponent entityItem = SCR_EntityToolbarItemEditorUIComponent.Cast(elementWidget.FindHandler(SCR_EntityToolbarItemEditorUIComponent));
		if (entityItem)
			entityItem.SetEntity(layerEntity, elementWidget, m_SlotManager);
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------ Next Layer Indicator ------------------------\\

	//------------------------------------------------------------------------------------------------
	protected void CreateNextLayerIndicator()
	{	
		Widget nextLayerIndicator = GetGame().GetWorkspace().CreateWidgets(m_NextLayerIndicatorPrefab, m_LayerElementHolder);
	}

	//------------------------ Drop menu creator ------------------------\\

	//------------------------------------------------------------------------------------------------
	protected void CreateMoreLayersIndicator()
	{
		Widget moreLayersIndicator = GetGame().GetWorkspace().CreateWidgets(m_MoreLayersIndicatorPrefab, m_LayerElementHolder);
	}
	
	//------------------------ Children Indicator ------------------------\\

	//---- REFACTOR NOTE START: Hardcoded search for button widget
	
	//------------------------------------------------------------------------------------------------
	protected void CreateWorldRootLayer()
	{		
		Widget elementWidget;
		
		//Spawn Widget
		if (m_bUsesGamepad)
		{
			elementWidget = GetGame().GetWorkspace().CreateWidgets(m_LayerWorldPrefab, m_LayerElementHolder);
		}
		else 
		{
			elementWidget = GetGame().GetWorkspace().CreateWidgets(m_LayerWorldButtonPrefab, m_LayerElementHolder);
			if (!elementWidget)
				return;
			
			Widget button = elementWidget.FindAnyWidget("Button");
			
			if (!button)
				return;
			
			SCR_ButtonBaseComponent buttonComponent = SCR_ButtonBaseComponent.Cast(button.FindHandler(SCR_ButtonBaseComponent));
			
			if (!buttonComponent)
				return;
			
			m_mButtonReferences.Insert(buttonComponent, null);
			buttonComponent.m_OnClicked.Insert(OnLayerButtonPressed);
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	//======================== MENU VISIBILITY ========================\\

	//------------------------------------------------------------------------------------------------
	//! Show/Hides Menu
	//! \param[in] show
	protected void SetVisible(bool show)
	{		
		if (!m_parentWidget)
			return;
			
		if (m_parentWidget.IsVisible() != show)
		{
			m_parentWidget.SetVisible(show);
			
			if (m_LayerEditorOverlay)
				m_LayerEditorOverlay.SetVisible(show);
		
			if (!show)
				ClearLayerElements();
		}
	}
	
	//======================== CLEAR UI ========================\\

	//------------------------------------------------------------------------------------------------
	//! Clears the array
	protected void ClearLayerElements()
	{
		Widget child = m_LayerElementHolder.GetChildren();
		Widget childtemp;
		while (child)
		{
			childtemp = child;
			child = child.GetSibling();
			childtemp.RemoveFromHierarchy();
		}
		
		m_SlotManager.ClearSlots();
		
		m_mButtonReferences.Clear();
	}
	
	//======================== BUTTON FUNCTIONS ========================\\

	//------------------------------------------------------------------------------------------------
	protected void OnEditorExitLayerButton()
	{
		if (m_LayersManager)
			m_LayersManager.SetCurrentLayerToParent();
	}

	//======================== INHERITED ========================\\

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{		
		if (SCR_Global.IsEditMode()) return; //--- Run-time only
		
		m_LayerElementHolder = w.FindAnyWidget(m_sLayerElementHolder);
		 
		m_parentWidget = GetMenu().GetRootWidget().FindAnyWidget(m_sParentWidgetName);
		m_SlotManager = SCR_EditableEntitySlotManagerUIComponent.Cast(w.FindHandler(SCR_EditableEntitySlotManagerUIComponent));
		
		if (m_LayerElementHolder)
		{
			//Input device changed
			OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
			GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
			
			//Get layersEditorComponent
			m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent, true));
			if (!m_LayersManager)
				return;
			
			//Subscribe to OnCompositionLayerChanged
			m_LayersManager.GetOnCurrentLayerChange().Insert(OnCompositionLayerChanged);

			//Get Core
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore)); //Remove and just get new in Handler
			if (!core)
				return;
		
			//Subscribe to OnParentEntityChanged
			core.Event_OnParentEntityChanged.Insert(OnParentEntityVisibleChanged);
			
			SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.VISIBLE);
			if (!filter)
				return;

			filter.GetOnChanged().Insert(OnEntityStateChanged);
			
			//Register buttons
			Widget widget = w.FindAnyWidget(m_sExitLayerButton);
			if (widget)
			{
				ButtonActionComponent exitLayerButton = ButtonActionComponent.Cast(widget.FindHandler(ButtonActionComponent));
				if (exitLayerButton)
					exitLayerButton.GetOnAction().Insert(OnEditorExitLayerButton);
			}
			
			m_EditorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
	
			SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
			if (!entitiesManager)
				return;
			
			m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
			
			m_HoverFilter = SCR_HoverEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.HOVER));
			if (!m_HoverFilter)
				return;
			
			MenuRootBase menu = GetMenu();
			if (menu)
				menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
			
			m_InputManager = GetGame().GetInputManager();
		}	
		
		//Widget overlay
		Widget parent = w.GetParent();
		if (parent)
		{
			m_LayerEditorOverlay = parent.FindAnyWidget(m_sLayerEditorOverlayName);
			
			if (m_LayerEditorOverlay)
				m_LayerEditorOverlay.SetVisible(false);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_LayersManager)
		{
			m_LayersManager.GetOnCurrentLayerChange().Remove(OnCompositionLayerChanged);
			
			//Unregister Buttons
			Widget widget = w.FindAnyWidget(m_sExitLayerButton);
			if (widget)
			{
				ButtonActionComponent exitLayerButton = ButtonActionComponent.Cast(widget.FindHandler(ButtonActionComponent));
				if (exitLayerButton)
					exitLayerButton.GetOnAction().Remove(OnEditorExitLayerButton);
			}
			
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			if (core)
				core.Event_OnParentEntityChanged.Remove(OnParentEntityVisibleChanged);
			
			SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.VISIBLE);
			if (filter)
				filter.GetOnChanged().Remove(OnEntityStateChanged);
			
			super.HandlerDeattached(w);
	
			MenuRootBase menu = GetMenu();
			if (menu)
				menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
		}
	}
}

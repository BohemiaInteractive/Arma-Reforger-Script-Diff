class SCR_CycleWaypointEditorUIComponent : SCR_ModularButtonComponent
{
	protected SCR_BaseEditableEntityFilter m_SelectedEntitiesManager;

	[Attribute()]
	protected string m_sCycleWaypointsToggleWidgetName;

	[Attribute()]
	protected string m_sSeparatorWidgetName;

	[Attribute()]
	protected string m_sRadialActionName;

	[Attribute("0.8")]
	protected float m_fGamepadScale;

	[Attribute("8.0")]
	protected float m_fMKBottomPadding;

	protected ButtonWidget m_wCycleWaypointsToggle;
	protected SCR_ModularButtonComponent m_ModularButtonToggleComponent;
	protected Widget m_wSeparator;
	protected SCR_PlacingEditorComponent m_PlacingEditor;

	protected bool m_bIsGamepad;

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_bIsGamepad = isGamepad;
		UpdateInputVisuals();
	}

	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Styles the widget according to the current input device
	protected void UpdateInputVisuals()
	{
		if (m_wSeparator)
			m_wSeparator.SetVisible(!m_bIsGamepad);

		float scale = 1.0;
		float bottomPadding = m_fMKBottomPadding;
		if (m_bIsGamepad)
		{
			scale = m_fGamepadScale;
			bottomPadding = 0;
		}

		ScaleWidget scaleWidget = ScaleWidget.Cast(m_wCycleWaypointsToggle.GetParent());
		if (scaleWidget)
			scaleWidget.SetScale(scale);

	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Runs when selected entities from entities manager change. Used to update visibility of toggle
	protected void OnSelectedEntitiesChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		Widget container = GetRootWidget();
		bool isVisible = CanShowToggleCycleWaypoint();
		if (container)
			container.SetVisible(isVisible);
				
		if (m_ModularButtonToggleComponent)
			m_ModularButtonToggleComponent.SetToggled(GetHasCycleWaypointsActivated());
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
			m_SelectedEntitiesManager = entitiesManager.GetFilter(EEditableEntityState.SELECTED);

		if (m_SelectedEntitiesManager)
			m_SelectedEntitiesManager.GetOnChanged().Insert(OnSelectedEntitiesChange);

		//++ Initializes toggle on
		m_wCycleWaypointsToggle = ButtonWidget.Cast(GetRootWidget().FindAnyWidget(m_sCycleWaypointsToggleWidgetName));
		if (!m_wCycleWaypointsToggle)
			return;

		m_ModularButtonToggleComponent = SCR_ModularButtonComponent.FindComponent(m_wCycleWaypointsToggle);
		if (m_ModularButtonToggleComponent)
		{
			m_ModularButtonToggleComponent.m_OnClicked.Insert(ToggleCycleWaypoints);

			//++ Style toggle
			m_ModularButtonToggleComponent.SetToggled(GetHasCycleWaypointsActivated());
		}
		
		m_PlacingEditor = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));

		//++ Initializes the separator
		m_wSeparator = GetRootWidget().FindAnyWidget(m_sSeparatorWidgetName);

		GetGame().GetInputManager().AddActionListener(m_sRadialActionName, EActionTrigger.PRESSED, ToggleCycleWaypoints);

		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		UpdateInputVisuals();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_ModularButtonToggleComponent)
			m_ModularButtonToggleComponent.m_OnClicked.Remove(ToggleCycleWaypoints);

		GetGame().GetInputManager().RemoveActionListener(m_sRadialActionName, EActionTrigger.PRESSED, ToggleCycleWaypoints);
	}

	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Toggle cycle waypoints for selected groups, based on the status of the first group
	protected void ToggleCycleWaypoints()
	{
		set<SCR_EditableGroupComponent> selectedGroups = new set<SCR_EditableGroupComponent>();
		if (!GetSelectedGroups(selectedGroups))
			return;

		bool isEnabled = !selectedGroups.Get(0).AreCycledWaypointsEnabled();
		if (isEnabled)
			EnableCycleWaypoints();
		else
			DisableCycleWaypoints();
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Enable cycle waypoints for selected groups
	protected void EnableCycleWaypoints()
	{
		set<SCR_EditableGroupComponent> selectedGroups = new set<SCR_EditableGroupComponent>();
		if (!GetSelectedGroups(selectedGroups))
			return;
		
		if(!m_PlacingEditor)
			return;
		
		m_PlacingEditor.SetCycleWaypoints(selectedGroups, true);

		if (m_ModularButtonToggleComponent)
			m_ModularButtonToggleComponent.SetToggled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Disable cycle waypoints for selected groups
	protected void DisableCycleWaypoints()
	{
		set<SCR_EditableGroupComponent> selectedGroups = new set<SCR_EditableGroupComponent>();
		if (!GetSelectedGroups(selectedGroups))
			return;
		
		if(!m_PlacingEditor)
			return;
		
		m_PlacingEditor.SetCycleWaypoints(selectedGroups, false);

		if (m_ModularButtonToggleComponent)
			m_ModularButtonToggleComponent.SetToggled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns groups from selected entities (unique)
	//! \param[out] selectedGroups set of groups, must be initialized
	//! \returns bool to say if it contains or not
	protected bool GetSelectedGroups(notnull out set<SCR_EditableGroupComponent> selectedGroups)
	{
		set<SCR_EditableEntityComponent> selectedEntities = new set<SCR_EditableEntityComponent>();
		m_SelectedEntitiesManager.GetEntities(selectedEntities);
		
		SCR_EditableGroupComponent group;
		foreach (SCR_EditableEntityComponent entity : selectedEntities)
		{
			group = SCR_EditableGroupComponent.Cast(entity.GetAIGroup());
			if (group)
				selectedGroups.Insert(group);
		}

		return !selectedGroups.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if any of the selected group has cycle waypoint activated
	//! \return bool to say if it contains or not
	protected bool GetHasCycleWaypointsActivated()
	{
		set<SCR_EditableEntityComponent> selectedEntities = new set<SCR_EditableEntityComponent>();
		if (m_SelectedEntitiesManager)
			m_SelectedEntitiesManager.GetEntities(selectedEntities);

		SCR_EditableGroupComponent group;
		foreach (SCR_EditableEntityComponent entity : selectedEntities)
		{
			group = SCR_EditableGroupComponent.Cast(entity.GetAIGroup());
			if (group && group.AreCycledWaypointsEnabled())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if toggle cycle waypoint button can be shown
	protected bool CanShowToggleCycleWaypoint()
	{
		set<SCR_EditableEntityComponent> selectedEntities = new set<SCR_EditableEntityComponent>();
		m_SelectedEntitiesManager.GetEntities(selectedEntities);
		
		foreach (SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (SCR_EditableGroupComponent.Cast(entity.GetAIGroup()))
				return true;
		}

		return false;
	}
}

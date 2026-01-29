class SCR_EntitiesEditorUIComponent : SCR_EditableEntitySlotManagerUIComponent
{
	[Attribute()]
	protected ref array<ref SCR_BaseEntitiesEditorUIEffect> m_aEffects;
	
	[Attribute()]
	protected ref array<ref SCR_EditableEntityUISlotSize> m_aSlotSizes;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_SlotPrefab;
	
	protected SCR_EditableEntityUIConfig m_EditableEntityUIConfig;
	protected ref map<EEditableEntityState, SCR_BaseEntitiesEditorUIEffect> m_aEffectsMap = new map<EEditableEntityState, SCR_BaseEntitiesEditorUIEffect>();
	protected WorkspaceWidget m_Workspace;
	protected InputManager m_InputManager;
	protected SCR_HoverEditableEntityFilter m_HoverManager;
	protected SCR_CursorEditorUIComponent m_CursorComponent;
	protected SCR_EditableEntityComponent m_EntityUnderCursor;
	protected SCR_MouseAreaEditorUIComponent m_MouseArea;
	protected SCR_MapEntity m_MapEntity;

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Registration
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityBaseSlotUIComponent FindSlot(SCR_EditableEntityComponent entity, bool createIfNull = false)
	{
		SCR_EditableEntityBaseSlotUIComponent slot = super.FindSlot(entity, createIfNull);
		if (slot || !createIfNull)
			return slot;
		
		//--- Create slot from prefab
		SCR_EditableEntitySceneSlotUIComponent sceneSlot;
		Widget slotWidget = GetGame().GetWorkspace().CreateWidgets(m_SlotPrefab, GetWidget());
		if (slotWidget)
		{
			sceneSlot = SCR_EditableEntitySceneSlotUIComponent.Cast(slotWidget.FindHandler(SCR_EditableEntitySceneSlotUIComponent));
		}
		
		//--- No prefab found, create object directly
		if (!sceneSlot)
		{
			if (slotWidget)
				slotWidget.RemoveFromHierarchy();
			
			slotWidget = GetGame().GetWorkspace().CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), 0, GetWidget());
			sceneSlot = new SCR_EditableEntitySceneSlotUIComponent();
			slotWidget.AddHandler(sceneSlot);
		}
		slot = sceneSlot;
		
		//--- Set slot size
		int slotSize = -1;
		EEditableEntityType entityType = entity.GetEntityType();
		foreach (SCR_EditableEntityUISlotSize slotSizeStruct: m_aSlotSizes)
		{
			if (slotSizeStruct.GetType() == entityType)
			{
				slotSize = slotSizeStruct.GetSize();
				break;
			}
			else if (slotSizeStruct.GetType() == EEditableEntityType.GENERIC)
			{
				slotSize = slotSizeStruct.GetSize();
			}
		}

		if (slotSize != -1)
			FrameSlot.SetSize(slotWidget, slotSize, slotSize);
		
		InsertSlot(entity, slot);
		ApplyAutoEffect(entity, sceneSlot);
		
		//--- Add existing dependent icons
		foreach (SCR_EditableEntityUIRuleTracker ruleTracker : m_RuleTrackers)
		{
			if (ruleTracker.GetRule().IsDependent())
				ruleTracker.AddEntity(entity);
		}
		
		return slot;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeleteSlot(SCR_EditableEntityComponent entity, bool forced = false)
	{
		bool deleted = super.DeleteSlot(entity, forced);
		if (deleted && entity == m_EntityUnderCursor)
			m_EntityUnderCursor = null;
		
		return deleted;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Mouse interaction
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SCR_EditableEntityBaseSlotUIComponent entitySlot = GetEntitySlot(w);
		if (entitySlot)
			m_EntityUnderCursor = entitySlot.GetEntity();
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		SCR_EditableEntityBaseSlotUIComponent entitySlot = GetEntitySlot(w);
		if (entitySlot)
			m_EntityUnderCursor = null;
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		//--- Reset the value, otherwise it would linger on after switching from gamepad to mouse&keyboard
		m_EntityUnderCursor = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEffect(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		SCR_EditableEntitySceneSlotUIComponent slot;
		if (entitiesRemove)
		{
			foreach (SCR_EditableEntityComponent entity: entitiesRemove)
			{
				if (!m_mEntitySlots.Find(entity, slot))
					continue;

				ApplyAutoEffect(entity, slot);
			}
		}

		if (entitiesInsert)
		{
			SCR_BaseEntitiesEditorUIEffect effect = null;
			m_aEffectsMap.Find(state, effect);
			if (!effect)
				return;

			foreach (SCR_EditableEntityComponent entity: entitiesInsert)
			{
				if (!m_mEntitySlots.Find(entity, slot))
					continue;

				effect.ApplyOn(slot.GetWidget());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] slot
	void ApplyAutoEffect(SCR_EditableEntityComponent entity, SCR_EditableEntitySceneSlotUIComponent slot)
	{
		foreach (EEditableEntityState effectState, SCR_BaseEntitiesEditorUIEffect effect : m_aEffectsMap)
		{
			if (entity.HasEntityState(effectState))
			{
				effect.ApplyOn(slot.GetWidget());
				break;
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Update
	////////////////////////////////////////////////////////////////////////////////////////////////

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! \param[in] tDelta
	void OnMenuUpdate(float tDelta)
	{
		//--- No scene interaction when the map is opened
		bool isMapOpen = m_MapEntity && m_MapEntity.IsOpen();
		GetWidget().SetVisible(!isMapOpen);
		if (isMapOpen)
			return;
		
		int screenW = m_Workspace.GetWidth();
		int screenH = m_Workspace.GetHeight();
		screenW = m_Workspace.DPIUnscale(screenW);
		screenH = m_Workspace.DPIUnscale(screenH);
		vector posCenter = Vector(screenW / 2, screenH / 2, 0);
		
		vector transformCam[4];
		GetGame().GetWorld().GetCurrentCamera(transformCam);
		vector posCam = transformCam[3];
		
		if (!m_bInteractive || !m_CursorComponent || (m_InputManager && m_InputManager.IsUsingMouseAndKeyboard()))
		{
			//--- Mouse & keyboard (or non-interactive)
			foreach (SCR_EditableEntityBaseSlotUIComponent entitySlot : m_mEntitySlots)
			{
				//--- If it is an attached entity, position it only if the parent entity slot is not found
				//--- An example of this is when it is attached to a character of a group, and the character slot gets hidden in the distance
				SCR_EditableEntityBaseSlotUIComponent parentSlot;
				if (entitySlot.GetEntity() && entitySlot.GetEntity().IsAttached() && entitySlot.GetEntity().GetAttachedTo() && m_mEntitySlots.Find(entitySlot.GetEntity().GetAttachedTo(), parentSlot))
					continue;
				
				vector position = entitySlot.UpdateSlot(screenW, screenH, posCenter, posCam);
				PlaceAttachedSlots(entitySlot, position);
			}
		}
		else
		{
			//--- Gamepad (check for icon distances)
			float distanceSq;
			float nearestDis = m_CursorComponent.GetCursorRadiusSq();
			vector cursorPos = m_CursorComponent.GetCursorPos();
			vector position;
			SCR_EditableEntityBaseSlotUIComponent nearestSlot;
			foreach (SCR_EditableEntityBaseSlotUIComponent entitySlot : m_mEntitySlots)
			{
				//--- If it is an attached entity, position it only if the parent entity slot is not found
				//--- An example of this is when it is attached to a character of a group, and the character slot gets hidden in the distance
				SCR_EditableEntityBaseSlotUIComponent parentSlot;
				if (entitySlot.GetEntity() && entitySlot.GetEntity().IsAttached() && entitySlot.GetEntity().GetAttachedTo() && m_mEntitySlots.Find(entitySlot.GetEntity().GetAttachedTo(), parentSlot))
					continue;
				
				position = entitySlot.UpdateSlot(screenW, screenH, posCenter, posCam);
				distanceSq = vector.DistanceSq(cursorPos, position);
				if (distanceSq < nearestDis && !entitySlot.IsPreview()) //--- Ignore preview entity, it's snapped cursor and would block actual entities under cursor
				{
					nearestDis = distanceSq;
					nearestSlot = entitySlot;
				}
				
				PlaceAttachedSlots(entitySlot, position);
			}
			if (nearestSlot)
				m_EntityUnderCursor = nearestSlot.GetEntity();
			else
				m_EntityUnderCursor = null;
		}
		
		if (m_HoverManager && m_MouseArea.IsMouseOn())
			m_HoverManager.SetEntityUnderCursor(m_EntityUnderCursor, true);
	}
	//---- REFACTOR NOTE END ----
		
	//------------------------------------------------------------------------------------------------
	//! Places the slot of the attached entites in an absolute mannear, with an offset to parent
	//! \param[in] entitySlot is the child (current) slot
	//! \param[in] vector position of current slot
	//! \param[in] offset offset to add to each slot
	protected void PlaceAttachedSlots(SCR_EditableEntityBaseSlotUIComponent entitySlot, vector position, float offset = 32)
	{
		SCR_EditableEntityComponent entity = entitySlot.GetEntity();
		if (!entity)
			return;
		
		set<SCR_EditableEntityComponent> attached = entity.GetAttachedEntities();
		
		foreach (int i, SCR_EditableEntityComponent attachment : attached) 
		{
			if (!attachment || !attachment.IsAttached())
				continue;
			
			SCR_EditableEntityBaseSlotUIComponent attachmentSlot;
			if (!m_mEntitySlots.Find(attachment, attachmentSlot))
				continue;
			
			FrameSlot.SetPos(attachmentSlot.GetWidget(), position[0], (position[1] - (i + 1) * offset));
		}	
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost()
	{
		//--- When menu focus is lost, OnMenuUpdate is not called anymore. Keep it updating with low frequency.
		GetGame().GetCallqueue().CallLater(OnMenuUpdate, 10, true, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusGained()
	{
		GetGame().GetCallqueue().Remove(OnMenuUpdate);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return; //--- Run-time only
		
		SCR_EntitiesManagerEditorComponent entityManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
		if (!entityManager)
			return;
			
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
		menu.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
		menu.GetOnMenuFocusGained().Insert(OnMenuFocusGained);
		
		if (!menu.IsFocused())
			OnMenuFocusLost();

		m_Workspace = w.GetWorkspace();
		if (!m_Workspace)
			return;
	
		m_InputManager = GetGame().GetInputManager();
		m_HoverManager = SCR_HoverEditableEntityFilter.Cast(entityManager.GetFilter(EEditableEntityState.HOVER));
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		MenuRootComponent root = GetRootComponent();
		if (root)
		{
			m_CursorComponent = SCR_CursorEditorUIComponent.Cast(root.FindComponent(SCR_CursorEditorUIComponent));
			m_MouseArea = SCR_MouseAreaEditorUIComponent.Cast(root.FindComponent(SCR_MouseAreaEditorUIComponent));
		}
		
		//--- Init effects
		foreach (SCR_BaseEntitiesEditorUIEffect effect: m_aEffects)
		{
			EEditableEntityState state = effect.GetState();
			if (state == 0)
				continue;
			
			SCR_BaseEditableEntityFilter stateManager = entityManager.GetFilter(state);
			if (!stateManager)
				continue;
			
			stateManager.GetOnChanged().Insert(OnEffect);
			m_aEffectsMap.Insert(state, effect);
		}
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		
		super.HandlerAttachedScripted(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		if (SCR_Global.IsEditMode())
			return; //--- Run-time only
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
			menu.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
			menu.GetOnMenuFocusGained().Remove(OnMenuFocusGained);
		}
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		if (m_EditableEntityUIConfig)
		{
			SCR_EditableEntityUIRuleTracker ruleTracker;
			foreach (SCR_EntitiesEditorUIRule rule: m_EditableEntityUIConfig.GetRules())
			{
				if (m_RuleTrackers.Find(rule, ruleTracker))
					delete ruleTracker;
			}
		}
		SCR_EntitiesManagerEditorComponent entityManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entityManager)
		{
			foreach (SCR_BaseEntitiesEditorUIEffect effect: m_aEffects)
			{
				EEditableEntityState state = effect.GetState();
				if (state == 0)
					continue;
				
				SCR_BaseEditableEntityFilter stateManager = entityManager.GetFilter(state);
				if (!stateManager)
					continue;
				
				if (stateManager && stateManager.GetOnChanged())
					stateManager.GetOnChanged().Remove(OnEffect);
			}
		}
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_Type")]
class SCR_EditableEntityUISlotSize
{
	[Attribute("0", UIWidgets.ComboBox, "Entity type.", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	private EEditableEntityType m_Type;
	
	[Attribute("48")]
	private int m_iSize;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EEditableEntityType GetType()
	{
		return m_Type;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetSize()
	{
		return m_iSize;
	}
}


//! @ingroup Editor_UI Editor_UI_Components

class SCR_EntitiesToolbarEditorUIComponent : SCR_BaseToolbarEditorUIComponent
{
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, "Which entity types are accepted", enums: SCR_Enum.GetList(EEditableEntityType, new ParamEnum("<ALL>", "-1")) )]
	protected EEditableEntityType m_Type;
	
	[Attribute("0", UIWidgets.ComboBox,  "List of entity types who are ignored even if 'm_Type' is valid.", "", ParamEnumArray.FromEnum(EEditableEntityType) )]
	protected ref array<EEditableEntityType> m_aTypeBlackList;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Toolbar will show all entities of this state", enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_State;
	
	[Attribute(desc: "Order offsets applied to individual states. Can be used to sort items beyond just their type.")]
	protected ref array<ref SCR_EntityToolbarStateOffset> m_aStateOffsets;
	
	[Attribute("0", UIWidgets.Flags, "Entities with following flags will *NOT* not be shown.", enums: ParamEnumArray.FromEnum(EEditableEntityFlag))]
	protected EEditableEntityFlag m_FlagsBlacklist;
	
	[Attribute(desc: "When enabled, only direct children of currently selected layer will be shown.")]
	protected bool m_bOnlyDirectChildren;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Order in which entities will be sorted", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	protected ref array<EEditableEntityType> m_aTypeOrder;
	
	[Attribute(desc: "Name of tab widget which controls entity types.")]
	protected string m_sTypeTabsWidgetName;
	
	[Attribute(desc: "List of entity types matching tabs defined in 'm_sTypeTabsWidgetName' widget. Also includes black list which are ignored even if type is valid.")]
	protected ref array<ref SCR_EntityToolbarTypeList> m_aEntityTypeTabs;
	
	protected bool m_bHasTabs;
	protected bool m_bHasEntities;
	protected ref SCR_SortedArray<SCR_EditableEntityComponent> m_Entities = new SCR_SortedArray<SCR_EditableEntityComponent>();
	
	protected ref map<SCR_EditableEntityComponent, Widget> m_ItemsMap = new map<SCR_EditableEntityComponent, Widget>();
	protected SCR_HoverEditableEntityFilter m_HoverFilter;
	protected SCR_EditableEntityComponent m_HoverEntity;
	protected SCR_EditableEntityComponent m_RepeatEntity;
	protected SCR_EditableEntitySlotManagerUIComponent m_SlotManager;
	protected SCR_LayersEditorComponent m_LayersManager;
	protected SCR_TabViewComponent m_TabView;
	protected int m_iTab;

	bool m_queuedRefresh = false;
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateItem(SCR_EditableEntityComponent entity)
	{
		Widget itemWidget;
		SCR_BaseToolbarItemEditorUIComponent item;
		if (!CreateItem(itemWidget, item))
			return null;
		
		SCR_EditableEntityLinkUIComponent entityLink = SCR_EditableEntityLinkUIComponent.Cast(itemWidget.FindHandler(SCR_EditableEntityLinkUIComponent));
		if (!entityLink)
		{
			Print(string.Format("'%1' must contain SCR_EditableEntityLinkUIComponent!", m_ItemLayout), LogLevel.WARNING);
			itemWidget.RemoveFromHierarchy();
			return null;
		}
		
		SCR_EntityToolbarItemEditorUIComponent entityItem = SCR_EntityToolbarItemEditorUIComponent.Cast(item);
		if (entityItem)
			entityItem.SetEntity(entity, itemWidget, m_SlotManager);
		
		entityLink.SetLinkedEntity(entity);
		m_ItemsMap.Insert(entity, itemWidget);
		itemWidget.SetName(entity.ToString());
		return itemWidget;
	}

	//Many actions can request a refresh for the GM toolbar, but we should only refresh it once per frame.
	protected void QueueRefresh()
	{
		if(m_queuedRefresh)
			return;
		
		GetGame().GetCallqueue().CallLater(Refresh);
		m_queuedRefresh = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Clear()
	{
		m_ItemsMap.Clear();
		m_SlotManager.ClearSlots();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		QueueRefresh();
	}

	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorSetSelection()
	{
		SCR_EntitiesToolbarEditorUIComponent linkedComponent = SCR_EntitiesToolbarEditorUIComponent.Cast(m_LinkedComponent);
		if (linkedComponent)
			linkedComponent.m_RepeatEntity = m_HoverEntity;
		
		CloseDialog();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTypeTab(SCR_TabViewComponent tabView, SCR_TabViewContent tabContent, int index)
	{
		m_iTab = index;
		m_Type = m_aEntityTypeTabs[index].GetType();
		m_aEntityTypeTabs[index].GetTypeBlackList(m_aTypeBlackList);

		QueueRefresh();
	}

	//------------------------------------------------------------------------------------------------
	override protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		Clear();
		
		indexEnd = Math.Min(indexEnd, m_Entities.Count());
		for (int i = indexStart; i < indexEnd; i++)
		{
			CreateItem(m_Entities[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override bool IsUnique()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void SetToolbarVisible(bool show)
	{
		super.SetToolbarVisible(show || m_bHasEntities);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Refresh()
	{
		if (m_State != -1)
		{
			SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(m_State, true);
			
			set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
			int count = filter.GetEntities(entities);
			
			m_bHasEntities = false;
			m_Entities.Clear();
			foreach (SCR_EditableEntityComponent entity: entities)
			{
				if (entity
					&& (!m_aTypeBlackList.Contains(entity.GetEntityType()))
					&& (m_FlagsBlacklist == 0 || (entity.GetEntityFlags() & m_FlagsBlacklist) == 0)
				)
				{					
					//--- Add only entities of desired type
					if (m_Type == -1 || entity.GetEntityType() == m_Type)
					{
						int order = m_aTypeOrder.Find(entity.GetEntityType());
						if (order == -1)
							order = 100 + entity.GetEntityType();
						
						foreach (SCR_EntityToolbarStateOffset stateOffset: m_aStateOffsets)
						{
							if (entity.HasEntityState(stateOffset.m_State))
								order += stateOffset.m_iOrderOffset;
						}
						
						m_Entities.Insert(order, entity);
						m_bHasEntities = true;
					}
					else
					{
						//--- Has entities even if tabs prevent them from being shown
						m_bHasEntities |= m_bHasTabs;
					}
				}
			}
			
			if (m_Pagination)
				m_Pagination.SetEntryCount(m_Entities.Count());
		}
		
		super.Refresh();
		m_queuedRefresh = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnRepeat()
	{
		if (!m_RepeatEntity)
			return;
		
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED); //--- ToDo: Don't hardcode?
		if (!filter)
			return;
		
		filter.Replace(m_RepeatEntity);
	}

	//------------------------------------------------------------------------------------------------
	override protected void CopyPage(SCR_DialogEditorUIComponent linkedComponent)
	{
		SCR_EntitiesToolbarEditorUIComponent toolbar = SCR_EntitiesToolbarEditorUIComponent.Cast(linkedComponent);
		if (m_TabView)
			m_TabView.ShowTab(toolbar.m_iTab);
		
		super.CopyPage(linkedComponent);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float timeSlice)
	{
		super.OnMenuUpdate(timeSlice);
		m_HoverFilter.SetEntityUnderCursor(m_HoverEntity, true);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		SCR_EditableEntityLinkUIComponent link = SCR_EditableEntityLinkUIComponent.Cast(w.FindHandler(SCR_EditableEntityLinkUIComponent));
		if (link)
		{
			bool canTeleport = m_HoverEntity != null; //--- Don't teleport to selected entity when the list was just opened, would be impractical
			m_HoverEntity = link.GetLinkedEntity();
			
			vector pos;
			if (canTeleport && m_HoverEntity.GetPos(pos))
			{
				SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
				if (camera)
				{
					SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
					if (teleportComponent)
						teleportComponent.TeleportCamera(pos);
				}
			}
		}

		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_SlotManager = SCR_EditableEntitySlotManagerUIComponent.Cast(w.FindHandler(SCR_EditableEntitySlotManagerUIComponent));
		m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		
		m_HoverFilter = SCR_HoverEditableEntityFilter.Cast(SCR_HoverEditableEntityFilter.GetInstance(EEditableEntityState.HOVER, true));
		if (!m_HoverFilter)
			return;
		
		if (m_State != -1)
		{
			SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(m_State, true);
			if (filter)
				filter.GetOnChanged().Insert(OnChanged);
		}

		foreach (SCR_EntityToolbarStateOffset stateOffset: m_aStateOffsets)
		{
			SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(stateOffset.m_State, true);
			if (filter)
				filter.GetOnChanged().Insert(OnChanged);
		}
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.AddActionListener("EditorSetSelection", EActionTrigger.DOWN, OnEditorSetSelection);
		
		if (m_sTypeTabsWidgetName)
		{
			Widget typeTabsWidget = w.FindAnyWidget(m_sTypeTabsWidgetName);
			if (typeTabsWidget)
			{
				m_bHasTabs = true;
				m_TabView = SCR_TabViewComponent.Cast(typeTabsWidget.FindHandler(SCR_TabViewComponent));

				m_TabView.GetOnContentSelect().Insert(OnTypeTab);
				
				//Init tab
				if (m_aEntityTypeTabs.Count() > 0)
					OnTypeTab(null, null, 0);
			}
		}
		
		super.HandlerAttachedScripted(w);
		
		if (m_bIsInDialog)
		{
			SCR_TransformingEditorComponent transformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
			if (transformingManager)
				transformingManager.GetOnTransformationStart().Insert(CloseDialog);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_State != -1)
		{
			SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(m_State);
			if (filter)
				filter.GetOnChanged().Remove(OnChanged);
		}
		foreach (SCR_EntityToolbarStateOffset stateOffset: m_aStateOffsets)
		{
			SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(stateOffset.m_State);
			if (filter)
				filter.GetOnChanged().Insert(OnChanged);
		}
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("EditorSetSelection", EActionTrigger.DOWN, OnEditorSetSelection);
		
		if (m_bIsInDialog)
		{
			SCR_TransformingEditorComponent transformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
			if (transformingManager)
				transformingManager.GetOnTransformationStart().Remove(CloseDialog);
		}
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_Type")]
class SCR_EntityToolbarTypeList
{
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, "Entity type which are shown in tabs defined in 'm_sTypeTabsWidgetName' widget.", enums: SCR_Enum.GetList(EEditableEntityType, new ParamEnum("<ALL>", "-1")) )]
	protected  EEditableEntityType m_Type;
	
	[Attribute("0", UIWidgets.ComboBox,  "List of entity types who are ignored even if 'm_Type' is valid.", "", ParamEnumArray.FromEnum(EEditableEntityType) )]
	protected ref array<EEditableEntityType> m_aTypeBlackList;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EEditableEntityType GetType()
	{
		return m_Type;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] blackList
	void GetTypeBlackList(out notnull array<EEditableEntityType> blackList)
	{
		blackList = m_aTypeBlackList;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
class SCR_EntityToolbarStateOffset
{
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, "Entity type which are shown in tabs defined in 'm_sTypeTabsWidgetName' widget.", enums: ParamEnumArray.FromEnum(EEditableEntityState) )]
	EEditableEntityType m_State;
	
	[Attribute()]
	int m_iOrderOffset;
}

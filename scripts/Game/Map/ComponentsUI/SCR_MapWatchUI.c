class SCR_MapWatchUI : SCR_MapRTWBaseUI
{
	protected static const string ICON_NAME = "watch";
	
	protected SCR_WristwatchComponent m_WristwatchComp;
	protected string m_sPrefabResource;
			
	//------------------------------------------------------------------------------------------------
	override void SetWidgetNames()
	{
		WIDGET_NAME = "WatchFrame";
		RT_WIDGET_NAME = "WatchRTW";
		WORLD_NAME = "WatchUIWorld";
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitPositionVectors()
	{
		m_WristwatchComp = SCR_WristwatchComponent.Cast( m_RTEntity.FindComponent(SCR_WristwatchComponent) );
		
		m_vPrefabPos = "0 0 0";
		m_vCameraPos = "0 0.14 0";
		m_vCameraAngle = "0 -90 0";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update for the wristwatch entity within preview world
	protected void UpdatePreviewEntity()
	{								
		if (m_WristwatchComp)
		{						
			// activate, apply proc anims
			if (m_WristwatchComp.GetMode() != EGadgetMode.IN_HAND)
				m_WristwatchComp.SetMapMode();
			
			// tick the frame
			m_WristwatchComp.UpdateTime();
		}
		
		// update		
		BaseWorld previewWorld = m_RTWorld.GetRef();
		previewWorld.UpdateEntities();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputQuickBind(float value, EActionTrigger reason)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && IsToolMenuFocused())
			return;	
		SetVisible(!m_bIsVisible);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetPrefabResource()
	{
		return m_sPrefabResource;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set wristwatch visibility
	//! \param[in] visible is true/false switch
	override void SetVisible(bool visible)
	{
		if (visible)
		{			
			IEntity wristwatch = FindRelatedGadget();
			if (!wristwatch)
			{
				super.SetVisible(false);
				return;
			}
			
			SCR_WristwatchComponent watchComp = SCR_WristwatchComponent.Cast(wristwatch.FindComponent(SCR_WristwatchComponent));
			if (!watchComp)
				return;
			
			m_sPrefabResource = watchComp.GetMapPrefabResource();
			
			super.SetVisible(visible);
			if (!m_RTWorld || !m_RTEntity)
				return;
					
			// Start anim
			ScriptCallQueue queue = GetGame().GetCallqueue();
			if (queue)
				queue.CallLater(UpdatePreviewEntity, 0, true);
		}
		else 
		{	
			ScriptCallQueue queue = GetGame().GetCallqueue();
			if (queue)
				queue.Remove(UpdatePreviewEntity);
			
			super.SetVisible(visible);
		}
	}
				
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_MapToolMenuUI toolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (toolMenu)
		{
			m_ToolMenuEntry = toolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, ICON_NAME, 12, m_bIsExclusive); // add to menu
			m_ToolMenuEntry.m_OnClick.Insert(ToggleVisible);
			
			GetGame().GetInputManager().AddActionListener("MapToolWatch", EActionTrigger.DOWN, OnInputQuickBind);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{		
		// Drag watch
		if (m_bIsVisible && m_bIsDragged)
		{
			// save position for map reopen
			WorkspaceWidget workspace = g_Game.GetWorkspace();
			m_wFrame.GetScreenPos(m_fPosX, m_fPosY);
			m_fPosX = workspace.DPIUnscale(m_fPosX);
			m_fPosY = workspace.DPIUnscale(m_fPosY);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MapWatchUI()
	{
		m_eGadgetType = EGadgetType.WRISTWATCH;
	}
}

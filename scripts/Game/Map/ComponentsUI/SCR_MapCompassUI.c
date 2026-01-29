//! Compass in map UI
class SCR_MapCompassUI : SCR_MapRTWBaseUI
{		
	protected static const string ICON_NAME = "compass";
	
	protected SCR_CompassComponent m_CompassComp;
	protected string m_sPrefabResource;

	//------------------------------------------------------------------------------------------------
	override void SetWidgetNames()
	{
		WIDGET_NAME = "CompassFrame";
		RT_WIDGET_NAME = "CompRTW";
		WORLD_NAME = "CompassUIWorld";
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitPositionVectors()
	{
		m_CompassComp = SCR_CompassComponent.Cast( m_RTEntity.FindComponent(SCR_CompassComponent) );
		
		m_vPrefabPos = "0 0 0";
		m_vCameraPos = m_CompassComp.GetMapCamPosition();
		m_vCameraAngle = "0 -90 0";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update for the compass entity within preview world
	protected void UpdateCompassEntity()
	{								
		if (m_CompassComp)
		{
			ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			if (!player || !player.GetCharacterController())
				return;
			
			vector anglesCompass = vector.Zero;
			anglesCompass[1] = player.GetCharacterController().GetInputContext().GetHeadingAngle() * Math.RAD2DEG;
			
			// if player is inside the vehicle - check for the vehicle rotation instead
			// GetHeadingAngle returns the wrong value when in vehicle and map is opened 
			if (player.IsInVehicle())
			{
				anglesCompass[1] = player.GetAngles()[1];
			}
			
			m_RTEntity.SetAngles(anglesCompass);
			
			// activate, apply proc anims
			if (m_CompassComp.GetMode() != EGadgetMode.IN_HAND)
				m_CompassComp.SetMapMode();
			
			// tick the frame
			float tick = System.GetFrameTimeS();
			m_CompassComp.Update( tick );
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
	override void SetVisible(bool visible)
	{		
		if (visible)
		{
			IEntity compass = FindRelatedGadget();
			if (!compass)
			{
				super.SetVisible(false);
				return;
			}
			
			SCR_CompassComponent compassComp = SCR_CompassComponent.Cast(compass.FindComponent(SCR_CompassComponent));
			if (!compassComp)
				return;
			
			m_sPrefabResource = compassComp.GetMapPrefabResource();
			
			super.SetVisible(visible);
			if (!m_RTWorld || !m_RTEntity)
				return;
					
			// Start anim
			ScriptCallQueue queue = GetGame().GetCallqueue();
			if (queue)
				queue.CallLater(UpdateCompassEntity, 0, true);
			
			UpdateCompassEntity();
			m_CompassComp.Init2DMapCompass();
		}
		else 
		{
			// stop anim
			ScriptCallQueue queue = GetGame().GetCallqueue();
			if (queue)
				queue.Remove(UpdateCompassEntity);
			
			super.SetVisible(visible);
		}
	}
									
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_MapToolMenuUI toolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (toolMenu)
		{
			m_ToolMenuEntry = toolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, ICON_NAME, 11, m_bIsExclusive);
			m_ToolMenuEntry.m_OnClick.Insert(ToggleVisible);
			
			GetGame().GetInputManager().AddActionListener("MapToolCompass", EActionTrigger.DOWN, OnInputQuickBind);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{		
		// Drag compass
		if (m_bIsVisible && m_bIsDragged)
		{
			// apply shake
			m_CompassComp.DragMapCompass();
			
			// save compass position for map reopen
			WorkspaceWidget workspace = g_Game.GetWorkspace();
			m_wFrame.GetScreenPos(m_fPosX, m_fPosY);
			m_fPosX = workspace.DPIUnscale(m_fPosX);
			m_fPosY = workspace.DPIUnscale(m_fPosY);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MapCompassUI()
	{
		m_eGadgetType = EGadgetType.COMPASS;
	}
}

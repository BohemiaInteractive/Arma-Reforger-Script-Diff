// Script File
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_SpeedBoostUpManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("1")]
	private bool m_bSpeedBoostUpEnabled;

	[Attribute("1")]
	private bool m_bHoldBoostUpEnabled;

	[Attribute("1")]
	private bool m_bDisableOnSprint;
	
	[Attribute("25")]
	private float m_iHeightMax;

	[Attribute("2")]
	private float m_HeightMultiplier;

	[Attribute("3")]
	private float m_fHoldSpeedMultiplier;

	[Attribute("2")]
	private float m_fHoldBoostMax;
	
	private float m_HoldMultiplier = 0;
	
	private bool m_bDebugEnabled;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		m_bDebugEnabled = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_SPEED_BOOST_UP);
		
		if (!m_bDebugEnabled && param.velocityOriginal[1] <= 0)
			return;
		
		DrawDebug1(param);
		
		if (!param.isManualInputEnabled)
			return;
		
		vector pos = CoordFromCamera(param.transform[3]);
		float surfaceY = param.world.GetSurfaceY(pos[0], pos[2]);
		if (pos[1] > 0)
			surfaceY = Math.Max(surfaceY, 0); //--- When above ground, use ASL, not ATL height
		float height = pos[1] - surfaceY;
		
		float multiplierVertical = 0;
		
		InputManager inputManager = GetGame().GetInputManager();
		
		if (m_bHoldBoostUpEnabled && height < m_iHeightMax && inputManager.GetActionValue("ManualCameraMoveVertical") > 0)
			m_HoldMultiplier = Math.Min( m_HoldMultiplier + (param.timeSlice * m_fHoldSpeedMultiplier), m_fHoldBoostMax);
		else
			m_HoldMultiplier = 0;
		
		if (height < m_iHeightMax && param.velocityOriginal[1] > 0)
			multiplierVertical = (1 - (height / m_iHeightMax)) * m_HeightMultiplier;
		
		if (m_bHoldBoostUpEnabled)
			multiplierVertical *= m_HoldMultiplier;
		
		DrawDebug2(height);
		
		if (!m_bSpeedBoostUpEnabled || (m_bDisableOnSprint && inputManager.GetActionValue("ManualCameraSpeedBoost") > 0)) 
		{
			DrawDebug3(param);
			return;
		}
		
		param.multiplier[1] = param.multiplier[1] * (1 + multiplierVertical);
		
		DrawDebug4(param, multiplierVertical);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_SPEED_BOOST_UP, "", "Speed boost up debug", "Manual Camera", false);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	private void DrawDebug1(SCR_ManualCameraParam param)
	{
		if (m_bDebugEnabled)
		{
			DbgUI.Begin("Camera speed boost up debug", 0, 0);
		
			DbgUI.Check("Enable ground up boost", m_bSpeedBoostUpEnabled);			
			DbgUI.Check("Enable hold delay+boost", m_bHoldBoostUpEnabled);
			DbgUI.Check("Disable boost up on sprint", m_bDisableOnSprint);
			
			DbgUI.SliderFloat("HeigthMax", m_iHeightMax, 10, 100);
			DbgUI.SameLine();
			DbgUI.Text(m_iHeightMax.ToString());
			
			DbgUI.SliderFloat("HeigtMultiplier", m_HeightMultiplier, 0, 10);
			DbgUI.SameLine();
			DbgUI.Text(m_HeightMultiplier.ToString());
			
			if (m_bHoldBoostUpEnabled)
			{
				DbgUI.SliderFloat("HoldBoostMax", m_fHoldBoostMax, 1, 10);
				DbgUI.SameLine();
				DbgUI.Text(m_fHoldBoostMax.ToString());
			
				DbgUI.SliderFloat("HoldSpeedupMultiplier", m_fHoldSpeedMultiplier, 1, 10);
				DbgUI.SameLine();
				DbgUI.Text(m_fHoldSpeedMultiplier.ToString());
			}
		
			DbgUI.Text("VelY: " + param.velocityOriginal[1]);
			DbgUI.PlotLive("VelY", 200, 100, param.velocityOriginal[1], 100, 100);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void DrawDebug2(float height)
	{
		if (m_bDebugEnabled)
		{
			DbgUI.Text("Height: " + height);
			DbgUI.PlotLive("Height", 200, 100, height, 100, 100);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void DrawDebug3(SCR_ManualCameraParam param)
	{
		if (m_bDebugEnabled)
		{
			DbgUI.Text("Param.MultiplierHorizontal: " + param.multiplier[0]);
			DbgUI.Text("Param.MultiplierVertical: " + param.multiplier[1]);
			DbgUI.PlotLive("Param.MultiplierHorizontal", 200, 100, param.multiplier[0], 100, 100);
			DbgUI.SameLine();
			DbgUI.PlotLive("Param.MultiplierVertical", 200, 100, param.multiplier[1], 100, 100);
			
			DbgUI.End();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void DrawDebug4(SCR_ManualCameraParam param, float multiplierVertical)
	{
		if (m_bDebugEnabled)
		{
			DbgUI.Text("Param.MultiplierHorizontal: " + param.multiplier[0]);
			DbgUI.Text("Param.MultiplierVertical: " + param.multiplier[1]);
			DbgUI.PlotLive("Param.MultiplierHorizontal", 200, 100, param.multiplier[0], 100, 100);
			DbgUI.SameLine();
			DbgUI.PlotLive("Param.MultiplierVertical", 200, 100, param.multiplier[1], 100, 100);
			
			DbgUI.Text("NultiplierVertical: " + (1 + multiplierVertical));
			DbgUI.PlotLive("NultiplierVertical", 200, 100, (1 + multiplierVertical), 100, 100);
			
			if (m_bHoldBoostUpEnabled)	
			{
				DbgUI.Text("HoldMultplier: " + (1 + m_HoldMultiplier));
				DbgUI.PlotLive("HoldMultplier", 200, 100, (1 + m_HoldMultiplier), 100, 100);
			}

			DbgUI.End();
		}
	}
}

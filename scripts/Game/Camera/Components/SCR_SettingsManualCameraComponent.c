//! @ingroup ManualCamera

//! Load global settings
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_SettingsManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EManualCameraFlag), category: "Manual Camera")]
	protected EManualCameraFlag m_Flags;
	
	[Attribute(desc: "When enabled, the cursor will be hidden when rotation with modifier is disabled.")]
	protected bool m_bCanHideCursor;
	
	protected float m_Multiplier = 1;
	
	//------------------------------------------------------------------------------------------------
	protected void LoadSettings()
	{
		SCR_ManualCameraSettings settings = new SCR_ManualCameraSettings();
		BaseContainerTools.WriteToInstance(settings, GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings"));
		
		m_Multiplier = settings.m_fCameraSpeedCoef;
		if (m_Multiplier == 0)
			m_Multiplier = 1;
		
		if (settings.m_bCameraMoveATL)
		{
			m_Flags = m_Flags | EManualCameraFlag.MOVE_ATL;

			if (settings.m_bCameraSpeedATL)
				m_Flags = m_Flags | EManualCameraFlag.SPEED_ATL;
			else
				m_Flags = m_Flags & ~EManualCameraFlag.SPEED_ATL;
		}
		else
		{
			m_Flags = m_Flags & ~EManualCameraFlag.MOVE_ATL;
			m_Flags = m_Flags & ~EManualCameraFlag.SPEED_ATL;
		}
		
		if (settings.m_bCameraRotateWithModifier)
			m_Flags = m_Flags | EManualCameraFlag.ROTATE_MODIFIER;
		else
			m_Flags = m_Flags & ~EManualCameraFlag.ROTATE_MODIFIER;
		
		if (settings.m_bCameraAboveTerrain)
			m_Flags = m_Flags | EManualCameraFlag.ABOVE_TERRAIN;
		else
			m_Flags = m_Flags & ~EManualCameraFlag.ABOVE_TERRAIN;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		param.multiplier *= m_Multiplier;
		param.flag = param.flag | m_Flags;
		
//		// use InputManager! instead of ShowCursor
//		if (m_bCanHideCursor)
//			ShowCursor(!param.isManualInputEnabled || (param.flag & EManualCameraFlag.ROTATE_MODIFIER));
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		LoadSettings();		
		GetGame().OnUserSettingsChangedInvoker().Insert(LoadSettings);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		GetGame().OnUserSettingsChangedInvoker().Remove(LoadSettings);
	}
}

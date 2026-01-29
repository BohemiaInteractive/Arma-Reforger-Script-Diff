//! @ingroup ManualCamera

//! Control player's avatar in the camera.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_ControlPlayerCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "MenuContext", desc: "Context which will block out character movement context debug menu entry for character movement in camera is disabled.")]
	protected string m_sBlockingContext;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_CONTROL_PLAYER))
		{
			//--- Block camera movement
			param.isManualInputEnabled = false;
		}
		else
		{
			//--- Block character movement
			m_InputManager.ActivateContext(m_sBlockingContext);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_CONTROL_PLAYER, "", "Control player", "Manual Camera", false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_CONTROL_PLAYER);
	}
};

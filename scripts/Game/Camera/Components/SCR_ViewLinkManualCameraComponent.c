//! @ingroup ManualCamera

//! Teleporting camera to position defined by Workbench link
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_ViewLinkManualCameraComponent : SCR_BaseManualCameraComponent
{
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_VIEW_LINK))
		{
			const string prefix = "enfusion://WorldEditor/"; // TODO: check for good const usage
			string link = System.ImportFromClipboard();
			if (link.StartsWith(prefix))
			{
				array<string> values = {};
				link.Split(";", values, false);
				if (values.Count() >= 2)
				{
					string fullLink = GetGame().GetWorldFile();
					int begin = fullLink.IndexOf(":") + 1;
					string worldPath = fullLink.Substring(begin, fullLink.Length() - begin);
					if (values[0] == prefix + worldPath)
					{
						array<string> valuesPos = {}, valuesDir = {};
						values[1].Split(",", valuesPos, false);
						values[2].Split(",", valuesDir, false);
						
						vector pos = Vector(valuesPos[0].ToFloat(), valuesPos[1].ToFloat(), valuesPos[2].ToFloat());
						vector dir = Vector(valuesDir[1].ToFloat(), valuesDir[0].ToFloat(), valuesDir[2].ToFloat());
						
						vector transform[4];
						Math3D.AnglesToMatrix(dir, transform);
						transform[3] = pos;
						GetCameraEntity().SetWorldTransform(transform);
						
						PrintFormat("View link applied: %1", link);
					}
				}
			}
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_VIEW_LINK, 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_VIEW_LINK, "lshift+lalt+l", "Paste view link", "Manual Camera");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_VIEW_LINK);
	}
}

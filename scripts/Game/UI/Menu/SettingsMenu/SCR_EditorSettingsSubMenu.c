
class SCR_EditorSettingsSubMenu: SCR_SettingsSubMenuBase
{
	SCR_SelectionWidgetComponent m_wHorizontalCameraSpeedCheckbox;
	SCR_SelectionWidgetComponent m_wATLSpeedCheckbox;
	SCR_SliderComponent m_wSpeedCoefSlider;
	
	private static const string GAME_MASTER = "Game Master";
	private static const string LAYER_EDITING = "m_bLayerEditing";
	private static const string PREVIEW_VERTICAL_SNAP = "m_PreviewVerticalSnap";
	private static const string PREVIEW_VERTICLE_MODE = "m_PreviewVerticleMode";
	private static const string CAMERA_MOVE_ATL = "m_bCameraMoveATL";
	private static const string CAMERA_SPEED_ATL = "m_bCameraSpeedATL";
	private static const string CAMERA_SPEED_COEF = "m_fCameraSpeedCoef";
	private static const string CAMERA_ABOVE_TERRAIN = "m_bCameraAboveTerrain";
	private static const string CAMERA_ROTATE_WITH_MODIFIER = "m_bCameraRotateWithModifier";
	private static const string SHOW_IDENTITY_BIO_TOOLTIP = "m_bShowIdentityBioTooltip";
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		BaseContainer editorCameraSettings = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		if (!editorCameraSettings) 
			return;
		if (!editorSettings) 
			return;
		
		//Layer Editing
		SCR_SelectionWidgetComponent checkBox = SCR_SelectionWidgetComponent.GetSelectionComponent("LayerEditing", m_wRoot);
		if (checkBox)
		{
			bool state;
			editorSettings.Get(LAYER_EDITING, state);
			
			checkBox.SetCurrentItem(state, false, false);
			checkBox.m_OnChanged.Insert(SetEnableLayerEditing);
		}
		else 
		{
			Print("Editor setting 'LayerEditing' not found", LogLevel.WARNING);
		}
		
		//Snap to terrain
		checkBox = SCR_SelectionWidgetComponent.GetSelectionComponent("VerticalSnap", m_wRoot);
		if (checkBox)
		{
			int value;
			editorSettings.Get(PREVIEW_VERTICAL_SNAP, value);
			
			checkBox.SetCurrentItem(value, false, false);
			checkBox.m_OnChanged.Insert(SetVerticalSnap);
		}
		else 
		{
			Print("Editor setting 'VerticalSnap' not found", LogLevel.WARNING);
		}
		
		//Orient to terrain
		checkBox = SCR_SelectionWidgetComponent.GetSelectionComponent("VerticalOrientation", m_wRoot);
		if (checkBox)
		{
			int value;
			editorSettings.Get(PREVIEW_VERTICLE_MODE, value);
			
			checkBox.SetCurrentItem(value >> 1, false, false); //--- Shift the value, because it's a flag
			checkBox.m_OnChanged.Insert(SetVerticalMode);
		}	
		else 
		{
			Print("Editor setting 'VerticalOrientation' not found", LogLevel.WARNING);
		}

		m_wHorizontalCameraSpeedCheckbox = SCR_SelectionWidgetComponent.GetSelectionComponent("HorizontalCameraSpeed", m_wRoot);
		if (m_wHorizontalCameraSpeedCheckbox)
		{
			bool value;
			editorCameraSettings.Get(CAMERA_MOVE_ATL, value);
			
			m_wHorizontalCameraSpeedCheckbox.SetCurrentItem(value, false, false);
			m_wHorizontalCameraSpeedCheckbox.m_OnChanged.Insert(SetHorizontalCameraSpeed);
		}
		else 
		{
			Print("Editor setting 'HorizontalCameraSpeed' not found", LogLevel.WARNING);
		}
		
		m_wATLSpeedCheckbox = SCR_SelectionWidgetComponent.GetSelectionComponent("ATLSpeed", m_wRoot);
		if (m_wATLSpeedCheckbox)
		{
			bool value;
			editorCameraSettings.Get(CAMERA_SPEED_ATL, value);
			
			m_wATLSpeedCheckbox.SetCurrentItem(value, false, false);
			m_wATLSpeedCheckbox.SetEnabled(m_wHorizontalCameraSpeedCheckbox.GetCurrentIndex() > 0);
			
			m_wATLSpeedCheckbox.m_OnChanged.Insert(SetATLSpeed);
			
			//Indent
			TextWidget text = TextWidget.Cast(m_wATLSpeedCheckbox.GetLabelWidget());
			
			if (text)
				text.SetTextOffset(20, 0);
		}
		else 
		{
			Print("Editor setting 'ATLSpeed' not found", LogLevel.WARNING);
		}
		
		//Todo: Add formatting. Add slider UI
		m_wSpeedCoefSlider = SCR_SliderComponent.GetSliderComponent("SpeedCoef", m_wRoot);
		if (m_wSpeedCoefSlider)
		{
			m_wSpeedCoefSlider.SetSliderSettings(0.1, 10, 0.05, "#AR-ValueUnit_Short_Multiplier");
			float value;
			editorCameraSettings.Get(CAMERA_SPEED_COEF, value);
			
			m_wSpeedCoefSlider.SetValue(value);
			m_wSpeedCoefSlider.ShowCustomValue(GetSliderText(value, 2));
			m_wSpeedCoefSlider.m_OnChanged.Insert(SetSpeedCoef);
		}
		else 
		{
			Print("Editor setting 'SpeedCoef' not found", LogLevel.WARNING);
		}
		
		SCR_SelectionWidgetComponent wb_CameraAboveTerrain = SCR_SelectionWidgetComponent.GetSelectionComponent("WB_CameraAboveTerrain", m_wRoot);
		if (wb_CameraAboveTerrain)
		{
			bool value;
			editorCameraSettings.Get(CAMERA_ABOVE_TERRAIN, value);
			
			wb_CameraAboveTerrain.SetCurrentItem(value, false, false);
			wb_CameraAboveTerrain.m_OnChanged.Insert(SetCameraAboveTerrain);
		}
		else
		{
			Print("Editor setting 'WB_CameraAboveTerrain' not found", LogLevel.WARNING);
		}
		
		SCR_SelectionWidgetComponent wb_CameraRotateWithModifier = SCR_SelectionWidgetComponent.GetSelectionComponent("WB_CameraRotateWithModifier", m_wRoot);
		if (wb_CameraRotateWithModifier)
		{
			bool value;
			editorCameraSettings.Get(CAMERA_ROTATE_WITH_MODIFIER, value);
			
			wb_CameraRotateWithModifier.SetCurrentItem(value, false, false);
			wb_CameraRotateWithModifier.m_OnChanged.Insert(SetCameraRotateWithModifier);
		}
		else
		{
			Print("Editor setting 'm_bCameraRotateWithModifier' not found", LogLevel.WARNING);
		}
		
		SCR_SelectionWidgetComponent showIdentityTooltip = SCR_SelectionWidgetComponent.GetSelectionComponent("ShowBioTooltip", m_wRoot);
		if (showIdentityTooltip)
		{
			bool state;
			editorSettings.Get(SHOW_IDENTITY_BIO_TOOLTIP, state);
			
			showIdentityTooltip.SetCurrentItem(state, false, false);
			showIdentityTooltip.m_OnChanged.Insert(SetShowIdentityTooltip);
		}
		
		//Workbench only
		#ifndef WORKBENCH
		Widget wb_Seperator = m_wRoot.FindAnyWidget("WB_Separator");
		if (wb_Seperator) wb_Seperator.SetVisible(false);
		if (wb_CameraAboveTerrain) wb_CameraAboveTerrain.GetRootWidget().SetVisible(false);
		if (wb_CameraRotateWithModifier) wb_CameraRotateWithModifier.GetRootWidget().SetVisible(false);
		#endif
		
	}

	//------------------------------------------------------------------------------------------------
	void SetEnableLayerEditing(SCR_SelectionWidgetComponent checkBox, bool state)
	{
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (!editorSettings) 
			return;
		
		editorSettings.Set(LAYER_EDITING, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, LAYER_EDITING);
		
		GetGame().UserSettingsChanged();
	}
	
	void SetVerticalSnap(SCR_SelectionWidgetComponent checkBox, bool state)
	{	
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (!editorSettings) 
			return;
		
		editorSettings.Set(PREVIEW_VERTICAL_SNAP, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, PREVIEW_VERTICAL_SNAP);

		GetGame().UserSettingsChanged();
	}
	
	void SetVerticalMode(SCR_SelectionWidgetComponent checkBox, int state)
	{
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (!editorSettings) 
			return;
		
		state = 1 << state; //--- Shift the value, because it's a flag
		
		editorSettings.Set(PREVIEW_VERTICLE_MODE, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, PREVIEW_VERTICLE_MODE);
		
		GetGame().UserSettingsChanged();
		
		SCR_PreviewEntityEditorComponent previewComponent = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		if (!previewComponent) 
			return;
		
		previewComponent.SetVerticalMode(state);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHorizontalCameraSpeed(SCR_SelectionWidgetComponent checkBox, bool state)
	{
		m_wATLSpeedCheckbox.SetEnabled(state);
		
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		if (!settings) 
			return;

		settings.Set(CAMERA_MOVE_ATL, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, CAMERA_MOVE_ATL);
		
		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetATLSpeed(SCR_SelectionWidgetComponent checkBox, bool state)
	{
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		if (!settings) 
			return;

		settings.Set(CAMERA_SPEED_ATL, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, CAMERA_SPEED_ATL);

		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpeedCoef(SCR_SliderComponent slider, float value)
	{
		m_wSpeedCoefSlider.ShowCustomValue(GetSliderText(value, 2));
		
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		if (!settings) 
			return;

		settings.Set(CAMERA_SPEED_COEF, value);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, CAMERA_SPEED_COEF);
		
		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCameraAboveTerrain(SCR_SelectionWidgetComponent checkBox, bool state)
	{
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		if (!settings) 
			return;

		settings.Set(CAMERA_ABOVE_TERRAIN, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, CAMERA_ABOVE_TERRAIN);
			
		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCameraRotateWithModifier(SCR_SelectionWidgetComponent checkBox, bool state)
	{
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		if (!settings) 
			return;
		
		settings.Set(CAMERA_ROTATE_WITH_MODIFIER, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, CAMERA_ROTATE_WITH_MODIFIER);

		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void SetShowIdentityTooltip(SCR_SelectionWidgetComponent checkBox, bool state)
	{
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (!editorSettings) 
			return;
		
		editorSettings.Set(SHOW_IDENTITY_BIO_TOOLTIP, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, SHOW_IDENTITY_BIO_TOOLTIP);

		GetGame().UserSettingsChanged();
	}
	
	string GetSliderText(float value, int decimals)
	{
		float coef = Math.Pow(10, decimals);
		value = Math.Round(value * coef);
		string valueText = value.ToString();
		if (decimals > 0)
		{
			for (int i = 0, count = decimals - valueText.Length() + 1; i < count; i++)
			{
				valueText = "0" + valueText;
			}
			int length = valueText.Length();
			valueText = valueText.Substring(0, length - decimals) + "." + valueText.Substring(length - decimals, decimals);
		}

		return valueText;
	}
};
void ScriptInvokerButtonMethod(SCR_InputButtonComponent button, string actionName);
typedef func ScriptInvokerButtonMethod;
typedef ScriptInvokerBase<ScriptInvokerButtonMethod> ScriptInvokerButton;

class SCR_SaveLoadEntryComponent: SCR_ListMenuEntryComponent
{
	protected ref SCR_Save_EntryWidgets m_Widgets = new SCR_Save_EntryWidgets();
	
	protected ref ScriptInvokerButton m_OnSave;
	protected ref ScriptInvokerButton m_OnLoad;
	protected ref ScriptInvokerButton m_OnDelete;
	
	protected bool m_bCanSave;
	protected bool m_bCanLoad;
	protected bool m_bCanDelete;
		
	protected string m_sFileName;
	protected string m_sDisplayName;
	
	protected SaveGame m_Save;
	
	protected bool m_bHasIssue;	
	protected bool m_bIsFocused;
	protected bool m_bIsDownloaded;
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceChange(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (newDevice != EInputDeviceType.MOUSE && m_bIsFocused)
			UpdateAllWidgets(true);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		bool result = super.OnFocus(w, x, y);

		UpdateAllWidgets(true);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		bool result = super.OnFocusLost(w, x, y);

		UpdateAllWidgets(false);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Update visual state of line
	protected void UpdateAllWidgets(bool visibility)
	{
		m_bIsFocused = visibility;
		
		if (visibility)
			visibility = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE;
		
		m_Widgets.m_SaveButtonComponent0.SetVisible(visibility && m_bCanSave);
		m_Widgets.m_LoadButtonComponent0.SetVisible(visibility && m_bCanLoad);
		m_Widgets.m_DeleteButtonComponent0.SetVisible(visibility && m_bCanDelete);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickSave()
	{
		if (m_OnSave)
			m_OnSave.Invoke(null, "");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickLoad()
	{
		if (m_OnLoad)
			m_OnLoad.Invoke(null, "");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickDelete()
	{
		if (m_OnDelete)
			m_OnDelete.Invoke(null, "");
	}
	
	//------------------------------------------------------------------------------------------------
	// --- Setters ---
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetCanSave(bool state)
	{
		m_bCanSave = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCanLoad(bool state)
	{
		m_bCanLoad = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCanDelete(bool state)
	{
		m_bCanDelete = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScenarioDataVisible(bool state)
	{
		//m_Widgets.m_wScenarioInfoList.SetVisibile(state);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScenarioName(string name)
	{
		//m_Widgets.m_wScenarioName.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScenarioIcon(Color imageColor, string icon)
	{
		//m_Widgets.m_wScenarioImage.LoadImageTexture(0, icon);
		//m_Widgets.m_wScenarioImage.SetColor(imageColor);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDateTime(string date, string time)
	{
		m_Widgets.m_wPreviewDate.SetText(date);
		m_Widgets.m_wPreviewTime.SetText(time);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVersion(string version, bool isVersionCompatible)
	{
		m_Widgets.m_wEntryVersion.SetText(version);
		
		if (!isVersionCompatible)
			m_Widgets.m_wEntryVersion.SetColor(UIColors.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsWarning()
	{
		m_bHasIssue = true;
		m_Widgets.m_wSaveImage.SetColor(UIColors.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsDownloaded(bool state)
	{
		m_bIsDownloaded = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSaveData(SaveGame saveData)
	{
		m_Save = saveData;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSaveIcon(Color imageColor, string icon)
	{
		m_Widgets.m_wSaveImage.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
		m_Widgets.m_wSaveImage.SetColor(imageColor);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFileName(string name)
	{
		m_sFileName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDisplayName(string name)
	{
		m_sDisplayName = name;
		m_Widgets.m_wEntryName.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- Getters ---
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	bool CanOverrideSave()
	{
		return m_bCanSave;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanLoadSave()
	{
		return m_bCanLoad;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanDeleteSave()
	{
		return m_bCanDelete;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsError()
	{
		return m_bHasIssue;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDownloaded()
	{
		return m_bIsDownloaded;
	}
	
	//------------------------------------------------------------------------------------------------
	SaveGame GetSaveData()
	{
		return m_Save;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFileName()
	{
		return m_sFileName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		return m_sDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetSaveIcon()
	{
		return m_Widgets.m_wSaveImage;
	}
	
	//------------------------------------------------------------------------------------------------
	// --- Invokers ---
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerButton GetOnDeleteSave()
	{
		if (!m_OnDelete)
			m_OnDelete = new ScriptInvokerButton();

		return m_OnDelete;
	}

	// --- Invokers ---
	//------------------------------------------------------------------------------------------------
	ScriptInvokerButton GetOnLoadSave()
	{
		if (!m_OnLoad)
			m_OnLoad = new ScriptInvokerButton();

		return m_OnLoad;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerButton GetOnOverrideSave()
	{
		if (!m_OnSave)
			m_OnSave = new ScriptInvokerButton();

		return m_OnSave;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		m_Widgets.Init(w);
		
		m_Widgets.m_SaveButtonComponent0.SetVisible(false);
		m_Widgets.m_LoadButtonComponent0.SetVisible(false);
		m_Widgets.m_DeleteButtonComponent0.SetVisible(false);
		
		m_Widgets.m_SaveButtonComponent0.m_OnClicked.Insert(OnClickSave);
		m_Widgets.m_LoadButtonComponent0.m_OnClicked.Insert(OnClickLoad);
		m_Widgets.m_DeleteButtonComponent0.m_OnClicked.Insert(OnClickDelete);
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChange);
		
		UpdateAllWidgets(false);

		super.HandlerAttached(w);
	}
}
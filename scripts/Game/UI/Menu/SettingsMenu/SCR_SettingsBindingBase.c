class SCR_SettingsBindingBase
{
	protected string m_sModule;
	protected string m_sSubModule;
	protected string m_sName;
	protected string m_sWidgetName;
	protected BaseContainer m_Setting;
	protected int m_iItem;
	protected bool m_bUseGameUserSettings;

	protected ref ScriptInvoker m_OnEntryChanged;

	// Values to be filled
	protected float m_fMin, m_fMax, m_fStep, m_fCurrent;
	protected ref array<string> m_aNames = new array<string>();
	protected ref array<int> m_aVals = new array<int>();
	protected bool m_bHasLimits = false;

	//------------------------------------------------------------------------------------------------
	void SCR_SettingsBindingBase(string module, string name, string widgetName, string subModule = string.Empty)
	{
		m_sModule = module;
		m_sName = name;
		m_sWidgetName = widgetName;
		m_sSubModule = subModule;
	}

	//------------------------------------------------------------------------------------------------
	SCR_WLibComponentBase GetComponent(string name, Widget parent)
	{
		if (!parent)
			return null;

		Widget w = parent.FindAnyWidget(name);
		if (!w)
			return null;

		return SCR_WLibComponentBase.Cast(w.FindHandler(SCR_WLibComponentBase));
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEntryChangedInvoker()
	{
		if (!m_OnEntryChanged)
			m_OnEntryChanged = new ScriptInvoker();

		return m_OnEntryChanged;
	}

	//------------------------------------------------------------------------------------------------
	string GetWidgetName()
	{
		return m_sWidgetName;
	}

	//------------------------------------------------------------------------------------------------
	void LoadEntry(Widget parent, bool forceLoadSettings = false, bool addEntryChangedEventHandler = true)
	{
		if ((forceLoadSettings || !m_Setting) && !SetBaseContainer())
			return;

		if (!m_Setting || m_iItem < 0)
		{
			Print("Settings item not found: " + m_sName, LogLevel.WARNING);
			return;
		}

		SCR_WLibComponentBase comp = GetComponent(m_sWidgetName, parent);
		if (!comp)
			comp = GetComponent(m_sWidgetName, parent.GetParent()); // VideoSettings Quality Preset "shortcut"

		if (!comp)
		{
			Print("Settings widget not found: " + m_sWidgetName, LogLevel.WARNING);
			return;
		}

		if (InitSpinBox(comp, addEntryChangedEventHandler))
			return;

		if (InitSlider(comp, addEntryChangedEventHandler))
			return;

		if (InitCheckBox(comp, addEntryChangedEventHandler))
			return;

		if (InitComboBox(comp, addEntryChangedEventHandler))
			return;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SpinBoxComponent InitSpinBox(SCR_WLibComponentBase comp, bool addEntryChangedEventHandler)
	{
		SCR_SpinBoxComponent spin = SCR_SpinBoxComponent.Cast(comp);
		if (!spin)
			return null;

		int current;
		m_Setting.Get(m_sName, current);
		spin.SetCurrentItem(current);
		m_Setting.GetEnumValues(m_iItem, m_aNames, m_aVals);
		if (m_Setting.GetLimits(m_iItem, m_fMin, m_fMax, m_fStep) == 3)
		{
			m_bHasLimits = true;

			int intendedSize = Math.Max(m_aNames.Count(), m_fMax + 1);
			if (intendedSize != spin.GetNumItems())
				Print(string.Format("Settings: %1 has wrong number of items: %2/%3", m_sName, spin.GetNumItems(), intendedSize), LogLevel.WARNING);
		}

		if (addEntryChangedEventHandler)
			spin.m_OnChanged.Insert(OnSelectableChanged);

		return spin;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SliderComponent InitSlider(SCR_WLibComponentBase comp, bool addEntryChangedEventHandler)
	{
		SCR_SliderComponent slider = SCR_SliderComponent.Cast(comp);
		if (!slider)
			return null;

		float val;
		if (m_Setting.GetLimits(m_iItem, m_fMin, m_fMax, m_fStep) == 3)
			m_bHasLimits = true;
		m_Setting.Get(m_sName, val);
		slider.SetMin(m_fMin);
		slider.SetMax(m_fMax);
		slider.SetStep(m_fStep);
		slider.SetValue(val);

		if (addEntryChangedEventHandler)
			slider.GetOnChangedFinal().Insert(OnSliderChanged);

		return slider;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CheckboxComponent InitCheckBox(SCR_WLibComponentBase comp, bool addEntryChangedEventHandler)
	{
		SCR_CheckboxComponent check = SCR_CheckboxComponent.Cast(comp);
		if (!check)
			return null;

		bool checked;
		m_Setting.Get(m_sName, checked);
		check.SetChecked(checked);
		if (addEntryChangedEventHandler)
			check.m_OnChanged.Insert(OnCheckboxChanged);

		return check;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ComboBoxComponent InitComboBox(SCR_WLibComponentBase comp, bool addEntryChangedEventHandler)
	{
		SCR_ComboBoxComponent combo = SCR_ComboBoxComponent.Cast(comp);
		if (!combo)
			return null;

		int current;
		m_Setting.Get(m_sName, current);
		combo.SetCurrentItem(current);

		// Check for array size
		m_Setting.GetEnumValues(m_iItem, m_aNames, m_aVals);
		if (m_Setting.GetLimits(m_iItem, m_fMin, m_fMax, m_fStep) == 3)
		{
			m_bHasLimits = true;

			int intendedSize = Math.Max(m_aNames.Count(), m_fMax);
			if (intendedSize != combo.GetNumItems())
				Print(string.Format("Settings: %1 has wrong number of items: %2/%3", m_sName, combo.GetNumItems(), intendedSize), LogLevel.WARNING);
		}

		if (addEntryChangedEventHandler)
			combo.m_OnChanged.Insert(OnSelectableChanged);

		return combo;
	}

	//------------------------------------------------------------------------------------------------
	protected BaseContainer SetBaseContainer()
	{
		UserSettings userSettings;
		if (m_bUseGameUserSettings)
			userSettings = GetGame().GetGameUserSettings();
		else
			userSettings = GetGame().GetEngineUserSettings();

		if (!userSettings)
			return null;

		if (m_sSubModule.IsEmpty())
			m_Setting = userSettings.GetModule(m_sModule);
		else
		{
			BaseContainer setting = GetGame().GetEngineUserSettings().GetModule(m_sModule);
			if (setting)
				m_Setting = setting.GetObject(m_sSubModule);
		}

		return m_Setting;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelectableChanged(SCR_WLibComponentBase comp, int i)
	{
		if (!m_Setting && !SetBaseContainer())
			return;

		if (i >= 0 && i < m_aVals.Count())
		{
			m_Setting.Set(m_sName, m_aVals[i]);
		}
		else if (m_bHasLimits)
		{
			int value = m_fMin + m_fStep * i;
			m_Setting.Set(m_sName, value);
		}
		else
		{
			m_Setting.Set(m_sName, i);
		}

		if (m_OnEntryChanged)
			m_OnEntryChanged.Invoke(this);		
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(m_sModule, m_sName);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSliderChanged(SCR_WLibComponentBase comp, float value)
	{
		if (!m_Setting && !SetBaseContainer())
			return;
		
		m_Setting.Set(m_sName, value);
		if (m_OnEntryChanged)
			m_OnEntryChanged.Invoke(this);

		SCR_AnalyticsApplication.GetInstance().ChangeSetting(m_sModule, m_sName);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCheckboxChanged(SCR_WLibComponentBase comp, bool checked)
	{
		if (!m_Setting && !SetBaseContainer())
			return;

		m_Setting.Set(m_sName, checked);
		if (m_OnEntryChanged)
			m_OnEntryChanged.Invoke(this);
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(m_sModule, m_sName);
	}
};

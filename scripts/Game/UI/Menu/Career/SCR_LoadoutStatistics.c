//! Class for switching and displaying stats for different loadouts 

//------------------------------------------------------------------------------------------------
class SCR_LoadoutStatisticsComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected ref array<ref LoadoutStatSet> m_aLoadouts; 
	
	// Resources 
	protected const ResourceName ENTRY_LAYOUT = "{C87F7F6734B61688}UI/layouts/Menus/Career/CareerEntry.layout";
	
	// Strings 
	protected const string WIDGET_LOADOUT_SPIN = "SpinLoadout";
	protected const string WIDGET_STAT_LIST = "vStatList";
	
	// Widgets 
	protected Widget m_wRoot;
	protected Widget m_wStatList;
	protected Widget m_wLoadoutSpin;
	
	// Objects and references 
	protected CareerMenuUI m_CareerUI;
	protected SCR_SpinBoxComponent m_LoadoutSpin;
	
	protected ref array<ref Widget> m_aStatWidgets = new array<ref Widget>();
	
	// Values 
	protected int m_iEntryCount = 0;
	protected int m_iSelected = 0;
	
	ref ScriptInvoker m_OnLoadoutChange = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wStatList = w.FindAnyWidget(WIDGET_STAT_LIST);
		
		// Loadout spin
		m_wLoadoutSpin = w.FindAnyWidget(WIDGET_LOADOUT_SPIN);
		if (m_wLoadoutSpin)
			m_LoadoutSpin = SCR_SpinBoxComponent.Cast(m_wLoadoutSpin.FindHandler(SCR_SpinBoxComponent));
		
		if (m_LoadoutSpin)
			m_LoadoutSpin.m_OnChanged.Insert(OnLoadoutChanged);
		
		// Setup 
		SetupLoadoutSpin();
		CreateStatEntries(m_iEntryCount);
		
		OnLoadoutChanged(null, m_LoadoutSpin.GetCurrentIndex());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fill spin 
	protected void SetupLoadoutSpin()
	{
		if (!m_LoadoutSpin)
			return;
		
		// Get info from loadout stats 
		int total = m_aLoadouts.Count();
		foreach (int i, LoadoutStatSet statSet : m_aLoadouts)
		{
			if (!statSet)
				continue;
			
			m_LoadoutSpin.AddItem(statSet.GetLoadout().GetLoadoutName(), i == total - 1);
			
			// Get max entry count 
			int c = 0;
			if (statSet)
			{
				if (statSet.GetStats())
					c = statSet.GetStats().Count();
			}
			
			if (c > m_iEntryCount)
				 m_iEntryCount = c;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateStatEntries(int count)
	{
		for (int i = 0; i < count; i++)
		{
			Widget entry = GetGame().GetWorkspace().CreateWidgets(ENTRY_LAYOUT, m_wStatList);
			m_aStatWidgets.Insert(entry);
			entry.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadoutChanged(SCR_SpinBoxComponent spin, int id)
	{	
		UpdateStats(id);
		m_OnLoadoutChange.Invoke(id);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateStats(int id)
	{
		m_iSelected = id;
		int count = 0;
		if (m_aLoadouts[id])
		{
			if (m_aLoadouts[id].GetStats())
				count = m_aLoadouts[id].GetStats().Count();
		}
		
		for (int i = 0; i < m_aStatWidgets.Count(); i++)
		{
			Widget entry = m_aStatWidgets[i];
			
			bool display = i < count;
			// Hiding entries 
			entry.SetVisible(display);
			if (!display)
				continue;
			
			string name = m_aLoadouts[id].GetStats()[i].m_sName;
			int valueId = m_aLoadouts[id].GetStats()[i].m_iValueId;
			string value = "";
			if (m_CareerUI)
				 value = m_CareerUI.GetBackendValues()[valueId];
			SetStatEntry("", name, value, entry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetStatEntry(string sName, string sLabel, string sValue, Widget wEntry = null)
	{
		if(!wEntry)
			return;
		
		SetStatLabel(sName, sLabel, wEntry);
		SetStatValue(sName, sValue, wEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetStatLabel(string sName, string sLabel, Widget wEntry = null)
	{
		if(!wEntry)
			return;
		
		const string sLabelName = "txtLabel";
		TextWidget wLabel = TextWidget.Cast(wEntry.FindAnyWidget(sLabelName));
		if(wLabel)
			wLabel.SetText(sLabel);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void SetStatValue(string sName, string sValue, Widget wEntry = null)
	{
		if(!wEntry)
			return;
		
		const string sValueName = "txtValue";
		TextWidget wValue = TextWidget.Cast(wEntry.FindAnyWidget(sValueName));
		if(wValue)
			wValue.SetText(sValue);
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref LoadoutStatSet> GetLodoutStatSets() { return m_aLoadouts; }
	int GetCurrentLoadoutId() { return m_iSelected; }	
	
	//------------------------------------------------------------------------------------------------
	void SetCareerUI(CareerMenuUI careerUI) { m_CareerUI = careerUI; }
};

//------------------------------------------------------------------------------------------------
//! 
[BaseContainerProps()] 
class LoadoutStatSet
{
	[Attribute("", UIWidgets.Object, "")]
	protected ref SCR_BasePlayerLoadout m_Loadout;
	[Attribute()]
	protected ref array<ref LoadoutStat> m_aStats; 
	
	SCR_BasePlayerLoadout GetLoadout() { return m_Loadout; }
	array<ref LoadoutStat> GetStats() { return m_aStats; }
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()] 
class LoadoutStat
{
	[Attribute()]
	string m_sName;
	/*[Attribute()]
	string m_sValue; */
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ECareerStatId))]
	ECareerStatId m_iValueId;
};
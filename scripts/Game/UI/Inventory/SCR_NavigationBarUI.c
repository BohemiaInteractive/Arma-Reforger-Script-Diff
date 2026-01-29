[BaseContainerProps(configRoot: true)]
class SCR_NavigationBarConfig
{
	[Attribute()]
	protected ref array<ref NavigationButtonEntry> m_aEntries;

	array<ref NavigationButtonEntry> GetEntries()
	{
		return m_aEntries;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class NavigationButtonEntry
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox, desc: "Display name of input action")]
	LocalizedString m_sDisplayName;
	[Attribute(desc: "Name of input action")]
	string m_sAction;
	[Attribute(desc: "Name of the button - ID")]
	string m_sButtonID;
	
	SCR_InputButtonComponent m_Component;
};

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory navigation bar handler
[BaseContainerProps()]
class SCR_NavigationBarUI : SCR_ScriptedWidgetComponent
{
	//#define BUTTON_LAYOUT				"{19A9CC7487AAD442}UI/layouts/Common/Buttons/NavigationButton.layout"
	
	[Attribute("{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/WLib_NavigationButton.layout", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_Layout;
	
	[Attribute(uiwidget: UIWidgets.Object, desc: "Action Buttons")]
	ref array<ref NavigationButtonEntry> m_aEntries;
	
	[Attribute("{E09350C3FD7F0812}Configs/Inventory/InventoryNavigationBar.conf")]
	ResourceName m_sConfig;
	
	ref ScriptInvoker m_OnAction = new ScriptInvoker;
			
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
	
	//------------------------------------------------------------------------------------------------
	void Refresh();
	
	//------------------------------------------------------------------------------------------------
	protected SCR_InputButtonComponent GetButton( string name )
	{
		foreach ( NavigationButtonEntry entry : m_aEntries )
		{
			SCR_InputButtonComponent comp = entry.m_Component;
			if (entry.m_sButtonID == name && comp)
				return comp;
		}
	
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavigation(SCR_InputButtonComponent comp, string action)
	{
		m_OnAction.Invoke(comp, action, null, -1);
	}

	//------------------------------------------------------------------------------------------------
	void SetButtonEnabled( string sButtonName, bool bEnable = true, string sName = "" )
	{
		SCR_InputButtonComponent pActionButton = GetButton( sButtonName );
		if(!pActionButton)
			return;

		//if state is the same then quit early
		if (pActionButton.IsEnabled() == bEnable)
			return;

		array<string> keyStack = {};
		bEnable = bEnable && pActionButton.IsKeybindAvailable(keyStack);
		pActionButton.SetEnabled(bEnable);
		pActionButton.GetRootWidget().SetVisible(bEnable);

		if(!sName.IsEmpty())
			pActionButton.SetLabel(sName);
	}		
	
	//------------------------------------------------------------------------------------------------
	void SetButtonActionName( string sButtonName, string sName )
	{
		SCR_InputButtonComponent pActionButton = GetButton( sButtonName );
		if( !pActionButton )
			return;
		pActionButton.SetLabel(sName);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAllButtonEnabled( bool bEnable = true )
	{
		foreach ( NavigationButtonEntry entry: m_aEntries )
		{
			SCR_InputButtonComponent comp = entry.m_Component;
			if (!comp)
				return;
			
			entry.m_Component.SetEnabled( bEnable );
			entry.m_Component.GetRootWidget().SetVisible( bEnable );
		}
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	void FillFromConfig()
	{
		if (m_sConfig.GetPath().IsEmpty())
			return;
		
		Resource res = BaseContainerTools.LoadContainer(m_sConfig);
		if (!res)
			return;

		BaseContainer container = res.GetResource().ToBaseContainer();
		if (!container)
			return;
		
		SCR_NavigationBarConfig config = SCR_NavigationBarConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!config)
			return;

		m_aEntries.Clear();
		m_aEntries = config.GetEntries();
		InitNavButtons();
	}
	
	protected void InitNavButtons()
	{
		foreach (NavigationButtonEntry entry : m_aEntries)
		{
			if (entry.m_sAction == "Inventory_Selected")
				continue;
			
			Widget button = GetGame().GetWorkspace().CreateWidgets(m_Layout, GetRootWidget());
			if (!button)
				continue;
			
			SCR_InputButtonComponent comp = SCR_InputButtonComponent.Cast(button.FindHandler(SCR_InputButtonComponent));
			if (!comp)
				continue;
			
			entry.m_Component = comp;
			comp.SetAction(entry.m_sAction, forceUpdate: true);
			comp.SetLabel(entry.m_sDisplayName);
			comp.m_OnActivated.Insert(OnNavigation);
			comp.SetClickedSound(string.Empty);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached(w);

		InitNavButtons();
	}
};
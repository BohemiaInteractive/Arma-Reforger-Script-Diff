class SCR_AddonExportDialog : DialogUI
{
	protected ref SCR_AddonsExportDialogWidgets m_Widgets = new SCR_AddonsExportDialogWidgets();
	
	protected SCR_AddonsExportSubMenuComponent m_ExportSubMenu;

	//---------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_Widgets.Init(GetRootWidget());
		
		// Button actions 
		m_Widgets.m_NavCopyComponent.m_OnActivated.Insert(OnClickCopy);
		m_Widgets.m_TabViewRootComponent.GetOnChanged().Insert(OnTabChanged);
		
		GenerateModContent();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnClickCopy()
	{
		if (m_ExportSubMenu)
			m_ExportSubMenu.CopyToClipboard();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void GenerateModContent()
	{
		// JSON 
		SCR_AddonsExportSubMenuComponent subMenu = SubMenuFromTab(m_Widgets.m_TabViewRootComponent, 0);
		if (subMenu)
			subMenu.GenerateJSONFormat();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnTabChanged(SCR_TabViewComponent tabView, Widget w, int id)
	{
		SCR_AddonsExportSubMenuComponent subMenu = SubMenuFromTab(tabView, id);
		
		if (!subMenu)
			return;
		
		m_ExportSubMenu = subMenu;
		
		switch (id)
		{
			case 0:
			{
				subMenu.GenerateJSONFormat();
				break;
			}
			
			case 1:
			{
				subMenu.GenerateCLIFormat();
				break;
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected SCR_AddonsExportSubMenuComponent SubMenuFromTab(SCR_TabViewComponent tabView, int id)
	{
		if (!tabView)
			return null;
		
		SCR_TabViewContent content = tabView.GetEntryContent(id);
		if (!content)
			return null;
		
		Widget w = content.m_wTab;
		if (!w)
			return null;
		
		return SCR_AddonsExportSubMenuComponent.Cast(w.FindHandler(SCR_AddonsExportSubMenuComponent));
	}
}
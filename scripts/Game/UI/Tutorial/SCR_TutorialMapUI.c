//------------------------------------------------------------------------------------------------
class SCR_TutorialMapUI : SCR_MapUIElementContainer
{
	[Attribute("{3AC52EED5AB6C1E2}UI/layouts/Tutorial/FakeBaseElement.layout", params: "layout")]
	protected ResourceName m_sBaseElement;

	[Attribute("{94F1E2223D7E0588}UI/layouts/Campaign/ServiceHint.layout", params: "layout")]
	protected ResourceName m_sServiceHint;
	
	protected ref array <SCR_TutorialFakeBaseComponent> m_aFakeBases = {};
	
	//------------------------------------------------------------------------------------------------
	void InsertFakeBase(notnull SCR_TutorialFakeBaseComponent fakeBase)
	{
		m_aFakeBases.Insert(fakeBase);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		foreach(SCR_TutorialFakeBaseComponent fakeBase : m_aFakeBases)
		{
			if (!fakeBase.m_bVisible)
				continue;
			
			CreateFakeBase(fakeBase);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateFakeBase(SCR_TutorialFakeBaseComponent fakeBaseComponent)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_wIconsContainer);
		if (!w)
			return;
		
		SCR_TutorialMapUIBase handler = SCR_TutorialMapUIBase.Cast(w.FindHandler(SCR_TutorialMapUIBase));
		if (!handler)
			return;
		
		handler.SetFakeBase(fakeBaseComponent);
		
		m_mIcons.Insert(w, handler);
		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);
		
		UpdateIcons();
		handler.UpdateMilitarySymbol();
	}
}
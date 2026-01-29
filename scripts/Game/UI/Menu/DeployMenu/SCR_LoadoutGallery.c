void OnLoadoutFocusLostDelegate(SCR_BasePlayerLoadout base);

typedef func OnLoadoutFocusLostDelegate;

typedef ScriptInvokerBase<OnLoadoutFocusLostDelegate> OnLoadoutFocusLostInvoker;

class SCR_LoadoutGallery : SCR_GalleryComponent
{
	[Attribute("{39D815C843414C76}UI/layouts/Menus/DeployMenu/LoadoutButton.layout")]
	protected ResourceName m_sLoadoutButton;
	protected ref set<SCR_LoadoutButton> m_aLoadoutButtons = new set<SCR_LoadoutButton>();	

	protected ref ScriptInvoker<SCR_LoadoutButton> m_OnLoadoutClicked;
	protected ref ScriptInvoker<SCR_BasePlayerLoadout> m_OnLoadoutHovered;
	protected ref OnLoadoutFocusLostInvoker m_OnLoadoutFocusLost;
	protected ref ScriptInvokerBool m_OnFocusChange = new ScriptInvokerBool();
	protected bool m_bFocused;
	protected SCR_LoadoutButton m_SelectedButton;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		EnablePagingInputListeners(false);
	}
	
	ScriptInvokerBool GetOnFocusChange()
	{
		return m_OnFocusChange;
	}
	
	void SetGalleryFocused(bool focused)
	{
		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
		if (m_bFocused && (focusedWidget && focusedWidget.IsInherited(SCR_LoadoutButton)))
			return;
		
		m_bFocused = focused;
		m_OnFocusChange.Invoke(m_bFocused);
	}
	
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		SetGalleryFocused(true);
		
		return false;
	}
	
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		SetGalleryFocused(false);
		
		return false;
	}
	
	bool GetFocused()
	{
		return m_bFocused;
	}
	
	void EnablePagingInputListeners(bool enable)
	{	
		GetGame().GetInputManager().RemoveActionListener(m_sActionLeft, EActionTrigger.DOWN, OnCustomLeft);
		GetGame().GetInputManager().RemoveActionListener(m_sActionRight, EActionTrigger.DOWN, OnCustomRight);
		
		if (!enable)
			return;
		
		GetGame().GetInputManager().AddActionListener(m_sActionLeft, EActionTrigger.DOWN, OnCustomLeft);
		GetGame().GetInputManager().AddActionListener(m_sActionRight, EActionTrigger.DOWN, OnCustomRight);
	}
	
	int AddItem(SCR_BasePlayerLoadout loadout, bool enabled = true)
	{
		Widget supplies = GetGame().GetWorkspace().FindAnyWidget("w_Supplies");
		Widget loadoutEntry = GetGame().GetWorkspace().CreateWidgets(m_sLoadoutButton, supplies);
		SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(loadoutEntry.FindHandler(SCR_LoadoutButton));
		if (loadoutBtn)
		{
			loadoutBtn.SetLoadout(loadout);
			loadoutBtn.SetEnabled(enabled);

			loadoutBtn.m_OnClicked.Insert(OnLoadoutClicked);
			loadoutBtn.m_OnFocus.Insert(OnLoadoutHovered);
			loadoutBtn.m_OnMouseEnter.Insert(OnLoadoutHovered);
			loadoutBtn.m_OnMouseLeave.Insert(OnLoadoutMouseLeave);
			loadoutBtn.m_OnFocusLost.Insert(OnLoadoutFocusLost);

			m_aLoadoutButtons.Insert(loadoutBtn);
		}

		int itm = AddItem(loadoutEntry);
		ShowPagingButtons(); // hack to go around hajkovinas

		return itm;
	}
	
	override void ShowPagingButtons(bool animate = true)
	{
		bool show = (m_aWidgets.Count() > m_iCountShownItems);
		if (m_PagingLeft)
			m_PagingLeft.SetVisible(show, false);
		
		if (m_PagingRight)
			m_PagingRight.SetVisible(show, false);
	}

	protected void OnLoadoutClicked(notnull SCR_LoadoutButton loadoutBtn)
	{
		GetOnLoadoutClicked().Invoke(loadoutBtn);
	}
	
	protected void OnLoadoutHovered(notnull Widget btn)
	{
		SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(btn.FindHandler(SCR_LoadoutButton));
		if (loadoutBtn)
			GetOnLoadoutHovered().Invoke(loadoutBtn.GetLoadout());
	}
	
	protected void OnLoadoutFocusLost(notnull Widget btn)
	{
		SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(btn.FindHandler(SCR_LoadoutButton));
		if(!loadoutBtn)
			return;
		
		if (loadoutBtn.IsSelected() && m_OnLoadoutFocusLost)
			GetOnLoadoutFocusLost().Invoke(loadoutBtn.GetLoadout());
		
		if(m_SelectedButton && !loadoutBtn.IsSelected() && m_OnLoadoutClicked)
			GetOnLoadoutClicked().Invoke(m_SelectedButton);
	}
	
	protected void OnLoadoutMouseLeave(notnull Widget btn)
	{
		OnLoadoutFocusLost(btn);
	}
	
	Widget GetContentRoot()
	{
		return m_wContentRoot;
	}

	void SetSelected(SCR_BasePlayerLoadout loadout)
	{
		foreach (SCR_LoadoutButton loadoutBtn : m_aLoadoutButtons)
		{
			if(loadoutBtn.GetLoadout() == loadout)
			{
				loadoutBtn.SetSelected(true);
				m_SelectedButton = loadoutBtn;
			}
			else
			{
				loadoutBtn.SetSelected(false);
			}
		}
	}

	override void ClearAll()
	{
		super.ClearAll();
		m_aLoadoutButtons.Clear();
	}

	SCR_LoadoutButton GetButtonForLoadout(SCR_BasePlayerLoadout loadout)
	{
		foreach (SCR_LoadoutButton btn : m_aLoadoutButtons)
		{
			if (btn.GetLoadout() == loadout)
				return btn;
		}

		return null;
	}
	
	ScriptInvoker GetOnLoadoutClicked()
	{
		if (!m_OnLoadoutClicked)
			m_OnLoadoutClicked = new ScriptInvoker();

		return m_OnLoadoutClicked;
	}
	
	ScriptInvoker GetOnLoadoutHovered()
	{
		if (!m_OnLoadoutHovered)
			m_OnLoadoutHovered = new ScriptInvoker();

		return m_OnLoadoutHovered;
	}	
	
	OnLoadoutFocusLostInvoker GetOnLoadoutFocusLost()
	{
		if (!m_OnLoadoutFocusLost)
			m_OnLoadoutFocusLost = new OnLoadoutFocusLostInvoker();

		return m_OnLoadoutFocusLost;
	}
};
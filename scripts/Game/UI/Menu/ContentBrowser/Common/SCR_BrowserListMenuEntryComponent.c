/*
Adds handling of favorites visualization for Scenarios and Servers browser menus
*/
void ScriptInvokerBrowserListMenuEntryMethod(SCR_BrowserListMenuEntryComponent entry, bool favorite);
typedef func ScriptInvokerBrowserListMenuEntryMethod;
typedef ScriptInvokerBase<ScriptInvokerBrowserListMenuEntryMethod> ScriptInvokerBrowserListMenuEntry;

class SCR_BrowserListMenuEntryComponent : SCR_ListMenuEntryComponent
{
	protected static const string BUTTON_FAVORITE = "FavoriteButton";
	
	protected ref SCR_ModularButtonComponent m_FavoriteButton;
	protected bool m_bFavorite;
	
	protected ref ScriptInvokerBrowserListMenuEntry m_OnFavorite;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		Widget favoriteButton = w.FindAnyWidget(BUTTON_FAVORITE);
		if (favoriteButton)
			m_FavoriteButton = SCR_ModularButtonComponent.FindComponent(favoriteButton);
		
		if (m_FavoriteButton)
		{
			m_aMouseButtons.Insert(m_FavoriteButton);
			m_FavoriteButton.m_OnClicked.Insert(OnFavoriteClicked);
		}

		super.HandlerAttached(w);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		super.UpdateModularButtons();
		
		SCR_ListEntryHelper.UpdateMouseButtonColor(m_FavoriteButton, m_bUnavailable, m_bFocused);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFavoriteClicked(SCR_ModularButtonComponent comp)
	{
		SetFavorite(!m_bFavorite);
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetFavorite(bool favorite)
	{
		if (m_FavoriteButton)
			m_FavoriteButton.SetToggled(favorite);

		if (m_bFavorite == favorite)
			return false;

		m_bFavorite = favorite;

		if (m_OnFavorite)
			m_OnFavorite.Invoke(this, favorite);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsFavorite()
	{
		return m_bFavorite;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBrowserListMenuEntry GetOnFavorite()
	{
		if (!m_OnFavorite)
			m_OnFavorite = new ScriptInvokerBrowserListMenuEntry();

		return m_OnFavorite;
	}
}

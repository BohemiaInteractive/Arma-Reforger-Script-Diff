/*
Class for storing server browser menu widget references
*/

class ServerBrowserMenuWidgets
{
	const string WIDGET_CONTENT = "ServerBrowserContent";

	// Constant names of widgets
	const string WIDGET_TAB_VIEW = "TabView";
	const string WIDGET_FILTER = "FilterPanel";
	const string WIDGET_SERVER_HEADER = "SortingHeader";
	const string WIDGET_SORT_SESSION_NAME = "sortName";
	const string WIDGET_SORT_SESSION_FAVORITE = "sortFavourite";
	const string WIDGET_SEARCH = "m_FilterSearch";
	const string WIDGET_SCROLL_LAYOUT = "ScrollLayout0";
	const string WIDGET_SERVER_SCENARIO_DETAILS_PANEL = "ServerScenarioDetailsPanel";
	const string WIDGET_SERVERS_FEEDBACK = "txtServersFeedback";
	const string WIDGET_LOADING_SERVERS = "LoadingServers";
	const string WIDGET_SCROLLABLE_LIST = "vServerListView";

	// Buttons
	const string WIDGET_BUTTON_JOIN = "BtnQuickJoin";
	const string WIDGET_BUTTON_DETAILS = "BtnDetails";
	const string WIDGET_BUTTON_FAVORITE = "BtnFavorite";
	const string WIDGET_BUTTON_HOST = "HostNewServerButton";
	const string WIDGET_BUTTON_REFRESH = "RefreshButton";
	const string WIDGET_BUTTON_DIRECTJOIN = "BtnManualConnect";
	const string WIDGET_BUTTON_FILTER = "Filter";

	// Messages widgets
	static string WIDGET_PANEL_EMPTY = "MenuPanelEmpty";
	static string WIDGET_MESSAGE_WRAP = "SimpleMessageWrap";
	static string WIDGET_MESSAGE_LIST = "SimpleMessageList";

	// Widgets elements
	Widget m_wRoot;
	Widget m_wContent;
	Widget m_wPanelEmpty;

	Widget m_wSearchEditBox;
	Widget m_wButtonConnect;
	Widget m_wControllerActionsOverlay;
	TextWidget m_wTxtServersFeedback;
	Widget m_wLoadingServers;
	Widget m_wSortSessionName;
	Widget m_wSortSessionFavorite;

	Widget m_wHostNewServerButton;
	SCR_HostButtonComponent m_HostButton;

	SCR_InputButtonComponent m_JoinButton;
	SCR_InputButtonComponent m_DetailsButton;
	SCR_InputButtonComponent m_FavoritesButton;
	
	SCR_InputButtonComponent m_RefreshButton;
	SCR_InputButtonComponent m_DirectJoinButton;
	SCR_InputButtonComponent m_FilterButton;
	ref array<SCR_InputButtonComponent> m_aEntryButtons = {};

	//------------------------------------------------------------------------------------------------
	//! Will find all required widgets
	void FindAllWidgets(Widget root)
	{
		m_wRoot = root;
		m_wContent = m_wRoot.FindAnyWidget(WIDGET_CONTENT);
		m_wPanelEmpty = m_wRoot.FindAnyWidget(WIDGET_PANEL_EMPTY);

		m_wSearchEditBox = m_wRoot.FindAnyWidget(WIDGET_SEARCH);

		m_wTxtServersFeedback = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SERVERS_FEEDBACK));

		m_wLoadingServers = m_wRoot.FindAnyWidget(WIDGET_LOADING_SERVERS);

		m_wSortSessionName = m_wRoot.FindAnyWidget(WIDGET_SORT_SESSION_NAME);
		m_wSortSessionFavorite = m_wRoot.FindAnyWidget(WIDGET_SORT_SESSION_FAVORITE);

		m_wHostNewServerButton = m_wRoot.FindAnyWidget(WIDGET_BUTTON_HOST);
		if (m_wHostNewServerButton)
			m_HostButton = SCR_HostButtonComponent.FindComponent(m_wHostNewServerButton);

		//! Entry Buttons
		m_JoinButton = FindButton(WIDGET_BUTTON_JOIN);
		m_DetailsButton = FindButton(WIDGET_BUTTON_DETAILS);
		m_FavoritesButton = FindButton(WIDGET_BUTTON_FAVORITE);

		m_aEntryButtons.Insert(m_JoinButton);
		m_aEntryButtons.Insert(m_DetailsButton);
		m_aEntryButtons.Insert(m_FavoritesButton);
		
		//! Left footer buttons
		m_RefreshButton = FindButton(WIDGET_BUTTON_REFRESH);
		m_DirectJoinButton = FindButton(WIDGET_BUTTON_DIRECTJOIN);
		m_FilterButton = FindButton(WIDGET_BUTTON_FILTER);
	}

	//------------------------------------------------------------------------------------------------
	SCR_InputButtonComponent FindButton(string name)
	{
		Widget button = m_wRoot.FindAnyWidget(name);
		SCR_InputButtonComponent buttonComp;

		if (button)
			buttonComp = SCR_InputButtonComponent.FindComponent(button);

		return buttonComp;
	}

	//------------------------------------------------------------------------------------------------
	//! Find widget by name on given parent
	// return SCR_WLibComponentBase based handler
	ScriptedWidgetEventHandler FindHandlerReference(out Widget target, string targetName, typename typeName, Widget parent = null)
	{
		typename type = ScriptedWidgetEventHandler;

		// Default parent
		if (!parent)
			parent = m_wRoot;

		// Find widget
		target = parent.FindAnyWidget(targetName);

		if (!target)
		{
			#ifdef SB_DEBUG
			string error = string.Format("Could not find widget of NAME: %1 of PARENT: %2", targetName, parent);
			Print(error, LogLevel.ERROR);
			#endif
			return null;
		}

		// Find handler
		return target.FindHandler(typeName);
	}
}

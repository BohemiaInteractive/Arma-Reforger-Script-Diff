[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Debriefing screen shown after game ends")]
class SCR_DebriefingScreenComponentClass : SCR_DeployMenuBaseScreenComponentClass
{
}

//! Debriefing screen component intended to be added to the GameMode.
class SCR_DebriefingScreenComponent : SCR_DeployMenuBaseScreenComponent
{
}

//! Class handling layout for UnevenFourTiles.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenUnevenFourTiles : SCR_DeployMenuBaseScreenLayout
{
	[Attribute(defvalue: "{F7138F20BD16D89A}UI/layouts/Menus/DeployMenu/DebriefingScreenFourUnevenTiles.layout")]
	protected ResourceName m_sFourUnevenTilesLayout;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_TopLeftTile;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_TopRightTile;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_BottomLeftTile;
	
	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_BottomRightTile;
	
	protected const string WELCOME_CONTENT = "WelcomeContent";
	
	protected const string TOP_LEFT_TILE = "TopLeftTile";
	protected const string TOP_RIGHT_TILE = "TopRightTile";
	protected const string BOTTOM_LEFT_TILE = "BottomLeftTile";
	protected const string BOTTOM_RIGHT_TILE = "BottomRightTile";
	
	protected const string TOP_LEFT_TILE_BUTTON = "TopLeftTileButton";
	protected const string TOP_RIGHT_TILE_BUTTON = "TopRightTileButton";
	protected const string BOTTOM_LEFT_TILE_BUTTON = "BottomLeftTileButton";
	protected const string BOTTOM_RIGHT_TILE_BUTTON = "BottomRightTileButton";

	//------------------------------------------------------------------------------------------------
	override void InitContent(SCR_WelcomeScreenMenu menu)
	{
		Widget targetColumn = menu.GetRootWidget().FindAnyWidget(WELCOME_CONTENT);
		if (!targetColumn)
			return;

		GetGame().GetWorkspace().CreateWidgets(m_sFourUnevenTilesLayout, targetColumn);

		if (m_TopLeftTile)
		{
			m_aScreenBaseContents.Insert(m_TopLeftTile);
			m_TopLeftTile.InitContent(menu, TOP_LEFT_TILE, TOP_LEFT_TILE_BUTTON);
		}

		if (m_TopRightTile)
		{
			m_aScreenBaseContents.Insert(m_TopRightTile);
			m_TopRightTile.InitContent(menu, TOP_RIGHT_TILE, TOP_RIGHT_TILE_BUTTON);
		}

		if (m_BottomLeftTile)
		{
			m_aScreenBaseContents.Insert(m_BottomLeftTile);
			m_BottomLeftTile.InitContent(menu, BOTTOM_LEFT_TILE, BOTTOM_LEFT_TILE_BUTTON);
		}
		
		if (m_BottomRightTile)
		{
			m_aScreenBaseContents.Insert(m_BottomRightTile);
			m_BottomRightTile.InitContent(menu, BOTTOM_RIGHT_TILE, BOTTOM_RIGHT_TILE_BUTTON);
		}
	}
}

//! Class handling big image content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenBigImageContent : SCR_WelcomeScreenBaseContent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Big image to be shown", params: "edds")]
	protected ResourceName m_sContentImage;

	[Attribute(defvalue: "{6CD5DC3DE2C54BBE}UI/layouts/Menus/DeployMenu/DebriefingScreenBigImageTile.layout")]
	protected ResourceName m_sBigImageLayout;

	//------------------------------------------------------------------------------------------------
	//! Initialises content for given column
	//! \param[in] menu
	//! \param[in] column
	//! \param[in] columnButton
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
			return;
		
		Widget bigImageWidget = GetGame().GetWorkspace().CreateWidgets(m_sBigImageLayout, targetTile);
		if (!bigImageWidget)
			return;

		ImageWidget imageWidget = ImageWidget.Cast(bigImageWidget.FindAnyWidget("ImageWidget"));
		if (imageWidget && m_sContentImage)
			imageWidget.LoadImageTexture(0, m_sContentImage, false, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Get image
	//! \return image
	ResourceName GetImage()
	{
		return m_sContentImage;
	}
}

//! Class handling summary content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenSummaryContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;
	
	[Attribute()]
	protected string m_sSubtitleText;
	
	[Attribute()]
	protected string m_sSubtitleLostText;
	
	[Attribute()]
	protected string m_sDescriptionText;

	[Attribute(defvalue: "{10A7983E48474CB2}UI/layouts/Menus/DeployMenu/DebriefingScreenSummaryTile.layout")]
	protected ResourceName m_sSummaryLayout;
	
	protected Widget m_wSummaryWidget;

	//------------------------------------------------------------------------------------------------
	//! Initialises content for given column
	//! \param[in] menu
	//! \param[in] column
	//! \param[in] columnButton
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
			return;
		
		m_wSummaryWidget = GetGame().GetWorkspace().CreateWidgets(m_sSummaryLayout, targetTile);
		if (!m_wSummaryWidget)
			return;

		FillSummaryWidget();
		
		//Invokers for in-game possibility to update these strings
		//SOMETHING.SOMEINVOKER().Insert(UpdateSummarySubtitle);
		//SOMETHING.SOMEINVOKER().Insert(UpdateSummaryDescription);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills content widget with summary
	protected void FillSummaryWidget()
	{
		string characterRank = "";
		string subtitle = GetSubtitleLostText();
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (player)
		{
			SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(player.FindComponent(SCR_CharacterRankComponent));
			if (rankComponent)
				characterRank = rankComponent.GetCharacterRankName(player);
		}
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
			
		Faction faction = factionManager.GetLocalPlayerFaction();
		if (!faction)
			return;
		
		SCR_GameModeEndData endData = gamemode.GetEndGameData();
		if (endData)
		{
			array<int> winnerIds = {};
			array<int> winnerFactionIds = {};
			endData.GetWinnerIds(winnerIds);
			endData.GetFactionWinnerIds(winnerFactionIds);
			if (!winnerFactionIds.IsEmpty())
			{
				if (winnerFactionIds.Contains(factionManager.GetFactionIndex(faction)))
					subtitle = GetSubtitleText();
			}
			else if (!winnerIds.IsEmpty())
			{
				int playerID = GetGame().GetPlayerController().GetPlayerId();
				if (winnerIds.Contains(playerID))
					subtitle = GetSubtitleText();
			}
		}
		
		RichTextWidget subtitleText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("SubtitleText"));
		if (subtitleText)
			subtitleText.SetTextFormat(subtitle, characterRank, string.Format("<color rgba=%1>%2</color>", UIColors.FormatColor(UIColors.CONTRAST_COLOR), SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(GetGame().GetPlayerController().GetPlayerId())));

		RichTextWidget descriptionText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("DescriptionText"));
		if (!descriptionText)
			return;
		
		descriptionText.SetText(GetDescriptionText());
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(GetDescriptionText())) 
		{
			SCR_GameOverScreenManagerComponent gameOverScreenMgr = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
			if (!gameOverScreenMgr)
				return;
			
			SCR_GameOverScreenConfig gameOverConfig = gameOverScreenMgr.GetGameOverConfig();
			if (!gameOverConfig)
				return;
			
			SCR_BaseGameOverScreenInfo gameOverScreenInfo;
			gameOverConfig.GetGameOverScreenInfo(gameOverScreenMgr.GetCurrentGameOverType(), gameOverScreenInfo);
			if (!gameOverScreenInfo)
				return;
			
			if (!SCR_StringHelper.IsEmptyOrWhiteSpace(gameOverScreenInfo.GetDebriefing(faction, null))) 
				descriptionText.SetText(gameOverScreenInfo.GetDebriefing(faction, null));
			else
				descriptionText.SetText(gameOverScreenInfo.GetSubtitle(faction, null));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates summary subtitle - intended for runtime updates
	//! \param[in] subtitle
	void UpdateSummarySubtitle(string subtitle)
	{
		m_sSubtitleText = subtitle;
		RichTextWidget subtitleText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("SubtitleText"));
		if (subtitleText)
			subtitleText.SetText(subtitle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates summary description - intended for runtime updates
	//! \param[in] description
	void UpdateSummaryDescription(string description)
	{
		m_sDescriptionText = description;
		RichTextWidget descriptionText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("DescriptionText"));
		if (descriptionText)
			descriptionText.SetText(description);
	}

	//------------------------------------------------------------------------------------------------
	//! \return title
	string GetTitleText()
	{
		return m_sTitleText;
	}

	//------------------------------------------------------------------------------------------------
	//! \return subtitle
	string GetSubtitleText()
	{
		return m_sSubtitleText;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return subtitle lost
	string GetSubtitleLostText()
	{
		return m_sSubtitleLostText;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return description
	string GetDescriptionText()
	{
		return m_sDescriptionText;
	}
}

//! Class handling finished mission objectives content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenFinishedObjectivesContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;
	
	[Attribute(defvalue: "{0B7DDF77E24F0C63}UI/layouts/Menus/DeployMenu/DebriefingScreenFinishedObjectivesTile.layout")]
	protected ResourceName m_sFinishedObjectivesTileLayout;
	
	[Attribute(defvalue: "{6B9E63C758849B7F}UI/layouts/Menus/DeployMenu/DebriefingScreenMissionObjective.layout")]
	protected ResourceName m_sFinishedObjectivesLayout;

	[Attribute(defvalue: "7", desc: "Max number of objectives that can be shown on a single page", params: "1 inf")]
	protected int m_iMaxNumberOfObjectives;
	
	[Attribute(defvalue: "false", desc: "Use Task Finish History Manager for getting the list of Task Info")]
	protected bool m_bUseTaskFinishHistoryManager;

	protected Widget m_wFinishedObjectivesWidget;
	protected ButtonWidget m_wColumnButton;
	protected ref array<Widget> m_aFinishedObjectivesWidgets = {};
	protected int m_iCurrentPage;
	protected int m_iFinishedTasksCount;
	protected ref array<ref SCR_TaskUIInfo> m_aFinishedTasksUIInfo;
	
	//------------------------------------------------------------------------------------------------
	//! Initialises content for given column
	//! \param[in] menu
	//! \param[in] column
	//! \param[in] columnButton
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
				return;
		
		m_wFinishedObjectivesWidget = GetGame().GetWorkspace().CreateWidgets(m_sFinishedObjectivesTileLayout, targetTile);
		if (!m_wFinishedObjectivesWidget)
				return;
		
		Widget finishedObjectivesContent = m_wFinishedObjectivesWidget.FindAnyWidget("FinishedObjectivesLayout");
		if (!finishedObjectivesContent)
				return;

		m_wColumnButton = ButtonWidget.Cast(menu.GetRootWidget().FindAnyWidget(columnButton));
		if (!m_wColumnButton)
			return;
		
		FillFinishedObjectivesWidget(finishedObjectivesContent);
		InitPagination();
		HandlePagination(m_iFinishedTasksCount > m_iMaxNumberOfObjectives);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggles interactions for this content
	//! \param[in] enabled
	override void ToggleInteractions(bool enabled)
	{
		HandlePagination(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Processes tasks according to the player faction and temporarily sorts out issues with Conflict.
	//! TODO - Will be fixed in the future
	void ProcessTasks()
	{
		if (!m_aFinishedTasksUIInfo)
			m_aFinishedTasksUIInfo = {};
		else
			m_aFinishedTasksUIInfo.Clear();

		if (m_bUseTaskFinishHistoryManager)
		{
			int playerId = GetGame().GetPlayerController().GetPlayerId();
			array<ref SCR_TaskFinishEntry> finishedTasks = SCR_TaskFinishHistoryManagerComponent.GetInstance().GetByPlayerID(playerId);
			if (!finishedTasks)
				return;

			foreach (SCR_TaskFinishEntry entry : finishedTasks)
			{
				if (!entry.WasSuccessful())
					continue;

				SCR_TaskUIInfo info;
				entry.GetUIInfo(info);
				m_aFinishedTasksUIInfo.Insert(info);
			}
		}
		else
		{
			SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
			if (!taskSystem)
				return;

			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager)
				return;

			Faction faction = factionManager.GetLocalPlayerFaction();
			if (!faction)
				return;

			array<SCR_Task> completedTasks = {};
			taskSystem.GetTasksByState(completedTasks, SCR_ETaskState.COMPLETED, faction.GetFactionKey());
			foreach (SCR_Task task : completedTasks)
			{
				m_aFinishedTasksUIInfo.Insert(task.GetTaskUIInfo());
			}
		}

		m_iFinishedTasksCount = m_aFinishedTasksUIInfo.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fills content widget with finished objectives
	//! \param[in] content
	void FillFinishedObjectivesWidget(Widget content)
	{
		RichTextWidget titleText = RichTextWidget.Cast(m_wFinishedObjectivesWidget.FindAnyWidget("TitleText"));
		if (titleText)
			titleText.SetText(GetTitleText());

		ProcessTasks();
		FlipPage(0);

		if (m_iFinishedTasksCount < m_iMaxNumberOfObjectives)
		{
			Widget paginationWidget = m_wFinishedObjectivesWidget.FindAnyWidget("BottomTitleSizeLayout");
			paginationWidget.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialises pagination by registering button actions
	protected void InitPagination()
	{
		Widget previousButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("PrevButton");
		if (!previousButtonWidget)
			return;
		
		SCR_PagingButtonComponent previousButton = SCR_PagingButtonComponent.Cast(previousButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (previousButton)
			previousButton.m_OnClicked.Insert(ButtonClicked);

		Widget nextButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("NextButton");
		if (!nextButtonWidget)
			return;
		
		SCR_PagingButtonComponent nextButton = SCR_PagingButtonComponent.Cast(nextButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (nextButton)
			nextButton.m_OnClicked.Insert(ButtonClicked);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes focus to the column from which that button originates. It is supposed to be invoked.
	protected void ButtonClicked()
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wColumnButton);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles inputs of buttons
	//! \param[in] enabled
	protected void HandlePagination(bool enabled = false)
	{
		Widget previousButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("PrevButton");
		if (!previousButtonWidget)
			return;
		
		SCR_PagingButtonComponent previousButton = SCR_PagingButtonComponent.Cast(previousButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (!previousButton)
			return;
		
		if (enabled)
			previousButton.m_OnActivated.Insert(PreviousButtonActivated);
		else
			previousButton.m_OnActivated.Remove(PreviousButtonActivated);

		Widget nextButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("NextButton");
		if (!nextButtonWidget)
			return;
		
		SCR_PagingButtonComponent nextButton = SCR_PagingButtonComponent.Cast(nextButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (!nextButton)
			return;
		
		if (enabled)
			nextButton.m_OnActivated.Insert(NextButtonActivated);
		else
			nextButton.m_OnActivated.Remove(NextButtonActivated);
	}

	//------------------------------------------------------------------------------------------------
	//! Flips page to the previous one
	protected void PreviousButtonActivated()
	{
		int currentPage = GetCurrentPage();
		currentPage--;
		if (currentPage < 0)
		{
			FlipPage(0);
			return;
		}

		FlipPage(currentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Flips page to the next one
	protected void NextButtonActivated()
	{
		int currentPage = GetCurrentPage();
		currentPage++;
		if (currentPage == Math.Ceil(m_iFinishedTasksCount/6))
			return;

		FlipPage(currentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the content of current page based on the provided number
	//! \param[in] currentPage
	protected void FlipPage(int currentPage)
	{
		int taskId;
		Widget entry;
		SCR_TaskUIInfo info;
		RichTextWidget name;
		Widget finishedObjectivesContentHolder;
		for (int i = 0; i < m_iMaxNumberOfObjectives - 1; i++)
		{
			taskId = currentPage * (m_iMaxNumberOfObjectives - 1) + i;
			if (m_aFinishedObjectivesWidgets.IsIndexValid(i))
				entry = m_aFinishedObjectivesWidgets[i];
			else
				entry = null;

			if (taskId >= m_iFinishedTasksCount)
			{
				if (entry)
					entry.SetVisible(false);

				continue;
			}

			if (!entry)
			{
				if (!finishedObjectivesContentHolder)
				{
					finishedObjectivesContentHolder = m_wFinishedObjectivesWidget.FindAnyWidget("FinishedObjectivesLayout");
					if (!finishedObjectivesContentHolder)
						continue;
				}

				entry = GetGame().GetWorkspace().CreateWidgets(m_sFinishedObjectivesLayout, finishedObjectivesContentHolder);
				m_aFinishedObjectivesWidgets.Insert(entry);
			}

			entry.SetVisible(true);
			name = RichTextWidget.Cast(entry.FindAnyWidget("ObjectiveText"));
			info = m_aFinishedTasksUIInfo[taskId];
			info.SetNameTo(name);
		}
		
		SetCurrentPage(currentPage);
		
		Widget pages = m_wFinishedObjectivesWidget.FindAnyWidget("Pages");
		if (!pages)
			return;
		
		SCR_SelectionHintComponent pagesVisualised = SCR_SelectionHintComponent.Cast(pages.FindHandler(SCR_SelectionHintComponent));
		if (!pagesVisualised)
			return;
		
		pagesVisualised.SetItemCount(m_iFinishedTasksCount/6);
		pagesVisualised.SetCurrentItem(currentPage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get current page
	//! \return current page
	int GetCurrentPage()
	{
		return m_iCurrentPage;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets current page number which just changes the member variable and nothing more. 
	//! Method FlipPage actually performs UI Changes based on this member variable.
	//! \param[in] page
	void SetCurrentPage(int page)
	{
		m_iCurrentPage = page;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return title
	string GetTitleText()
	{
		return m_sTitleText;
	}
}

//! Class handling Statistics retrieved from the DataCollector
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenStatisticsContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;
	
	[Attribute()]
	protected ref array<ref SCR_DebriefingScreenStatisticBaseClass> m_aStatistics;
	
	[Attribute(defvalue: "{64B98222099C391E}UI/layouts/Menus/DeployMenu/DebriefingScreenStatisticsTile.layout")]
	protected ResourceName m_sStatisticsTileLayout;
	
	[Attribute(defvalue: "{F2B6C48FDA2E261E}UI/layouts/Menus/DeployMenu/DebriefingScreenStatistic.layout")]
	protected ResourceName m_sStatisticsLayout;
	
	protected Widget m_wStatisticsWidget;
	
	//------------------------------------------------------------------------------------------------
	//! Initialises content for given column
	//! \param[in] menu
	//! \param[in] column
	//! \param[in] columnButton
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
			return;
		
		m_wStatisticsWidget = GetGame().GetWorkspace().CreateWidgets(m_sStatisticsTileLayout, targetTile);
		if (!m_wStatisticsWidget)
			return;
		
		Widget statisticsContent = m_wStatisticsWidget.FindAnyWidget("StatsLayout");
		if (!statisticsContent)
			return;

		FillStatisticsWidget(statisticsContent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fills content widget with statistics
	//! \param[in] content
	void FillStatisticsWidget(Widget content)
	{
		RichTextWidget titleText = RichTextWidget.Cast(m_wStatisticsWidget.FindAnyWidget("TitleText"));
		titleText.SetText(GetTitleText());
		
		array<ref SCR_DebriefingScreenStatisticBaseClass> statistics = {};
		int statisticsCount;
		statisticsCount = GetStatistics(statistics);
		
		int statsCycleCount;
		if (statisticsCount < 7)
			statsCycleCount = statisticsCount;
		else
			statsCycleCount = 6;
		
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(0, true, false);
		//We Calculate the stats difference only applicable for this particular session
		array<float> stats = playerData.CalculateStatsDifference();
		//Statistics are capped for 6 at the time
		for (int i = 0; i < statsCycleCount; ++i)
		{
			//statsCycleCount--;
			Widget statistic = GetGame().GetWorkspace().CreateWidgets(m_sStatisticsLayout, content);

			statistics[i].InitStatistic(stats);
			RichTextWidget name = RichTextWidget.Cast(statistic.FindAnyWidget("StatisticResult"));
			name.SetText(statistics[i].GetStatisticResult());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get statistics
	//! \param[out] statistics array of statistics
	//! \return number of statistics
	int GetStatistics(out array<ref SCR_DebriefingScreenStatisticBaseClass> statistics)
	{
		statistics = m_aStatistics;

		return m_aStatistics.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return title
	string GetTitleText()
	{
		return m_sTitleText;
	}
}

//! Base class that holds and handles information about statistic
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticBaseClass
{
	protected string m_sStatisticResult;
	
	//------------------------------------------------------------------------------------------------
	//! Initialises statistics and performs necessary data retrieval and assigment
	//! \param[in] stats
	void InitStatistic(notnull array<float> stats);
	
	//------------------------------------------------------------------------------------------------
	//! \return statistic result
	string GetStatisticResult()
	{
		return m_sStatisticResult;
	}
}

//! Class that holds information and handles information about shots fired
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticShotsFired : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_Rounds</b>", Math.Floor(stats[SCR_EDataStats.SHOTS])));
	}
}

//! Class that holds information and handles information about kills
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticKills : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_Kills</b>", Math.Floor(stats[SCR_EDataStats.AI_KILLS] + stats[SCR_EDataStats.KILLS])));
	}
}

//! Class that holds information and handles information about deaths
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticDeaths : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------

	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_Deaths</b>", Math.Floor(stats[SCR_EDataStats.DEATHS])));
	}
}

//! Class that holds information and handles information about distance walked
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticDistanceWalked : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_DistanceWalked</b>", Math.Floor(stats[SCR_EDataStats.DISTANCE_WALKED]/1000)));
	}
}

//! Class that holds information and handles information about distance driven
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticDistanceDriven : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_DistanceDriven</b>", Math.Floor(stats[SCR_EDataStats.DISTANCE_DRIVEN]/1000)));
	}
}

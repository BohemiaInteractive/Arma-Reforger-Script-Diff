[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Welcome screen shown in respawn menu.")]
class SCR_WelcomeScreenComponentClass : SCR_DeployMenuBaseScreenComponentClass
{
}

//! Welcome screen component intended to be added to the GameMode.
class SCR_WelcomeScreenComponent : SCR_DeployMenuBaseScreenComponent
{
};

//! Class handling layout for ThreeHorizontalColumns.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_WelcomeScreenThreeHorizontalColumns : SCR_DeployMenuBaseScreenLayout
{
	[Attribute(defvalue: "{16B14338A8911683}UI/layouts/Menus/DeployMenu/WelcomeScreenThreeHorizontalColumns.layout")]
	protected ResourceName m_sThreeHorizontalColumnsLayout;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_LeftColumn;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_MiddleColumn;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_RightColumn;
	
	protected const string WELCOME_CONTENT = "WelcomeContent";
	
	protected const string LEFT_COLUMN = "LeftColumn";
	protected const string MIDDLE_COLUMN = "MiddleColumn";
	protected const string RIGHT_COLUMN = "RightColumn";
	
	protected const string LEFT_COLUMN_BUTTON = "LeftColumnButton";
	protected const string MIDDLE_COLUMN_BUTTON = "MiddleColumnButton";
	protected const string RIGHT_COLUMN_BUTTON = "RightColumnButton";

	//------------------------------------------------------------------------------------------------
	override void InitContent(SCR_WelcomeScreenMenu menu)
	{
		Widget targetColumn = menu.GetRootWidget().FindAnyWidget(WELCOME_CONTENT);
		if (!targetColumn)
			return;

		Widget threeHorizontalColumns = GetGame().GetWorkspace().CreateWidgets(m_sThreeHorizontalColumnsLayout, targetColumn);

		if (m_LeftColumn)
		{
			m_aScreenBaseContents.Insert(m_LeftColumn);
			m_LeftColumn.InitContent(menu, LEFT_COLUMN, LEFT_COLUMN_BUTTON);
		}

		if (m_MiddleColumn)
		{
			m_aScreenBaseContents.Insert(m_MiddleColumn);
			m_MiddleColumn.InitContent(menu, MIDDLE_COLUMN, MIDDLE_COLUMN_BUTTON);
		}

		if (m_RightColumn)
		{
			m_aScreenBaseContents.Insert(m_RightColumn);
			m_RightColumn.InitContent(menu, RIGHT_COLUMN, RIGHT_COLUMN_BUTTON);
		}
	}
}

//! Base class for screen contents.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_WelcomeScreenBaseContent : ScriptAndConfig
{
	[Attribute()]
	protected bool m_bIsInteractible;
	
	protected string m_sContentName;
	
	//------------------------------------------------------------------------------------------------
	//! Initialises content for given column
	//! \param[in] menu
	//! \param[in] column
	//! \param[in] columnButton
	void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggles whether or not this content is interactive
	//! \param[in] enabled
	void ToggleInteractions(bool enabled);
	
	//------------------------------------------------------------------------------------------------
	//! \return content name
	string GetContentName()
	{
		return m_sContentName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if it is interactible
	//! \return if it is interactive or not
	bool GetIsInteractible()
	{
		return m_bIsInteractible;
	}
}

//! Class handling mission objectives content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_WelcomeScreenMissionObjectivesContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;

	[Attribute()]
	protected ref array<ref SCR_WelcomeScreenMissionObjectives> m_aMissionObjectives;

	[Attribute()]
	protected string m_sBottomTitleText;

	protected Widget m_wMissionObjectivesWidget;
	protected int m_iObjectivesCount;

	[Attribute(defvalue: "{ECDF3386111C93B6}UI/layouts/Menus/DeployMenu/WelcomeScreenMissionObjectivesColumn.layout")]
	protected ResourceName m_sMissionObjectivesColumnLayout;

	[Attribute(defvalue: "{B03D5FCC04D25B95}UI/layouts/Menus/DeployMenu/WelcomeScreenMissionObjective.layout")]
	protected ResourceName m_sMissionObjectiveLayout;

	//------------------------------------------------------------------------------------------------
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		if (!m_aMissionObjectives)
			return;

		m_iObjectivesCount = m_aMissionObjectives.Count();
		Widget targetColumn = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetColumn)
			return;
		
		m_wMissionObjectivesWidget = GetGame().GetWorkspace().CreateWidgets(m_sMissionObjectivesColumnLayout, targetColumn);
		if (!m_wMissionObjectivesWidget)
			return;
		
		Widget missionObjectivesContent = m_wMissionObjectivesWidget.FindAnyWidget("MiddleContentVertical");
		if (!missionObjectivesContent)
			return;
		
		FillMissionObjectivesWidget(missionObjectivesContent)
	}

	//------------------------------------------------------------------------------------------------
	//! Fills content widget with mission objectives
	//! \param[in] content
	protected void FillMissionObjectivesWidget(Widget content)
	{
		RichTextWidget titleText = RichTextWidget.Cast(m_wMissionObjectivesWidget.FindAnyWidget("TitleText"));
		if (titleText)
			titleText.SetText(GetTitleText());

		array<ref SCR_WelcomeScreenMissionObjectives> objectives = {};
		SCR_WelcomeScreenMissionObjectives objective;
		int objectivesCount;
		objectivesCount = GetMissionObjectives(objectives);

		int cycleCount;
		if (objectivesCount < 11)
			cycleCount = objectivesCount;
		else
			cycleCount = 10;

		//Mission objectives are capped for 10 at the time
		for (int i = 0; i < cycleCount; ++i)
		{
			objective = objectives[i];
			objectivesCount--;
			Widget cond = GetGame().GetWorkspace().CreateWidgets(m_sMissionObjectiveLayout, content);
			if (!cond)
				break;
			
			RichTextWidget name = RichTextWidget.Cast(cond.FindAnyWidget("ObjectiveText"));
			if (name)
				name.SetText(objective.GetDescription());

			ImageWidget icon = ImageWidget.Cast(cond.FindAnyWidget("Icon"));
			if (icon)
				icon.LoadImageFromSet(0, objective.GetImageSet(), objective.GetObjectiveQuadName(), false);
		}

		RichTextWidget bottomTitleText = RichTextWidget.Cast(m_wMissionObjectivesWidget.FindAnyWidget("BottomTitleText"));
		if (!bottomTitleText)
			return;
		
		if (objectivesCount < 1)
			bottomTitleText.SetOpacity(0);
		else
			bottomTitleText.SetTextFormat(GetBottomTitleText(), objectivesCount);
	}

	//------------------------------------------------------------------------------------------------
	//! Get mission objectives
	//! \param[out] missionObjectives array of mission objectives
	//! \return number of mission objectives
	int GetMissionObjectives(out array<ref SCR_WelcomeScreenMissionObjectives> missionObjectives)
	{
		missionObjectives = m_aMissionObjectives;

		return m_aMissionObjectives.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return title
	string GetTitleText()
	{
		return m_sTitleText;
	}

	//------------------------------------------------------------------------------------------------
	//! \return bottom title
	string GetBottomTitleText()
	{
		return m_sBottomTitleText;
	}
}

//! Class that holds information about mission objective
[BaseContainerProps()]
class SCR_WelcomeScreenMissionObjectives
{
	[Attribute("{88966C7AB9720818}UI/Textures/DeployMenu/Objectives-Briefing/Objectives-briefing.imageset", UIWidgets.ResourcePickerThumbnail, "Image set for the icons", params: "edds")]
	protected ResourceName m_sObjectiveImageSet;
	
	[Attribute("faction")]
	protected string m_sObjectiveQuadName;

	[Attribute()]
	protected string m_sDescription;

	//------------------------------------------------------------------------------------------------
	//! \return image set
	ResourceName GetImageSet()
	{
		return m_sObjectiveImageSet;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return objective quad name
	string GetObjectiveQuadName()
	{
		return m_sObjectiveQuadName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return description
	string GetDescription()
	{
		return m_sDescription;
	}
}

//! Class handling mission objectives that are dynamically created during the mission.
//! Currently not enabled for WB usage due to GM tasks not properly working in MP.
//[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_WelcomeScreenDynamicObjectivesContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;
	
	[Attribute()]
	protected ref array<ref SCR_WelcomeScreenDynamicTaskFaction> m_aFactions;
	
	[Attribute(defvalue: "{0B7DDF77E24F0C63}UI/layouts/Menus/DeployMenu/WelcomeScreenDynamicObjectivesColumn.layout")]
	protected ResourceName m_sFinishedObjectivesTileLayout;
	
	[Attribute(defvalue: "{B03D5FCC04D25B95}UI/layouts/Menus/DeployMenu/WelcomeScreenMissionObjective.layout")]
	protected ResourceName m_sFinishedObjectivesLayout;
	
	protected Widget m_wFinishedObjectivesWidget;
	protected Widget m_wPaginationWidget;
	protected ButtonWidget m_wColumnButton;
	protected ref array<SCR_Task> m_aObjectivesToDisplay = {};
	protected ref array<Widget> m_aDynamicObjectivesWidgets = {};
	protected int m_iCurrentPage;
	protected int m_iDynamicObjectivesCount;
	protected SCR_FactionManager m_FactionManager;
	
	//------------------------------------------------------------------------------------------------
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

		FillDynamicObjectivesWidget(finishedObjectivesContent);
		InitPagination();
		HandlePagination();
	}
	
	//------------------------------------------------------------------------------------------------
	override void ToggleInteractions(bool enabled)
	{
		HandlePagination(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fills content widget with dynamic mission objectives
	//! \param[in] content
	void FillDynamicObjectivesWidget(Widget content)
	{
		RichTextWidget titleText = RichTextWidget.Cast(m_wFinishedObjectivesWidget.FindAnyWidget("TitleText"));
		titleText.SetText(GetTitleText());
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!m_FactionManager)
			return;

		m_aObjectivesToDisplay.Clear();
		taskSystem.GetTasks(m_aObjectivesToDisplay);
		m_iDynamicObjectivesCount = m_aObjectivesToDisplay.Count();
		
		foreach (SCR_WelcomeScreenDynamicTaskFaction faction : m_aFactions)
		{
			string factionKey = faction.GetFactionKey();
			foreach (SCR_Task task : m_aObjectivesToDisplay)
			{
				if (!task.GetOwnerFactionKeys().Contains(factionKey))
					continue;
			
				faction.AddFactionTask(task);
			}
		}

		//Statistics are capped for 10 at the time
		m_aDynamicObjectivesWidgets.Clear();
		for (int i = 0; i < 10; ++i)
		{
			Widget objective = GetGame().GetWorkspace().CreateWidgets(m_sFinishedObjectivesLayout, content);
			m_aDynamicObjectivesWidgets.Insert(objective);
		}
		
		//SCR_BaseTaskManager.s_OnTaskCreated.Insert(AddTask);
		
		//SCR_BaseTaskManager.s_OnTaskFinished.Insert(RemoveTask);		
		//SCR_BaseTaskManager.s_OnTaskCancelled.Insert(RemoveTask);
		//SCR_BaseTaskManager.s_OnTaskFailed.Insert(RemoveTask);
		
		FlipPage(GetCurrentPage());
		
		m_wPaginationWidget = m_wFinishedObjectivesWidget.FindAnyWidget("BottomTitleSizeLayout");
		if (!m_wPaginationWidget)
			return;
		
		if (m_iDynamicObjectivesCount < 11)
		{
			m_wPaginationWidget.SetOpacity(0);
		}
		else
		{
			Widget pages = m_wFinishedObjectivesWidget.FindAnyWidget("Pages");
			if (!pages)
				return;
			
			SCR_SelectionHintComponent pagesVisualised = SCR_SelectionHintComponent.Cast(pages.FindHandler(SCR_SelectionHintComponent));
			if (!pagesVisualised)
				return;
			
			pagesVisualised.SetItemCount(m_iDynamicObjectivesCount/10);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls method with delay to add a task to the array of objectives to be displayed.
	//! \param[in] task
	protected void AddTask(notnull SCR_Task task)
	{
		//Since this is called right after task creation, the task itself might not have the 
		//Title, faction or other properties set just yet. By introducing slight delay, we mitigate it in most cases
		GetGame().GetCallqueue().CallLater(AddTaskCalledLater, 1000, false, task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds task to the array of objectives to be displayed.
	//! \param[in] task
	protected void AddTaskCalledLater(notnull SCR_Task task)
	{
		foreach (SCR_WelcomeScreenDynamicTaskFaction faction : m_aFactions)
		{
			if (!task.GetOwnerFactionKeys().Contains(faction.GetFactionKey()))
				continue;
			
			faction.AddFactionTask(task);
			break;
		}
		
		m_aObjectivesToDisplay.Clear();
		foreach (SCR_WelcomeScreenDynamicTaskFaction faction : m_aFactions)
		{
			m_aObjectivesToDisplay.InsertAll(faction.GetFactionTasks());
		}
		
		m_iDynamicObjectivesCount = m_aObjectivesToDisplay.Count();
		
		FlipPage(GetCurrentPage());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes task from the array of objectives to be displayed.
	//! \param[in] task
	protected void RemoveTask(notnull SCR_Task task)
	{
		foreach (SCR_WelcomeScreenDynamicTaskFaction faction : m_aFactions)
		{
			if (!task.GetOwnerFactionKeys().Contains(faction.GetFactionKey()))
				continue;
			
			faction.RemoveFactionTask(task);
			break;
		}
		
		m_aObjectivesToDisplay.Clear();
		foreach (SCR_WelcomeScreenDynamicTaskFaction faction : m_aFactions)
		{
			m_aObjectivesToDisplay.InsertAll(faction.GetFactionTasks());
		}
		
		m_iDynamicObjectivesCount = m_aObjectivesToDisplay.Count();

		FlipPage(GetCurrentPage());
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
			return;

		FlipPage(currentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Flips page to the next one
	protected void NextButtonActivated()
	{
		int currentPage = GetCurrentPage();
		currentPage++;
		if (currentPage == Math.Ceil(m_iDynamicObjectivesCount/10))
			return;

		FlipPage(currentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the content of current page based on the provided number
	//! \param[in] currentPage
	protected void FlipPage(int currentPage)
	{
		for (int i = 0; i < 10; ++i)
		{
			if (!m_aDynamicObjectivesWidgets[i])
				continue;
			
			m_aDynamicObjectivesWidgets[i].SetOpacity(1);
			if (currentPage == 0)
			{
				if (i < m_iDynamicObjectivesCount)
				{
					RichTextWidget name = RichTextWidget.Cast(m_aDynamicObjectivesWidgets[i].FindAnyWidget("ObjectiveText"));
					SCR_EditorTask editorTask = SCR_EditorTask.Cast(m_aObjectivesToDisplay[i]);
					SCR_CampaignMilitaryBaseTaskEntity campaignTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(m_aObjectivesToDisplay[i]);
					if (editorTask)
						name.SetTextFormat(editorTask.GetTaskName(), editorTask.GetLocationName());
					else if (campaignTask)
						name.SetTextFormat(campaignTask.GetTaskName(), campaignTask.GetMilitaryBase().GetCallsign());
					else
						name.SetText(m_aObjectivesToDisplay[i].GetTaskName());
				
					ImageWidget image = ImageWidget.Cast(m_aDynamicObjectivesWidgets[i].FindAnyWidget("Background"));
					image.SetColor(m_FactionManager.GetFactionByKey(m_aObjectivesToDisplay[i].GetOwnerFactionKeys()[0]).GetFactionColor());
				}
				else
				{
					m_aDynamicObjectivesWidgets[i].SetOpacity(0);
				}
			}
			else
			{
				if ((currentPage * 10) + i < m_iDynamicObjectivesCount)
				{
					RichTextWidget name = RichTextWidget.Cast(m_aDynamicObjectivesWidgets[i].FindAnyWidget("ObjectiveText"));
					SCR_EditorTask editorTask = SCR_EditorTask.Cast(m_aObjectivesToDisplay[(currentPage * 10) + i]);
					SCR_CampaignMilitaryBaseTaskEntity campaignTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(m_aObjectivesToDisplay[(currentPage * 10) + i]);
					if (editorTask)
						name.SetTextFormat(editorTask.GetTaskName(), editorTask.GetLocationName());
					else if (campaignTask)
						name.SetTextFormat(campaignTask.GetTaskName(), campaignTask.GetMilitaryBase().GetCallsign());
					else
						name.SetText(m_aObjectivesToDisplay[(currentPage * 10) + i].GetTaskName());
					
					ImageWidget image = ImageWidget.Cast(m_aDynamicObjectivesWidgets[i].FindAnyWidget("Background"));
					image.SetColor(m_FactionManager.GetFactionByKey(m_aObjectivesToDisplay[(currentPage * 10) + i].GetOwnerFactionKeys()[0]).GetFactionColor());
				}
				else
				{
					m_aDynamicObjectivesWidgets[i].SetOpacity(0);
				}
			}
		}
		
		SetCurrentPage(currentPage);
		
		if (m_wPaginationWidget)
		{
			if (m_iDynamicObjectivesCount < 11)
				m_wPaginationWidget.SetOpacity(0);
			else
				m_wPaginationWidget.SetOpacity(1);
			
			Widget pages = m_wFinishedObjectivesWidget.FindAnyWidget("Pages");
			if (!pages)
				return;
			
			SCR_SelectionHintComponent pagesVisualised = SCR_SelectionHintComponent.Cast(pages.FindHandler(SCR_SelectionHintComponent));
			if (!pagesVisualised)
				return;
			
			pagesVisualised.SetItemCount(m_iDynamicObjectivesCount/10);
			pagesVisualised.SetCurrentItem(currentPage);
		}
	}
	
	//------------------------------------------------------------------------------------------------
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

//! Class that holds information about dynamic tasks assigned to faction
[BaseContainerProps()]
class SCR_WelcomeScreenDynamicTaskFaction
{
	[Attribute()]
	protected string m_sFactionKey;

	[Attribute()]
	protected ref array<SCR_Task> m_aFactionTasks = {};

	//------------------------------------------------------------------------------------------------
	//! \return array of faction tasks
	array<SCR_Task> GetFactionTasks()
	{
		return m_aFactionTasks;
	}

	//------------------------------------------------------------------------------------------------
	//! \return faction key
	string GetFactionKey()
	{
		return m_sFactionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds task to this faction class
	//! \param[in] task
	void AddFactionTask(notnull SCR_Task task)
	{
		if (!m_aFactionTasks.Contains(task))
			m_aFactionTasks.Insert(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds task from this faction class
	//! \param[in] task
	void RemoveFactionTask(notnull SCR_Task task)
	{
		m_aFactionTasks.RemoveItem(task);
	}
}

//! Class handling introduction content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_WelcomeScreenIntroductionContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;

	[Attribute()]
	protected ref array<ref SCR_WelcomeScreenIntroduction> m_aIntroduction;

	protected int m_iCurrentPage;
	protected int m_iIntroductionCount;

	[Attribute(defvalue: "{49EEED554D68F43A}UI/layouts/Menus/DeployMenu/WelcomeScreenIntroductionColumn.layout")]
	protected ResourceName m_sIntroductionColumnLayout;

	protected Widget m_wIntroductionContentWidget;
	protected ButtonWidget m_wColumnButton;

	//------------------------------------------------------------------------------------------------
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		if (!m_aIntroduction)
			return;

		m_iIntroductionCount = m_aIntroduction.Count();
		Widget targetColumn = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetColumn)
			return;
		
		m_wColumnButton = ButtonWidget.Cast(menu.GetRootWidget().FindAnyWidget(columnButton));
		if (!m_wColumnButton)
			return;
		
		m_wIntroductionContentWidget = GetGame().GetWorkspace().CreateWidgets(m_sIntroductionColumnLayout, targetColumn);
		if (!m_wIntroductionContentWidget)
			return;

		FillIntroductionWidget(targetColumn);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ToggleInteractions(bool enabled)
	{
		HandlePagination(enabled);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills content widget with dynamic introductions
	//! \param[in] column
	protected void FillIntroductionWidget(Widget column)
	{
		RichTextWidget titleTextContent = RichTextWidget.Cast(column.FindAnyWidget("TitleText"));
		if (titleTextContent)
			titleTextContent.SetText(GetTitleText());

		array<ref SCR_WelcomeScreenIntroduction> introductionPages = {};
		int introductionCount = GetIntroduction(introductionPages);
		if (introductionCount == 0)
			return;

		Widget contentVertical = m_wIntroductionContentWidget.FindAnyWidget("ContentVertical");
		if (!contentVertical)
			return;
		
		ImageWidget imageWidget = ImageWidget.Cast(contentVertical.FindAnyWidget("ImageWidget"));
		if (imageWidget && introductionPages[0].GetImage())
			imageWidget.LoadImageTexture(0, introductionPages[0].GetImage(), false, false);

		RichTextWidget titleText = RichTextWidget.Cast(contentVertical.FindAnyWidget("TitleText"));
		if (titleText)
			titleText.SetText(introductionPages[0].GetTitleText());

		RichTextWidget descriptionText = RichTextWidget.Cast(contentVertical.FindAnyWidget("DescriptionText"));
		if (descriptionText)
			descriptionText.SetText(introductionPages[0].GetDescriptionText());

		Widget pages = m_wIntroductionContentWidget.FindAnyWidget("Pages");
		if (!pages)
			return;
		
		SCR_SelectionHintComponent pagesVisualised = SCR_SelectionHintComponent.Cast(pages.FindHandler(SCR_SelectionHintComponent));
		if (!pagesVisualised)
			return;
		
		Widget pagination = m_wIntroductionContentWidget.FindAnyWidget("Pagination");
		if (!pagination)
			return;
		
		if (introductionCount == 1)
		{
			pagination.SetOpacity(0);
		}
		else
		{
			pagesVisualised.SetItemCount(introductionCount);
			InitPagination();
			HandlePagination();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialises pagination by registering button actions
	protected void InitPagination()
	{
		Widget previousButtonWidget = m_wIntroductionContentWidget.FindAnyWidget("PrevButton");
		if (!previousButtonWidget)
			return;
		
		SCR_PagingButtonComponent previousButton = SCR_PagingButtonComponent.Cast(previousButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (previousButton)
			previousButton.m_OnClicked.Insert(ButtonClicked);

		Widget nextButtonWidget = m_wIntroductionContentWidget.FindAnyWidget("NextButton");
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
		Widget previousButtonWidget = m_wIntroductionContentWidget.FindAnyWidget("PrevButton");
		if (!previousButtonWidget)
			return;
		
		SCR_PagingButtonComponent previousButton = SCR_PagingButtonComponent.Cast(previousButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (!previousButton)
			return;
		
		if (enabled)
			previousButton.m_OnActivated.Insert(PreviousButtonActivated);
		else
			previousButton.m_OnActivated.Remove(PreviousButtonActivated);

		Widget nextButtonWidget = m_wIntroductionContentWidget.FindAnyWidget("NextButton");
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
			return;

		FlipPage(currentPage)
	}

	//------------------------------------------------------------------------------------------------
	//! Flips page to the next one
	protected void NextButtonActivated()
	{
		int currentPage = GetCurrentPage();
		currentPage++;
		if (currentPage == m_iIntroductionCount)
			return;

		FlipPage(currentPage)
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the content of current page based on the provided number
	//! \param[in] currentPage
	protected void FlipPage(int currentPage)
	{
		Widget contentVertical = m_wIntroductionContentWidget.FindAnyWidget("ContentVertical");
		if (!contentVertical)
			return;
		
		SetCurrentPage(currentPage);
		SCR_WelcomeScreenIntroduction targetIntroduction = m_aIntroduction[currentPage];
		if (!targetIntroduction)
			return;
		
		ImageWidget imageWidget = ImageWidget.Cast(contentVertical.FindAnyWidget("ImageWidget"));
		if (imageWidget && targetIntroduction.GetImage())
			imageWidget.LoadImageTexture(0, targetIntroduction.GetImage(), false, false);

		RichTextWidget titleText = RichTextWidget.Cast(contentVertical.FindAnyWidget("TitleText"));
		if (titleText)
			titleText.SetText(targetIntroduction.GetTitleText());

		RichTextWidget descriptionText = RichTextWidget.Cast(contentVertical.FindAnyWidget("DescriptionText"));
		if (descriptionText)
			descriptionText.SetText(targetIntroduction.GetDescriptionText());

		Widget pages = m_wIntroductionContentWidget.FindAnyWidget("Pages");
		if (!pages)
			return;
		
		SCR_SelectionHintComponent pagesVisualised = SCR_SelectionHintComponent.Cast(pages.FindHandler(SCR_SelectionHintComponent));
		if (!pagesVisualised)
			return;
		
		pagesVisualised.SetCurrentItem(currentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Get Introduction which can be separated into several pages
	//! \param[out] introduction array of introductions
	//! \return number of introductions
	int GetIntroduction(out array<ref SCR_WelcomeScreenIntroduction> introduction)
	{
		introduction = m_aIntroduction;

		return m_aIntroduction.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return title
	string GetTitleText()
	{
		return m_sTitleText;
	}

	//------------------------------------------------------------------------------------------------
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
}

//! Class that holds information about Introduction
[BaseContainerProps()]
class SCR_WelcomeScreenIntroduction
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Flag icon of this particular faction.", params: "edds")]
	protected ResourceName m_sContentImage;

	[Attribute()]
	protected string m_sContentTitleText;

	[Attribute()]
	protected string m_sContentDescriptionText;

	//------------------------------------------------------------------------------------------------
	//! \return image
	ResourceName GetImage()
	{
		return m_sContentImage;
	}

	//------------------------------------------------------------------------------------------------
	//! \return title
	string GetTitleText()
	{
		return m_sContentTitleText;
	}

	//------------------------------------------------------------------------------------------------
	//! \return description
	string GetDescriptionText()
	{
		return m_sContentDescriptionText;
	}
}

//! Class handling Factions that are present in the game.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_WelcomeScreenFactionContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;

	[Attribute()]
	protected string m_sBottomTitleText;

	[Attribute(defvalue: "{4C4460227BBC5134}UI/layouts/Menus/DeployMenu/WelcomeScreenFactionColumn.layout")]
	protected ResourceName m_sFactionColumnLayout;

	[Attribute(defvalue: "{68CCF1BD8F7AF86C}UI/layouts/Menus/DeployMenu/WelcomeScreenFaction.layout")]
	protected ResourceName m_sFactionLayout;
	
	[Attribute(defvalue: "{1228936E5DB30403}UI/Textures/GroupManagement/FlagIcons/GroupFlagsBlufor.imageset")]
	protected ResourceName m_sBluforLayout;
	
	[Attribute(defvalue: "{7CD99D22C7AE8195}UI/Textures/GroupManagement/FlagIcons/GroupFlagsOpfor.imageset")]
	protected ResourceName m_sOpforLayout;
		
	[Attribute(defvalue: "{301FC1A2A46D3E0D}UI/Textures/GroupManagement/FlagIcons/GroupFlagsIndfor.imageset")]
	protected ResourceName m_sIndforLayout;
	
	[Attribute("US")]
	protected string m_sUSFaction;
	
	[Attribute("USSR")]
	protected string m_sUSSRFaction;
	
	[Attribute("FIA")]
	protected string m_sFIAFaction;

	protected Widget m_wFactionContentWidget;
	protected ref array<Widget> m_aFactionWidgets = {};
	protected ref SCR_SortedArray<SCR_Faction> m_SortedFactions = new SCR_SortedArray<SCR_Faction>();

	//------------------------------------------------------------------------------------------------
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		Widget targetColumn = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetColumn)
			return;
		
		m_wFactionContentWidget = GetGame().GetWorkspace().CreateWidgets(m_sFactionColumnLayout, targetColumn);
		if (!m_wFactionContentWidget)
			return;
		
		AddFactionWidget();
	}

	//------------------------------------------------------------------------------------------------
	//! Adds new faction widget to the content
	protected void AddFactionWidget()
	{
		RichTextWidget titleTextContent = RichTextWidget.Cast(m_wFactionContentWidget.FindAnyWidget("TitleText"));
		if (titleTextContent)
			titleTextContent.SetText(GetTitleText());

		//This part is WIP and prepared for sorting factions by their sides
		Widget blueforContent = m_wFactionContentWidget.FindAnyWidget("Bluefor"); //#22c4f4
		Widget opforContent = m_wFactionContentWidget.FindAnyWidget("Opfor"); 		//#ee312f
		Widget independentContent = m_wFactionContentWidget.FindAnyWidget("Independent"); 	//#00b14f
		Widget civilianContent = m_wFactionContentWidget.FindAnyWidget("Civilian"); 		//WIP
		
		if (!blueforContent)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		factionManager.GetOnPlayerFactionCountChanged().Insert(UpdateFactionPlayerCount);

		int factionCount = factionManager.GetSortedFactionsList(m_SortedFactions);
		
		for (int i = 0; i < factionCount; i++)
		{
			m_SortedFactions[i].GetOnFactionPlayableChanged().Insert(UpdateFactionPlayability);
		}
		//Color blueforColor = new Color(4.0, 141.0, 231.0, 255.0);
		
		int cycleCount;
		if (factionCount < 7)
			cycleCount = factionCount;
		else
			cycleCount = 6;

		//Factions are capped for 6 at the time
		for (int i = 0; i < cycleCount; i++)
		{
			factionCount--;
			Color factionColor = m_SortedFactions[i].GetFactionColor();
			FillFactionWidget(m_SortedFactions[i], blueforContent, factionColor);
		}

		RichTextWidget bottomTitleText = RichTextWidget.Cast(m_wFactionContentWidget.FindAnyWidget("BottomTitleText"));
		if (!bottomTitleText)
			return;
		
		if (factionCount < 1)
			bottomTitleText.SetOpacity(0);
		else
			bottomTitleText.SetTextFormat(GetBottomTitleText(), factionCount);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills content widget with factions
	//! \param[in] faction
	//! \param[in] content
	//! \param[in] color
	protected void FillFactionWidget(notnull SCR_Faction faction, notnull Widget content, Color color)
	{
		if (!faction.IsPlayable() && !faction.IsShownInWelcomeScreenIfNonPlayable())
			return;
		
		Widget factionWidget = GetGame().GetWorkspace().CreateWidgets(m_sFactionLayout, content);
		if (!factionWidget)
			return;
		
		m_aFactionWidgets.Insert(factionWidget);

		ImageWidget flag = ImageWidget.Cast(factionWidget.FindAnyWidget("FlagImage"));
		if (flag)
			flag.LoadImageTexture(0, faction.GetFactionFlag(), false, false);

		ImageWidget side = ImageWidget.Cast(factionWidget.FindAnyWidget("SideImage"));
		if (side)
		{
			ResourceName ImageSetResource;
			
			if (faction.GetFactionKey() ==  m_sUSFaction)
				ImageSetResource = m_sBluforLayout;
			
			else if (faction.GetFactionKey() ==  m_sUSSRFaction)
				ImageSetResource = m_sOpforLayout;
			
			else if (faction.GetFactionKey() ==  m_sFIAFaction)
				ImageSetResource = m_sIndforLayout;
			
			else 
			{
				side.SetEnabled(false);
				side.SetVisible(false);
				return;
			}
			
			side.LoadImageFromSet(0, ImageSetResource, "undefined");
			side.SetColor(faction.GetFactionColor());
		}	

		RichTextWidget name = RichTextWidget.Cast(factionWidget.FindAnyWidget("FactionNameText"));
		if (name)
			name.SetText(faction.GetFactionName());
		
		ImageWidget bar = ImageWidget.Cast(factionWidget.FindAnyWidget("Bar"));
		if (bar)
			bar.SetVisible(false);

		RichTextWidget playerCount = RichTextWidget.Cast(factionWidget.FindAnyWidget("FactionPlayerCount"));
		if (!playerCount)
			return;
		
		int playerLimit = faction.GetPlayerLimit();
		ImageWidget playerIcon = ImageWidget.Cast(factionWidget.FindAnyWidget("PlayerIcon"));
		if (playerLimit != 0 && faction.IsPlayable())
		{
			if (playerLimit > 0)
				playerCount.SetTextFormat("#AR-SupportStation_ActionFormat_ItemAmount", faction.GetPlayerCount(), playerLimit);
			else
				playerCount.SetText(faction.GetPlayerCount().ToString());
		}
		else
		{
			if (playerIcon)
				playerIcon.SetVisible(false);
			
			playerCount.SetText("#AR-DeployScreen_NonPlayableFaction");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates faction player count when player count of said faction changes
	//! \param[in] faction
	//! \param[in] playerCountParam
	protected void UpdateFactionPlayerCount(Faction faction, int playerCountParam)
	{
		SCR_Faction factionScripted = SCR_Faction.Cast(faction);
		if (!factionScripted)
			return;
		
		Widget factionWidget = m_aFactionWidgets[m_SortedFactions.Find(factionScripted)];
		if (!factionWidget)
			return;
		
		RichTextWidget playerCount = RichTextWidget.Cast(factionWidget.FindAnyWidget("FactionPlayerCount"));
		if (!playerCount)
			return;
		
		int playerLimit = factionScripted.GetPlayerLimit();
		if (playerLimit > 0)
			playerCount.SetTextFormat("#AR-SupportStation_ActionFormat_ItemAmount", playerCountParam, playerLimit);
		else
			playerCount.SetText(playerCountParam.ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates faction playability when playability of said faction changes
	//! \param[in] faction
	//! \param[in] playable
	protected void UpdateFactionPlayability(Faction faction, bool playable)
	{
		SCR_Faction factionScripted = SCR_Faction.Cast(faction);
		if (!factionScripted)
			return;
		
		Widget factionWidget = m_aFactionWidgets[m_SortedFactions.Find(factionScripted)];
		if (!factionWidget)
			return;
		
		RichTextWidget playerCount = RichTextWidget.Cast(factionWidget.FindAnyWidget("FactionPlayerCount"));
		if (!playerCount)
			return;
		
		ImageWidget playerIcon = ImageWidget.Cast(factionWidget.FindAnyWidget("PlayerIcon"));
		int playerLimit = factionScripted.GetPlayerLimit();
		if (factionScripted.IsPlayable())
		{
			if (playerIcon)
				playerIcon.SetVisible(true);
			
			if (playerLimit >= 0)
				playerCount.SetTextFormat("#AR-SupportStation_ActionFormat_ItemAmount", factionScripted.GetPlayerCount(), playerLimit);
			else
				playerCount.SetText((factionScripted.GetPlayerCount()).ToString());
		}
		else
		{
			if (playerIcon)
				playerIcon.SetVisible(false);
			
			playerCount.SetText("#AR-DeployScreen_NonPlayableFaction");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return title
	string GetTitleText()
	{
		return m_sTitleText;
	}

	//------------------------------------------------------------------------------------------------
	//! \return bottom title
	string GetBottomTitleText()
	{
		return m_sBottomTitleText;
	}
}

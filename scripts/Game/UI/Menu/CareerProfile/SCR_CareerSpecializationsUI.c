//------------------------------------------------------------------------------------------------
class SCR_CareerSpecializationsUI : ScriptedWidgetComponent
{
	protected Widget m_wRootWidget;
	protected Widget m_wCareerLeftSide;
	protected Widget m_wCareerRightSide;
	protected Widget m_wSpiderNet;
	protected SCR_SpiderNet m_SpiderNetHandler;

	//Controller navigation buttons
	protected Widget m_wPagingButtons;
	SCR_InputButtonComponent m_HandlerChangeSpLeft;
	SCR_InputButtonComponent m_HandlerChangeSpRight;

	protected SCR_PlayerData m_PlayerData;

	protected ref array<Widget> m_aSpecializationsStatsWidgets = {};

	RichTextWidget m_twSpecializationTitle, m_twSpecializationProgress;
	protected SCR_ProgressBarWithSegments m_SpecializationBar;

	protected int m_iSelectedLegend = 0;
	protected ref array<SCR_ButtonLegendComponent> m_aSpLegendButtonHandlers = {};
	protected ref array<SCR_ButtonSpecializationProgressComponent> m_aSpProgressButtonHandlers = {};

	[Attribute(params: "SpecializationProgress layout")]
	protected ResourceName m_sSpProgressLayout;

	protected ResourceName m_sStatsLayout;
	protected ResourceName m_sHeaderStatsLayout;
	protected ResourceName m_sProgressionStatsLayout;

	protected bool m_bShowProgression = false;

	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wRootWidget = w;

		m_wCareerLeftSide = m_wRootWidget.FindAnyWidget("CareerLeftSide");
		m_wCareerRightSide = m_wRootWidget.FindAnyWidget("CareerRightSide");

		if (!m_wCareerLeftSide || !m_wCareerRightSide)
			return;

		m_wSpiderNet = m_wCareerLeftSide.FindAnyWidget("SpiderNet0");
		if (!m_wSpiderNet)
			return;

		m_SpiderNetHandler = SCR_SpiderNet.Cast(m_wSpiderNet.FindHandler(SCR_SpiderNet));
		if (!m_SpiderNetHandler)
			return;

		m_wPagingButtons = m_wCareerLeftSide.FindAnyWidget("PagingButtons");

		if (!m_wPagingButtons)
			return;

		Widget navLeft = m_wPagingButtons.FindAnyWidget("NavigationButtonNextSp");
		Widget navRight = m_wPagingButtons.FindAnyWidget("NavigationButtonPrevSp");

		if (!navLeft || !navRight)
			return;

		m_HandlerChangeSpLeft = SCR_InputButtonComponent.Cast(navLeft.FindHandler(SCR_InputButtonComponent));
		m_HandlerChangeSpRight = SCR_InputButtonComponent.Cast(navRight.FindHandler(SCR_InputButtonComponent));

		if (!m_HandlerChangeSpLeft || !m_HandlerChangeSpRight)
			return;

		//Navigation actions found in the CareerProfileContext and also in the EndgameScreenContext
		m_HandlerChangeSpLeft.m_OnActivated.Insert(NavLeft);
		m_HandlerChangeSpRight.m_OnActivated.Insert(NavRight);
	}

	//------------------------------------------------------------------------------------------------
	protected void NavLeft(SCR_InputButtonComponent handler, string action)
	{
		int next = m_iSelectedLegend + 1;
		if (next >= m_aSpLegendButtonHandlers.Count())
			next = 0;
		UpdateSpecialization(next);
	}

	//------------------------------------------------------------------------------------------------
	protected void NavRight(SCR_InputButtonComponent handler, string action)
	{
		int prev = m_iSelectedLegend - 1;
		if (prev < 0)
			prev = m_aSpLegendButtonHandlers.Count() - 1;
		UpdateSpecialization(prev);
	}

	//------------------------------------------------------------------------------------------------
	void SetShowProgression(bool b)
	{
		m_bShowProgression = b;
	}

	//------------------------------------------------------------------------------------------------
	void FillSpecializations(SCR_PlayerData playerData, ResourceName statsLayout, ResourceName headerStatsLayout, ResourceName progressionStatsLayout)
	{
		if (!playerData)
			return;

		m_PlayerData = playerData;
		m_sStatsLayout = statsLayout;
		m_sHeaderStatsLayout = headerStatsLayout;
		m_sProgressionStatsLayout = progressionStatsLayout;

		m_SpiderNetHandler.RegisterCareerProfileHandler(this);
		array<float> SpPoints = {};

		//Prepare vertices for the spidernet
		m_PlayerData.FillArrayWithSpecializationPoints(SpPoints);
		m_SpiderNetHandler.SetSpPoints(SpPoints);

		//Progression with previous stats?
		if (m_bShowProgression)
		{
			//Prepare old vertices for the spidernet
			SpPoints.Clear();
			m_PlayerData.FillArrayWithSpecializationPoints(SpPoints, false);
			m_SpiderNetHandler.SetSpPoints(SpPoints, false);

			m_PlayerData.PrepareSpecializationProgressionStatsDisplay();
		}
		else
		{
			m_PlayerData.PrepareSpecializationStatsDisplay();
		}

		SetUpSpecializationDisplay();
		//Draw spidernet
		m_SpiderNetHandler.DrawSpiderNet();
	}

	//! 1 - Prepare Specializations summary
	//! 2 - Prepare Specializations Panel
	//------------------------------------------------------------------------------------------------
	protected void SetUpSpecializationDisplay()
	{
		Widget specializationProgressWidget = m_wCareerLeftSide.FindAnyWidget("SpecializationsSummary");
		Widget specializationsPanelWidget = m_wCareerRightSide.FindAnyWidget("SpecializationsPanel");
		if (!specializationProgressWidget || !specializationsPanelWidget)
			return;

		//Specializations Panel
		m_twSpecializationTitle = RichTextWidget.Cast(specializationsPanelWidget.FindAnyWidget("SpecializationTitleText"));
		m_twSpecializationProgress = RichTextWidget.Cast(specializationsPanelWidget.FindAnyWidget("SpecializationProgressText"));
		m_SpecializationBar = SCR_ProgressBarWithSegments.Cast(SCR_WLibProgressBarComponent.GetProgressBar("SpecializationBar", specializationsPanelWidget, true));

		m_SpecializationBar.SetMin(0);
		m_SpecializationBar.SetMax(SCR_PlayerDataConfigs.SPECIALIZATION_MAX);

		//Specializations Progress Summary
		Widget progressLeft = specializationProgressWidget.FindAnyWidget("LeftSpecializations");
		Widget progressRight = specializationProgressWidget.FindAnyWidget("RightSpecializations");

		if (!progressLeft || !progressRight)
			return;

		Widget tempWidget;

		int tempCount;
		int maxCount = m_PlayerData.GetSpecializationCount(0);
		int i;
		int specializationsCount = SCR_PlayerDataConfigs.GetInstance().SPECIALIZATIONS_COUNT;

		if (specializationsCount % 2 != 0)
			specializationsCount += 1;

		for (i = 0; i < specializationsCount; i++)
		{
			tempCount = m_PlayerData.GetSpecializationCount(i);
			maxCount = Math.Max(tempCount, maxCount);

			if (i < specializationsCount * 0.5)
				tempWidget = progressLeft;
			else
				tempWidget = progressRight;

			CreateSpecializationProgressButtonWidget(tempWidget, i);
		}

		for (i = 0; i < maxCount; i++)
		{
			Widget tempW;

			if (!m_bShowProgression)
				tempW = SCR_CareerUI.CreateStatEntry(specializationsPanelWidget, m_sStatsLayout);
			else
				tempW = SCR_CareerUI.CreateProgressionStatEntry(specializationsPanelWidget, m_sProgressionStatsLayout);

			if (tempW)
				m_aSpecializationsStatsWidgets.Insert(tempW);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSpecializationPanel(int n)
	{
		if (n < 0 || n >= SCR_PlayerDataConfigs.GetInstance().SPECIALIZATIONS_COUNT || !m_twSpecializationTitle || !m_twSpecializationProgress)
			return;

		string title = ""+(n+ 1)+". "+SCR_PlayerDataConfigs.GetInstance().GetSpecializationName(n);
		float value = m_PlayerData.GetSpecializationPoints(n) * SCR_PlayerDataConfigs.SPPOINTS_CONVERSIONPERCENTAGE;

		m_twSpecializationTitle.SetText(title);
		m_twSpecializationProgress.SetText(""+value.ToString(-1, 2)+"%");

		float warCrimes = 0;
		array<float> EarntPoints;

		if (m_bShowProgression)
		{
			if (!m_PlayerData.IsDataProgressionReady())
				return;
			EarntPoints = m_PlayerData.GetArrayEarntPoints();

			if (m_PlayerData.GetStat(SCR_EDataStats.WARCRIMES) != 0)
				warCrimes = SCR_PlayerDataConfigs.WARCRIMES_PUNISHMENT;

			int spEnumId;
			switch (n)
			{
				case 0: spEnumId = SCR_EDataStats.SPPOINTS0;
					break;
				case 1: spEnumId = SCR_EDataStats.SPPOINTS1;
					break;
				case 2: spEnumId = SCR_EDataStats.SPPOINTS2;
					break;
				default: return;
			}
			float original = m_PlayerData.GetSpecializationPoints(n, false);
			float minus = EarntPoints[spEnumId] * warCrimes;
			float progressed = EarntPoints[spEnumId] - minus;

			m_SpecializationBar.SetProgressBarValues(original, progressed, minus);
		}
		else
		{
			m_SpecializationBar.SetValue(value);
		}

		int numStats = SCR_PlayerDataConfigs.GetInstance().GetSpecializationStatsCount(n);
		SpecializationStatsSetVisible(numStats);

		array<ref SCR_PlayerDataSpecializationDisplay> statsToDisplay = SCR_PlayerDataConfigs.GetInstance().GetSpecializationArray(n);

		for (int i = 0; i < numStats; i++)
		{
			if (m_bShowProgression)
				SCR_CareerUI.UpdateStatProgressionEntry(m_aSpecializationsStatsWidgets[i], statsToDisplay[i].GetTitle(), EarntPoints[statsToDisplay[i].GetEnumId()] * warCrimes, EarntPoints[statsToDisplay[i].GetEnumId()], statsToDisplay[i].GetUnits(), ""+statsToDisplay[i].GetValue());
			else
				SCR_CareerUI.UpdateStatEntry(m_aSpecializationsStatsWidgets[i], statsToDisplay[i].GetTitle(), statsToDisplay[i].GetUnits(), ""+statsToDisplay[i].GetValue());
		}
	}

	//! Makes n stat widgets of the Specialization Panel visible, and the remaining ones invisible
	//------------------------------------------------------------------------------------------------
	protected void SpecializationStatsSetVisible(int n)
	{
		int statWidgetsCount = m_aSpecializationsStatsWidgets.Count();

		if (n < 0 || n > statWidgetsCount)
			return;

		int i;

		//Make n stats visible
		for (i = 0; i < n; i++)
		{
			m_aSpecializationsStatsWidgets[i].SetVisible(true);
		}

		//Make the remaining ones invisible
		for (i = n; i < statWidgetsCount; i++)
		{
			m_aSpecializationsStatsWidgets[i].SetVisible(false);
		}
	}

	//! Legends of the spidernet are responsible for adding a call to this method OnButtonClick
	//! This instance is responsible for calling this when a ProgressButton is invoked
	//------------------------------------------------------------------------------------------------
	void UpdateSpecialization(int specializationId)
	{
		if (!m_aSpecializationsStatsWidgets || m_aSpecializationsStatsWidgets.IsEmpty() || !m_aSpLegendButtonHandlers ||m_aSpLegendButtonHandlers.IsEmpty())
			return;

		//Deactivate old selected legend
		m_aSpLegendButtonHandlers[m_iSelectedLegend].Deactivate();
		m_aSpProgressButtonHandlers[m_iSelectedLegend].Deactivate();

		//Activate new selected legend
		m_iSelectedLegend = specializationId;
		m_aSpLegendButtonHandlers[m_iSelectedLegend].Activate();
		m_aSpProgressButtonHandlers[m_iSelectedLegend].Activate();

		//We set the focus in case the player uses a controller
		m_aSpProgressButtonHandlers[m_iSelectedLegend].SetFocus();

		//Update panel
		UpdateSpecializationPanel(specializationId);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateHoveredSpecialization(int specializationId, bool hover)
	{
		m_aSpLegendButtonHandlers[specializationId].SetOnHover(hover);
		m_aSpProgressButtonHandlers[specializationId].SetOnHover(hover);
	}

	//------------------------------------------------------------------------------------------------
	protected Widget CreateSpecializationProgressButtonWidget(Widget container, int id)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;

		Widget SpecializationProgressButton = workspace.CreateWidgets(m_sSpProgressLayout, container);
		if (!SpecializationProgressButton)
			return null;

		SCR_ButtonSpecializationProgressComponent handler = SCR_ButtonSpecializationProgressComponent.Cast(SpecializationProgressButton.FindHandler(SCR_ButtonSpecializationProgressComponent));
		if (!handler)
			return null;

		//If ID is higher to the number of Specializations, we fill it empty and make it invisible because its created for layout purposes
		if (id >= SCR_PlayerDataConfigs.GetInstance().SPECIALIZATIONS_COUNT)
		{
			handler.SetValue(0);
			handler.Hide();
			return null;
		}

		handler.SetButtonId(id);
		handler.SetValue(m_PlayerData.GetSpecializationPoints(id) * SCR_PlayerDataConfigs.SPPOINTS_CONVERSIONPERCENTAGE);
		handler.GetOnClicked().Insert(UpdateSpecialization);
		handler.GetOnMouseEnter().Insert(UpdateHoveredSpecialization);
		handler.GetOnMouseLeave().Insert(UpdateHoveredSpecialization);

		AddProgressButtonHandler(handler);

		return SpecializationProgressButton;
	}

	//------------------------------------------------------------------------------------------------
	void AddLegendButtonHandler(SCR_ButtonLegendComponent handler)
	{
		if (handler)
			m_aSpLegendButtonHandlers.Insert(handler);
	}

	//------------------------------------------------------------------------------------------------
	void AddProgressButtonHandler(SCR_ButtonSpecializationProgressComponent handler)
	{
		if (handler)
			m_aSpProgressButtonHandlers.Insert(handler);
	}

	//------------------------------------------------------------------------------------------------
	void SetLeftAndRightUIActivate(bool flag)
	{
		for (int i = m_aSpLegendButtonHandlers.Count(); i > 0; i--)
		{
			m_aSpLegendButtonHandlers[m_iSelectedLegend].SetLeftAndRightUIActivate(flag);
		}
	}

	//War crimes
	//------------------------------------------------------------------------------------------------
	void FillWarCrimes()
	{
		//TODO: choose the texture dynamically: m_wImgTitleIcon.LoadImageFromSet(0, image, imageName);
		//where image is the resourceName of imageset, and imageName is the string of the specific icon in the imageset

		Widget WarCrimesPanelWidget = m_wCareerRightSide.FindAnyWidget("WarCrimesPanel");
		if (!WarCrimesPanelWidget)
			return;

		SCR_WarCrimesPanelUI handler = SCR_WarCrimesPanelUI.Cast(WarCrimesPanelWidget.FindHandler(SCR_WarCrimesPanelUI));
		if (!handler)
			return;

		if (m_PlayerData.GetStat(SCR_EDataStats.WARCRIMES) > 0)
		{
			if (m_PlayerData.GetStat(SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES) > 0)
			{
				handler.CreateWarCrimeEntry(SCR_EWarCrimes.HARMINGFRIENDLIES);
			}
			else
			{
				Print ("SCR_CareerSpecializationsUI:FillWarCrimes: War Crimes amount not 0 but Harming Friendlies is 0", LogLevel.ERROR);
			}
		}
	}
};

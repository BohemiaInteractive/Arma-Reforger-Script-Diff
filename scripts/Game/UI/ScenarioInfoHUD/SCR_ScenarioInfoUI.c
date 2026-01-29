class SCR_ScenarioInfoUI: ScriptedWidgetComponent
{
	[Attribute(SCR_SoundEvent.SOUND_FE_TEXT_ENTERED, UIWidgets.EditBox, "")]
	protected string m_sSoundCharEntered;
	
	[Attribute(SCR_SoundEvent.SOUND_FE_TEXT_LEFT, UIWidgets.EditBox, "")]
	protected string m_sSoundCharLeft;

    protected string m_sCharacters;

	[Attribute(defvalue: "34", uiwidget: UIWidgets.EditBox, desc: "Variable for loop frequency time")]
	protected float LOOP_FREQUENCY;
	
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.EditBox, desc: "variable for time needed to remove character")]
	protected float TIME_REMOVE_CHARACTER;
	
	protected ref array<RichTextWidget> m_wTextWidgets;
    protected ref array<string> m_aLines;
	protected ref array<string> m_aLinesTranslated = {};
	
    protected float m_fAccumulatedTime;
    protected int m_iCurrentLine;
    protected int m_iCurrentCharacter;
    protected int m_iCharactersCount;
    protected int m_iLinesCount;
	protected Widget m_wOwner;
	
    protected SCR_ScenarioFrameworkActionShowWidget m_ScenarioAction;
	
	//------------------------------------------------------------------------------------------------
    override void HandlerAttached(Widget w)
    {
        m_wTextWidgets = {};
        int i = 1;

        RichTextWidget widgetLine = RichTextWidget.Cast(w.FindAnyWidget("m_wScenarioLine" + i.ToString()));
        while (widgetLine)
        {
            m_wTextWidgets.Insert(widgetLine);
            i++;
            widgetLine = RichTextWidget.Cast(w.FindAnyWidget("m_wScenarioLine" + i.ToString()));
        }
		
		m_wOwner = w;
    }
	
	//------------------------------------------------------------------------------------------------
    void Show(notnull array<string> lines, SCR_ScenarioFrameworkActionShowWidget scenarioAction)
    {
        m_aLines = lines;
        m_ScenarioAction = scenarioAction;

        int widgetsCount = m_wTextWidgets.Count();
        m_iLinesCount = m_aLines.Count();
        if (m_iLinesCount > widgetsCount)
            m_iLinesCount = widgetsCount;

        for (int i = 0; i < m_iLinesCount; i++)
        {
            m_wTextWidgets[i].SetVisible(!m_aLines[i].IsEmpty());
			m_wTextWidgets[i].SetText(" ");
			m_aLinesTranslated.Insert(WidgetManager.Translate(m_aLines[i]));
        }

        m_sCharacters = "";
        m_iCurrentLine = 0;
        m_iCurrentCharacter = 0;

		m_iCharactersCount = m_aLinesTranslated[0].Length();

		SCR_GameplaySettingsSubMenu.m_OnLanguageChanged.Insert(OnLanguageChanged);

        SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(OnTickStartAnimation, LOOP_FREQUENCY);
    }

	//------------------------------------------------------------------------------------------------
	protected void OnLanguageChanged(SCR_GameplaySettingsSubMenu menu)
	{
		for (int i = 0; i < m_iLinesCount; i++)
		{
			m_wTextWidgets[i].SetVisible(!m_aLines[i].IsEmpty());
			m_wTextWidgets[i].SetText(m_aLines[i]);
		}

		m_iCurrentLine = m_iLinesCount;
	}

	//------------------------------------------------------------------------------------------------
	//! TODO: Security for HTML tags
    protected void OnTickStartAnimation()
    {
        if (m_iCurrentLine >= m_iLinesCount)
        {
            m_ScenarioAction.ShowAnimationEnded();
            return;
        }

        if (m_iCurrentCharacter < m_iCharactersCount)
        {
			m_sCharacters += m_aLinesTranslated[m_iCurrentLine][m_iCurrentCharacter];
            m_wTextWidgets[m_iCurrentLine].SetText(m_sCharacters);
            m_iCurrentCharacter++;
        }
        else
        {
            m_iCurrentLine++;
            if (m_iCurrentLine < m_iLinesCount)
            {
				m_iCharactersCount = m_aLinesTranslated[m_iCurrentLine].Length();
                m_iCurrentCharacter = 0;
                m_sCharacters = "";
            }
        }
		
        SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(OnTickStartAnimation, LOOP_FREQUENCY);
		SCR_UISoundEntity.SoundEvent(m_sSoundCharEntered);
    }

	//------------------------------------------------------------------------------------------------
    void Hide()
    {
		m_fAccumulatedTime = 0;
        m_iCurrentLine = m_iLinesCount - 1;
        if (m_iCurrentLine >= 0)
			m_iCurrentCharacter = m_aLinesTranslated[m_iCurrentLine].Length();

        SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(OnTickHideAnimation, LOOP_FREQUENCY);
    }

	//------------------------------------------------------------------------------------------------
    protected void OnTickHideAnimation()
    {
        if (m_wOwner.GetOpacity() <= 0)
        {
			SCR_GameplaySettingsSubMenu.m_OnLanguageChanged.Remove(OnLanguageChanged);
            m_ScenarioAction.HideAnimationEnded();
            return;
        }

        if (m_fAccumulatedTime < TIME_REMOVE_CHARACTER)
        {
            m_fAccumulatedTime += TIME_REMOVE_CHARACTER;
            SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(OnTickHideAnimation, TIME_REMOVE_CHARACTER);
            return;
        }

        m_fAccumulatedTime = 0;

        m_wOwner.SetOpacity(m_wOwner.GetOpacity() -0.05);

        SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(OnTickHideAnimation, LOOP_FREQUENCY);
    }
}

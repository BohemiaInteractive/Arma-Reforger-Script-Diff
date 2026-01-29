class SCR_FeedbackDialogPredefinedSentences
{
	// Available predefined answers for console
	protected LocalizedString m_sConsoleSentencesGeneral[] =
	{
		"#AR-Feedback_Predefined_General_1",
		"#AR-Feedback_Predefined_General_2",
		"#AR-Feedback_Predefined_General_3",
		"#AR-Feedback_Predefined_General_4",
		"#AR-Feedback_Predefined_General_5",
		"#AR-Feedback_Predefined_General_6",
		"#AR-Feedback_Predefined_General_7"
	};
	
	protected LocalizedString m_sConsoleSentencesUI[] =
	{
		"#AR-Feedback_Predefined_General_1",
		"#AR-Feedback_Predefined_UI_1",
		"#AR-Feedback_Predefined_UI_2",
		"#AR-Feedback_Predefined_UI_3",
		"#AR-Feedback_Predefined_UI_4",
		"#AR-Feedback_Predefined_UI_5",
		"#AR-Feedback_Predefined_UI_6"
	};
	
	protected LocalizedString m_sConsoleSentencesEditor[] =
	{
		"#AR-Feedback_Predefined_General_1",
		"#AR-Feedback_Predefined_Editor_1",
		"#AR-Feedback_Predefined_Editor_2",
		"#AR-Feedback_Predefined_Editor_3",
		"#AR-Feedback_Predefined_Editor_4",
		"#AR-Feedback_Predefined_Editor_5",
		"#AR-Feedback_Predefined_Editor_6",
		"#AR-Feedback_Predefined_Editor_7"
	};
	
	protected LocalizedString m_sConsoleSentencesCharacter[] =
	{
		"#AR-Feedback_Predefined_General_1",
		"#AR-Feedback_Predefined_Character_1",
		"#AR-Feedback_Predefined_Character_2",
		"#AR-Feedback_Predefined_Character_3",
		"#AR-Feedback_Predefined_Character_4",
		"#AR-Feedback_Predefined_Character_5",
		"#AR-Feedback_Predefined_Character_6",
		"#AR-Feedback_Predefined_Character_7"
	};
	
	protected LocalizedString m_sConsoleSentencesVehicles[] =
	{
		"#AR-Feedback_Predefined_General_1",
		"#AR-Feedback_Predefined_Vehicles_1",
		"#AR-Feedback_Predefined_Vehicles_2",
		"#AR-Feedback_Predefined_Vehicles_3",
		"#AR-Feedback_Predefined_Vehicles_4",
		"#AR-Feedback_Predefined_Vehicles_5",
		"#AR-Feedback_Predefined_Vehicles_6"
	};
	
	protected LocalizedString m_sConsoleSentencesWeapons[] =
	{
		"#AR-Feedback_Predefined_General_1",
		"#AR-Feedback_Predefined_Weapons_1",
		"#AR-Feedback_Predefined_Weapons_2",
		"#AR-Feedback_Predefined_Weapons_3",
		"#AR-Feedback_Predefined_Weapons_4",
		"#AR-Feedback_Predefined_Weapons_5",
		"#AR-Feedback_Predefined_Weapons_6",
		"#AR-Feedback_Predefined_Weapons_7"
	};
	
	protected LocalizedString m_sConsoleSentencesMultiplayer[] =
	{
		"#AR-Feedback_Predefined_General_1",
		"#AR-Feedback_Predefined_Multiplayer_1",
		"#AR-Feedback_Predefined_Multiplayer_2",
		"#AR-Feedback_Predefined_Multiplayer_3",
		"#AR-Feedback_Predefined_Multiplayer_4"
	};
	
	protected LocalizedString m_sConsoleSentencesConflict[] =
	{
		"#AR-Feedback_Predefined_General_1"
	};
	
	protected ref array<ref array<LocalizedString>> m_aAllSentences = {};
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		AddToArray(m_sConsoleSentencesGeneral);
		AddToArray(m_sConsoleSentencesUI);
		AddToArray(m_sConsoleSentencesEditor);
		AddToArray(m_sConsoleSentencesCharacter);
		AddToArray(m_sConsoleSentencesVehicles);
		AddToArray(m_sConsoleSentencesWeapons);
		AddToArray(m_sConsoleSentencesMultiplayer);
		AddToArray(m_sConsoleSentencesConflict);
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref array<LocalizedString>> GetAllSentences()
	{
		return m_aAllSentences;
	}
	
	//------------------------------------------------------------------------------------------------
	array<LocalizedString> GetSentencesByIndex(int i)
	{
		if (!m_aAllSentences[i])
			return null;
		
		return m_aAllSentences[i];
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddToArray(LocalizedString[] sentences)
	{
		array<LocalizedString> sentencesArray = {};
		foreach (LocalizedString sentence : sentences)
		{
			sentencesArray.Insert(sentence);	
		}

		if (!sentencesArray.IsEmpty())
			m_aAllSentences.Insert(sentencesArray);
	}
}
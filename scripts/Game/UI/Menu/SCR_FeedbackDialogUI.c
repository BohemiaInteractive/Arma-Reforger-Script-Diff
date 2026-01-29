//------------------------------------------------------------------------------------------------
class FeedbackData extends JsonApiStruct
{
	//! Type of feedback: issue, feedback..
	protected string m_sTypeTag;
	protected string m_sTypeName;
	//! Category of feedback: character, vehicle, UI..
	protected string m_sCategoryTag;
	protected string m_sCategoryName;
	
	//! Visibility of feedback
	protected bool m_bPrivate;

	//! User provided content
	protected string m_sContent;

	//! User data
	protected string m_sUserName;

	//! Version information
	//! Game build time
	protected string m_sBuildTime;
	//! Game build version
	protected string m_sBuildVersion;

	//! Game information
	//! Current world file
	protected string m_sWorldFile;
	//! When feedback was submit
	//! Formatted as YYYYY-MM-DD HH-MM-SS
	protected string m_sSubmitTime;

	//! Current world position taken from camera if any
	protected vector m_vWorldPosition;
	//! Current camera orientation taken from camera if any
	protected vector m_vWorldRotation;

	//! Message string for Slack (not part of JSON)
	protected string m_sMessage;

	//------------------------------------------------------------------------------------------------
	string FormatMessage()
	{
		const string altsep = "-----------------------------\n";
		string res = "\n";

		res += string.Format("Type: %1\n", GetFeedbackType());
		res += string.Format("Category: %1\n", GetFeedbackCategory());
		res += string.Format("User: %1\n", GetUserName());

		// assembly link to WE
		res += string.Format("<enfusion://WorldEditor/%1;%2,%3,%4;%5,%6,%7|Enfusion Link>\n",
		GetWorldFile(),
		m_vWorldPosition[0].ToString(),
		m_vWorldPosition[1].ToString(),
			m_vWorldPosition[2].ToString(),
			m_vWorldRotation[1].ToString(),
			m_vWorldRotation[2].ToString(),
			m_vWorldRotation[0].ToString());

		res += string.Format("Build Version: %1\n", GetBuildVersion());
		res += string.Format("Submit Time: %1\n", GetFeedbackSubmitTime());
		res += string.Format("Build Time: %1\n", GetBuildTime());
		res += string.Format("World File: %1\n", GetWorldFile());

		// position && message
		res += string.Format("Position: %1\n", m_vWorldPosition.ToString());
		res += string.Format("Rotation: %1\n", m_vWorldRotation.ToString());

		res += altsep;
		res += "FEEDBACK USER DATA:\n";
		res += m_sContent;


		m_sMessage = res;

		return m_sMessage;
	}

	//------------------------------------------------------------------------------------------------
	string GetWorldFile()
	{
		return m_sWorldFile;
	}

	//------------------------------------------------------------------------------------------------
	string GetUserName()
	{
		return m_sUserName;
	}

	//------------------------------------------------------------------------------------------------
	string GetBuildTime()
	{
		return m_sBuildTime;
	}

	//------------------------------------------------------------------------------------------------
	string GetBuildVersion()
	{
		return m_sBuildVersion;
	}

	//------------------------------------------------------------------------------------------------
	string GetFeedbackSubmitTime()
	{
		return m_sSubmitTime;
	}

	//------------------------------------------------------------------------------------------------
	string GetFeedbackType()
	{
		return m_sTypeName;
	}

	//------------------------------------------------------------------------------------------------
	string GetFeedbackCategory()
	{
		return m_sCategoryName;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPrivate()
	{
		return m_bPrivate;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFeedbackContent()
	{
		return m_sContent;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetCurrentTimestamp()
	{
		int year, month, day, hour, minute, sec;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, sec);
		string smonth, sday, shour, sminute, ssec;
		if (month < 10)
			smonth = string.Format("0%1", month);
		else
			smonth = string.Format("%1", month);

		if (day < 10)
			sday = string.Format("0%1", day);
		else
			sday = string.Format("%1", day);

		if (hour < 10)
			shour = string.Format("0%1", hour);
		else
			shour = string.Format("%1", hour);

		if (minute < 10)
			sminute = string.Format("0%1", minute);
		else
			sminute = string.Format("%1", minute);

		if (sec < 10)
			ssec = string.Format("0%1", sec);
		else
			ssec = string.Format("%1", sec);

		return string.Format("%1-%2-%3 %4:%5:%6", year, smonth, sday, shour, sminute, ssec);
	}

		//------------------------------------------------------------------------------------------------
	protected bool FetchCoords()
	{
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			BaseWorld world = game.GetWorld();
			if (world)
			{
				int cameraId = world.GetCurrentCameraId();
				// get current engine camera transformation
				vector mat[4];
				world.GetCamera(cameraId, mat);
				vector yawPitchRoll = Math3D.MatrixToAngles(mat);
				m_vWorldPosition = mat[3];

				// yawpitchroll->pitchyawroll
				m_vWorldRotation[0] = yawPitchRoll[1];
				m_vWorldRotation[1] = yawPitchRoll[0];
				m_vWorldRotation[2] = yawPitchRoll[2];
				return true;
			}
		}

		// Plausible defaults
		m_vWorldPosition = Vector(0, 0, 0);
		m_vWorldRotation = Vector(0, 0, 0);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates feedback data container
	void FeedbackData(string typeTag, string typeName, string catTag, string catName, bool bPrivate, string content)
	{
		// ...
		// data below are structured, send as JSON!
		RegV("m_sTypeName");
		RegV("m_sCategoryName");
		RegV("m_bPrivate");
		RegV("m_sContent");
		RegV("m_sUserName");

		RegV("m_sBuildTime");
		RegV("m_sBuildVersion");

		RegV("m_sWorldFile");
		RegV("m_sSubmitTime");

		RegV("m_vWorldPosition");
		RegV("m_vWorldRotation");
		// ...


		m_sTypeTag = typeTag;
		m_sCategoryTag = catTag;

		m_sTypeName = typeName;
		m_sCategoryName = catName;
		
		m_bPrivate = bPrivate;

		m_sContent = content;

		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;

		// TODO: Not sure if the user name should be directly sent. Same goes for platform specific UID
		m_sUserName = SCR_Global.GetProfileName();
		m_sBuildTime = game.GetBuildTime();
		m_sBuildVersion = game.GetBuildVersion();

		m_sWorldFile = game.GetWorldFile();
		m_sSubmitTime = GetCurrentTimestamp();

		// Fetch coords data
		FetchCoords();
	}

}

sealed class SCR_FeedbackDialogUI : SCR_ConfigurableDialogUi
{
	//! Available types of feedback
	static const int TYPE_NAMES_COUNT = 2;
	static const string TYPE_TAGS[TYPE_NAMES_COUNT] =
	{
		"feedback",
		"issue"
	};

	static const LocalizedString TYPE_NAMES[TYPE_NAMES_COUNT] =
	{
		"#AR-MainMenu_Feedback_Name",
		"#AR-Feedback_Issue"
	};

	//! Available categories of feedback
	static const int CATEGORY_NAMES_COUNT = 8;
	static const string CATEGORY_TAGS[CATEGORY_NAMES_COUNT] =
	{
		"general",
		"ui",
		"editor",
		"character",
		"vehicles",
		"weapons",
		"multiplayer",
		"conflict"
	};

	static const LocalizedString CATEGORY_NAMES[CATEGORY_NAMES_COUNT] =
	{
		"#AR-Feedback_General",
		"#AR-Feedback_UI",
		"#AR-MainMenu_Editor_Name",
		"#AR-Feedback_Character",
		"#AR-Feedback_Vehicles",
		"#AR-Feedback_Weapons",
		"#AR-MainMenu_Multiplayer_Name",
		"#AR-MainMenu_Conflict_Name"
	};

	protected ref SCR_FeedbackDialogContentWidgets m_Widgets;
	protected ref SCR_FeedbackDialogPredefinedSentences m_FeedbackSentences;

	protected SCR_InputButtonComponent m_ConfirmButton;
	protected SCR_InputButtonComponent m_TOSButton;

	protected const string TOS_BUTTON = "ProfileTOS";
	protected const string TOS_LINK = "Link_PrivacyPolicy";

	protected static const string FEEDBACK_DIALOG_NAME = "feedback_dialog";

	protected static ref FeedbackData m_LastFeedback;
	protected static float m_fLastFeedbackTime = -float.MAX;
	protected static const float FEEDBACK_SEND_TIMEOUT = 5000; // ms

	protected bool m_bShouldEnableConfirmButton;

	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		if (!m_Widgets.m_FeedbackTypeComponent0 || !m_Widgets.m_FeedbackCategoryComponent0 || !m_Widgets.m_FeedbackEditBoxComponent || !m_Widgets.m_ConsoleFeedbackTopicComponent0)
			return;

		// Fetch feedback data

		// Selected type (Feedback or issue)
		int type = m_Widgets.m_FeedbackTypeComponent0.GetCurrentIndex();

		// Selected category (general, ui, editor, etc)
		int category = m_Widgets.m_FeedbackCategoryComponent0.GetCurrentIndex();

		// User written content from editbox
		string content = m_Widgets.m_FeedbackEditBoxComponent.GetValue();

		// Selected topic (only availabe on console. string.Empty by default)
		array<LocalizedString> predefinedAnswers = m_FeedbackSentences.GetSentencesByIndex(category);
		string console_sentence = predefinedAnswers[m_Widgets.m_ConsoleFeedbackTopicComponent0.GetCurrentIndex()];

		// Put the selected topic in front of the message
		if (System.GetPlatform() != EPlatform.WINDOWS)
			content = string.Format("%1 %2", console_sentence, content);

		// Get the state of the AttachLogs button (1 = Log should be attached, 0 = no log will be attached)
		bool attachLogs;
		if (m_Widgets.m_AttachLogsButtonComponent)
			attachLogs = m_Widgets.m_AttachLogsButtonComponent.IsToggled();

		// OnAttachFile() also handles when the button to attach a File is clicked

		// Current implementation of sending Feedback to backend (will probably be changed)
		SendFeedback(content, type, category, false);

		super.OnConfirm();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		if (m_Widgets.m_FeedbackTypeComponent0)
			GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_FeedbackTypeComponent0.GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	static bool CanSendFeedback()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		float timeSinceLast = world.GetWorldTime() - m_fLastFeedbackTime;
		return timeSinceLast > FEEDBACK_SEND_TIMEOUT;
	}

	//------------------------------------------------------------------------------------------------
	static void SendFeedback(string content, int type, int category, bool bPrivate)
	{
		// Create container for our feedback
		m_LastFeedback = null;
		m_LastFeedback = new FeedbackData(TYPE_TAGS[type], TYPE_NAMES[type], CATEGORY_TAGS[category], CATEGORY_NAMES[category], bPrivate, content);
		string summary = m_LastFeedback.FormatMessage();

		GetGame().GetBackendApi().FeedbackMessage(null, m_LastFeedback, summary);
		m_fLastFeedbackTime = GetGame().GetWorld().GetWorldTime();
	}

	//------------------------------------------------------------------------------------------------
	static void ClearFeedback()
	{
		m_LastFeedback = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAttachFile()
	{
		//TODO:LangePaul Create Screenshot and attach it to feedback
	}

	//------------------------------------------------------------------------------------------------
	private void OnTOSButton()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PrivacyPolicyMenu);
		Close();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		m_FeedbackSentences = new SCR_FeedbackDialogPredefinedSentences();
		m_FeedbackSentences.Init();
		
		m_Widgets = new SCR_FeedbackDialogContentWidgets();
		m_Widgets.Init(GetContentLayoutRoot());

		if (m_Widgets.m_FeedbackTypeComponent0)
		{
			for (int i = 0; i < TYPE_NAMES_COUNT; i++)
			{
				m_Widgets.m_FeedbackTypeComponent0.AddItem(TYPE_NAMES[i]);
			}
			m_Widgets.m_FeedbackTypeComponent0.SetCurrentItem(0);
		}

		if (m_Widgets.m_ConsoleFeedbackTopicComponent0 && System.GetPlatform() == EPlatform.WINDOWS)
			m_Widgets.m_ConsoleFeedbackTopicComponent0.SetVisible(false);
		else if (m_Widgets.m_ConsoleFeedbackTopicComponent0)
			m_Widgets.m_ConsoleFeedbackTopicComponent0.SetCurrentItem(0);
		
		if (m_Widgets.m_FeedbackCategoryComponent0)
		{
			for (int i = 0; i < CATEGORY_NAMES_COUNT; i++)
			{
				m_Widgets.m_FeedbackCategoryComponent0.AddItem(CATEGORY_NAMES[i]);
			}
			m_Widgets.m_FeedbackCategoryComponent0.m_OnChanged.Insert(OnCategoryChanged);
			m_Widgets.m_FeedbackCategoryComponent0.SetCurrentItem(0);
			OnCategoryChanged();
		}

		m_TOSButton = FindButton(TOS_BUTTON);
		if (m_TOSButton)
			m_TOSButton.m_OnActivated.Insert(OnTOSButton);

		m_ConfirmButton = FindButton(BUTTON_CONFIRM);
		if (m_ConfirmButton)
			m_ConfirmButton.SetEnabled(false);

		if (m_Widgets.m_FeedbackEditBoxComponent)
		{
			m_Widgets.m_FeedbackEditBoxComponent.m_OnWriteModeLeave.Insert(OnWriteModeLeaveInternal);
			m_Widgets.m_FeedbackEditBoxComponent.m_OnTextChange.Insert(OnTextChange);
		}

		if (m_Widgets.m_AttachFileButtonComponent)
			m_Widgets.m_AttachFileButtonComponent.m_OnClicked.Insert(OnAttachFile);

		// AttachLogs button should not be visible on consoles
		if (m_Widgets.m_AttachLogsButtonComponent && System.GetPlatform() != EPlatform.WINDOWS)
		{
			m_Widgets.m_wAttachFileVertialLayout.SetVisible(false);
			m_Widgets.m_AttachLogsButtonComponent.SetVisible(false);
			m_Widgets.m_AttachLogsButtonComponent.SetEnabled(false);
			m_Widgets.m_AttachFileButtonComponent.SetVisible(false);
			m_Widgets.m_AttachFileButtonComponent.SetEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWriteModeLeaveInternal(string text)
	{
		OnTextChange(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTextChange(string text)
	{
		if (m_Widgets.m_FeedbackEditBoxComponent)
			m_bShouldEnableConfirmButton = m_Widgets.m_FeedbackEditBoxComponent.GetValue() != string.Empty && SCR_FeedbackDialogUI.CanSendFeedback();

		if (m_ConfirmButton)
			m_ConfirmButton.SetEnabled(m_bShouldEnableConfirmButton);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCategoryChanged(SCR_ComboBoxComponent comp = null, int index = 0)
	{
		if (!m_Widgets.m_ConsoleFeedbackTopicComponent0 || !m_Widgets.m_ConsoleFeedbackTopicComponent0.IsVisible())
			return;
		
		m_Widgets.m_ConsoleFeedbackTopicComponent0.ClearAll();
		
		array<LocalizedString> sentences = m_FeedbackSentences.GetSentencesByIndex(index);
		
		foreach(LocalizedString sentence : sentences)
		{
			m_Widgets.m_ConsoleFeedbackTopicComponent0.AddItem(sentence);
		}
		
		m_Widgets.m_ConsoleFeedbackTopicComponent0.SetCurrentItem(0);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_FeedbackDialogUI OpenFeedbackDialog()
	{
		SCR_FeedbackDialogUI dialog = new SCR_FeedbackDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, FEEDBACK_DIALOG_NAME, dialog);

		return dialog;
	}
}

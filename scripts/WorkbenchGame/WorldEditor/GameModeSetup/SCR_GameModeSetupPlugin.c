#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Game Mode Setup", description: "Set ups the world for a specific game mode", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF6E8)]
class SCR_GameModeSetupPlugin : WorkbenchPlugin
{
	[Attribute("{6389BA4D41B187DC}Configs/Workbench/GameModeSetup/GameMaster.conf", desc: "Game mode configuration rules", params: "conf class=GameModeSetupConfig", category: "Game Mode Template")]
	protected ResourceName m_sTemplate;

	//[Attribute(category: "Game Mode Template")] //--- Used only for debugging, game crashes when recompiling Workbench scripts and running the plugin repeatedly.
	protected ref GameModeSetupConfig m_Config;

	protected bool m_bIsWorldValid;
	protected bool m_bIsWorldAttentionNeeded;
	protected bool m_bCanAutogenerateWorld;
	protected bool m_bIsMissionHeaderValid;
	protected ref array<SCR_EGameModeSetupPage> m_aPageHistory = { SCR_EGameModeSetupPage.INTRO };

	protected string m_sDialogMessageValidation;
	protected string m_sDialogMessageGeneration;
	protected string m_sDialogMessageMissionHeader;

	protected static const string CAPTION_INTRO =						"Game Mode Setup";
	protected static const string CAPTION_VALIDATION =					"Game Mode Setup - World Scan";
	protected static const string CAPTION_VALIDATION_RESULTS =			"Game Mode Setup - World Scan Results";
	protected static const string CAPTION_GENERATION =					"Game Mode Setup - World Configuration";
	protected static const string CAPTION_GENERATION_RESULTS =			"Game Mode Setup - World Configuration Completed";
	protected static const string CAPTION_MISSION_HEADER =				"Game Mode Setup - Mission Header";
	protected static const string CAPTION_MISSION_HEADER_RESULTS =		"Game Mode Setup - Mission Header Created";
	protected static const string CAPTION_OUTRO_GOOD =					"Game Mode Setup - Success";
	protected static const string CAPTION_OUTRO_BAD =					"Game Mode Setup - Actions Required";

	protected static const string DESCRIPTION_INTRO =					"Welcome to step-by-step setup of a game mode.\nA game mode is a set of rules that define how the world will function.\nWithout it, players won't be able to try your world in game.\nEven opening the world in Game Master requires you to create a Game Master game mode for it.\n\nThe plugin will explain what's needed to get a game mode up an running,\nand offer automatic creation of required configuration.\n\nBefore we start, please select a template of the game mode you wish to set up.\n";
	protected static const string DESCRIPTION_VALIDATION =				"To know what needs to be configured the plugin will scan all entities currently present in the world.\nDepending on the size of the world, this may take several seconds or minutes.\n\nYou can decide to skip this step, but unless you understand how to set up game mode manually,\nit's better to let the plugin check the current status first.";
	protected static const string DESCRIPTION_VALIDATION_RESULTS =		"All entities scanned, results are listed below:\n";

	protected static const string DESCRIPTION_GENERATION =				"The plugin can now create required entities in the world.\nThey will be created in the current layer, but you can later move them to any other layer.";
	protected static const string DESCRIPTION_GENERATION_RESULTS =		"Required entities were created in the current layer.\nTry to re-scan the world to see if nothing is missing anymore.\n";
	protected static const string DESCRIPTION_MISSION_HEADER =			"For the game mode to appear in main menu's scenario list, it needs a mission header.\nIt's a config file which exists outside of the world, but points to it.\n\nIn this step you can let the plugin create the file automatically.";
	protected static const string DESCRIPTION_MISSION_HEADER_RESULTS =	"";
	protected static const string DESCRIPTION_OUTRO_COMPLETE =			"Congratulations, the game mode is configured correctly.\n\nGive it a try and hit that Play button!";
	protected static const string DESCRIPTION_OUTRO_INCOMPLETE =		"Configuration was not finished.\nTo ensure the game mode is set up correctly, please fix all issues and go through each step again.";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog(CAPTION_INTRO, DESCRIPTION_INTRO, this))
			return;

		string error;
		if (!LoadConfig(error))
		{
			Print(CAPTION_INTRO + ": " + error, LogLevel.WARNING);
			if (Workbench.ScriptDialog(CAPTION_INTRO, error, new SCR_GameModeSetupPluginError()) != 0)
				Run(); // not using ShowPage to prevent filling the history

			return;
		}

		//--- Next
		ShowPage(SCR_EGameModeSetupPage.VALIDATION);
	}

	//------------------------------------------------------------------------------------------------
	protected void RunValidation()
	{
		m_Config.Init();

		SCR_GameModeSetupPluginValidation dialog = new SCR_GameModeSetupPluginValidation();
		if (!Workbench.ScriptDialog(CAPTION_VALIDATION, DESCRIPTION_VALIDATION, dialog))
			return;

		m_bIsWorldValid = false;
		switch (dialog.m_eResult)
		{
			case SCR_EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;

			case SCR_EGameModeSetupButton.NEXT:
				m_sDialogMessageValidation = string.Empty;
				m_bCanAutogenerateWorld = true;
				m_bIsWorldValid = m_Config.ValidateWorld(m_sDialogMessageValidation, m_bCanAutogenerateWorld);
				ShowPage(SCR_EGameModeSetupPage.VALIDATION_RESULTS);
				break;

			case SCR_EGameModeSetupButton.SKIP:
				ShowPage(SCR_EGameModeSetupPage.GENERATION);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RunValidationResults()
	{
		SCR_GameModeSetupPluginResults dialog = new SCR_GameModeSetupPluginResults();
		if (!Workbench.ScriptDialog(CAPTION_VALIDATION_RESULTS, DESCRIPTION_VALIDATION_RESULTS + m_sDialogMessageValidation, dialog))
			return;

		switch (dialog.m_eResult)
		{
			case SCR_EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;

			case SCR_EGameModeSetupButton.NEXT:
				if (m_bIsWorldValid)
				{
					if (m_bIsMissionHeaderValid)
						ShowPage(SCR_EGameModeSetupPage.OUTRO);
					else
						ShowPage(SCR_EGameModeSetupPage.MISSION_HEADER);
				}
				else if (!m_bCanAutogenerateWorld)
					ShowPage(SCR_EGameModeSetupPage.OUTRO);
				else
					ShowPage(SCR_EGameModeSetupPage.GENERATION);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RunGeneration()
	{
		SCR_GameModeSetupPluginGeneration dialog = new SCR_GameModeSetupPluginGeneration();
		if (!Workbench.ScriptDialog(CAPTION_GENERATION, DESCRIPTION_GENERATION, dialog))
			return;

		switch (dialog.m_eResult)
		{
			case SCR_EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;

			case SCR_EGameModeSetupButton.NEXT:
				m_sDialogMessageGeneration = string.Empty;
				m_bIsWorldValid = m_Config.GenerateWorld(m_sDialogMessageGeneration);
				ShowPage(SCR_EGameModeSetupPage.GENERATION_RESULTS);
				break;

			case SCR_EGameModeSetupButton.SKIP:
				ShowPage(SCR_EGameModeSetupPage.MISSION_HEADER);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RunGenerationResults()
	{
		SCR_GameModeSetupPluginGenerationResults dialog = new SCR_GameModeSetupPluginGenerationResults();
		if (!Workbench.ScriptDialog(CAPTION_GENERATION_RESULTS, DESCRIPTION_GENERATION_RESULTS + m_sDialogMessageGeneration, dialog))
			return;

		switch (dialog.m_eResult)
		{
			case SCR_EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;

			case SCR_EGameModeSetupButton.NEXT:
				ShowPage(SCR_EGameModeSetupPage.MISSION_HEADER);
				break;

			case SCR_EGameModeSetupButton.VALIDATE:
				ShowPage(SCR_EGameModeSetupPage.VALIDATION);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RunMissionHeader()
	{
		//--- Mission header already exists, skip this step
		m_bIsMissionHeaderValid = m_Config.ValidateMissionHeader(m_sDialogMessageMissionHeader);
		if (m_bIsMissionHeaderValid)
		{
			m_aPageHistory.Resize(m_aPageHistory.Count() - 1);
			ShowPage(SCR_EGameModeSetupPage.MISSION_HEADER_RESULTS);
			return;
		}

		SCR_GameModeSetupPluginMissionHeader dialog = new SCR_GameModeSetupPluginMissionHeader();
		if (!Workbench.ScriptDialog(CAPTION_MISSION_HEADER, DESCRIPTION_MISSION_HEADER, dialog))
			return;

		switch (dialog.m_eResult)
		{
			case SCR_EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;

			case SCR_EGameModeSetupButton.NEXT:
				m_sDialogMessageMissionHeader = string.Empty;
				m_bIsMissionHeaderValid = m_Config.GenerateMissionHeader(m_sTemplate, m_sDialogMessageMissionHeader);
				ShowPage(SCR_EGameModeSetupPage.MISSION_HEADER_RESULTS);
				break;

			case SCR_EGameModeSetupButton.SKIP:
				ShowPage(SCR_EGameModeSetupPage.OUTRO);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RunMissionHeaderResults()
	{
		SCR_GameModeSetupPluginResults dialog = new SCR_GameModeSetupPluginResults();
		if (!Workbench.ScriptDialog(CAPTION_MISSION_HEADER_RESULTS, DESCRIPTION_MISSION_HEADER_RESULTS + m_sDialogMessageMissionHeader, dialog))
			return;

		switch (dialog.m_eResult)
		{
			case SCR_EGameModeSetupButton.BACK:
				m_bIsMissionHeaderValid = false;
				ShowPrevPage();
				break;

			case SCR_EGameModeSetupButton.NEXT:
				ShowPage(SCR_EGameModeSetupPage.OUTRO);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RunOutro()
	{
		SCR_GameModeSetupPluginOutro dialog = new SCR_GameModeSetupPluginOutro();
		if (m_bIsWorldValid && m_bIsMissionHeaderValid)
		{
			if (!Workbench.ScriptDialog(CAPTION_OUTRO_GOOD, DESCRIPTION_OUTRO_COMPLETE, dialog))
				return;
		}
		else
		{
			if (!Workbench.ScriptDialog(CAPTION_OUTRO_BAD, DESCRIPTION_OUTRO_INCOMPLETE, dialog))
				return;
		}

		switch (dialog.m_eResult)
		{
			case SCR_EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowPage(SCR_EGameModeSetupPage page)
	{
		if (!m_aPageHistory.Contains(page))
			m_aPageHistory.Insert(page);

		switch (page)
		{
			case SCR_EGameModeSetupPage.INTRO:					Run(); break;
			case SCR_EGameModeSetupPage.VALIDATION:				RunValidation(); break;
			case SCR_EGameModeSetupPage.VALIDATION_RESULTS:		RunValidationResults(); break;
			case SCR_EGameModeSetupPage.GENERATION:				RunGeneration(); break;
			case SCR_EGameModeSetupPage.GENERATION_RESULTS:		RunGenerationResults(); break;
			case SCR_EGameModeSetupPage.MISSION_HEADER:			RunMissionHeader(); break;
			case SCR_EGameModeSetupPage.MISSION_HEADER_RESULTS:	RunMissionHeaderResults(); break;
			case SCR_EGameModeSetupPage.OUTRO:					RunOutro(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowPrevPage()
	{
		int pageHistoryCountMinus1 = m_aPageHistory.Count() - 1;
		m_aPageHistory.Resize(pageHistoryCountMinus1);
		ShowPage(m_aPageHistory[pageHistoryCountMinus1 - 1]);
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadConfig(out string error)
	{
		string worldPath;
		SCR_WorldEditorToolHelper.GetWorldEditorAPI().GetWorldPath(worldPath);
		if (worldPath.IsEmpty())
		{
			error = "No world is currently loaded, or the current world is not saved.";
			return false;
		}

		if (m_sTemplate.IsEmpty())
		{
			error = "No template defined! Please fill the Template field.";
			return false;
		}

		Resource templateResource = Resource.Load(m_sTemplate);
		if (!templateResource.IsValid())
		{
			error = "Template config " + FilePath.StripPath(m_sTemplate) + " is invalid.";
			return false;
		}

		BaseContainer templateContainer = templateResource.GetResource().ToBaseContainer();
		m_Config = GameModeSetupConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(templateContainer));
		if (!m_Config)
		{
			error = "Failed to load the " + FilePath.StripPath(m_sTemplate) + " template config.";
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Next", true)]
	protected bool ButtonNext()
	{
		return true;
	}
}

class SCR_GameModeSetupPluginValidation
{
	SCR_EGameModeSetupButton m_eResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Skip")]
	protected bool ButtonSkip()
	{
		m_eResult = SCR_EGameModeSetupButton.SKIP;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Back")]
	protected bool ButtonBack()
	{
		m_eResult = SCR_EGameModeSetupButton.BACK;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Scan world", true)]
	protected bool ButtonValidate()
	{
		m_eResult = SCR_EGameModeSetupButton.NEXT;
		return true;
	}
}

class SCR_GameModeSetupPluginResults
{
	SCR_EGameModeSetupButton m_eResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Back")]
	protected bool ButtonBack()
	{
		m_eResult = SCR_EGameModeSetupButton.BACK;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Next", true)]
	protected bool ButtonNext()
	{
		m_eResult = SCR_EGameModeSetupButton.NEXT;
		return true;
	}
}

class SCR_GameModeSetupPluginGenerationResults
{
	SCR_EGameModeSetupButton m_eResult;

	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	[ButtonAttribute("Re-scan")]
	protected bool ButtonValidate()
	{
		m_eResult = SCR_EGameModeSetupButton.VALIDATE;
		return true;
	}

	[ButtonAttribute("Back")]
	protected bool ButtonBack()
	{
		m_eResult = SCR_EGameModeSetupButton.BACK;
		return true;
	}

	[ButtonAttribute("Next", true)]
	protected bool ButtonNext()
	{
		m_eResult = SCR_EGameModeSetupButton.NEXT;
		return true;
	}
}

class SCR_GameModeSetupPluginGeneration
{
	SCR_EGameModeSetupButton m_eResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Skip")]
	protected bool ButtonSkip()
	{
		m_eResult = SCR_EGameModeSetupButton.SKIP;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Back")]
	protected bool ButtonBack()
	{
		m_eResult = SCR_EGameModeSetupButton.BACK;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create entities", true)]
	protected bool ButtonGenerate()
	{
		m_eResult = SCR_EGameModeSetupButton.NEXT;
		return true;
	}
}

class SCR_GameModeSetupPluginMissionHeader
{
	SCR_EGameModeSetupButton m_eResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Skip")]
	protected bool ButtonSkip()
	{
		m_eResult = SCR_EGameModeSetupButton.SKIP;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Back")]
	protected bool ButtonBack()
	{
		m_eResult = SCR_EGameModeSetupButton.BACK;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create header", true)]
	protected bool ButtonGenerate()
	{
		m_eResult = SCR_EGameModeSetupButton.NEXT;
		return true;
	}
}

class SCR_GameModeSetupPluginOutro
{
	SCR_EGameModeSetupButton m_eResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Back")]
	protected bool ButtonBack()
	{
		m_eResult = SCR_EGameModeSetupButton.BACK;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected bool ButtonClose()
	{
		return false;
	}
}

class SCR_GameModeSetupPluginError
{
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Back", true)]
	protected int ButtonBack()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return 0;
	}
}

[EnumLinear()]
enum SCR_EGameModeSetupPage
{
	INTRO,
	VALIDATION,
	VALIDATION_RESULTS,
	GENERATION,
	GENERATION_RESULTS,
	MISSION_HEADER,
	MISSION_HEADER_RESULTS,
	OUTRO,
}

[EnumLinear()]
enum SCR_EGameModeSetupButton
{
	CANCEL,
	BACK,
	NEXT,
	SKIP,
	VALIDATE,
}
#endif // WORKBENCH

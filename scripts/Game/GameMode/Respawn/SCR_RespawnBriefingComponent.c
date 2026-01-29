[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Briefing screen shown in respawn menu.")]
class SCR_RespawnBriefingComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_RespawnBriefingComponent : SCR_BaseGameModeComponent
{
	[Attribute("")]
	protected ResourceName m_sJournalConfigPath;

	protected ref SCR_JournalSetupConfig m_JournalConfig;

	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	[Attribute("{324E923535DCACF8}UI/Textures/DeployMenu/Briefing/conflict_HintBanner_1_UI.edds", desc: "background shown when briefing has no hints")]
	protected ResourceName m_SimpleBriefingBackground;
	
	[Attribute()]
	protected ref array<ref SCR_UIInfo> m_aGameModeHints;

	[Attribute()]
	protected ref array<ref SCR_BriefingVictoryCondition> m_aWinConditions;
	
	[Attribute()]
	protected bool m_bShowJournalOnStart;
	
	protected bool m_bWasShown = false;
	protected ref map<int, ref array<string>> m_BriefingInfo = new map<int, ref array<string>>();
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_BriefingInfo.Count());
		array<string> temporaryInfo = {};
		foreach (int entryID, array<string> info : m_BriefingInfo)
		{
			temporaryInfo.Copy(info);
			
			writer.WriteString(temporaryInfo[0]);
			writer.WriteInt(entryID);
			writer.WriteString(temporaryInfo[1]);
			
			//We need to remove faction key and text (Which are the first two elements) so only parameters remain
			temporaryInfo.RemoveOrdered(0);
			temporaryInfo.RemoveOrdered(0);

			writer.WriteInt(temporaryInfo.Count());
			foreach (string briefingStringParam : temporaryInfo)
			{
				writer.WriteString(briefingStringParam);
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int count;
		FactionKey factionKey;
		int entryID;
		string newText;
		int paramCount;
		string paramTemp;
		array<string> param1;
		reader.ReadInt(count);
		for (int i = 0; i < count; i++)
		{
			param1 = {};
			reader.ReadString(factionKey);
			reader.ReadInt(entryID);
			reader.ReadString(newText);
			reader.ReadInt(paramCount);
			for (int j = 0; j < paramCount; j++)
			{
				reader.ReadString(paramTemp);
				param1.Insert(paramTemp);
			}
			
			RewriteEntryMain(factionKey, entryID, newText, param1);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] factionKey
	//! \param[in] entryID
	//! \param[in] newText
	//! \param[in] param1
	void RewriteEntry_SA(FactionKey factionKey, int entryID, string newText, array<string> param1)
	{
		RewriteEntryMain(factionKey, entryID, newText, param1);
		Rpc(RpcDo_RewriteEntry, factionKey, entryID, newText, param1);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] factionKey
	//! \param[in] entryID
	//! \param[in] newText
	//! \param[in] param1
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RewriteEntry(FactionKey factionKey, int entryID, string newText, array<string> param1)
	{
		RewriteEntryMain(factionKey, entryID, newText, param1);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] factionKey
	//! \param[in] entryID
	//! \param[in] newText
	//! \param[in] param1
	void RewriteEntryMain(FactionKey factionKey, int entryID, string newText, array<string> param1)
	{
		m_BriefingInfo.Remove(entryID);
		array<string> infoStrings = {};
		infoStrings.Insert(factionKey);
		infoStrings.Insert(newText);
		infoStrings.InsertAll(param1);
		m_BriefingInfo.Insert(entryID, infoStrings);

		RewriteEntry(factionKey, entryID, newText, param1);
		
		SCR_GameplaySettingsSubMenu.m_OnLanguageChanged.Remove(OnLanguageChanged);
		SCR_GameplaySettingsSubMenu.m_OnLanguageChanged.Insert(OnLanguageChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] factionKey
	//! \param[in] entryID
	//! \param[in] newText
	//! \param[in] param1
	void RewriteEntry(FactionKey factionKey, int entryID, string newText, array<string> param1)
	{
		if (!m_JournalConfig)
			LoadJournalConfig();
		
		if (!m_JournalConfig)
			return;

		SCR_JournalConfig journalConfig = m_JournalConfig.GetJournalConfig(factionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{

			if (journalEntry.GetEntryID() != entryID)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}
		
		if (!targetJournalEntry)
			return;
		
		targetJournalEntry.SetEntryText(newText);
		string stringParam1;

		for (int i = 0, count = param1.Count() * 2; i < count; i += 2) // step 2
		{
			if (!param1.IsIndexValid(i))
				break;
			
			stringParam1 = stringParam1 + "<br/><br/>" + LocalizedString.Format(WidgetManager.Translate(param1[i], param1[i + 1]));
			
		}

		targetJournalEntry.SetEntryTextParam1(stringParam1);
		
		Widget widgetToRefresh;
		widgetToRefresh = targetJournalEntry.GetWidget();
		if (widgetToRefresh)
			targetJournalEntry.SetEntryLayoutTo(widgetToRefresh);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLanguageChanged(SCR_GameplaySettingsSubMenu menu)
	{
		array<string> temporaryInfo = {};
		foreach (int entryID, array<string> info : m_BriefingInfo)
		{
			temporaryInfo.Copy(info);
			FactionKey factionKey = temporaryInfo[0];
			string newText = temporaryInfo[1];
			
			//We need to remove faction key and text (Which are the first two elements) so only parameters remain
			temporaryInfo.RemoveOrdered(0);
			temporaryInfo.RemoveOrdered(0);
		
			RewriteEntry(factionKey, entryID, newText, temporaryInfo);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] targetID
	//! \return
	array<string> GetBriefingStringParamByID(int targetID)
	{
		// change it to get last element
		array<string> strings = {};
		strings = m_BriefingInfo.Get(targetID);
		if (!strings || strings.IsEmpty())
			return null;
			
		array<string> stringsCopy = {};
		stringsCopy.Copy(strings);
			
		//We need to remove faction key and text (Which are the first two elements) so only parameters remain
		stringsCopy.RemoveOrdered(0);
		stringsCopy.RemoveOrdered(0);
			
		return stringsCopy;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_JournalSetupConfig GetJournalSetup()
	{
		if (!m_JournalConfig)
			LoadJournalConfig();
		
		return m_JournalConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetConfig()
	{
		m_JournalConfig = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool LoadJournalConfig()
	{
		if (m_JournalConfig)
			return true;
		
		if (m_sJournalConfigPath.GetPath().IsEmpty())
		{
			Print("Journal config path is empty!", LogLevel.WARNING);
			return false;
		}

		Resource holder = BaseContainerTools.LoadContainer(m_sJournalConfigPath);
		if (!holder || !holder.IsValid())
			return false;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return false;

		m_JournalConfig = SCR_JournalSetupConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!m_JournalConfig)
		{
			Print("Journal config couldn't be created!", LogLevel.WARNING);
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Local instance of the briefing component.
	static SCR_RespawnBriefingComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return SCR_RespawnBriefingComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_RespawnBriefingComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Briefing UI info
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Simple briefing background image
	ResourceName GetSimpleBriefingBackground()
	{
		return m_SimpleBriefingBackground;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetJournalConfigPath()
	{
		return m_sJournalConfigPath;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] hints
	//! \return
	int GetGameModeHints(out array<ref SCR_UIInfo> hints)
	{
		hints = m_aGameModeHints;

		return m_aGameModeHints.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] conditions
	//! \return
	int GetWinConditions(out array<ref SCR_BriefingVictoryCondition> conditions)
	{
		conditions = m_aWinConditions;

		return m_aWinConditions.Count();
	}

	//------------------------------------------------------------------------------------------------	
	//! \param[in] shown
	void SetBriefingShown(bool shown = true)
	{
		m_bWasShown = shown;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetWasBriefingShown()
	{
		return m_bWasShown;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ShowJournalOnStart()
	{
		return m_bShowJournalOnStart;
	}
}

[BaseContainerProps()]
class SCR_BriefingVictoryCondition
{
	[Attribute("1", UIWidgets.ComboBox, "Type of victory condition", "", ParamEnumArray.FromEnum(ETaskIconType))]
	protected ETaskIconType victoryCondition;
	
	[Attribute()]
	protected string name;
	
	[Attribute()]
	protected string description;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetName()
	{
		return name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetDescription()
	{
		return description;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ETaskIconType GetConditionType()
	{
		return victoryCondition;
	}
}

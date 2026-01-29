/*!
Scripted DS config to store server config from script
*/

class SCR_DSConfig : DSConfig
{
	protected const string DEFAULT_FILE_NAME = "GeneratedServerConfig";

	protected ref array<ref SCR_WidgetListEntry> m_aDSConfigEntries = {};

	protected ref SCR_DSOperating m_Operating;
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
		super.OnPack();
		StoreObject("operating", m_Operating);
	}
	
	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SCR_DSConfig()
	{
		game = new DSGameConfig();
		game.gameProperties = new SCR_DSGameProperties();
		game.mods = {};
		
		// Register queue settings 
		m_Operating = new SCR_DSOperating();
		m_Operating.joinQueue = new SCR_DSJoinQueue();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Store all json values by groups 
	void StoreFullJson(array<ref SCR_WidgetListEntry> entries, array<ref DSMod> modList, WorkshopItem scenarioMod)
	{
		m_aDSConfigEntries = entries;
		
		// Config 
		bindAddress = FindValue("bindAddress");
		bindPort = StringToNumber(FindValue("bindPort"));
		publicAddress = FindValue("publicAddress");
		publicPort = StringToNumber(FindValue("publicPort"));
		
		// Game
		game.name = FindValue("name");
		game.maxPlayers = StringToNumber(FindValue("maxPlayers"));
		game.password = FindValue("password");
		game.passwordAdmin = FindValue("passwordAdmin");
		game.visible = SCR_JsonApiStructHandler.StringToBool(FindValue("visible"));
		game.crossPlatform = SCR_JsonApiStructHandler.StringToBool(FindValue("crossPlatform"));
		
		// Game properties 
		SCR_DSGameProperties gamePropertiesSCr = SCR_DSGameProperties.Cast(game.gameProperties);
		
		if (gamePropertiesSCr)
		{
			gamePropertiesSCr.battlEye = SCR_JsonApiStructHandler.StringToBool(FindValue("battlEye"));
			gamePropertiesSCr.disableThirdPerson = SCR_JsonApiStructHandler.StringToBool(FindValue("disableThirdPerson"));
			gamePropertiesSCr.VONDisableUI = SCR_JsonApiStructHandler.StringToBool(FindValue("VONDisableUI"));
			gamePropertiesSCr.VONDisableDirectSpeechUI = SCR_JsonApiStructHandler.StringToBool(FindValue("VONDisableDirectSpeechUI"));
			gamePropertiesSCr.VONCanTransmitCrossFaction = SCR_JsonApiStructHandler.StringToBool(FindValue("VONCanTransmitCrossFaction"));
			gamePropertiesSCr.serverMaxViewDistance = StringToNumber(FindValue("serverMaxViewDistance"));
			gamePropertiesSCr.networkViewDistance = StringToNumber(FindValue("networkViewDistance"));
			gamePropertiesSCr.serverMinGrassDistance = StringToNumber(FindValue("serverMinGrassDistance"));
		}
		
		// Scenario 
		game.scenarioId = FindValue("scenarioId");
		
		// Mods
		game.mods = modList;
		
		/*if (!game.name.IsEmpty())
			m_sConfigName = game.name;*/
		
		// Scenario mod
		if (scenarioMod)
		{
			game.hostedScenarioModId = scenarioMod.Id();
			
			// Check mod in list 
			bool inList = false;
			
			for (int i = 0, count = game.mods.Count(); i < count; i++)
			{
				if (game.mods[i].modId == scenarioMod.Id())
				{
					inList = true;
					break;
				}
			}
			
			// Add to the list
			if (!inList)
			{
				DSMod scenarioDSMod = new DSMod();
				scenarioDSMod.modId = scenarioMod.Id();
				scenarioDSMod.name = scenarioMod.Name();
				scenarioDSMod.version = scenarioMod.GetActiveRevision().GetVersion(); 
				
				
				game.mods.Insert(scenarioDSMod);
			}
		}
		
		// Join queue 
		m_Operating.joinQueue.maxSize = StringToNumber(FindValue("joinQueueMaxSize"));
	}
	
	//------------------------------------------------------------------------------------------------
	string GetConfigName()
	{
		string name = FindValue("fileName");
		if (name != "")
			return name;
		
		name = FindValue("name");
		if (name != "")
			return name;
		
		return DEFAULT_FILE_NAME;
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Return value as string by property name
	protected string FindValue(string propertyName, string groupTag = "")
	{
		string nameTrimmed = "";
		
		for (int i = 0, count = m_aDSConfigEntries.Count(); i < count; i++)
		{
			nameTrimmed = m_aDSConfigEntries[i].GetPropertyName();
			nameTrimmed = nameTrimmed.Trim();
			
			if (groupTag == "")
			{
				// Has given property name
				if (nameTrimmed == propertyName)
					return m_aDSConfigEntries[i].ValueAsString();
			}
			else 
			{
				// Has propery and group
				if (nameTrimmed == propertyName && m_aDSConfigEntries[i].GetGroupTag() == groupTag)
					return m_aDSConfigEntries[i].ValueAsString();
			}
		}
		
		return "";
	}
	
	//------------------------------------------------------------------------------------------------
	protected float StringToNumber(string str)
	{
		if (str.IsEmpty())
			return 0;
		
		return str.ToFloat();
	}
}

//------------------------------------------------------------------------------------------------
class SCR_DSGameProperties : DSGameProperties
{
	bool disableThirdPerson = false;
	bool VONDisableUI = false;
	bool VONDisableDirectSpeechUI = false;
	bool VONCanTransmitCrossFaction = false;
	float serverMaxViewDistance = 0.0;
	int networkViewDistance = 0;
	int serverMinGrassDistance = 0;
	
	//------------------------------------------------------------------------------------------------
	void SCR_DSGameProperties()
	{
		RegV("serverMaxViewDistance");
		RegV("networkViewDistance");
		RegV("serverMinGrassDistance");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
		super.OnPack();
		
		UnregV("disableThirdPerson");
		StoreBoolean("disableThirdPerson", disableThirdPerson);
		UnregV("VONDisableUI");
		StoreBoolean("VONDisableUI", VONDisableUI);
		UnregV("VONDisableDirectSpeechUI");
		StoreBoolean("VONDisableDirectSpeechUI", VONDisableDirectSpeechUI);
		UnregV("VONCanTransmitCrossFaction");
		StoreBoolean("VONCanTransmitCrossFaction", VONCanTransmitCrossFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExpand()
	{
		RegV("disableThirdPerson");
		RegV("VONDisableUI");
		RegV("VONDisableDirectSpeechUI");
		RegV("VONCanTransmitCrossFaction");
	}
}

class SCR_DSOperating : JsonApiStruct
{
	ref SCR_DSJoinQueue joinQueue;
	
	//------------------------------------------------------------------------------------------------
	void SCR_DSOperating()
	{
		joinQueue = new SCR_DSJoinQueue();
		RegV("joinQueue");
	}
}

class SCR_DSJoinQueue : JsonApiStruct
{
	int maxSize;
	
	//------------------------------------------------------------------------------------------------
	void SCR_DSJoinQueue()
	{
		RegV("maxSize");
	}
}
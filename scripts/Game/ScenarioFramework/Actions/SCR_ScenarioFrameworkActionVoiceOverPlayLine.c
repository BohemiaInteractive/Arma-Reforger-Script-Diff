[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionVoiceOverPlayLine : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Config with voice over data for this action.", params: "conf class=SCR_VoiceoverData")]
	ResourceName m_sVoiceOverDataConfig;
	
	[Attribute(desc: "Name of the line as defined in Voice Over Data Config")]
	string	m_sLineName;
	
	[Attribute(desc: "Entity playing the voiceover. Order must be the same as the order of actor enums in Voice Over Data Config. Reminder: Related .acp needs to be present in the SCR_CommunicationSoundComponent of target entity.")]
	ref SCR_ScenarioFrameworkGet m_ActorGetter;
	
	[Attribute(desc: "Player entity or entities that the voice over will be played for. Reminder: Related .acp needs to be present in the SCR_CommunicationSoundComponent of target entity.")]
	ref SCR_ScenarioFrameworkGet m_PlayerGetter;
	
	[Attribute(defvalue: "1", desc: "If Voice Line should play right now or wait for the current voice lines to finish and then play it")]
	bool m_bPlayImmediately;
	
	[Attribute(desc: "Allow VO Playing In Gameover and when game is stopped")]
	bool m_bAllowPlayingInGameOver;
	
	[Attribute(desc: "Actions that will be triggered once Sequence finishes playing")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_sVoiceOverDataConfig || m_sVoiceOverDataConfig.IsEmpty())
		{
			SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
			if (!scenarioFrameworkSystem)
				return;
			
			m_sVoiceOverDataConfig = scenarioFrameworkSystem.m_sVoiceOverDataConfig;
			
			if (!m_sVoiceOverDataConfig || m_sVoiceOverDataConfig.IsEmpty())
				return;
		}
		
		if (!CanActivate())
			return;
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode || (!m_bAllowPlayingInGameOver && !gamemode.IsRunning()))
			return;
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		IEntity entity;
		array<IEntity> entities = {};
		if (m_PlayerGetter)
		{
			SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_PlayerGetter.Get());
			if (entityArrayWrapper)
			{
				entities = entityArrayWrapper.GetValue();
				if (!entities || entities.IsEmpty())
					return;
			}
			else
			{
				SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_PlayerGetter.Get());
				if (!entityWrapper)
				{
					if (object)
						Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
					
				return;
				}
	
				IEntity playerEntity = entityWrapper.GetValue();
				
				entities.Insert(playerEntity);
			}
		}	
		
		if (!ValidateInputEntity(object, m_ActorGetter, entity))
			return;
		
		array<int> playerIDs = {};
		foreach(IEntity possiblePlayerEntity : entities)
		{
			if (EntityUtils.IsPlayer(possiblePlayerEntity))
				playerIDs.Insert(playerManager.GetPlayerIdFromControlledEntity(possiblePlayerEntity))
		}
		
		SCR_VoiceoverSystem voiceoverSystem = SCR_VoiceoverSystem.GetInstance();
		if (!voiceoverSystem)
			return;
		
		voiceoverSystem.SetData(m_sVoiceOverDataConfig);
		voiceoverSystem.GetOnFinished().Insert(OnFinished);
		
		if (RplSession.Mode() == RplMode.None)
		{
			voiceoverSystem.PlayLine(m_sLineName, entity, m_bPlayImmediately);
		}
		else
		{
			if (!Replication.FindId(entity))
				PrintFormat("ScenarioFramework Action %1 detected %2 without RplComponent thus VO won't work!", object, entity);
		
			voiceoverSystem.PlayLineFor(m_sLineName, entity, playerIDs, m_bPlayImmediately);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFinished()
	{
		SCR_VoiceoverSystem voiceoverSystem = SCR_VoiceoverSystem.GetInstance();
		if (voiceoverSystem)
			voiceoverSystem.GetOnFinished().Remove(OnFinished);
		
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}
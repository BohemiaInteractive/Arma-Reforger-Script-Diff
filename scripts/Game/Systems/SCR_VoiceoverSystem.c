//------------------------------------------------------------------------------------------------
class SCR_VoiceoverSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	protected ResourceName m_sDataResourceName;
	
	protected ref ScriptInvokerVoid m_OnFinished;
	protected ref ScriptInvokerString m_OnFinishedEvent;

	protected ref SCR_VoiceoverData m_Data;

	protected ref SCR_VoiceoverSubtitles m_SubtitlesDisplay;

	protected ref array<ref SCR_VoiceoverLine> m_aQueue = {};
	protected ref array<IEntity> m_aActors = {};
	protected ref array<SCR_CommunicationSoundComponent> m_aHandledComponents = {};
	
	protected ref map<SCR_EVoiceoverActor, string> m_mActorNames = new map<SCR_EVoiceoverActor, string>();

	protected SCR_CommunicationSoundComponent m_PlayingSoundComponent;

	protected AudioHandle m_iAudioHandle;
	
	protected bool m_bSubtitlesHandled = true;

	protected int m_iCurrentEndCommand = 0;

	protected float m_fTimer;
	protected float m_fCustomPause = -1;

	protected string m_sCurrentSequence;
	protected string m_sCurrentSoundEvent;
	protected string m_sCurrentSubtitleEvent;
	protected string m_sCurrentActorName;
	protected string m_sCurrentSubtitle;

	protected IEntity m_CurrentActor;

	protected static const float PAUSE_BETWEEN_LINES = 0.5;
	
	protected static const int SUBTITLE_VISIBILITY_DISTANCE_SQ = 30 * 30;
	protected static const int SUBTITLE_FAILSAFE_TOGGLE_DELAY_MS = 15000;
	
	protected static const string DEFAULT_ANIM_COMMAND = "CMD_Gesture_NPC";
	protected static const string DEFAULT_ANIM_VARIABLE = "speech";

	//------------------------------------------------------------------------------------------------
	override event protected void OnStarted()
	{
		// Dedicated server doesn't have to process start event as no sound is supposed to be played there
		if (System.IsConsoleApp())
			return;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!gameMode)
			return;

		gameMode.GetOnControllableDestroyed().Insert(OnControllableDestroyed);
		gameMode.GetOnControllableDeleted().Insert(OnControllableDeleted);

	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnStopped()
	{
		// Dedicated server doesn't have to process stop event as no sound is supposed to be played there
		if (System.IsConsoleApp())
			return;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!gameMode)
			return;

		gameMode.GetOnControllableDestroyed().Remove(OnControllableDestroyed);
		gameMode.GetOnControllableDeleted().Remove(OnControllableDeleted);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		// Dedicated server doesn't have to process update event as no sound is supposed to be played there
		if (System.IsConsoleApp())
			return;

		if (!m_PlayingSoundComponent)
		{
			m_fTimer = 0;
			Enable(false);
			OnFinished();
			return;
		}

		if (m_iAudioHandle != AudioHandle.Invalid && !m_PlayingSoundComponent.IsFinishedPlaying(m_iAudioHandle))
			return;

		if (m_Data && m_CurrentActor)
		{
			SCR_VoiceoverLine line = m_Data.GetLineBySoundEvent(m_sCurrentSoundEvent);

			if (!line)
				line = m_Data.GetLineBySoundEvent(m_sCurrentSoundEvent, m_sCurrentSequence);

			if (line && line.GetFinishAnimCommandParameter() != 0)
				CallAnimCommand(m_CurrentActor, line.GetFinishAnimCommandParameter());	
		}

		m_CurrentActor = null;

		if (m_aQueue.IsEmpty())
		{
			// Unregister previous actor
			if (m_PlayingSoundComponent && m_PlayingSoundComponent.GetOwner())
				RemoveActorUnconsciousCheck(m_PlayingSoundComponent.GetOwner());
			
			m_fTimer = 0;
			Enable(false);
			OnFinished();
		}
		else
		{
			m_fTimer += GetWorld().GetFixedTimeSlice();
			float pause;
			
			if (m_fCustomPause >= 0)
				pause = m_fCustomPause;
			else
				pause = PAUSE_BETWEEN_LINES;

			if (m_fTimer >= pause)
			{
				m_fTimer = 0;
				PlayFromQueue();
			}
			else if (!m_bSubtitlesHandled && m_fTimer >= SCR_VoiceoverSubtitles.GetLingerDuration())
			{
				m_bSubtitlesHandled = true;

				if (m_SubtitlesDisplay)
					m_SubtitlesDisplay.Show(false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_VoiceoverSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return SCR_VoiceoverSystem.Cast(world.FindSystem(SCR_VoiceoverSystem));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stop the voiceover when the speaker dies
	protected void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (!IsPlaying())
			return;
		
		IEntity entity = instigatorContextData.GetVictimEntity();
		
		if (!entity)
			return;
		
		SCR_CommunicationSoundComponent comp = SCR_CommunicationSoundComponent.Cast(entity.FindComponent(SCR_CommunicationSoundComponent));
		
		if (!comp || comp != m_PlayingSoundComponent)
			return;
		
		Stop();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stop the voiceover when the speaker is deleted
	protected void OnControllableDeleted(IEntity entity)
	{
		if (!entity || !IsPlaying())
			return;

		SCR_CommunicationSoundComponent comp = SCR_CommunicationSoundComponent.Cast(entity.FindComponent(SCR_CommunicationSoundComponent));
		
		if (!comp || comp != m_PlayingSoundComponent)
			return;
		
		Stop();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stop the voiceover when the speaker becomes unconscious
	protected void OnActorLifeStateChanged()
	{
		IEntity currentActor = m_PlayingSoundComponent.GetOwner();
		
		if (!currentActor)
			return;
		
		SCR_CharacterControllerComponent actorCharController = SCR_CharacterControllerComponent.Cast(currentActor.FindComponent(SCR_CharacterControllerComponent));
		
		if (actorCharController && actorCharController.IsUnconscious())
			Stop();
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a line or sequence finishes playing
	ScriptInvokerVoid GetOnFinished()
	{
		if (!m_OnFinished)
			m_OnFinished = new ScriptInvokerVoid();

		return m_OnFinished;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when an event finishes playing
	ScriptInvokerString GetOnFinishedEvent()
	{
		if (!m_OnFinishedEvent)
			m_OnFinishedEvent = new ScriptInvokerString();

		return m_OnFinishedEvent;
	}

	//------------------------------------------------------------------------------------------------
	//! Loads voiceover data from the given SCR_VoiceoverData config resource
	//! Must be called before PlaySequence or PlayLine which both use data from this config!
	void SetData(ResourceName config)
	{
		m_sDataResourceName = config;
		
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		Resource container = BaseContainerTools.LoadContainer(config);
		m_Data = SCR_VoiceoverData.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
	}

	//------------------------------------------------------------------------------------------------
	bool IsPlaying()
	{
		return IsEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Actor name associated with the given enum will be shown in the subtitle
	void SetActorName(SCR_EVoiceoverActor actor, string name)
	{
		m_mActorNames.Set(actor, name);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearActorNames()
	{
		m_mActorNames.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void AddActorUnconsciousCheck(notnull IEntity actor)
	{
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(actor.FindComponent(SCR_CharacterControllerComponent));
		
		if (!charController)
			return;
		
		charController.m_OnLifeStateChanged.Remove(OnActorLifeStateChanged); // Avoid duplicate calls
		charController.m_OnLifeStateChanged.Insert(OnActorLifeStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveActorUnconsciousCheck(notnull IEntity actor)
	{
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(actor.FindComponent(SCR_CharacterControllerComponent));
		
		if (!charController)
			return;
		
		charController.m_OnLifeStateChanged.Remove(OnActorLifeStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays a sequence of voiceover lines defined in loaded config (see SetData)
	// \param sequenceName Name of the sequence as defined in SCR_VoiceoverData
	// \param actor1 Entity playing the voiceover as defined in SCR_VoiceoverData by ACTOR enum (default: player character)
	// \param playImmediately True to play straight away, cutting the currently spoken VO; false to wait until the current VO finishes
	void PlaySequence(string sequenceName, IEntity actor1 = null, IEntity actor2 = null, IEntity actor3 = null, IEntity actor4 = null, IEntity actor5 = null, bool playImmediately = true)
	{
		if (!playImmediately && IsPlaying())
		{
			GetGame().GetCallqueue().CallLater(PlaySequence, PAUSE_BETWEEN_LINES, false, sequenceName, actor1, actor2, actor3, actor4, actor5, playImmediately);
			return;
		}
		
		if (!m_Data)
			return;
		
		SCR_VoiceoverSequence sequence = m_Data.GetSequenceByName(sequenceName);

		if (!sequence)
			return;

		m_sCurrentSequence = sequenceName;
		sequence.GetLines(m_aQueue);
		m_aActors = {actor1, actor2, actor3, actor4, actor5};

		PlayFromQueue(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Same as PlaySequence but accepts an array of actors
	void PlaySequenceActorsArray(string sequenceName, notnull array<IEntity> actors, bool playImmediately = true)
	{
		if (!playImmediately && IsPlaying())
		{
			GetGame().GetCallqueue().CallLater(PlaySequenceActorsArray, PAUSE_BETWEEN_LINES, false, sequenceName, actors, playImmediately);
			return;
		}
		
		if (!m_Data)
			return;
		
		SCR_VoiceoverSequence sequence = m_Data.GetSequenceByName(sequenceName);

		if (!sequence)
			return;

		m_sCurrentSequence = sequenceName;
		sequence.GetLines(m_aQueue);
		m_aActors = actors;

		PlayFromQueue(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Same as PlaySequence but plays the VO for all clients. Has to be executed on server.
	void PlaySequenceGlobal(string sequenceName, notnull array<IEntity> actors, bool playImmediately = true)
	{
		array<RplId> actorIds = {};
		
		foreach (IEntity actor : actors)
		{
			actorIds.Insert(Replication.FindId(actor));
		}
		
		Rpc(RpcDo_PlaySequence, m_sDataResourceName, sequenceName, actorIds, playImmediately);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_PlaySequence(m_sDataResourceName, sequenceName, actorIds, playImmediately);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Same as PlaySequenceGlobal but plays the VO for clients defined by player IDs
	// \param targetPlayerIds array of player IDs to play the voiceover for
	void PlaySequenceFor(string sequenceName, notnull array<IEntity> actors, notnull array<int> targetPlayerIds, bool playImmediately = true)
	{
		if (targetPlayerIds.IsEmpty())
			return;
		
		array<RplId> actorIds = {};
		
		foreach (IEntity actor : actors)
		{
			actorIds.Insert(Replication.FindId(actor));
		}
		
		Rpc(RpcDo_PlaySequenceFor, m_sDataResourceName, sequenceName, actorIds, targetPlayerIds, playImmediately);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_PlaySequenceFor(m_sDataResourceName, sequenceName, actorIds, targetPlayerIds, playImmediately);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlaySequence(string dataName, string sequenceName, array<RplId> actorIds, bool playImmediately)
	{
		array<IEntity> actors = {};
		
		foreach (RplId actorId : actorIds)
		{
			actors.Insert(IEntity.Cast(Replication.FindItem(actorId)));
		}
		
		if (dataName != m_sDataResourceName)
			SetData(dataName);
		
		PlaySequenceActorsArray(sequenceName, actors, playImmediately);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlaySequenceFor(string dataName, string sequenceName, array<RplId> actorIds, array<int> targetPlayerIds, bool playImmediately)
	{
		if (!targetPlayerIds.Contains(SCR_PlayerController.GetLocalPlayerId()))
			return;

		array<IEntity> actors = {};
		
		foreach (RplId actorId : actorIds)
		{
			actors.Insert(IEntity.Cast(Replication.FindItem(actorId)));
		}
		
		if (dataName != m_sDataResourceName)
			SetData(dataName);
		
		PlaySequenceActorsArray(sequenceName, actors, playImmediately);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays a voiceover line defined in loaded config (see SetData)
	// \param lineName Name of the line as defined in SCR_VoiceoverData
	// \param actor1 Entity playing the voiceover (default: player character)
	// \param playImmediately True to play straight away, cutting the currently spoken VO; false to wait until the current VO finishes
	void PlayLine(string lineName, IEntity actor = null, bool playImmediately = true)
	{
		if (!playImmediately && IsPlaying())
		{
			GetGame().GetCallqueue().CallLater(PlayLine, PAUSE_BETWEEN_LINES, false, lineName, actor, playImmediately);
			return;
		}
		
		if (!m_Data)
			return;
		
		SCR_VoiceoverLineStandalone line = m_Data.GetLineByName(lineName);

		if (!line)
			return;

		m_aQueue.Clear();
		
		int distanceThreshold = SUBTITLE_VISIBILITY_DISTANCE_SQ;
		int customThreshold = line.GetCustomSubtitleDistanceThreshold();
		
		if (customThreshold > 0)
			distanceThreshold = customThreshold * customThreshold;
		
		CameraBase playerCamera = GetGame().GetCameraManager().CurrentCamera();

		if (!actor || (playerCamera && vector.DistanceSq(actor.GetOrigin(), playerCamera.GetOrigin()) <= distanceThreshold))
			PlayLineActual(line.GetSoundEventName(), line.GetSubtitleText(), actor, line.GetActorName(), line.GetStartAnimCommandParameter(), line.InterruptCurrentLine(), line.GetFinishAnimCommandParameter());
		else
			PlayLineActual(line.GetSoundEventName(), string.Empty, actor, animCommandParam: line.GetStartAnimCommandParameter(), line.InterruptCurrentLine(), line.GetFinishAnimCommandParameter());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Same as PlayLine but plays the VO for all clients. Has to be executed on server.
	void PlayLineGlobal(string lineName, notnull IEntity actor, bool playImmediately = true)
	{
		Rpc(RpcDo_PlayLine, m_sDataResourceName, lineName, Replication.FindId(actor), playImmediately);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_PlayLine(m_sDataResourceName, lineName, Replication.FindId(actor), playImmediately);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Same as PlayLineGlobal but plays the VO for clients defined by player IDs
	// \param targetPlayerIds array of player IDs to play the voiceover for
	void PlayLineFor(string lineName, notnull IEntity actor, notnull array<int> targetPlayerIds, bool playImmediately = true)
	{
		if (targetPlayerIds.IsEmpty())
			return;
		
		Rpc(RpcDo_PlayLineFor, m_sDataResourceName, lineName, Replication.FindId(actor), targetPlayerIds, playImmediately);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_PlayLineFor(m_sDataResourceName, lineName, Replication.FindId(actor), targetPlayerIds, playImmediately);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlayLine(string dataName, string lineName, RplId actorId, bool playImmediately)
	{
		if (dataName != m_sDataResourceName)
			SetData(dataName);
		
		PlayLine(lineName, IEntity.Cast(Replication.FindItem(actorId)), playImmediately);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlayLineFor(string dataName, string lineName, RplId actorId, array<int> targetPlayerIds, bool playImmediately)
	{
		if (!targetPlayerIds.Contains(SCR_PlayerController.GetLocalPlayerId()))
			return;

		if (dataName != m_sDataResourceName)
			SetData(dataName);
		
		PlayLine(lineName, IEntity.Cast(Replication.FindItem(actorId)), playImmediately);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays a custom voiceover line
	// \param soundEventName Sound event name as defined in the appropriate .acp file in the actor's SCR_CommunicationSoundComponent
	// \param actor Entity playing the voiceover (default: player character)
	// \param playImmediately True to play straight away, cutting the currently spoken VO; false to wait until the current VO finishes
	void PlayCustomLine(string soundEventName, string subtitle, IEntity actor = null, string actorName = string.Empty, bool playImmediately = true)
	{
		if (!playImmediately && IsPlaying())
		{
			GetGame().GetCallqueue().CallLater(PlayCustomLine, PAUSE_BETWEEN_LINES, false, soundEventName, subtitle, actor, actorName, playImmediately);
			return;
		}
		
		PlayLineActual(soundEventName, subtitle, actor, actorName);
	}

	//------------------------------------------------------------------------------------------------
	void Stop()
	{
		if (m_PlayingSoundComponent && m_iAudioHandle != AudioHandle.Invalid)
			m_PlayingSoundComponent.Terminate(m_iAudioHandle);
		
		m_iAudioHandle = AudioHandle.Invalid;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CallAnimCommand(IEntity actor, int commandParam)
	{
		if (!actor)
			return;
		
		ChimeraCharacter char = ChimeraCharacter.Cast(actor);

		if (!char)
			return;

		CharacterAnimationComponent comp = char.GetAnimationComponent();

		if (!comp)
			return;

		TAnimGraphCommand commandId = comp.BindCommand(DEFAULT_ANIM_COMMAND);
		comp.CallCommand4I(commandId, 0, 0, commandParam, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Registers an actor so subtitles can be shown when a voiceover is not played from the voiceover system (like from animations etc.)
	//! Needs to be called before the sound event is played
	//! Voiceover data with subtitles still has to be set properly with corresponding sound event
	void RegisterActor(notnull IEntity entity)
	{
		SCR_CommunicationSoundComponent playingSoundComponent = SCR_CommunicationSoundComponent.Cast(entity.FindComponent(SCR_CommunicationSoundComponent));

		if (!playingSoundComponent || m_aHandledComponents.Contains(playingSoundComponent))
			return;
		
		m_aHandledComponents.Insert(playingSoundComponent);
		playingSoundComponent.GetOnSoundEventStarted().Insert(OnSoundEventStarted);
		playingSoundComponent.GetOnSoundEventFinished().Insert(OnSoundEventFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlayFromQueue(bool start = false)
	{
		if (m_aQueue.IsEmpty())
			return;

		if (start)
		{
			m_fCustomPause = m_aQueue[0].GetPauseBeforeLine();
			
			if (m_fCustomPause > 0)
			{
				// First line in the sequence has a custom pause added run it later
				GetGame().GetCallqueue().CallLater(PlayFromQueue, m_fCustomPause * 1000, false, false);
				return;
			}
		}

		SCR_VoiceoverLine line = m_aQueue[0];
		m_aQueue.RemoveOrdered(0);
		
		string actorName = line.GetActorName();
		
		if (actorName.IsEmpty())
			actorName = m_mActorNames.Get(line.GetActor());

		PlayLineActual(line.GetSoundEventName(), line.GetSubtitleText(), m_aActors[line.GetActor()], actorName, line.GetStartAnimCommandParameter(), line.InterruptCurrentLine());
		m_bSubtitlesHandled = false;

		if (!m_aQueue.IsEmpty())
			m_fCustomPause = m_aQueue[0].GetPauseBeforeLine();
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayLineActual(string eventName, string subtitle, IEntity actor = null, string actorName = string.Empty, int animCommandParam = 0, bool interruptCurrentLine = 1, int finishAnimCommandParam = -1)
	{
		if (!actor)
			actor = GetDefaultActor();

		if (!actor)
			return;
		
		// Unregister previous actor
		if (m_PlayingSoundComponent && m_PlayingSoundComponent.GetOwner())
			RemoveActorUnconsciousCheck(m_PlayingSoundComponent.GetOwner());

		m_PlayingSoundComponent = SCR_CommunicationSoundComponent.Cast(actor.FindComponent(SCR_CommunicationSoundComponent));

		if (!m_PlayingSoundComponent)
			return;

		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(actor.FindComponent(SCR_CharacterControllerComponent));
		
		if (!comp || !comp.IsUnconscious())
		{
			if (interruptCurrentLine && m_iAudioHandle != AudioHandle.Invalid)
			{
				AudioSystem.TerminateSound(m_iAudioHandle);
				
				CallAnimCommand(m_CurrentActor, m_iCurrentEndCommand);	
			}
			
			m_iAudioHandle = m_PlayingSoundComponent.SoundEvent(eventName);
			m_iCurrentEndCommand = finishAnimCommandParam;
			m_sCurrentSoundEvent = eventName;
			m_CurrentActor = actor;
            m_sCurrentActorName = actorName;
			m_sCurrentSubtitle = subtitle;
		
			if (m_iAudioHandle != AudioHandle.Invalid && !subtitle.IsEmpty())
				ShowSubtitle(subtitle, actorName, eventName);
			
			AddActorUnconsciousCheck(actor);

			if (animCommandParam > 0)
				CallAnimCommand(actor, animCommandParam);
		}

		Enable(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSoundEventStarted(SCR_CommunicationSoundComponent component, string eventName, AudioHandle handle, int priority)
	{
		if (!m_Data)
			return;
		
		SCR_VoiceoverLine line = m_Data.GetLineBySoundEvent(eventName);
		
		if (!line)
			return;

		if (handle == AudioHandle.Invalid)
			return;
		
		m_iAudioHandle = handle;
		m_PlayingSoundComponent = component;
		
		if (m_PlayingSoundComponent && m_PlayingSoundComponent.GetOwner())
			AddActorUnconsciousCheck(m_PlayingSoundComponent.GetOwner());

		CameraBase playerCamera = GetGame().GetCameraManager().CurrentCamera();
		
		int distanceThreshold = SUBTITLE_VISIBILITY_DISTANCE_SQ;
		int customThreshold = line.GetCustomSubtitleDistanceThreshold();
		
		if (customThreshold > 0)
			distanceThreshold = customThreshold * customThreshold;
		
		if (playerCamera && vector.DistanceSq(component.GetOwner().GetOrigin(), playerCamera.GetOrigin()) <= distanceThreshold)
			ShowSubtitle(line.GetSubtitleText(), line.GetActorName(), eventName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSoundEventFinished(SCR_CommunicationSoundComponent component, string eventName, AudioHandle handle, int priority, bool terminated)
	{
		if (m_PlayingSoundComponent && m_PlayingSoundComponent.GetOwner())
			RemoveActorUnconsciousCheck(m_PlayingSoundComponent.GetOwner());
		
		// Hide the subtitle after desired delay
		if (m_SubtitlesDisplay && eventName == m_sCurrentSubtitleEvent)
			GetGame().GetCallqueue().CallLater(m_SubtitlesDisplay.Show, SCR_VoiceoverSubtitles.GetLingerDuration() * 1000, false, false, UIConstants.FADE_RATE_INSTANT, EAnimationCurve.LINEAR);

		OnFinished(eventName, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowSubtitle(string subtitle, string actorName, string eventName)
	{
		if (!m_SubtitlesDisplay)
		{
			SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();

			if (!hudManager)
				return;

			m_SubtitlesDisplay = SCR_VoiceoverSubtitles.Cast(hudManager.FindInfoDisplay(SCR_VoiceoverSubtitles));

			if (!m_SubtitlesDisplay)
				return;
		}

		GetGame().GetCallqueue().Remove(m_SubtitlesDisplay.Show);
		
		if (actorName.IsEmpty())
			m_SubtitlesDisplay.ShowSubtitle(subtitle);
		else
			m_SubtitlesDisplay.ShowSubtitle(actorName + ":<br/>" + subtitle);
		
		m_sCurrentSubtitleEvent = eventName;
		
		GetGame().GetCallqueue().Remove(FailsafeSubtitleToggle);
		GetGame().GetCallqueue().CallLater(FailsafeSubtitleToggle, SUBTITLE_FAILSAFE_TOGGLE_DELAY_MS + (SCR_VoiceoverSubtitles.GetLingerDuration() * 1000), false, eventName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FailsafeSubtitleToggle(string eventName)
	{
		if (!m_SubtitlesDisplay || !m_SubtitlesDisplay.IsShown() || eventName != m_sCurrentSubtitleEvent)
			return;
		
		m_SubtitlesDisplay.Show(false);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetDefaultActor()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return null;

		return pc.GetControlledEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFinished(string eventName = string.Empty, bool hideSubtitles = true)
	{
		m_iAudioHandle = AudioHandle.Invalid;
		m_fCustomPause = -1;

		if (m_OnFinished)
			m_OnFinished.Invoke();
		
		if (m_OnFinishedEvent)
			m_OnFinishedEvent.Invoke(eventName);

		if (!m_SubtitlesDisplay || !hideSubtitles)
			return;

		// Hide the subtitle after desired delay
		GetGame().GetCallqueue().Remove(m_SubtitlesDisplay.Show);
		GetGame().GetCallqueue().CallLater(m_SubtitlesDisplay.Show, SCR_VoiceoverSubtitles.GetLingerDuration() * 1000, false, false, UIConstants.FADE_RATE_INSTANT, EAnimationCurve.LINEAR);
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_VoiceoverSystemSerializer)]
	protected SCR_VoiceoverSystemSnapshot CreateSnapshot()
	{
		SCR_VoiceoverSystemSnapshot snapshot = new SCR_VoiceoverSystemSnapshot();
		snapshot.m_sData = m_sDataResourceName;

		foreach (SCR_VoiceoverLine line : m_aQueue)
		{
			snapshot.m_aQueue.Insert(line);
		}

		snapshot.m_aActors.Copy(m_aActors);
		snapshot.m_mActorNames.Copy(m_mActorNames);
		snapshot.m_sCurrentSoundEvent = m_sCurrentSoundEvent;
		snapshot.m_CurrentActor = m_CurrentActor;
		snapshot.m_sCurrentActorName = m_sCurrentActorName;
		snapshot.m_sCurrentSubtitle = m_sCurrentSubtitle;
		snapshot.m_sCurrentSequence = m_sCurrentSequence;
		return snapshot;
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_VoiceoverSystemSerializer)]
	protected void ApplySnapshot(notnull SCR_VoiceoverSystemSnapshot snapshot)
	{
		SetData(snapshot.m_sData);
		Stop();
		PlayCustomLine(snapshot.m_sCurrentSoundEvent, snapshot.m_sCurrentSubtitle, snapshot.m_CurrentActor, snapshot.m_sCurrentActorName, true);

		m_aQueue.Clear();

		foreach (SCR_VoiceoverLine line : snapshot.m_aQueue)
		{
			m_aQueue.Insert(line);
		}

		m_aActors.Copy(snapshot.m_aActors);
		m_mActorNames.Copy(snapshot.m_mActorNames);
		m_sCurrentSequence = snapshot.m_sCurrentSequence;

		if (!m_aQueue.IsEmpty())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_VoiceoverSystem()
	{
		// Unregister previous actor
		if (m_PlayingSoundComponent && m_PlayingSoundComponent.GetOwner())
			RemoveActorUnconsciousCheck(m_PlayingSoundComponent.GetOwner());
	}
}

class SCR_VoiceoverSystemSnapshot
{
	ResourceName m_sData;
	ref array<ref SCR_VoiceoverLine> m_aQueue = {};
	ref array<IEntity> m_aActors = {};
	ref map<SCR_EVoiceoverActor, string> m_mActorNames = new map<SCR_EVoiceoverActor, string>();
	string m_sCurrentSoundEvent;
	IEntity m_CurrentActor;
	string m_sCurrentActorName;
	string m_sCurrentSubtitle;
	string m_sCurrentSequence;
}

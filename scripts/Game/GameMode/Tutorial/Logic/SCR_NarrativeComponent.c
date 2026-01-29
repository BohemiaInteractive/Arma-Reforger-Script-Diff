class SCR_NarrativeComponentClass : ScriptComponentClass
{
}

class SCR_NarrativeComponent : ScriptComponent
{
	[Attribute("Initial gesture")]
	protected int m_iCharacterID;

	[Attribute("false")]
	protected bool m_bDeathAnimation;

	[Attribute("false")]
	protected bool m_bRootMotion;
	
	[Attribute("")]
	protected string m_sPositionToEntity;
	
	[Attribute("false")]
	protected bool m_bDisablePhysics;

	[Attribute("0", params: "0 6")]
	protected int m_iDefaultEmotion;

	[Attribute("true")]
	protected bool m_bProximityLook;

	[Attribute("5", params: "1 100")]
	protected float m_fProximityLookRadius;

	[Attribute("1.1", params: "0.01 3.1")]
	protected float m_fProximityLookWidth;

	[Attribute("false")]
	protected bool m_bProximityTalk;

	[Attribute("5", params: "1 100")]
	protected float m_fProximityTalkRadius;

	[Attribute("false")]
	protected bool m_bProximityTalkOnlyOnce;

	[Attribute("1", params: "0 100")]
	protected int m_iProximityTalkDefaultStage;

	[Attribute("1", params: "1 10")]
	protected int m_iTalkVariants;

	[Attribute("false")]
	protected bool m_bRandomTalkVariant;

	[Attribute("false")]
	protected bool m_bStopTalkWhenOutOfProximity;

	protected bool m_bStoppingTalk;
	protected bool m_bProximityTalkEnabled = true;

	protected int m_iPermanentNarrativeStage;
	protected int m_iActualTalkVariant = 1;
	protected int m_iPlayerBoneId;

	protected float m_fLookAtIntensity;
	protected float m_fDistanceToPlayer;

	protected vector m_vLook[4];
	protected vector m_vLookEnd[4];
	protected vector m_vLookAxis;
	protected vector m_vPlayerHead[4];
	protected vector m_vPlayerPosition[4];
	protected vector m_vPlayerHeadPosition;

	protected IEntity m_EntityInHand;
	protected IEntity m_Player;
	protected IEntity entityToAttachTo;

	protected EntitySlotInfo m_PointOfViewStart;
	protected EntitySlotInfo m_PointOfViewEnd;

	protected TAnimGraphVariable m_LookAtVariable;
	protected TAnimGraphVariable m_EmotionId;

	protected CharacterAnimationComponent m_CharacterAnimation;

	protected SCR_CommunicationSoundComponent m_CommSoundComp;

	protected SlotManagerComponent m_SlotManager;

	protected SCR_CharacterDamageManagerComponent m_DamageManager;

	protected SCR_CharacterControllerComponent m_CharController;

	//------------------------------------------------------------------------------------------------
	void PlayAnimation(bool in = false, string command = "CMD_NarrativeStage", int animId = 0)
	{
		if (!m_CharacterAnimation)
			return;

		TAnimGraphCommand commandId = m_CharacterAnimation.BindCommand(command);

		if (m_bRootMotion && m_CharController && animId > -1)
		{
			SCR_LoiterCustomAnimData customAnimData = new SCR_LoiterCustomAnimData();
			customAnimData.m_CustomCommand = commandId;

			SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(m_CharacterAnimation.GetCommandHandler());
			scrCmdHandler.StartCommand_Move();
			
			vector targetPosition[4] = { "1 0 0", "0 1 0", "0 0 1", "0 0 0" };
			
			bool alignToPosition = false;
			
			if (m_sPositionToEntity != "")
			{
				entityToAttachTo = GetGame().GetWorld().FindEntityByName(m_sPositionToEntity);
				if (entityToAttachTo)
				{
					entityToAttachTo.GetTransform(targetPosition);
					alignToPosition = true;
				}
			}

			m_CharController.StartLoitering(animId, false, true, alignToPosition, targetPosition, false, customAnimData);
		}
		else
		{
			if (in)
				m_CharacterAnimation.CallCommand4I(commandId, 0, animId, 0, 0, 0);
			else
				m_CharacterAnimation.CallCommand(commandId, animId, 0);
		}

		if (m_iActualTalkVariant < m_iTalkVariants)
			m_iActualTalkVariant++;
		else
			m_iActualTalkVariant = 1;

		if (m_bRandomTalkVariant)
			m_iActualTalkVariant = Math.RandomInt(1, m_iTalkVariants + 1);

		TAnimGraphVariable variableId = m_CharacterAnimation.BindVariableInt("TalkVariant");

		if (variableId > 0)
			m_CharacterAnimation.SetVariableInt(variableId, m_iActualTalkVariant);

		if (m_EmotionId)
			m_CharacterAnimation.SetVariableInt(m_EmotionId, m_iDefaultEmotion);
	}

	//------------------------------------------------------------------------------------------------
	void SetPermanentNarrativeStage(int stage)
	{
		m_iPermanentNarrativeStage = stage;
		m_bProximityTalkEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		m_Player = EntityUtils.GetPlayer();

		if (!m_Player)
			return;

		m_Player.GetTransform(m_vPlayerPosition);
		m_fDistanceToPlayer = vector.Distance(m_vPlayerPosition[3], owner.GetOrigin());

		if (m_fDistanceToPlayer > m_fProximityTalkRadius) // NPC is too far to speak...
		{
			if (m_fDistanceToPlayer < m_fProximityLookRadius) // ... but can see the player
				LookAtPlayer(owner, timeSlice);
			else if (m_fDistanceToPlayer < 50) // NPC is too far to speak and look
				LookAtPlayer(owner, timeSlice, true);

			if (m_bStopTalkWhenOutOfProximity && !m_bStoppingTalk && IsActualCourseFreeRoam()) //NPC need tho shut-up when player goes away ONLY IN FREE_ROAM
			{
				PlayAnimation(true, "CMD_NarrativeStage", -1); // Shut-up BUT only for FREE_ROAM
				m_bStoppingTalk = true;
			}

			if (!m_bProximityTalkOnlyOnce) // When NPC too far, we enable possibility to talk again
				m_bProximityTalkEnabled = true;
		}
		else if (LookAtPlayer(owner, timeSlice)) // NPC is near to speak and see the player
		{
			m_bStoppingTalk = false;

			if (m_bProximityTalk && m_bProximityTalkEnabled) // Proximity talk is ON, NPC din´t talk yet
			{
				if (!m_bProximityTalkOnlyOnce) // NPC should talk whenever see player
				{
					PlayAnimation(true, "CMD_NarrativeStage", m_iPermanentNarrativeStage);
					m_bProximityTalkEnabled = false;
				}
				else if (IsActualCourseFreeRoam()) // NPC has just specific line need to be said only once ... and just in FREE_ROAM
				{
					PlayAnimation(true, "CMD_NarrativeStage", m_iPermanentNarrativeStage);
					m_bProximityTalkEnabled = false;
				}
			}
		}
		else if (m_bStopTalkWhenOutOfProximity && !m_bStoppingTalk && IsActualCourseFreeRoam()) // Player is near but at NPC's back
		{
			PlayAnimation(true, "CMD_NarrativeStage", -1); // Shut-up BUT only for FREE_ROAM
			m_bStoppingTalk = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	bool LookAtPlayer(IEntity owner, float timeSlice, bool lookAway = false)
	{
		m_fLookAtIntensity = Math.Clamp(m_fLookAtIntensity, 0.01, 0.99);

		if (m_LookAtVariable)
			m_CharacterAnimation.SetVariableFloat(m_LookAtVariable, 1 / (1 + Math.Pow(m_fLookAtIntensity / (1 - m_fLookAtIntensity), -2)));

		if (lookAway)
		{
			m_fLookAtIntensity -= timeSlice * 1.3;
			return false;
		}

		// Compute lookAt region origin
		if (m_PointOfViewStart)
			m_PointOfViewStart.GetWorldTransform(m_vLook);

		if (m_PointOfViewEnd)
			m_PointOfViewEnd.GetWorldTransform(m_vLookEnd);

		m_vLookAxis = vector.Direction(m_vLook[3], m_vLookEnd[3]);
		m_vLookAxis.Normalize();

		if (Math3D.IntersectionSphereCone(m_vPlayerPosition[3], 1, m_vLook[3], m_vLookAxis, m_fProximityLookWidth))
		{
			Animation playerAnimation = m_Player.GetAnimation();
			m_iPlayerBoneId = playerAnimation.GetBoneIndex("Head");
			playerAnimation.GetBoneMatrix(m_iPlayerBoneId, m_vPlayerHead);

			Math3D.MatrixMultiply4(m_vPlayerPosition, m_vPlayerHead, m_vPlayerHead);

			m_vPlayerHeadPosition = GetOwner().CoordToLocal(m_vPlayerHead[3]);
			m_CharacterAnimation.SetIKTarget("HeadLook", "HeadLook", m_vPlayerHeadPosition, {0, 0, 0});

			m_fLookAtIntensity += timeSlice * 1.3;
			return true;
		}
		else
		{
			m_fLookAtIntensity -= timeSlice * 1.3;
			return false;
		}

		//DebugTextWorldSpace.Create(GetOwner().GetWorld(), "X", DebugTextFlags.ONCE, m_vPlayerHead[3][0], m_vPlayerHead[3][1], m_vPlayerHead[3][2]);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		Initialize(owner);
		
		m_iPermanentNarrativeStage = m_iProximityTalkDefaultStage;
	}
	
	//------------------------------------------------------------------------------------------------
	void Initialize(IEntity owner)
	{
		
		if (m_bDisablePhysics)
			SCR_PhysicsHelper.ChangeSimulationState(owner, SimulationState.NONE, false);

		if (m_bProximityLook)
			SetEventMask(owner, EntityEvent.FIXEDFRAME);

		m_SlotManager = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));

		if (m_SlotManager)
		{
			m_PointOfViewStart = m_SlotManager.GetSlotByName("PointOfViewStart");
			m_PointOfViewEnd = m_SlotManager.GetSlotByName("PointOfViewEnd");
		}

		m_CharacterAnimation = CharacterAnimationComponent.Cast(owner.FindComponent(CharacterAnimationComponent));

		if (m_CharacterAnimation)
		{
			m_LookAtVariable = m_CharacterAnimation.BindVariableFloat("NarrativeLookAtIntensity");
			m_EmotionId = m_CharacterAnimation.BindVariableInt("emotion");
		}

		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));

		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamage().Remove(OnDamage);
			m_DamageManager.GetOnDamage().Insert(OnDamage);
		}

		m_CharController = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));

		if (m_CharController)
		{
			m_CharController.GetOnAnimationEvent().Remove(OnAnimationEvent);
			m_CharController.GetOnAnimationEvent().Insert(OnAnimationEvent);
		}

		m_CommSoundComp = SCR_CommunicationSoundComponent.Cast(owner.FindComponent(SCR_CommunicationSoundComponent));
		
		if (m_CommSoundComp)
			m_CommSoundComp.TerminateAll();

		// Play initial animation
		GetGame().GetCallqueue().CallLater(PlayAnimation, 1000, false, true, "CMD_Narrative", m_iCharacterID);
	}

	//------------------------------------------------------------------------------------------------
	bool IsActualCourseFreeRoam()
	{
		SCR_TutorialGamemodeComponent tutorialGamemodeComponent = SCR_TutorialGamemodeComponent.GetInstance();

		if (!tutorialGamemodeComponent)
			return true;

		SCR_ETutorialCourses actualCourse = tutorialGamemodeComponent.GetActiveConfig().GetCourseType();

		return actualCourse == SCR_ETutorialCourses.FREE_ROAM || actualCourse == SCR_ETutorialCourses.OUTRO || actualCourse == SCR_ETutorialCourses.INTRO;
	}

	//------------------------------------------------------------------------------------------------
	void OnDamage()
	{
		array<EntitySlotInfo> npcSlotInfos = {};

		if (m_SlotManager)
		{
			m_SlotManager.GetSlotInfos(npcSlotInfos);

			foreach (EntitySlotInfo slotInfo : npcSlotInfos)
			{
				if (slotInfo.GetAttachedEntity())
					slotInfo.DetachEntity(false);
			}
		}

		if (m_bDeathAnimation)
		{
			PlayAnimation(true, "CMD_Narrative", -1);
		}
		else
		{
			CharacterControllerComponent controller = CharacterControllerComponent.Cast(GetOwner().FindComponent(CharacterControllerComponent));

			if (controller)
				controller.ForceDeath();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		string eventName = GameAnimationUtils.GetEventString(animEventType);

		if (eventName == "TutorialVoice")
		{
			if (m_CommSoundComp)
				m_CommSoundComp.SoundEventPriority(GameAnimationUtils.GetEventString(animUserString), intParam, true);
		}
		else if (eventName == "PropDetach")
		{
			BaseSlotComponent baseSlot = BaseSlotComponent.Cast(GetOwner().FindComponent(BaseSlotComponent));

			if (baseSlot)
			{
				EntitySlotInfo slotInfo = baseSlot.GetSlotInfo();

				if (slotInfo)
				{
					m_EntityInHand = slotInfo.GetAttachedEntity();
					slotInfo.DetachEntity(true);
				}
			}
		}
		else if (eventName == "PropAttach")
		{
			BaseSlotComponent baseSlot = BaseSlotComponent.Cast(GetOwner().FindComponent(BaseSlotComponent));

			if (baseSlot)
			{
				EntitySlotInfo telephoneSlot = baseSlot.GetSlotInfo();

				if (telephoneSlot && m_EntityInHand)
					telephoneSlot.AttachEntity(m_EntityInHand);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_VoiceoverData
{
	[Attribute()]
	protected ref array<ref SCR_VoiceoverLineStandalone> m_aLines;

	[Attribute()]
	protected ref array<ref SCR_VoiceoverSequence> m_aSequences;

	//------------------------------------------------------------------------------------------------
	SCR_VoiceoverLineStandalone GetLineByName(string name)
	{
		foreach (SCR_VoiceoverLineStandalone line : m_aLines)
		{
			if (line.GetName() == name)
				return line;
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VoiceoverLineStandalone GetLineBySoundEvent(string eventName)
	{
		foreach (SCR_VoiceoverLineStandalone line : m_aLines)
		{
			if (line.GetSoundEventName() == eventName)
				return line;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_VoiceoverLine GetLineBySoundEvent(string eventName, string sequenceName)
	{
		SCR_VoiceoverSequence sequence = GetSequenceByName(sequenceName);

		if (!sequence)
			return null;

		array<ref SCR_VoiceoverLine> lines = {};
		sequence.GetLines(lines);

		foreach (SCR_VoiceoverLine line : lines)
		{
			if (line.GetSoundEventName() == eventName)
				return line;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_VoiceoverSequence GetSequenceByName(string name)
	{
		foreach (SCR_VoiceoverSequence sequence : m_aSequences)
		{
			if (sequence.GetName() == name)
				return sequence;
		}

		return null;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_VoiceoverSequence
{
	[Attribute("")]
	protected string m_sName;

	[Attribute()]
	protected ref array<ref SCR_VoiceoverLine> m_aLines;

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	void GetLines(out notnull array<ref SCR_VoiceoverLine> lines)
	{
		if (!m_aLines)
			return;
	
		foreach (SCR_VoiceoverLine line : m_aLines)
		{
			lines.Insert(line);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sSoundEventName")]
class SCR_VoiceoverLine
{
	[Attribute(SCR_EVoiceoverActor.ACTOR_1.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EVoiceoverActor))]
	protected SCR_EVoiceoverActor m_eActor;

	[Attribute("", desc: "Sound event name as defined in the appropriate .acp file in the actor's SCR_CommunicationSoundComponent.")]
	protected string m_sSoundEventName;

	[Attribute("")]
	protected string m_sSubtitleText;
	
	[Attribute("", desc: "Actor name to be shown in the subtitle.")]
	protected string m_sActorName;
	
	[Attribute("0", desc: "Non-zero value overrides the maximum distance from the voiceover source at which a subtitle will still be displayed.", params: "0 inf 1")]
	protected int m_iCustomSubtitleDistanceThreshold;

	[Attribute("0", desc: "Non-zero value executes an animation command with this parameter upon event start.")]
	protected int m_iStartAnimCommandParameter;

	[Attribute("0", desc: "Non-zero value executes an animation command with this parameter upon event finish.")]
	protected int m_iFinishAnimCommandParameter;
	
	[Attribute("1", desc: "Interrupt the currently played line when this line starts playing.")]
	protected bool m_bInterruptCurrentLine;
	
	[Attribute("-1", desc: "Non-negative value overrides the default pause between lines.")]
	protected float m_fPauseBeforeLine;

	//------------------------------------------------------------------------------------------------
	SCR_EVoiceoverActor GetActor()
	{
		return m_eActor;
	}

	//------------------------------------------------------------------------------------------------
	string GetSoundEventName()
	{
		return m_sSoundEventName;
	}

	//------------------------------------------------------------------------------------------------
	string GetSubtitleText()
	{
		return m_sSubtitleText;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActorName()
	{
		return m_sActorName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCustomSubtitleDistanceThreshold()
	{
		return m_iCustomSubtitleDistanceThreshold;
	}

	//------------------------------------------------------------------------------------------------
	int GetStartAnimCommandParameter()
	{
		return m_iStartAnimCommandParameter;
	}

	//------------------------------------------------------------------------------------------------
	int GetFinishAnimCommandParameter()
	{
		return m_iFinishAnimCommandParameter;
	}

	//------------------------------------------------------------------------------------------------
	bool InterruptCurrentLine()
	{
		return m_bInterruptCurrentLine;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPauseBeforeLine()
	{
		return m_fPauseBeforeLine;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_VoiceoverLineStandalone : SCR_VoiceoverLine
{
	[Attribute("")]
	protected string m_sName;

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
}

enum SCR_EVoiceoverActor
{
	ACTOR_1,
	ACTOR_2,
	ACTOR_3,
	ACTOR_4,
	ACTOR_5,
	ACTOR_6,
	ACTOR_7,
	ACTOR_8,
	ACTOR_9,
	ACTOR_10,
	ACTOR_11,
	ACTOR_12,
	ACTOR_13,
	ACTOR_14,
	ACTOR_15,
	ACTOR_16,
	ACTOR_17,
	ACTOR_18,
	ACTOR_19,
	ACTOR_20
}

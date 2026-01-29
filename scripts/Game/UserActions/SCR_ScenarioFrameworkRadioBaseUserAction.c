class SCR_ScenarioFrameworkRadioBaseUserAction : SCR_RadioBaseUserAction
{
	[Attribute(desc: "Fill the conditions and it's values to be avaluated with this user action. If the condition isn't met, the user action will not be shown.")]
	protected ref array<ref SCR_ScenarioFrameworkUserActionConditionData> m_aCustomConditionsList;

	[Attribute("0", UIWidgets.ComboBox, desc: "How to evaluate set conditions.", enums: ParamEnumArray.FromEnum(SCR_EEvaluationType))]
	protected SCR_EEvaluationType m_eEvaluationType;

	[Attribute("", UIWidgets.Auto)]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	protected SCR_ScenarioFrameworkSystem m_ScenarioFrameworkSystem;

	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_ScenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		super.Init(pOwnerEntity, pManagerComponent);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(pUserEntity.GetWorld());
		if (!soundManager)
			return;

		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;

		SCR_AudioSource audioSource = soundManager.CreateAudioSource(GetOwner(), m_AudioSourceConfiguration);
		if (!audioSource)
			return;

		soundManager.PlayAudioSource(audioSource);

		m_AudioHandle = audioSource.m_AudioHandle;

		super.OnActionStart(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(pUserEntity.GetWorld());
		if (!soundManager)
			return;

		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;

		soundManager.TerminateAudioSource(pOwnerEntity);

		super.OnActionCanceled(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluate all set conditions for this user action.
	override protected bool CustomRadioCondition()
	{
		if (!m_ScenarioFrameworkSystem)
			return false;

		string value;
		foreach (SCR_ScenarioFrameworkUserActionConditionData condition : m_aCustomConditionsList)
		{
			if (!m_ScenarioFrameworkSystem.GetVariable(condition.GetVariableName(), value))
			{
				if (m_eEvaluationType == SCR_EEvaluationType.AND)
					return false;
				else
					continue;
			}

			if (value == condition.GetVariableValue())
			{
				if (m_eEvaluationType == SCR_EEvaluationType.AND)
					continue;
				else
					return true;
			}
			else
			{
				if (m_eEvaluationType == SCR_EEvaluationType.AND)
					return false;
				else
					continue;
			}

		}

		return true;
	}
}


enum SCR_EEvaluationType
{
	AND = 0,
	OR = 1
}

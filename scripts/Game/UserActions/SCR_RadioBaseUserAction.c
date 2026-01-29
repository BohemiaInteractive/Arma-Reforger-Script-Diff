class SCR_RadioBaseUserAction : SCR_ScriptedUserAction
{
	[Attribute(desc: "Frequency that should be tuned at the radio to allow communication. For an exampel with HQ. If wrong one is tuned at the radio, the user action will not be visible.")]
	int m_iTargetFrequency;

	protected BaseRadioComponent m_BaseRadioComponent;
	protected ref array<BaseTransceiver> m_aTsvList = {};

	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_BaseRadioComponent = BaseRadioComponent.Cast(pOwnerEntity.FindComponent(BaseRadioComponent));
		if (!m_BaseRadioComponent)
			return;

		int count = m_BaseRadioComponent.TransceiversCount();

		for (int i = 0; i < count; i++)
		{
			m_aTsvList.Insert(m_BaseRadioComponent.GetTransceiver(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	// The user action is shown when radio is turned on, the correct frequency is set and custom condition is true.
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_BaseRadioComponent || m_aTsvList.IsEmpty())
			return false;

		if (!m_BaseRadioComponent.IsPowered())
			return false;

		bool frequencyMatch;
		foreach (BaseTransceiver tsv : m_aTsvList)
		{
			if (tsv.GetFrequency() == m_iTargetFrequency)
			{
				frequencyMatch = true;
				break;
			}
		}

		if (!frequencyMatch)
			return false;

		return CustomRadioCondition();
	}

	//------------------------------------------------------------------------------------------------
	// To be overridden in inherited classes. Allow set a custom condition when the action could be shown.
	protected bool CustomRadioCondition()
	{
		return true;
	}
}

class SCR_RepairConfig : ScriptAndConfig
{
	[Attribute(desc: "Configuration containing health above which system will try to apply the provided state when the vehicle is repaired", category: "Repairs")]
	protected ref array<ref SCR_HealthAndBurn> m_aSmokeRemovalHealthPercentage;

	//------------------------------------------------------------------------------------------------
	//! Retrives burn state for provided health
	//! \param health percenatage of health (0.0 to 1.0)
	//! \param[out] state output that will contain state that coresponds to the provided health or SCR_EBurningState.NONE if there is no such state
	//! \return true if coresponding state was found
	bool GetBurnStateForHealth(float health, out SCR_EBurningState state)
	{
		SCR_HealthAndBurn chosenConfig;
		foreach (SCR_HealthAndBurn config : m_aSmokeRemovalHealthPercentage)
		{
			if (!chosenConfig && config.GetPercentage() <= health)
			{
				chosenConfig = config;
				continue;
			}

			if (chosenConfig && config.GetPercentage() > chosenConfig.GetPercentage())
				chosenConfig = config;
		}

		if (!chosenConfig)
		{
			state = SCR_EBurningState.NONE;
			return false;
		}

		state = chosenConfig.GetState();
		return true;
	}
}

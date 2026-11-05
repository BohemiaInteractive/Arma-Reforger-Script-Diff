[BaseContainerProps(visible: false)]
class SCR_AIAnimation_Recovery : SCR_AILoiterAnimation_Base
{
	//! Number of recovery types encoded in VehicleRecovery (0 push, 1 pull)
	protected const int RECOVERY_TYPE_COUNT = 2;
	//! Number of animation variants per recovery type
	protected const int RECOVERY_VARIANT_COUNT = 2;

	//------------------------------------------------------------------------------------------------
	//! Picks the animation variant on the owner and encodes it as VehicleRecovery = type + RECOVERY_TYPE_COUNT * variant.
	//! The value travels with the loiter start data, so the server and all clients play the same variant.
	override protected void PrepareStartAnimationData(notnull SCR_LoiterCustomAnimData startAnimationData)
	{
		// Only a plain recovery type gets a random variant. Negative values (inactive, stops) and values that already
		// select a variant are kept as configured.
		int recoveryType = startAnimationData.m_iControlVariableValue;
		if (recoveryType < 0 || recoveryType >= RECOVERY_TYPE_COUNT)
			return;

		startAnimationData.m_iControlVariableValue = recoveryType + RECOVERY_TYPE_COUNT * Math.RandomInt(0, RECOVERY_VARIANT_COUNT);
	}
}

[BaseContainerProps()]
class SCR_AIAnimation_Recovery_Pull : SCR_AIAnimation_Recovery
{
	//------------------------------------------------------------------------------------------------
	void SCR_AIAnimation_Recovery_Pull()
	{
		m_sConfigPath = "{30770F5B3A4F8039}Configs/AI/SCR_AIAnimation_Recovery_Pull.conf";
	}
}

[BaseContainerProps()]
class SCR_AIAnimation_Recovery_Push : SCR_AIAnimation_Recovery
{
	//------------------------------------------------------------------------------------------------
	void SCR_AIAnimation_Recovery_Push()
	{
		m_sConfigPath = "{A0EB816CDEF44266}Configs/AI/SCR_AIAnimation_Recovery_Push.conf";
	}
}
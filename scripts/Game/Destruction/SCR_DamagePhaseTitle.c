class SCR_DamagePhaseTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		float health = 0;
		source.Get("m_fPhaseHealth", health);
		title = "Phase | HP: " + health.ToString();
		return true;
	}
}
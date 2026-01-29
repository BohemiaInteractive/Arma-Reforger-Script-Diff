class SCR_MissionHeaderCampaign : SCR_MissionHeader
{
	[Attribute("-1", UIWidgets.EditBox, "How many control points are required to win (override, -1 for default)")]
	int m_iControlPointsCap;
	
	[Attribute("-1", UIWidgets.EditBox, "How long a faction needs to hold the control points, in seconds (override, -1 for default)")]
	float m_fVictoryTimeout;

	[Attribute("-1", UIWidgets.EditBox, "How much supplies should the main HQ start with (override, -1 for default)")]
	int m_iStartingHQSupplies;
	
	[Attribute("-1", UIWidgets.EditBox, "Minimum starting amount of supplies in small bases (override, -1 for default)")]
	int m_iMinimumBaseSupplies;
	
	[Attribute("-1", UIWidgets.EditBox, "Maximum starting amount of supplies in small bases (override, -1 for default")]
	int m_iMaximumBaseSupplies;
	
	[Attribute("0", UIWidgets.CheckBox, "Use custom base list whitelist instead of blacklist")]
	bool m_bCustomBaseWhitelist;
	
	[Attribute("0", UIWidgets.CheckBox, "Disable rank requirements for spawning vehicles ")]
	bool m_bIgnoreMinimumVehicleRank;
	
	[Attribute("-1", desc: "Fraction of XP awarded to players unloading supplies which they have not loaded themselves (override, -1 for default)", params: "-1 inf")]
	float m_fSupplyOffloadAssistanceReward;
	
	[Attribute()]
	ref array<ref SCR_CampaignCustomBase> m_aCampaignCustomBaseList;

	[Attribute("0", desc: "Player can volunteer for Commander role")]
	bool m_bCommanderRoleEnabled;

	[Attribute("0", desc: "Players can establish bases. If disabled, the game starts with existing FOBs")]
	bool m_bEstablishingBasesEnabled;

	[Attribute("1", desc: "When enabled, FOBs automatically regenerate supplies")]
	bool m_bSuppliesAutoRegenerationEnabled;

	[Attribute(SCR_ECharacterRank.PRIVATE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "All players will start the match at this rank.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	SCR_ECharacterRank m_eStartingRank;
}

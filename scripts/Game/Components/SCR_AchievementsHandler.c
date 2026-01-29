[EntityEditorProps(description: "Achievements activator on client-side")]
class SCR_AchievementsHandlerClass : ScriptComponentClass
{
}

class SCR_AchievementsHandler : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] achievement
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void UnlockOnClient(AchievementId achievement)
	{
		Print("Unlocking achievement with ID " + achievement, LogLevel.DEBUG);
		Achievements.UnlockAchievement(achievement);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] achievement
	void UnlockAchievement(AchievementId achievement)
	{
		Rpc(UnlockOnClient, achievement);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] achievementStat
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void IncrementOnClient(AchievementStatId achievementStat)
	{
		Print("Incrementing achievement stat with ID " + achievementStat, LogLevel.DEBUG);
		Achievements.IncrementAchievementProgress(achievementStat, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] achievementStat
	void IncrementAchievementProgress(AchievementStatId achievementStat)
	{
		Rpc(IncrementOnClient, achievementStat);
	}
}

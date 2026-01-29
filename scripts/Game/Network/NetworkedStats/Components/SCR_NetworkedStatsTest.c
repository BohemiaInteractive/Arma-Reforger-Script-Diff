[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_NetworkedStatsTestComponentClass : ScriptComponentClass
{
}

class SCR_NetworkedStatsTestComponent : ScriptComponent
{
	SCR_NetworkedStatsComponent statsComponent;

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		statsComponent = SCR_NetworkedStatsComponent.Cast(owner.FindComponent(SCR_NetworkedStatsComponent));
		statsComponent.GetNetworkedStatPrefabData().GetOnStatsChanged(true).Insert(OnStatUpdate);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] stat
	void OnStatUpdate(SCR_BaseNetworkedStat stat)
	{
		SCR_FPSNetworkedStat fpsStat = SCR_FPSNetworkedStat.Cast(stat);
		SCR_MemoryNetworkedStat memoryStat = SCR_MemoryNetworkedStat.Cast(stat);

		if (fpsStat)
		{
			Print("Local FPS: " + fpsStat.GetLastCheckedLocalFPS());
			Print("Authority FPS: " + fpsStat.GetLastCheckedAuthorityFPS());
		}
		else if (memoryStat)
		{
			Print("Local Memory: " + memoryStat.GetLastCheckedLocalMemory());
			Print("Authority Memory: " + memoryStat.GetLastCheckedAuthorityMemory());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}

void SCR_OnStatChangedMethod(SCR_BaseNetworkedStat changedStat);
typedef func SCR_OnStatChangedMethod;
typedef ScriptInvokerBase<SCR_OnStatChangedMethod> SCR_OnStatChangedInvoker;

[ComponentEditorProps(category: "GameScripted/Network", description: "Stats Tracker between the Authority and the Proxy")]
class SCR_NetworkedStatsComponentClass : ScriptComponentClass
{
	protected ref SCR_OnStatChangedInvoker m_OnStatsChanged;

	//------------------------------------------------------------------------------------------------
	//! \param[in] createIfNull
	//! \return
	SCR_OnStatChangedInvoker GetOnStatsChanged(bool createIfNull = true)
	{
		if (!m_OnStatsChanged && createIfNull)
			m_OnStatsChanged = new SCR_OnStatChangedInvoker();

		return m_OnStatsChanged;
	}
}

class SCR_NetworkedStatsComponent : ScriptComponent
{
	[Attribute()]
	protected ref array<ref SCR_BaseNetworkedStat> m_aStats;
	protected RplComponent m_RplComponent;

	[Attribute("3", uiwidget: UIWidgets.EditBox, desc: "How many seconds should we wait before checking and updating the stats again?")]
	protected int m_iUpdateTimer;

	//------------------------------------------------------------------------------------------------
	//! \param[in] statType
	//! \return
	SCR_BaseNetworkedStat GetNetworkedStat(SCR_ENetworkedStatType statType)
	{
		foreach (SCR_BaseNetworkedStat stat : m_aStats)
		{
			if (stat.GetStatType() == statType)
				return stat;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_NetworkedStatsComponentClass GetNetworkedStatPrefabData()
	{
		return SCR_NetworkedStatsComponentClass.Cast(GetOwner().FindComponentData(SCR_NetworkedStatsComponentClass));
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	RplComponent GetRplComponent()
	{
		return m_RplComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] statType
	//! \param[in] valueInt
	void SetAuthorityStatValue(SCR_ENetworkedStatType statType, int valueInt)
	{
		if (m_RplComponent.IsProxy())
			return;

		Rpc(SetStatValueOfAuthority, statType, valueInt);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] statType
	//! \param[in] value
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void SetStatValueOfAuthority(SCR_ENetworkedStatType statType, int value)
	{
		SCR_BaseNetworkedStat networkedStat = GetNetworkedStat(statType);
		if (!networkedStat)
			return;

		networkedStat.OnAuthorityStatValueSet(value);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateStats()
	{
		foreach (SCR_BaseNetworkedStat stat : m_aStats)
		{
			stat.UpdateStat();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
			return;
#endif

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		// For continuously updating stats. As the tick intervals are huge, EOnUpdate is a serious overkill and not needed.
		GetGame().GetCallqueue().CallLater(UpdateStats, m_iUpdateTimer * 1000, true);

		foreach (SCR_BaseNetworkedStat networkedStat : m_aStats)
		{
			networkedStat.SetOwner(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(UpdateStats);
	}
}

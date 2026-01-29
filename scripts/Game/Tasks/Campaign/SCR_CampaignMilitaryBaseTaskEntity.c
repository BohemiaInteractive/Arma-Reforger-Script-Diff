void TaskMilitaryBaseCallsignInvokerDelegate(SCR_CampaignMilitaryBaseTaskEntity task, int callsign);
typedef func TaskMilitaryBaseCallsignInvokerDelegate;
typedef ScriptInvokerBase<TaskMilitaryBaseCallsignInvokerDelegate> SCR_TaskMilitaryBaseCallsignInvoker;

class SCR_CampaignMilitaryBaseTaskEntityClass : SCR_TaskClass
{
}

class SCR_CampaignMilitaryBaseTaskEntity : SCR_Task
{
	protected SCR_CampaignMilitaryBaseTaskData m_MilitaryBaseTaskData;
	protected SCR_CampaignMilitaryBaseComponent m_MilitaryBase;
	protected ref SCR_TaskMilitaryBaseCallsignInvoker m_OnMilitaryBaseCallsignAssigned;

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetMilitaryBase()
	{
		return m_MilitaryBase;
	}
	
	SCR_TaskMilitaryBaseCallsignInvoker GetOnMilitaryBaseCallsignAssigned()
	{
		if (!m_OnMilitaryBaseCallsignAssigned)
			m_OnMilitaryBaseCallsignAssigned = new SCR_TaskMilitaryBaseCallsignInvoker();

		return m_OnMilitaryBaseCallsignAssigned;
	}

	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		
		// Award XP
		if (comp && m_MilitaryBase && state == SCR_ETaskState.COMPLETED)
			AwardXP(comp);

		super.SetTaskState(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Give out XP to relevant players following task completion
	protected void AwardXP(notnull SCR_XPHandlerComponent handlerXP);

	//------------------------------------------------------------------------------------------------
	void SetMilitaryBaseCallSign(int callSign)
	{
		if (callSign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return;

		RpcDo_SetMilitaryBaseCallSign(callSign);
		Rpc(RpcDo_SetMilitaryBaseCallSign, callSign);

		if (!m_MilitaryBase)
			return;

		SCR_MilitaryBaseSystem baseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseSystem)
			return;

		baseSystem.GetOnBaseFactionChanged().Insert(OnTargetBaseCaptured);
		baseSystem.GetOnBaseUnregistered().Insert(OnBaseUnregistered);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetMilitaryBaseCallSign(int callSign)
	{
		if (!m_MilitaryBaseTaskData)
			return;

		m_MilitaryBaseTaskData.m_iBaseCallSign = callSign;
		
		AssignMilitaryBaseByCallsign(callSign);
	}
	
	//------------------------------------------------------------------------------------------------
	void AssignMilitaryBaseByCallsign(int callsign)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return;
		
		m_MilitaryBase = baseManager.FindBaseByCallsign(callsign);

		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();

		if (m_OnMilitaryBaseCallsignAssigned)
			m_OnMilitaryBaseCallsignAssigned.Invoke(this, callsign);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTargetBaseCaptured(SCR_MilitaryBaseComponent base, Faction faction)
	{
		if (!m_MilitaryBase || base != m_MilitaryBase || !faction || !GetOwnerFactionKeys().Contains(faction.GetFactionKey()))
			return;

		SetTaskState(SCR_ETaskState.COMPLETED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseUnregistered(SCR_MilitaryBaseComponent base)
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;

		// base is removed - cancel task
		SCR_MilitaryBaseComponent targetBase = GetMilitaryBase();
		if (!base || base != targetBase)
			return;

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.CANCELLED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteTask()
	{
		if (!m_TaskSystem)
			return;

		m_TaskSystem.DeleteTask(this);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;

		if (!m_MilitaryBaseTaskData)
			writer.WriteInt(SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN);
		else
			writer.WriteInt(m_MilitaryBaseTaskData.m_iBaseCallSign);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;

		if (!m_MilitaryBaseTaskData)
			m_MilitaryBaseTaskData = SCR_CampaignMilitaryBaseTaskData.Cast(m_TaskData);

		int baseCallSign;
		reader.ReadInt(baseCallSign);
		RpcDo_SetMilitaryBaseCallSign(baseCallSign);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void InitializeData()
	{
		if (!m_TaskData)
			m_TaskData = new SCR_CampaignMilitaryBaseTaskData();

		super.InitializeData();

		m_MilitaryBaseTaskData = SCR_CampaignMilitaryBaseTaskData.Cast(m_TaskData);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignMilitaryBaseTaskEntity()
	{
		SCR_MilitaryBaseSystem militaryBaseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (militaryBaseSystem)
			militaryBaseSystem.GetOnBaseFactionChanged().Remove(OnTargetBaseCaptured);

		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		if (baseManager)
			baseManager.GetOnBaseUnregistered().Remove(OnBaseUnregistered);
	}
}

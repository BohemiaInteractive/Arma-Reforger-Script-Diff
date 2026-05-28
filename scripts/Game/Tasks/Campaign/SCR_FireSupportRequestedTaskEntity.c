class SCR_FireSupportRequestedTaskEntityClass : SCR_BaseRequestedTaskEntityClass
{
}

class SCR_FireSupportRequestedTaskEntity : SCR_BaseRequestedTaskEntity
{
	[Attribute("150", UIWidgets.EditBox, "Area radius [m]", "1 inf")]
	protected float m_fAreaRadius;

	[Attribute("4", UIWidgets.EditBox, desc: "Number of explosion events in the area required to complete the task", "1 inf")]
	protected int m_iNeededExplosionEvents;

	[Attribute("", UIWidgets.ResourceAssignArray, "Allowed explosion ammo for task evaluation", "et")]
	protected ref array<ResourceName> m_aAllowedExplosionAmmo;

	protected RplComponent m_RplComponent;
	protected float m_fAreaRadiusSq;
	protected int m_iExplosionEvents;

	//------------------------------------------------------------------------------------------------
	protected void AddXPReward()
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (!comp)
			return;

		// add XP to all players in the assigned group
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		foreach (int playerID : assigneePlayerIDs)
		{
			comp.AwardXP(playerID, SCR_EXPRewards.FIRE_SUPPORT_TASK_COMPLETED, 1.0, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		if (state == SCR_ETaskState.COMPLETED)
			AddXPReward();

		super.SetTaskState(state);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsAllowedExplosionAmmo(IEntity damageSource)
	{
		if (!damageSource)
			return false;

		EntityPrefabData damageSourcePrefabData = damageSource.GetPrefabData();
		if (!damageSourcePrefabData)
			return false;

		ResourceName damageSourcePrefabName = damageSourcePrefabData.GetPrefabName();
		if (damageSourcePrefabName.IsEmpty())
			return false;

		return m_aAllowedExplosionAmmo.Contains(damageSourcePrefabName);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnExplosionAmmoEffect(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed)
	{
		int instigatorPlayerID = instigator.GetInstigatorPlayerID();
		if (instigatorPlayerID <= 0)
			return;

		if (!IsAllowedExplosionAmmo(damageSource))
			return;

		if (vector.DistanceSqXZ(outMat[0], GetTaskPosition()) > m_fAreaRadiusSq)
			return;

		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		if (!assigneePlayerIDs.Contains(instigatorPlayerID))
			return;

		m_iExplosionEvents++;

		EvaluateTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void EvaluateTask()
	{
		if (m_iExplosionEvents >= m_iNeededExplosionEvents)
		{
			SetTaskState(SCR_ETaskState.COMPLETED);
			DeleteTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		m_fAreaRadiusSq = m_fAreaRadius * m_fAreaRadius;

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		SCR_ExplosionAmmoEffect.GetOnExplosionAmmoEffect().Insert(OnExplosionAmmoEffect);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_FireSupportRequestedTaskEntity()
	{
		SCR_ExplosionAmmoEffect.GetOnExplosionAmmoEffect().Remove(OnExplosionAmmoEffect);
	}
}

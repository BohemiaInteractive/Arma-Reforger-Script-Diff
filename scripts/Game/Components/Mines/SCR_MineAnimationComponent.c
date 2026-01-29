[EntityEditorProps(category: "GameScripted/Mines", description: "Base component for handling animation.")]
class SCR_MineAnimationComponentClass : WeaponAnimationComponentClass
{
	protected static const string MINE_PLACED_EVENT_NAME = "MineDown";
	protected static const string MINE_ACTIVATED_EVENT_NAME = "MineActivated";
	protected static const string MINE_DISARMED_EVENT_NAME = "MineDisarmed";
	
	protected AnimationEventID m_iMineDown = -1;
	protected AnimationEventID m_iMineActivated = -1;
	protected AnimationEventID m_iMineDisarmed = -1;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] componentSource
	//! \param[in] parentSource
	//! \param[in] prefabSource
	void SCR_MineAnimationComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		m_iMineDown = GameAnimationUtils.RegisterAnimationEvent(MINE_PLACED_EVENT_NAME);
		m_iMineActivated = GameAnimationUtils.RegisterAnimationEvent(MINE_ACTIVATED_EVENT_NAME);
		m_iMineDisarmed = GameAnimationUtils.RegisterAnimationEvent(MINE_DISARMED_EVENT_NAME);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMineDownID()
	{
		return m_iMineDown;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMineActivatedID()
	{
		return m_iMineActivated;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMineDisarmedID()
	{
		return m_iMineDisarmed;
	}
}

class SCR_MineAnimationComponent : WeaponAnimationComponent
{
	protected IEntity m_Owner;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_MineAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = ent;
	}

	//------------------------------------------------------------------------------------------------
	protected override event void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);

		SCR_MineAnimationComponentClass prefabData = SCR_MineAnimationComponentClass.Cast(GetComponentData(GetOwner()));

		if (animEventType == prefabData.GetMineDownID())
		{
			SoundComponent soundComponent = SoundComponent.Cast(m_Owner.FindComponent(SoundComponent));
			if (!soundComponent)
				return;
			
			TraceParam param = new TraceParam();
			param.Start = m_Owner.GetOrigin() + vector.Up * 0.3;//compensate for slopes as during the animation object may be pushed under the surface
			param.End = m_Owner.GetOrigin() - vector.Up;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			array<IEntity> excluded = {m_Owner};
			if (m_Owner != m_Owner.GetRootParent())
				excluded.Insert(m_Owner.GetRootParent());

			param.ExcludeArray = excluded;
			param.LayerMask = EPhysicsLayerPresets.Projectile;
			m_Owner.GetWorld().TraceMove(param, null);
			
			GameMaterial material = param.SurfaceProps;
			if (!material)
				return;
			
			soundComponent.SetSignalValueStr("Surface", material.GetSoundInfo().GetSignalValue());
		}

		if (animEventType != prefabData.GetMineActivatedID() && animEventType != prefabData.GetMineDisarmedID())
			return;//it is cheaper to compare two ints than it is to call for a method to find a component

		// Master only
		RplComponent rplComponent = SCR_EntityHelper.GetEntityRplComponent(m_Owner);
		if (!rplComponent || rplComponent.IsProxy())
			return;

		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(m_Owner.FindComponent(SCR_PressureTriggerComponent));
		if (!pressureTriggerComponent)
			return;

		if (animEventType == prefabData.GetMineActivatedID())
		{
			pressureTriggerComponent.ActivateTrigger();
			return;
		}

		if (animEventType == prefabData.GetMineDisarmedID())
			pressureTriggerComponent.DisarmTrigger();
	}
}

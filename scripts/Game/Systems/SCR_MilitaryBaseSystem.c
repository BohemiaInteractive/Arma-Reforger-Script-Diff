void OnBaseFactionChangedDelegate(SCR_MilitaryBaseComponent base, Faction faction);
void OnLogicPresenceChangedDelegate(SCR_MilitaryBaseComponent base, SCR_MilitaryBaseLogicComponent logic);
void OnBaseRegisteredDelegate(SCR_MilitaryBaseComponent base);
void OnBaseUnregisteredDelegate(SCR_MilitaryBaseComponent base);

typedef func OnBaseFactionChangedDelegate;
typedef func OnLogicPresenceChangedDelegate;
typedef func OnBaseRegisteredDelegate;
typedef func OnBaseUnregisteredDelegate;

typedef ScriptInvokerBase<OnBaseFactionChangedDelegate> OnBaseFactionChangedInvoker;
typedef ScriptInvokerBase<OnLogicPresenceChangedDelegate> OnLogicPresenceChangedInvoker;
typedef ScriptInvokerBase<OnBaseRegisteredDelegate> OnBaseRegisteredInvoker;
typedef ScriptInvokerBase<OnBaseUnregisteredDelegate> OnBaseUnregisteredInvoker;

//------------------------------------------------------------------------------------------------
class SCR_MilitaryBaseSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	protected ref array<SCR_MilitaryBaseComponent> m_aBases = {};
	protected ref array<SCR_MilitaryBaseLogicComponent> m_aLogicComponents = {};
	protected ref array<int> m_aAvailableCallsignIds;

	protected ref OnBaseFactionChangedInvoker m_OnBaseFactionChanged;
	protected ref OnLogicPresenceChangedInvoker m_OnLogicRegisteredInBase;
	protected ref OnLogicPresenceChangedInvoker m_OnLogicUnregisteredInBase;
	protected ref OnBaseRegisteredInvoker m_OnBaseRegistered;
	protected ref OnBaseUnregisteredInvoker m_OnBaseUnregistered;

	//------------------------------------------------------------------------------------------------
	static SCR_MilitaryBaseSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return SCR_MilitaryBaseSystem.Cast(world.FindSystem(SCR_MilitaryBaseSystem));
	}

	//------------------------------------------------------------------------------------------------
	OnBaseRegisteredInvoker GetOnBaseRegistered()
	{
		if (!m_OnBaseRegistered)
			m_OnBaseRegistered = new OnBaseRegisteredInvoker();

		return m_OnBaseRegistered;
	}

	//------------------------------------------------------------------------------------------------
	OnBaseUnregisteredInvoker GetOnBaseUnregistered()
	{
		if (!m_OnBaseUnregistered)
			m_OnBaseUnregistered = new OnBaseUnregisteredInvoker();

		return m_OnBaseUnregistered;
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnCleanup()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));

		if (core)
			core.Event_OnEntityTransformChanged.Remove(OnEntityTransformChanged);
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseFactionChanged(Faction faction, SCR_MilitaryBaseComponent base)
	{
		if (m_OnBaseFactionChanged)
			m_OnBaseFactionChanged.Invoke(base, faction);
	}

	//------------------------------------------------------------------------------------------------
	void OnLogicRegisteredInBase(SCR_MilitaryBaseLogicComponent logic, SCR_MilitaryBaseComponent base)
	{
		if (m_OnLogicRegisteredInBase)
			m_OnLogicRegisteredInBase.Invoke(base, logic);
	}

	//------------------------------------------------------------------------------------------------
	void OnLogicUnregisteredInBase(SCR_MilitaryBaseLogicComponent logic, SCR_MilitaryBaseComponent base)
	{
		if (m_OnLogicUnregisteredInBase)
			m_OnLogicUnregisteredInBase.Invoke(base, logic);
	}

	//------------------------------------------------------------------------------------------------
	OnBaseFactionChangedInvoker GetOnBaseFactionChanged()
	{
		if (!m_OnBaseFactionChanged)
			m_OnBaseFactionChanged = new OnBaseFactionChangedInvoker();

		return m_OnBaseFactionChanged;
	}

	//------------------------------------------------------------------------------------------------
	OnLogicPresenceChangedInvoker GetOnLogicRegisteredInBase()
	{
		if (!m_OnLogicRegisteredInBase)
			m_OnLogicRegisteredInBase = new OnLogicPresenceChangedInvoker();

		return m_OnLogicRegisteredInBase;
	}

	//------------------------------------------------------------------------------------------------
	OnLogicPresenceChangedInvoker GetOnLogicUnregisteredInBase()
	{
		if (!m_OnLogicUnregisteredInBase)
			m_OnLogicUnregisteredInBase = new OnLogicPresenceChangedInvoker();

		return m_OnLogicUnregisteredInBase;
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));

		if (core)
			core.Event_OnEntityTransformChanged.Insert(OnEntityTransformChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! When a base or logic component is moved, recalculate registered members
	protected void OnEntityTransformChanged(notnull SCR_EditableEntityComponent editComponent)
	{
		array<IEntity> queue = {editComponent.GetOwner()};
		SCR_MilitaryBaseComponent baseComponent;
		SCR_MilitaryBaseLogicComponent logicComponent;
		IEntity processedEntity;
		IEntity nextInHierarchy;
		const bool checkeEditableEntityComponent; // TODO: check for good const usage

		while (!queue.IsEmpty())
		{
			processedEntity = queue[0];
			queue.Remove(0);
			
			if (!processedEntity)
				continue;

			if (!checkeEditableEntityComponent || !SCR_EditableEntityComponent.Cast(processedEntity.FindComponent(SCR_EditableEntityComponent)))
			{
				baseComponent = SCR_MilitaryBaseComponent.Cast(processedEntity.FindComponent(SCR_MilitaryBaseComponent));
				logicComponent = SCR_MilitaryBaseLogicComponent.Cast(processedEntity.FindComponent(SCR_MilitaryBaseLogicComponent));

				if (baseComponent)
					OnBaseTransformChanged(baseComponent);

				if (logicComponent)
					OnLogicTransformChanged(logicComponent);
			}

			nextInHierarchy = processedEntity.GetChildren();

			while (nextInHierarchy)
			{
				queue.Insert(nextInHierarchy);
				nextInHierarchy = nextInHierarchy.GetSibling();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Base has been moved, recalculate members in radius
	protected void OnBaseTransformChanged(notnull SCR_MilitaryBaseComponent baseComponent)
	{
		vector position = baseComponent.GetOwner().GetOrigin();
		int distanceLimit = baseComponent.GetRadius() * baseComponent.GetRadius();

		foreach (SCR_MilitaryBaseLogicComponent logicComponent : m_aLogicComponents)
		{
			if (vector.DistanceSqXZ(position, logicComponent.GetOwner().GetOrigin()) > distanceLimit)
			{
				baseComponent.UnregisterLogicComponent(logicComponent);
				logicComponent.UnregisterBase(baseComponent);
			}
			else
			{
				baseComponent.RegisterLogicComponent(logicComponent);
				logicComponent.RegisterBase(baseComponent);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Logic has been moved, recalculate members in radius of bases
	protected void OnLogicTransformChanged(notnull SCR_MilitaryBaseLogicComponent logicComponent)
	{
		vector position = logicComponent.GetOwner().GetOrigin();
		int radius;

		foreach (SCR_MilitaryBaseComponent baseComponent : m_aBases)
		{
			radius = baseComponent.GetRadius();

			if (vector.DistanceSqXZ(position, baseComponent.GetOwner().GetOrigin()) > (radius * radius))
			{
				baseComponent.UnregisterLogicComponent(logicComponent);
				logicComponent.UnregisterBase(baseComponent);
			}
			else
			{
				baseComponent.RegisterLogicComponent(logicComponent);
				logicComponent.RegisterBase(baseComponent);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void LoadAvailableCallsigns()
	{
		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return;

		m_aAvailableCallsignIds = {};
		array<Faction> allFactions = {};
		array<SCR_Faction> playableFactions = {};
		fManager.GetFactionsList(allFactions);
		array<int> callsignIndexesSource = {};

		foreach (Faction faction : allFactions)
		{
			SCR_Faction factionCast = SCR_Faction.Cast(faction);

			if (factionCast && factionCast.IsPlayable())
				playableFactions.Insert(factionCast);
		}

		if (playableFactions.IsEmpty())
			return;

		// All playable factions need to have a callsign with the same ID registered in order for it to be used
		// Grab the indexes available from the first valid faction in the list so we can compare it with the other factions
		callsignIndexesSource = playableFactions[0].GetBaseCallsignIndexes();
		bool canAdd;

		foreach (int indexSource : callsignIndexesSource)
		{
			canAdd = true;

			// Ignore the index unless it's registered in all other factions
			foreach (int i, SCR_Faction faction : playableFactions)
			{
				// No need to check it against the original source faction
				if (i == 0)
					continue;

				if (!faction.GetBaseCallsignByIndex(indexSource))
				{
					canAdd = false;
					break;
				}
			}

			if (!canAdd)
				continue;

			m_aAvailableCallsignIds.Insert(indexSource);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RegisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		m_aBases.Insert(base);

		vector basePosition = base.GetOwner().GetOrigin();
		int baseRadius = base.GetRadius();
		int distanceLimit = baseRadius * baseRadius;

		if (Replication.IsServer())
		{
			// If not done already, grab all available callsign IDs
			if (!m_aAvailableCallsignIds)
				LoadAvailableCallsigns();

			// Assign random available callsign
			if (m_aAvailableCallsignIds && !m_aAvailableCallsignIds.IsEmpty())
			{
				int i = m_aAvailableCallsignIds.GetRandomIndex();

				base.SetCallsignIndexAutomatic(m_aAvailableCallsignIds[i]);
				m_aAvailableCallsignIds.Remove(i);
			}
		}

		foreach (SCR_MilitaryBaseLogicComponent component : m_aLogicComponents)
		{
			if (vector.DistanceSqXZ(component.GetOwner().GetOrigin(), basePosition) <= distanceLimit)
			{
				base.RegisterLogicComponent(component);
				component.RegisterBase(base);
			}
		}

		if (m_OnBaseRegistered)
			m_OnBaseRegistered.Invoke(base);
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (!m_aBases.Contains(base))
			return;

		if (m_aAvailableCallsignIds)
			m_aAvailableCallsignIds.Insert(base.GetCallsign());

		m_aBases.RemoveItem(base);

		if (m_OnBaseUnregistered)
			m_OnBaseUnregistered.Invoke(base);
	}

	//------------------------------------------------------------------------------------------------
	int GetBases(notnull out array<SCR_MilitaryBaseComponent> bases)
	{
		return bases.Copy(m_aBases);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBasesCount()
	{
		return m_aBases.Count();
	}

	//------------------------------------------------------------------------------------------------
	void RegisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		m_aLogicComponents.Insert(component);

		vector position = component.GetOwner().GetOrigin();
		int radius;

		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			radius = base.GetRadius();

			if (vector.DistanceSqXZ(base.GetOwner().GetOrigin(), position) <= (radius * radius))
			{
				base.RegisterLogicComponent(component);
				component.RegisterBase(base);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		m_aLogicComponents.RemoveItem(component);
	}
}

[EntityEditorProps(category: "GameScripted/Rally", description: "Hides children outside a configured time window")]
class SCR_TimeWindowVisibilityManagerClass : GenericEntityClass
{
}

class SCR_TimeWindowVisibilityManager : GenericEntity
{
	[Attribute(defvalue: "6", uiwidget: UIWidgets.Slider, desc: "Hour when children become visible", params: "0 23 1")]
	protected int m_iVisibleFromHour;

	[Attribute(defvalue: "20", uiwidget: UIWidgets.Slider, desc: "Hour when children become hidden", params: "0 23 1")]
	protected int m_iVisibleToHour;

	protected TimeAndWeatherManagerEntity m_TimeWeatherManager;
	protected bool m_bCurrentlyVisible;
	protected ref map<IEntity, int> m_mOriginalLayers;

	static ref ScriptInvokerVoid s_OnGameTimeChanged = new ScriptInvokerVoid();

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		m_TimeWeatherManager = world.GetTimeAndWeatherManager();
		if (!m_TimeWeatherManager)
			return;

		m_mOriginalLayers = new map<IEntity, int>();
		CacheChildLayers();

		m_bCurrentlyVisible = IsInsideTimeWindow();
		if (!m_bCurrentlyVisible)
			HideChildren(true);

		SCR_TimeWindowVisibilityManager.s_OnGameTimeChanged.Insert(OnGameTimeChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void CacheChildLayers()
	{
		IEntity child = GetChildren();
		while (child)
		{
			Physics phys = child.GetPhysics();
			if (phys)
				m_mOriginalLayers.Set(child, phys.GetInteractionLayer());

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGameTimeChanged()
	{
		bool shouldBeVisible = IsInsideTimeWindow();
		if (shouldBeVisible == m_bCurrentlyVisible)
			return;

		m_bCurrentlyVisible = shouldBeVisible;
		HideChildren(!shouldBeVisible);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideChildren(bool hidden)
	{
		IEntity child = GetChildren();
		while (child)
		{
			Physics phys = child.GetPhysics();
			if (phys)
			{
				if (hidden)
					phys.SetInteractionLayer(0);
				else
					phys.SetInteractionLayer(m_mOriginalLayers.Get(child));
			}

			if (hidden)
				child.ClearFlags(EntityFlags.VISIBLE, true);
			else
				child.SetFlags(EntityFlags.VISIBLE, true);

			// light components emit independently of the VISIBLE flag
			SetLightEnabled(child, !hidden);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetLightEnabled(notnull IEntity entity, bool enabled)
	{
		// recurse into children since lamp prefabs nest their LightEntity one or more levels deep
		IEntity child = entity.GetChildren();
		while (child)
		{
			LightEntity light = LightEntity.Cast(child);
			if (light)
				light.SetEnabled(enabled);

			SetLightEnabled(child, enabled);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsInsideTimeWindow()
	{
		TimeContainer time = m_TimeWeatherManager.GetTime();
		int hour = time.m_iHours;

		if (m_iVisibleFromHour <= m_iVisibleToHour)
			return hour >= m_iVisibleFromHour && hour < m_iVisibleToHour;
		else
			return hour >= m_iVisibleFromHour || hour < m_iVisibleToHour;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_TimeWindowVisibilityManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		Activate();
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_TimeWindowVisibilityManager()
	{
		SCR_TimeWindowVisibilityManager.s_OnGameTimeChanged.Remove(OnGameTimeChanged);
	}
}

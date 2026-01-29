class SCR_DamageAreaComponentClass : DamageAreaComponentClass
{
}

class SCR_DamageAreaComponent : DamageAreaComponent
{
	protected ref array<ref DamageArea> m_aDamageAreas = {};

#ifdef ENABLE_DIAG
	protected ref SCR_DebugShapeManager m_DebugShapeMgr = new SCR_DebugShapeManager();

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_DebugShapeManager GetDebugShapeMgr()
	{
		return m_DebugShapeMgr;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		m_DebugShapeMgr.Clear();
		DrawMarker(owner);
		InitializeShapes(owner);
		DrawCustomShapes(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_SetTransform(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		m_DebugShapeMgr.Clear();
		DrawMarker(owner);
		DrawCustomShapes(owner);
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		m_DebugShapeMgr.Clear();
		DrawMarker(owner);
		DrawCustomShapes(owner);

		array<ref DamageArea> damageAreas = {};
		GetDamageAreas(damageAreas);

		SCR_BaseDamageAreaShape shape;
		bool somethingChanged;
		foreach (DamageArea area : damageAreas)
		{
			if (!area)
				continue;

			shape = SCR_BaseDamageAreaShape.Cast(area.GetShape());
			if (!shape)
				continue;

			somethingChanged = shape._WB_OnKeyChanged(owner, src, key, ownerContainers, parent) || somethingChanged;
		}

		return somethingChanged;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	protected void DrawCustomShapes(IEntity owner)
	{
		array<ref DamageArea> damageAreas = {};
		GetDamageAreas(damageAreas);

		SCR_BaseDamageAreaShape shape;
		foreach (DamageArea area : damageAreas)
		{
			if (!area)
				continue;

			shape = SCR_BaseDamageAreaShape.Cast(area.GetShape());
			if (!shape)
				continue;

			shape.DrawDebug(owner, m_DebugShapeMgr);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	protected void DrawMarker(IEntity owner)
	{
		vector origin = owner.GetOrigin();
		m_DebugShapeMgr.AddArrow(origin + vector.Up * 5, origin, 1);
		string prefab = GetPrefabname(owner);
		if (prefab.IsEmpty())
			return;

		m_DebugShapeMgr.AddText("Damage area\n" + prefab, origin + vector.Up * 5.5, 0.5);
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! \param[in] owner
	//! \return just the name of the prefab file from which this entity was created
	static string GetPrefabname(IEntity owner)
	{
		string prefab = SCR_ResourceNameUtils.GetPrefabName(owner);
		const int lastSlash = prefab.LastIndexOf(SCR_StringHelper.SLASH);
		if (lastSlash < 0)
			return string.Empty;

		return prefab.Substring(lastSlash + 1, prefab.Length() - lastSlash - 1);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in,out] damageAreas
	protected void InitializeShapes(IEntity owner, inout array<ref DamageArea> damageAreas = null)
	{
		if (!damageAreas)
		{
			damageAreas = {};
			GetDamageAreas(damageAreas);
		}

		SCR_BaseDamageAreaShape shape;
		foreach (DamageArea area : damageAreas)
		{
			if (!area)
			{
				Print("SCR_DamageAreaComponent->InitializeShapes: " + owner + " contains null areas!!!", LogLevel.ERROR);
				continue;
			}

			shape = SCR_BaseDamageAreaShape.Cast(area.GetShape());
			if (!shape)
			{
				Print("SCR_DamageAreaComponent->InitializeShapes: " + owner + " has a " + area.Type().ToString() + " which contains null shape!!!", LogLevel.ERROR);
				continue;
			}

			shape.Init(owner);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		m_aDamageAreas = {};
		GetDamageAreas(m_aDamageAreas);
		InitializeShapes(owner, m_aDamageAreas);

		if (!GetGame().InPlayMode())
			return;

		SCR_DamageArea scriptedArea;
		foreach (int i, DamageArea area : m_aDamageAreas)
		{
			if (!area)
			{
				Print("SCR_DamageAreaComponent->EOnInit: " + GetPrefabname(owner) + " contains null areas at position " + i + "!!!", LogLevel.ERROR);
				continue;
			}

			scriptedArea = SCR_DamageArea.Cast(area);
			if (!area.GetShape())
			{
				Print("SCR_DamageAreaComponent->EOnInit: " + GetPrefabname(owner) + " is missing a shape for area with id = " + i, LogLevel.ERROR);
				continue;
			}

			if (scriptedArea && !scriptedArea.IsRegisteredAutomatically())
				continue;

			RegisterDamageArea(area);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		foreach (DamageArea area : m_aDamageAreas)
		{
			UnregisterDamageArea(area);
		}
	}
}

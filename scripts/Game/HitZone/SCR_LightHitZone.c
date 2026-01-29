//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
class SCR_LightHitZone : SCR_VehicleHitZone
{
	private BaseLightManagerComponent m_LightManager;
	private ref array<BaseLightSlot> m_aLightSlots = {};
	protected string m_sSurfaceName;
	protected int m_iParentSurfaceID;

	//------------------------------------------------------------------------------------------------
	void SetSurfaceName(string surfaceName)
	{
		m_aLightSlots.Clear();
		m_iParentSurfaceID = -1;
		if (surfaceName == string.Empty)
			return;

		m_sSurfaceName = surfaceName;
		IEntity parent = GetOwner();
		while (parent && !m_LightManager)
		{
			m_LightManager = BaseLightManagerComponent.Cast(parent.FindComponent(BaseLightManagerComponent));
			if (!m_LightManager)
				parent = parent.GetParent();
		}

		if (!m_LightManager)
			return;

		SCR_LightSlot scrLightSlot;
		array<BaseLightSlot> lights = {};
		m_LightManager.GetLights(lights);
		foreach (BaseLightSlot lightSlot : lights)
		{
			scrLightSlot = SCR_LightSlot.Cast(lightSlot);
			if (scrLightSlot && m_sSurfaceName.Compare(scrLightSlot.GetParentSurfaceName(), false) == 0)
			{
				m_aLightSlots.Insert(lightSlot);
				if (m_iParentSurfaceID == -1)
					m_iParentSurfaceID = lightSlot.GetSurfaceID();

#ifdef WORKBENCH
				if (m_iParentSurfaceID != -1 && m_iParentSurfaceID != lightSlot.GetSurfaceID())
					Print("WARNING! Multiple lights tied to the hit zone from this entity: "+GetOwner()+" while having different parent surface id ("+m_iParentSurfaceID+" != "+lightSlot.GetSurfaceID()+")", LogLevel.ERROR);
#endif
			}
		}

		UpdateLightState(GetDamageState());
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		UpdateLightState(newState);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateLightState(EDamageState newState)
	{
		if (!m_LightManager)
			return;

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(GetOwner());
		if (rplComp && rplComp.IsProxy())
			return;

		bool isAlive = newState != EDamageState.DESTROYED;

		bool functionalityStateChanged;
		if (m_iParentSurfaceID > -1)
			functionalityStateChanged = m_LightManager.TrySetSurfaceFunctional(m_iParentSurfaceID, isAlive);

		int lightSide;
		bool lastState;
		foreach (BaseLightSlot lightSlot : m_aLightSlots)
		{
			if (!lightSlot)
				continue;

			lightSlot.SetLightFunctional(isAlive);
			if (isAlive && functionalityStateChanged && lightSlot.GetLightType() == ELightType.Hazard)
			{//hazards need to be reset as its possible that they will get out of sync
				lightSide = lightSlot.GetLightSide();
				lastState = m_LightManager.GetLightsState(lightSlot.GetLightType(), lightSide);
				if (lightSide != -1)
				{//check if other side is also turned on and in such case we need to reset both sides
					if (lightSide == 0)
						lightSide = 1;//right
					else
						lightSide = 0;//left

					if (lastState == m_LightManager.GetLightsState(lightSlot.GetLightType(), lightSide))
						lightSide = -1;//all sides
					else
						lightSide = lightSlot.GetLightSide();
				}

				if (!lastState)
					return;

				m_LightManager.SetLightsState(lightSlot.GetLightType(), false, lightSide);//first we need to turn them off or else they will blink faster
				m_LightManager.SetLightsState(lightSlot.GetLightType(), lastState, lightSide);

				functionalityStateChanged = false;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_LightHitZone()
	{
		m_aLightSlots.Clear();
		m_aLightSlots = null;
	}
}
//---- REFACTOR NOTE END ----

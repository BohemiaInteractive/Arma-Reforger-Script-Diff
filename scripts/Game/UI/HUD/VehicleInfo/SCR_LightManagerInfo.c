class SCR_LightManagerInfo : SCR_BaseVehicleInfo
{
	[Attribute(SCR_Enum.GetDefault(ELightType.NoLight), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	protected ELightType m_eLightType;

	[Attribute(SCR_Enum.GetDefault(ELightType.NoLight), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	protected ELightType m_eSuppressedBy;

	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Either", "-1"), ParamEnum("Left", "0"), ParamEnum("Right", "1")})]
	protected int m_iLightSide;

	protected BaseLightManagerComponent m_LightManager;
	protected BaseLightSlot m_LightSlot;
	protected ref ScriptInvoker m_OnLightStateChanged;

	protected bool m_bIsLit = true;		//!< Used for directional/hazard lights that blink

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		// Light manager does not have events yet
		bool isSuppressed;
		if (m_eSuppressedBy != ELightType.NoLight)
			isSuppressed = m_LightManager && m_LightManager.GetLightsState(m_eSuppressedBy, m_iLightSide);

		// TODO: Beam lights should be dark blue!
		if (m_bIsLit && !isSuppressed && m_LightManager && m_LightManager.GetLightsState(m_eLightType, m_iLightSide))
			return EVehicleInfoState.ENABLED;
		else
			return EVehicleInfoState.DISABLED;
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if there is no light manager
		if (!m_LightManager)
			return false;

		array<BaseLightSlot> outLights = new array<BaseLightSlot>;
		int iLights = m_LightManager.GetLights(outLights);

		BaseLightSlot lightSlot;
		ELightType lightType;
		int lightSide;

		for (int i = 0; i < iLights; i++)
		{
			lightSlot = outLights.Get(i);

			lightType = lightSlot.GetLightType();
			lightSide = lightSlot.GetLightSide();

			if ((m_eLightType == lightType) && (m_iLightSide == -1 || m_iLightSide == lightSide))
			{
					#ifdef DEBUG_VEHICLE_UI
					PrintFormat("[%1] BaseLightSlot detected %2 | type: %3 | side: %4", i, lightSlot, lightType, lightSide);
					#endif

					m_LightSlot = lightSlot;

					if (SCR_LightSlot.Cast(m_LightSlot))
						m_OnLightStateChanged = SCR_LightSlot.Cast(m_LightSlot).GetOnLightStateChanged();

					break;
			};
		}

		// Terminate if there is no light slot associated with the setup
		if (!m_LightSlot)
			return false;

		// Add monitoring of light states changes
		if (m_OnLightStateChanged)
			m_OnLightStateChanged.Insert(OnLightStateChanged);

		return super.DisplayStartDrawInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		// Remove monitoring of light states changes
		if (m_OnLightStateChanged)
			m_OnLightStateChanged.Remove(OnLightStateChanged);

		super.DisplayStopDraw(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! \param state
	void OnLightStateChanged(bool state)
	{
		m_bIsLit = state;

		#ifdef DEBUG_VEHICLE_UI
		PrintFormat("%1 OnLightStateChanged: %2", this, m_bIsLit);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);

		m_LightManager = BaseLightManagerComponent.Cast(owner.FindComponent(BaseLightManagerComponent));
	}
}

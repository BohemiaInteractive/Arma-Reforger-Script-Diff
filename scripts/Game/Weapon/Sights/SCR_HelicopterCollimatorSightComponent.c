[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_HelicopterCollimatorSightComponentClass : SCR_CollimatorSightsComponentClass
{
}

class SCR_HelicopterCollimatorSightComponent : SCR_CollimatorSightsComponent
{
	[Attribute(defvalue: "SightDeployment", desc: "Animation signal name used to deploy the sight")]
	protected string m_sDeploySignalName;

	[Attribute(defvalue: "-1", desc: "Dictates where in hierarchy game will look for signals manager component in which value will be changed.\n-1 == Parent while 1 == First Child")]
	protected int m_iSignalHierarchyLevel;

	[Attribute(defvalue: "v_stow", desc: "Name of the bone from which sound will be played.")]
	protected string m_sSoundEventBoneName;

	[Attribute(defvalue: "SOUND_SIGHT_DEPLOY", desc: "Deploy Sound Event Name")]
	protected string m_sDeploySoundEventName;

	[Attribute(defvalue: "SOUND_SIGHT_UNDEPLOY", desc: "Stow Sound Event Name")]
	protected string m_sStowSoundEventName;

	[Attribute(defvalue: "-150", desc: "Min sight elevation angle")]
	protected float m_fMinSightAngle;

	[Attribute(defvalue: "150", desc: "Max sight elevation angle")]
	protected float m_fMaxSightAngle;

	[Attribute(defvalue: "10", desc: "Elevation angle step", params: "0 inf 0.01")]
	protected float m_fElevationAngleStep;

	[Attribute(defvalue: "1", desc: "Factor through which current elevation will be multiplied to get desired signal value", params: "0 inf 0.001")]
	protected float m_fElevationToSignalFactor;

	[Attribute(defvalue: "SightElevation", desc: "Animation signal name used to deploy the sight")]
	protected string m_sElevationSignalName;

	[Attribute(defvalue: "v_elevation_wheel", desc: "Name of the bone from which elevation change sound will be played")]
	protected string m_sElevationSoundEventBoneName;

	[RplProp(onRplName: "OnSightStateSynced")]
	protected bool m_bIsDeployed;

	protected const string DEPLOY_COLLIMATOR_ACTION_NAME = "HelicopterSightDeploy";
	protected const string CHANGE_COLLIMATOR_ELEVATION_ACTION_NAME = "HelicopterSightZeroing";

	//------------------------------------------------------------------------------------------------
	override void OnSightADSActivate()
	{
		SetUpActionListeners();
	}

	//------------------------------------------------------------------------------------------------
	override void OnSightADSDeactivated()
	{
		RemoveActionListeners();
	}

	//------------------------------------------------------------------------------------------------
	//! Keybind callback responsible for switching deplyed state of the sight
	//! \param[in] value
	//! \param[in] reason
	//! \param[in] actionName
	protected void ToggleSightState(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(GetOwner());
		if (!rplComp)
			return;

		ChangeSightDeployedState(!m_bIsDeployed);//this wont cause too much of a desync thus it can be done locally to not wait for network delay
		controller.ReplicateHelicopterSightState(m_bIsDeployed, rplComp.Id());
	}

	//------------------------------------------------------------------------------------------------
	//! Method called by authority that can broadcast the change
	//! \param[in] newState
	void OwnerSyncSightState(bool newState)
	{
		if (newState == m_bIsDeployed)
			return;

		ChangeSightDeployedState(newState);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSightStateSynced()
	{
		ChangeSightDeployedState(m_bIsDeployed);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to locally override current state of the sight
	//! \param[in] newState
	void OverrideSightState(bool newState)
	{
		m_bIsDeployed = newState;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds signals manager component that is responsible for animation playback
	protected SignalsManagerComponent FindSignalsManagerComponent()
	{
		IEntity signalManagerOwner = GetOwner();
		for (int i, maxLevel = Math.AbsInt(m_iSignalHierarchyLevel); i < maxLevel; i++)
		{
			if (!signalManagerOwner)
				return null;

			if (m_iSignalHierarchyLevel < 0)
				signalManagerOwner = signalManagerOwner.GetParent();
			else
				signalManagerOwner = signalManagerOwner.GetChildren();
		}

		if (!signalManagerOwner)
			return null;

		return SignalsManagerComponent.Cast(signalManagerOwner.FindComponent(SignalsManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newState
	protected void ChangeSightDeployedState(bool newState)
	{
		m_bIsDeployed = newState;

		SignalsManagerComponent signalMgr = FindSignalsManagerComponent();
		if (!signalMgr)
			return;

		int id = signalMgr.FindSignal(m_sDeploySignalName);
		if (signalMgr.GetSignalValue(id) == newState)
			return;

		signalMgr.SetSignalValue(id, m_bIsDeployed);

		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComp)
			return;

		if (m_bIsDeployed)
			soundComp.SoundEventBone(m_sDeploySoundEventName, m_sSoundEventBoneName);
		else
			soundComp.SoundEventBone(m_sStowSoundEventName, m_sSoundEventBoneName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Keybind callback that locally changes the elevation of the sight
	//! \param[in] value
	//! \param[in] reason
	//! \param[in] actionName
	protected void ChangeSightElevation(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (value == 0)
			return;

		value /= Math.AbsFloat(value);
		value *= m_fElevationAngleStep;
		value = Math.Clamp(value + GetVerticalAngularCorrection(), m_fMinSightAngle, m_fMaxSightAngle);
		SetVerticalAngularCorrection(value);

		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComp)
			soundComp.SoundEventBone(SCR_SoundEvent.SOUND_SIGHT_ELEVATION, m_sElevationSoundEventBoneName);

		if (m_fElevationToSignalFactor == 0 || m_sElevationSignalName.IsEmpty())
			return;

		SignalsManagerComponent signalMgr = FindSignalsManagerComponent();
		if (!signalMgr)
			return;

		int id = signalMgr.FindSignal(m_sElevationSignalName);
		signalMgr.SetSignalValue(id, value * m_fElevationToSignalFactor);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUpActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (!m_sDeploySignalName.IsEmpty())
			inputMgr.AddActionListener(DEPLOY_COLLIMATOR_ACTION_NAME, EActionTrigger.DOWN, ToggleSightState);

		if (m_fMinSightAngle < m_fMaxSightAngle)
			inputMgr.AddActionListener(CHANGE_COLLIMATOR_ELEVATION_ACTION_NAME, EActionTrigger.VALUE, ChangeSightElevation);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.RemoveActionListener(DEPLOY_COLLIMATOR_ACTION_NAME, EActionTrigger.DOWN, ToggleSightState);
		inputMgr.RemoveActionListener(CHANGE_COLLIMATOR_ELEVATION_ACTION_NAME, EActionTrigger.VALUE, ChangeSightElevation);
	}
}
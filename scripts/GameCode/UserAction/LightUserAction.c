class LightUserAction : BaseLightUserAction
{
	//! Type of action instead of 8 different classes
	[Attribute(defvalue: SCR_Enum.GetDefault(ELightType.Head), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	protected ELightType m_eLightType;

	//! Side of turn signals
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Either", "-1"), ParamEnum("Left", "0"), ParamEnum("Right", "1")})]
	protected int m_iLightSide;

	//! Will only be shown if user is in vehicle?
	[Attribute("1")]
	protected bool m_bInteriorOnly;

	//! Will action be available for entities seated in pilot compartment only?
	[Attribute("1")]
	protected bool m_bPilotOnly;

	[Attribute("1", "True, if the action should be available to other passengers as long as pilot is not there or unconscious")]
	protected bool m_bPilotUncapableOverride;

	[Attribute("#AR-UserAction_ControlledByDriver", "Text that will be shown to the passengers when driver of the vehicle is in control of this action")]
	protected LocalizedString m_sBlockedByPilotText;

	[Attribute(desc: "When anything is defined in here, this action will only be visible if the compartment section of the compartment the player is in is defined in here.")]
	protected ref array<int> m_aDefinedCompartmentSectionsOnly;

	[Attribute(desc: "When aynthing is defined in here, this action won't be visible if the compartment section of the compartment the player is in is defined in here.")]
	protected ref array<int> m_aExcludeDefinedCompartmentSections;

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		//check if user can actually manipulate controls if he is not the pilot
		if (!SCR_InteractionHandlerComponent.CanBeShownInVehicle(character, this, m_bPilotOnly, m_bPilotUncapableOverride, m_bInteriorOnly, m_aDefinedCompartmentSectionsOnly, m_aExcludeDefinedCompartmentSections))
		{
			SetCannotPerformReason(m_sBlockedByPilotText);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!GetLightManager())
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		//check only if user is inside same vehicle as long as we require that
		return SCR_InteractionHandlerComponent.CanBeShownInVehicle(character, this, false, false, m_bInteriorOnly, m_aDefinedCompartmentSectionsOnly, m_aExcludeDefinedCompartmentSections);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (!lightManager)
			return;

		bool lightsState;
				

		lightsState = lightManager.GetLightsState(m_eLightType, m_iLightSide);

		if (RplSession.Mode() != RplMode.Client)
			lightsState = !lightsState;

		lightManager.SetLightsState(m_eLightType, lightsState, m_iLightSide);

		if (lightsState)
		{
			// Disable opposite turn signal
			if (m_iLightSide == 0)
				lightManager.SetLightsState(m_eLightType, false, 1);
			else if (m_iLightSide == 1)
				lightManager.SetLightsState(m_eLightType, false, 0);
		}

		// Sound
		PlaySound(pOwnerEntity, lightsState);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		BaseLightManagerComponent lightManager = GetLightManager();

		auto prefix = "";
		UIInfo actionInfo = GetUIInfo();
		if(actionInfo)
			prefix = actionInfo.GetName() + " ";

		if (lightManager && lightManager.GetLightsState(m_eLightType))
			outName = prefix + "#AR-UserAction_State_Off";
		else
			outName = prefix + "#AR-UserAction_State_On";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool GetLightAudioPos(out vector pos)
	{
		UserActionContext userActionContext = GetActiveContext();
		if (!userActionContext)
		{
			return false;
		}
		else
		{
			pos = GetOwner().CoordToLocal(userActionContext.GetOrigin());
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	void PlaySound(IEntity ownerEntity, bool lightsState)
	{
		// Sound
		SoundComponent soundComponent = SoundComponent.Cast(ownerEntity.FindComponent(SoundComponent));
		if (!soundComponent)
			return;

		vector offset;
		if (GetLightAudioPos(offset))
		{
			if (lightsState)
				soundComponent.SoundEventOffset(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_ON, offset);
			else
				soundComponent.SoundEventOffset(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_OFF, offset);
		}
		else
		{
			if (lightsState)
				soundComponent.SoundEvent(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_ON);
			else
				soundComponent.SoundEvent(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_OFF);
		}
	}
}

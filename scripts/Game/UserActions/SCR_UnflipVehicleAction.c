class SCR_UnflipVehicleAction : SCR_PushVehicleAction
{
	[Attribute(defvalue: "1", desc: "Factor that will multiply number of suggested required players to better indicate difficulty of flipping this vehicle\nDue to the fact that some vehicles are very boxy they are inherently harder to flip back on wheels", params: "0.01 inf")]
	protected float m_fSuggestedUsersMultiplier;

	protected ref array<IEntity> m_aUsers = {};
	protected SCR_RepairSupportStationComponent m_HelpingStation;
	protected bool m_bSelected;

	//! squared radius in meters
	protected const float SUPPORT_STATION_SEARCH_RADIUS_SQ = 100;

	//------------------------------------------------------------------------------------------------
	//! Can this action be shown in the UI to the provided user entity?
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		vector angles = GetOwner().GetAngles();
		if (float.AlmostEqual(0, angles[0], 45) && float.AlmostEqual(0, angles[2], 45))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when someone tries to perform the continuous action, user entity is typically character
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		// Offset force target slightly above the center of mass to provide better leverage
		// Slightly unrealistic, but it should be good for gameplay.
		vector mins, maxs;
		IEntity rootEntity = pOwnerEntity.GetRootParent();
		rootEntity.GetWorldBounds(mins, maxs);
		vector forceLeverage = vector.Up * (maxs[1] - mins[1]) * 0.3;
		ApplyForce(rootEntity, pUserEntity, timeSlice, forceLeverage);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);

		if (!m_aUsers.Contains(pUserEntity))
			m_aUsers.Insert(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pOwnerEntity, pUserEntity);

		m_aUsers.RemoveItem(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! If overridden and true is returned, outName is returned when BaseUserAction.GetActionName is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetActionNameScript(out string outName)
	{
		UIInfo info = GetUIInfo();
		if (!info)
			return false;

		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return false;

		float mass = physics.GetMass();
		float forcePerUser = Math.Min(m_fMassToForce * mass, m_fForceLimit);
		if (forcePerUser == 0)
			return false;

		float assistedForce;
		if (m_bSelected) // only check this when this action is selected
			assistedForce = GetAdditionalForce();

		int suggestedUsers = Math.Max(1, Math.Ceil(((mass - assistedForce) / forcePerUser) * m_fSuggestedUsersMultiplier));

		// Manually translate instead of using BaseUserAction.ActionNameParams, as SCR_ActionMenuInteractionDisplay.CompareNewDateWithOld() will not see the change in ActionNameParams
		// In this case it wont cause the problems, as it is being refreshed every other frame or so, thus even after language change string will regenerate
		outName = WidgetManager.Translate(info.GetName(), m_aUsers.Count(), suggestedUsers);
		if (assistedForce > 0)
			outName = WidgetManager.Translate(info.GetDescription(), outName);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the progress of this action in seconds.
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return 1;

		float mass = physics.GetMass();
		float forcePerUser = Math.Min(m_fMassToForce * mass, m_fForceLimit);
		if (forcePerUser == 0)
			return 1;

		float assistedForce;
		if (m_HelpingStation)
			assistedForce = m_HelpingStation.GetUnflippingPower();

		int suggestedUsers = Math.Max(1, Math.Ceil(((mass - assistedForce) / forcePerUser) * m_fSuggestedUsersMultiplier));

		return m_aUsers.Count() / suggestedUsers;
	}

	//------------------------------------------------------------------------------------------------
	override protected float GetAdditionalForce()
	{
		if (!m_HelpingStation)
		{
			FindNearbySupportStation();
			if (!m_HelpingStation)
				return 0;
		}
		else if (vector.DistanceSq(GetOwner().GetOrigin(), m_HelpingStation.GetOwner().GetOrigin()) > SUPPORT_STATION_SEARCH_RADIUS_SQ)
		{
			m_HelpingStation = null;
			FindNearbySupportStation();
			if (!m_HelpingStation)
				return 0;
		}

		return m_HelpingStation.GetUnflippingPower();
	}

	//------------------------------------------------------------------------------------------------
	override protected float GetMaxForceToMassFactor()
	{
		if (m_fSuggestedUsersMultiplier == 1)
			return 1;

		float mass = GetOwner().GetPhysics().GetMass();
		float forcePerUser = Math.Min(m_fMassToForce * mass, m_fForceLimit);
		return forcePerUser * Math.Ceil((mass / forcePerUser) * m_fSuggestedUsersMultiplier + 1) / mass;
	}

	//------------------------------------------------------------------------------------------------
	//! Queries the area around the owner of this action, in order to search for support station, which can provide help for this action
	protected void FindNearbySupportStation()
	{
		IEntity owner = GetOwner();

		owner.GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), Math.Sqrt(SUPPORT_STATION_SEARCH_RADIUS_SQ), EvaluateFoundEntity, QueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to quickly filter most of unwanted objects
	//! \param[in] ent entity that was found with the query
	//! \return true if entity should be evaluated further, otherwise false
	protected bool QueryFilter(IEntity ent)
	{
		if (!ent)
			return false;

		IEntity owner = GetOwner();
		if (ent == owner)
			return false; // skip itself

		IEntity parent = ent.GetParent();
		if (parent && (parent == owner || ChimeraCharacter.Cast(parent)))
			return false; // check if slotted repair station is no the child of this action owner or some character

		if (!ent.GetPrefabData())
			return false; // f.e. particles dont have prefab data

		if (ChimeraCharacter.Cast(ent))
			return false; // character are not support stations

		if (DestructibleEntity.Cast(ent))
			return false; // dont check buildings and trees

		if (vector.DistanceSq(owner.GetOrigin(), ent.GetOrigin()) > SUPPORT_STATION_SEARCH_RADIUS_SQ)
			return false; // BBOX of an object can be large, and we will later use origin for easy validation if it is in range

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to evaluate if found is a valid support station
	//! \param[in] ent entity that was found with the query
	//! \return true if query should continue fetching more entities
	protected bool EvaluateFoundEntity(IEntity ent)
	{
		SCR_RepairSupportStationComponent foundStation = SCR_RepairSupportStationComponent.Cast(ent.FindComponent(SCR_RepairSupportStationComponent));
		if (!foundStation)
			return true;

		float flippingPower = foundStation.GetUnflippingPower();
		if (flippingPower <= 0)
			return true;

		if (m_HelpingStation == foundStation)
			return false; // stop searching as we still have the old station

		if (m_HelpingStation && m_HelpingStation.GetUnflippingPower() >= flippingPower)
			return true;

		m_HelpingStation = foundStation;
		return true; // maybe we will find a better station
	}

	//------------------------------------------------------------------------------------------------
	override protected event void OnActionSelected()
	{
		m_bSelected = true;
		FindNearbySupportStation();
	}

	//------------------------------------------------------------------------------------------------
	override protected event void OnActionDeselected()
	{
		m_bSelected = false;
	}

	//------------------------------------------------------------------------------------------------
	//! If HasLocalEffectOnly() is false this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript()
	{
		return true;
	}
}

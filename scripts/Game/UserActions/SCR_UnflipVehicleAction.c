class SCR_UnflipVehicleAction : SCR_PushVehicleAction
{
	[Attribute(defvalue: "1", desc: "Factor that will multiply number of suggested required players to better indicate difficulty of flipping this vehicle\nDue to the fact that some vehicles are very boxy they are inherently harder to flip back on wheels", params: "0.01 inf")]
	protected float m_fSuggestedUsersMultiplier;

	protected ref array<IEntity> m_aUsers = {};
	protected SCR_RepairSupportStationComponent m_HelpingStation;
	protected bool m_bSelected;
	protected bool m_bEndSlowly; // determines if stopAnimation should have slow or fast version

	//! squared radius in meters
	protected const float SUPPORT_STATION_SEARCH_RADIUS_SQ = 100;
	//! How far in front of the vehicle body the character should stand (capsule radius + a bit).
	protected const float ALIGN_STANDOFF = 0.7;
	//! Fallback stand-off distance from the center, used when the raycast fails to hit the body.
	protected const float ALIGN_FALLBACK_DIST = 1.2;
	//! Push and pull animations are disabled. The animation could end on the owner while it was still playing on the server,
	//! which desynced the character state between them.
	protected const bool USE_ANIMATIONS = false;

	//------------------------------------------------------------------------------------------------
	//! Can this action be shown in the UI to the provided user entity?
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		bool isVehicleFlipped = SCR_AIVehicleUsability.IsVehicleFlipped(GetOwner());
		m_bEndSlowly = !isVehicleFlipped;
		
		return isVehicleFlipped;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsUserPlayingBlockingAnimation_Wrapper(notnull ChimeraCharacter character)
	{
		return !m_aUsers.Contains(character) && super.IsUserPlayingBlockingAnimation(character);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected vector GetForceDirection(notnull IEntity forceTarget, vector forceTargetPos, vector forceOrigin)
	{
		vector forceDirection = vector.Direction(forceTargetPos, forceOrigin);
		vector vehTransform[4];
		forceTarget.GetWorldTransform(vehTransform);
		vector rollAxis = Vector(vehTransform[2][0], 0, vehTransform[2][2]); // flatForward
		if (rollAxis.LengthSq() > 0.0001)
		{
			rollAxis.Normalize();
			forceDirection = forceDirection - rollAxis * vector.Dot(forceDirection, rollAxis);
		}

		forceDirection[1] = 0; // keep horizontal (as before via forceOrigin[1] = forceTargetPos[1])
		return forceDirection;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when someone tries to perform the continuous action, user entity is typically character
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		IEntity rootEntity = pOwnerEntity.GetRootParent();
		if (!SCR_AIVehicleUsability.IsVehicleFlipped(rootEntity))
		{
			m_bEndSlowly = true;
			return;
		}

		// Offset force target slightly above the center of mass to provide better leverage
		// Slightly unrealistic, but it should be good for gameplay.
		vector mins, maxs;
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

		ChimeraCharacter char = ChimeraCharacter.Cast(pUserEntity);
		if (!char || !SCR_CharacterHelper.IsPlayerOrAIOwner(char))
			return;

		// check if he isnt playing an animation as there might be a delay between check and execution
		if (IsUserPlayingBlockingAnimation(char))
		{
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
			if (controller)
				controller.StopLoitering(true);

			return; // if character is playing an animation, then finishing it may take some time, thus we cannot start with unflipping animation
		}

		//Unflipping is dangerous - protect the user from vehicle hitting him
		SCR_CharacterDamageManagerComponent charDmgManager = SCR_CharacterDamageManagerComponent.Cast(pUserEntity.FindComponent(SCR_CharacterDamageManagerComponent));
		if (charDmgManager)
			charDmgManager.SetUnflipCollisionGuardActive(true, GetOwner().GetRootParent());
		
		if (!USE_ANIMATIONS)
			return;
		
		// play animation on correct spot
		SCR_AIAnimation_Recovery unflip = GetPushOrPullAnimation();
		if (!unflip)
			return;
		
		vector mat[4];
		if (!BuildAlignTransform(char, mat))
		{
			char.GetWorldTransform(mat);
			IEntity vehicle = GetOwner().GetRootParent();
			if (vehicle)
			{
				vector dirToVehicle = vehicle.GetOrigin() - mat[3];
				dirToVehicle[1] = 0;
				if (dirToVehicle.LengthSq() > 0.001)
				{	
					dirToVehicle.Normalize();
					mat[0] = vector.Up * dirToVehicle;
					mat[1] = vector.Up;
					mat[2] = dirToVehicle;
				}
			}
		}
		
		unflip.StartAnimation(pUserEntity, mat);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pOwnerEntity, pUserEntity);

		m_aUsers.RemoveItem(pUserEntity);

		ChimeraCharacter char = ChimeraCharacter.Cast(pUserEntity);
		if (char && !SCR_CharacterHelper.IsPlayerOrAIOwner(char))
			return;

		//Unflipping is dangerous
		SCR_CharacterDamageManagerComponent charDmgManager = SCR_CharacterDamageManagerComponent.Cast(pUserEntity.FindComponent(SCR_CharacterDamageManagerComponent));
		if (charDmgManager)
			charDmgManager.SetUnflipCollisionGuardActive(false, GetOwner().GetRootParent());
		
		if (!USE_ANIMATIONS)
			return;
		
		SCR_AIAnimation_Recovery unflip = GetPushOrPullAnimation();
		if (!unflip)
			return;
		
		unflip.StopAnimation(pUserEntity, !m_bEndSlowly);
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
		m_aUsers.RemoveItem(SCR_PlayerController.GetLocalControlledEntity());
	}

	//------------------------------------------------------------------------------------------------
	//! If HasLocalEffectOnly() is false this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Determines which animation set to use
	protected SCR_AIAnimation_Recovery GetPushOrPullAnimation()
	{
		if (m_bPull)
		{
			SCR_AIAnimation_Recovery_Pull pull = new SCR_AIAnimation_Recovery_Pull();
			if (!pull.GetAnimDataFromConfig())
				return null;
			return pull;
		}

		SCR_AIAnimation_Recovery_Push push = new SCR_AIAnimation_Recovery_Push();
		if (!push.GetAnimDataFromConfig())
			return null;
		return push;
	}

	//------------------------------------------------------------------------------------------------
	//! Builds a world-space transform that puts the character in front of the vehicle,
	//! facing it (+Z into the vehicle). Works for both AI and player.
	//! \param pUserEntity  the character performing the action
	//! \param[out] outMat   resulting world transform (right, up, forward, position)
	//! \return true if a valid transform was built, false -> caller should fall back to identity or facing vehicle
	protected bool BuildAlignTransform(notnull ChimeraCharacter character, out vector outMat[4])
	{
		IEntity vehicle = GetOwner().GetRootParent();
		if (!vehicle)
			return false;

		Physics physics = vehicle.GetPhysics();
		if (!physics)
			return false;

		// --- Reference points (mirror what ApplyForce already uses) ---
		vector charPos = character.GetOrigin();
		vector vehCenter = vehicle.CoordToParent(physics.GetCenterOfMass());

		// --- Facing direction: from agent to the vehicle center, flattened to horizontal ---
		// This is the same idea as ShouldPush()'s toAgentFlat and is stable on a flipped car.
		vector dirToVehicle = vehCenter - charPos;
		dirToVehicle[1] = 0;
		if (dirToVehicle.LengthSq() < 0.0001)
			return false; // degenerate: character on top of center

		dirToVehicle.Normalize();
		// Character forward (+Z) must point INTO the vehicle => opposite of "away".
		vector right = vector.Up * dirToVehicle;
		right.Normalize();
		// --- Find where the body surface is, so we can stand just in front of it ---
		// Ray goes from the agent side towards the vehicle center, horizontally.
		vector rayStart = character.EyePosition();
		// flatten Y (same trick as ApplyForce)
		rayStart[1] = vehCenter[1];
		float rayLen = vector.Distance(rayStart, vehCenter) + 1.0;
		vector rayEnd = rayStart + dirToVehicle * rayLen;

		TraceParam trace = new TraceParam();
		trace.Start = rayStart;
		trace.End = rayEnd;
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		trace.Exclude = character;

		float frac = vehicle.GetWorld().TraceMove(trace, null);

		vector standPos;
		if (frac < 1.0 && trace.TraceEnt && trace.TraceEnt.GetRootParent() == vehicle)
		{
			// Hit the body: back off from the hit point along our facing.
			vector hitPos = rayStart + (rayEnd - rayStart) * frac;
			standPos = hitPos - dirToVehicle * ALIGN_STANDOFF;
		}
		else
		{
			// No hit: place the character a fixed distance out from the center.
			standPos = vehCenter - dirToVehicle * ALIGN_FALLBACK_DIST;
		}

		// Keep the character on their own ground height (don't lift/sink them).
		standPos[1] = charPos[1];

		// --- Layout expected by Enfusion: [0]=right, [1]=up, [2]=forward, [3]=position ---
		outMat[0] = right;
		outMat[1] = vector.Up;
		outMat[2] = dirToVehicle;
		outMat[3] = standPos;
		
		return true;
	}
}

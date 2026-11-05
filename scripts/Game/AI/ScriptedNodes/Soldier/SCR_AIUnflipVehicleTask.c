class SCR_AIUnflipVehicleTask : AITaskScripted
{
	[Attribute("20", UIWidgets.Auto, "Lenght of one unflipping effort [s]")]
	protected float m_fUnflippingEffortLength;
	
	[Attribute("0.1", UIWidgets.Auto, "Lenght of one unflipping tick [s]")]
	protected float m_fUnflippingTickLength;
	
	[Attribute("0", UIWidgets.Auto, "Should apply force continuously (as player pressing action button)")]
	protected bool m_bUnflipContinuously;
	
	protected float m_fNextUnflippingEffortTick;
	
	static const string PORT_VEHICLE_ENTITY 		= "VehicleEntity";
	
	protected bool m_bAbortDone;
	IEntity m_vehicleEntity, m_ownerEntity;
	bool m_bShouldPush;
	bool m_bAnimStarted;
	float m_fUnflippingTime;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_bAbortDone = false;
		m_bAnimStarted = false;
		m_fUnflippingTime = 0;
		m_fNextUnflippingEffortTick = m_fUnflippingTickLength + Math.RandomFloat01() * 0.1;
		m_ownerEntity = owner.GetControlledEntity();
		GetVariableIn(PORT_VEHICLE_ENTITY, m_vehicleEntity);
		if (!m_vehicleEntity)
			return;
		
		m_bShouldPush = ShouldPush(m_ownerEntity, m_vehicleEntity);
		
		SCR_UnflipVehicleAction unflipAction = FindUnflipAction(m_bShouldPush, m_vehicleEntity);
		if (unflipAction && SCR_AIVehicleUsability.IsVehicleFlipped(m_vehicleEntity))
		{
			m_bAnimStarted = true;
			unflipAction.OnActionStart(m_ownerEntity);		
		}	
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_vehicleEntity || !m_ownerEntity)
			return ENodeResult.FAIL;
		
		SCR_UnflipVehicleAction unflipAction = FindUnflipAction(m_bShouldPush, m_vehicleEntity);
		// early test, is vehicle flipped at all
		if (!SCR_AIVehicleUsability.IsVehicleFlipped(m_vehicleEntity))
		{
			if (m_bAnimStarted && unflipAction)
				StopAction(m_ownerEntity, unflipAction);
			return ENodeResult.FAIL;
		}
		
		bool shouldPush = ShouldPush(m_ownerEntity, m_vehicleEntity);
		
		if (shouldPush != m_bShouldPush)
		{
			// new direction detected
			StopAction(m_ownerEntity, unflipAction);
			unflipAction = FindUnflipAction(shouldPush, m_vehicleEntity);
			unflipAction.OnActionStart(m_ownerEntity);
			unflipAction.PerformContinuousAction(m_vehicleEntity, m_ownerEntity, dt);
			m_bShouldPush = shouldPush;
		}
		
		m_fUnflippingTime += dt;
		if (m_fUnflippingTime < dt + 0.001)
		{
			unflipAction.PerformContinuousAction(m_vehicleEntity, m_ownerEntity, dt);
		}
		else if (m_fUnflippingTime > m_fUnflippingEffortLength)
		{
			StopAction(m_ownerEntity, unflipAction);
			return ENodeResult.SUCCESS;
		}
		else if (m_fUnflippingTime > m_fNextUnflippingEffortTick || m_bUnflipContinuously)
		{
			unflipAction.PerformContinuousAction(m_vehicleEntity, m_ownerEntity, dt);
			m_fNextUnflippingEffortTick += m_fUnflippingTime;
		}
		
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bAbortDone || !m_vehicleEntity)
			return;
		
		SCR_UnflipVehicleAction unflipAction = FindUnflipAction(m_bShouldPush, m_vehicleEntity);
		StopAction(m_ownerEntity, unflipAction);
		m_bAbortDone = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StopAction(IEntity ownerEntity, SCR_UnflipVehicleAction unflipAction)
	{
		if (!unflipAction)
			return;
		
		unflipAction.OnActionCanceled(unflipAction.GetOwner(), ownerEntity);		
	}
	
	//------------------------------------------------------------------------------------------------
	// method decides from orientation of the vehicle if character should push - true or pull - false
	protected bool ShouldPush(IEntity performer, IEntity vehicleEntity)
	{
		vector myLoc = performer.GetOrigin();
 		vector vehLoc = vehicleEntity.GetOrigin();
		vector vehUp = vehicleEntity.GetWorldTransformAxis(1).Normalized();
		vector vehRight = vehicleEntity.GetWorldTransformAxis(0).Normalized();
		vector vehForward = vehicleEntity.GetWorldTransformAxis(2).Normalized();
		vector toAgent = myLoc - vehLoc;
		vector toAgentFlat = Vector(toAgent[0], 0, toAgent[2]);
		
		float dotVehicleUpWorldUp = vector.Dot(vehUp, vector.Up);
		bool vehicleOnRoof = dotVehicleUpWorldUp < 0 && !float.AlmostEqual(dotVehicleUpWorldUp, 0);
		
		bool shouldPush;
		
		if (vehicleOnRoof)
		{
		    // Car is upside down — we need to determine left vs right side
		    // Use vehRight which is still roughly horizontal when on roof
		    // If agent is on the right side of the car, dot > 0
		    vector vehRightFlat = Vector(vehRight[0], 0, vehRight[2]);
			float dotAgentVehRight = vector.Dot(toAgentFlat, vehRightFlat);
			shouldPush = dotAgentVehRight > 0;
		}
		else
		{
		    // Project vehUp onto the plane perpendicular to flatForward
			vector vehUpFlat = Vector(vehUp[0], 0, vehUp[2]);
			vector flatForward = Vector(vehForward[0], 0, vehForward[2]);
			// Remove the component along flatForward
			// so we ONLY keep the left-right component
			float fwdComponent = vector.Dot(vehUpFlat, flatForward);
			vector vehUpSideOnly = vehUpFlat - flatForward * fwdComponent;
			float dotAgentSide = vector.Dot(toAgentFlat, vehUpSideOnly);
			shouldPush = dotAgentSide > 0;
		}
		return shouldPush;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_UnflipVehicleAction FindUnflipAction (bool shouldPush, notnull IEntity vehicleEntity)
	{
		ActionsManagerComponent actionsMgr = ActionsManagerComponent.Cast(vehicleEntity.FindComponent(ActionsManagerComponent));
		if (!actionsMgr)
			return null;
		
		SCR_UnflipVehicleAction unflipAction;
		array<BaseUserAction> actions = {};
		actionsMgr.GetActionsList(actions);
		
		foreach (BaseUserAction action : actions)
		{
			unflipAction = SCR_UnflipVehicleAction.Cast(action);
			if (unflipAction && (shouldPush == !unflipAction.IsPull()))
				break;
		}
		
		return unflipAction;
	}
	
	//------------------------------------------------------------------------------------------------
	override static event protected bool VisibleInPalette() { return true; }
	override static event protected bool CanReturnRunning() { return true; }
	override static event protected string GetOnHoverDescription() { return "Tries to unflip the given vehicle. Returns FAIL when vehicle is flipped."; }
	
	protected static ref TStringArray s_aVarsIn = {	PORT_VEHICLE_ENTITY };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
};
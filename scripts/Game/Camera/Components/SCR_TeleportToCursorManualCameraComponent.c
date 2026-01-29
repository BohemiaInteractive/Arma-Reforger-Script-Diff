//! @ingroup ManualCamera

//! Teleport the camera to the cursor's world position
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_TeleportToCursorManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected static const float SETTLE_THRESHOLD = 0.01;

	[Attribute("")]
	private string m_sActionTeleportToCursor;

	[Attribute("")]
	private string m_sActionTeleportToPlayer;
	
	[Attribute("5", UIWidgets.Auto, "")]
	private float m_fMinDistance;
	
	[Attribute("30", UIWidgets.Auto, "")]
	private float m_fMaxDistance;
	
	[Attribute(desc: "How far from the target position is camera teleported before it plays settle-in animation.", defvalue: "5")]
	private float m_fSettleInDistance;
	
	[Attribute(defvalue: "275", desc: "When further than this limit, teleport the camera instantly, otherwise start transition from its current position.")]
	private float m_fTeleportDistance;
	
	[Attribute(defvalue: "0.12")]
	private float m_fTransitionStrength;
	
	[Attribute("15", UIWidgets.Auto, "")]
	private float m_fMinAngle;
	
	[Attribute("-30", UIWidgets.Slider, params: "-89 89 0.1")]
	private float m_fDefaultAngle;
	
	[Attribute()]
	private string m_sSoundEvent;
	
	private bool m_bIsSettling;
	private bool m_bIsTeleportInit;
	private bool m_bIsDefaultDistance;
	private bool m_bIsDefaultAngle;
	private bool m_bIsStartAtCurrentPos;
	private bool m_bDisableInterruption;
	private vector m_vInitPos;
	private vector m_vTargetPos;
	private vector m_vTargetRot;
	private float m_fDistance;
	private float m_fDistancePrev;
	
	//------------------------------------------------------------------------------------------------
	//! Teleport camera to given coordinates.
	//! \param[in] position Target position
	//! \param[in] forceDefaultDistance True to force default offset from the target, even when the camera starts closer
	//! \param[in] forceDefaultAngle True to force default angle instead preserving current camera angle
	//! \param[in] forceStartAtCurrentPos True to move camera smoothly to the position, even when it's too far
	//! \param[in] disableInterruption True to prevent user input when teleporting animation is playing
	//! \param[in] distance How far from target position will the camera be positioned (also influenced by forceDefaultDistance setting)
	//! \param[in] noSound When true, no sound effect will be played
	//! \return True when teleported
	bool TeleportCamera(vector position, bool forceDefaultDistance = false, bool forceDefaultAngle = false, bool forceStartAtCurrentPos = false, bool disableInterruption = false, float distance = -1, bool noSound = false)
	{
		if (position == vector.Zero)
			return false;

		position = CoordToCamera(position);
		
		m_vTargetPos = position;
		m_bIsTeleportInit = true;
		m_bIsDefaultDistance = forceDefaultDistance;
		m_bIsDefaultAngle = forceDefaultAngle;
		m_bIsStartAtCurrentPos = forceStartAtCurrentPos;
		m_bDisableInterruption = disableInterruption;
		if (distance < 0)
			distance = m_fMaxDistance;

		m_fDistance = distance;
		
		//--- Force default angle when the target position is off-screen
		ArmaReforgerScripted game = GetGame();
		if (game && !m_bIsDefaultAngle)
		{
			WorkspaceWidget workspace = game.GetWorkspace();
			if (workspace)
			{
				vector screenPos = workspace.ProjWorldToScreen(m_vTargetPos, game.GetWorld());
				float screenW, screenH;
				workspace.GetScreenSize(screenW, screenH);
				screenW = workspace.DPIUnscale(screenW);
				screenH = workspace.DPIUnscale(screenH);
				if (screenPos[0] < 0 || screenPos[1] < 0 || screenPos[0] > screenW || screenPos[1] > screenH)
				{
					m_bIsDefaultAngle = true;
				}
			}
		}
			
		if (!noSound && !m_sSoundEvent.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sSoundEvent);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (param.isManualInputEnabled)
		{
			if (m_sActionTeleportToCursor && GetInputManager().GetActionValue(m_sActionTeleportToCursor))
			{
				//--- Teleport under cursor
				vector pos;
				if (!param.GetCursorWorldPos(pos))
					return;

				if (!TeleportCamera(pos))
					return;
			}
			if (m_sActionTeleportToPlayer && GetInputManager().GetActionValue(m_sActionTeleportToPlayer))
			{
				//--- Teleport to player (or on initial camera position if player is not present)
				vector pos = m_vInitPos;
				IEntity player = SCR_PlayerController.GetLocalMainEntity();
				if (player)
					pos = player.GetOrigin();
				
				if (!TeleportCamera(pos, false, true))
					return;
			}
		}
		
		//--- Start teleporting
		if (m_bIsTeleportInit)
		{
			//--- Calculate target rotation
			vector angles = param.transform[2].VectorToAngles().MapAngles();
			if (m_bIsDefaultAngle)
			{
				angles[1] = m_fDefaultAngle;
			}
			else if (angles[1] > -m_fMinAngle)
			{
				angles[1] = -m_fMinAngle; //--- Never look up
			}
			m_vTargetRot = angles.AnglesToVector();

			//--- Calculate target position. Don't make it further away than current distance from camera.
			float dis = m_fDistance;
			if (!m_bIsDefaultDistance)
			{
				dis = Math.Min(dis, vector.Distance(m_vTargetPos, param.transform[3]));
				
				//--- When closer than limit, factor in camera height as well
				if (dis < m_fDistance)
				{
					vector pos = param.transform[3];
					float height = pos[1] - param.world.GetSurfaceY(pos[0], pos[2]);
					dis = Math.Min(dis, height);
				}
				dis = Math.Max(dis, m_fMinDistance);
			}
			m_vTargetPos = m_vTargetPos - m_vTargetRot * dis;
			param.isManualInput = false; //--- Cancel manual input, it prevents inertia from catapulting camera far away
			
			//--- When further than given limit, teleport the camera nearby, otherwise start transition from the current position.
			if (!m_bIsStartAtCurrentPos && vector.Distance(m_vTargetPos, param.transform[3]) > m_fTeleportDistance)
			{
				//--- Modify original values, because normal ones get overridden later (see MatrixCopy)
				param.transformOriginal[2] = m_vTargetRot;
				vector dirTo = (m_vTargetPos - param.transformOriginal[3]).Normalized();
				param.transformOriginal[3] = m_vTargetPos - dirTo * m_fSettleInDistance;
			}
			
			m_bIsSettling = true;
			m_bIsTeleportInit = false;
		}
		
		if (param.isManualInput && !m_bDisableInterruption)
		{
			//--- Some other system moved the camera, terminate
			m_bIsSettling = false;
		}
		else if (m_bIsSettling)
		{
			//--- Erase previous changes (e.g., inertia)
			Math3D.MatrixCopy(param.transformOriginal, param.transform);
			
			//--- Settled at target position or cannot move further for some reason, terminate
			float dis = vector.Distance(param.transform[3], m_vTargetPos);
			if (vector.Distance(param.transform[3], m_vTargetPos) < SETTLE_THRESHOLD || Math.AbsFloat(dis - m_fDistancePrev) < SETTLE_THRESHOLD)
			{
				m_bIsSettling = false;
				return;
			}
			m_fDistancePrev = dis;
			
			//--- Play transition animation
			float progress = Math.Min(param.timeSlice / m_fTransitionStrength, 1);
			param.transform[2] = vector.Lerp(param.transform[2], m_vTargetRot, progress);
			param.transform[3] = vector.Lerp(param.transform[3], m_vTargetPos, progress);
			param.rotDelta = vector.Zero;
			param.isDirty = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_vInitPos = GetCameraEntity().GetOrigin();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraParentChange(bool attached, IEntity parent)
	{
		if (attached)
			m_vTargetPos = parent.CoordToLocal(m_vTargetPos);
		else
			m_vTargetPos = parent.CoordToParent(m_vTargetPos);
	}
}

//! @ingroup ManualCamera

//! Focus camera on cursor position.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_FocusManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(params: "layout")]
	private ResourceName m_Layout;
	
	[Attribute()]
	private string m_sSoundEvent;
	
	private bool m_bIsInit;
	private bool m_bIsFocus;
	private bool m_bIsFocusRequsted;
	private bool m_bIsFocusPosLocal;
	private vector m_vFocusPos;
	private vector m_vFocusPosWorld;
	private float m_fFocusDistance;
	private IEntity m_FocusEntity;
	private SCR_CameraPostProcessEffect m_Effect;
	private Widget m_Widget;
	protected ref ScriptInvoker m_OnFocusChange = new ScriptInvoker();
	protected ref ScriptInvoker m_OnFocusDistanceChange = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! Get focus position in the world.
	//! \param[out] outPos Focus position
	//! \return True if focus is set
	bool GetFocusPos(out vector outPos)
	{
		if (!m_bIsFocus)
			return false;
		
		outPos = m_vFocusPosWorld;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Distance to focused position.
	float GetFocusDistance()
	{
		return m_fFocusDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set focus to cursor position
	void SetFocusToCursor()
	{
		if (!IsEnabled())
			return;
		
		if (GetCameraEntity().GetCameraParam().GetCursorWorldPos(m_vFocusPos))
		{
			m_FocusEntity = GetCameraEntity().GetCameraParam().target;
			m_bIsFocusRequsted = true;
		}
		else
		{
			ResetFocus();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pos
	//! \param[in] entity
	void SetFocus(vector pos, IEntity entity = null)
	{
		m_vFocusPos = pos;
		m_FocusEntity = entity;
		m_bIsFocusRequsted = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Reset focus
	void ResetFocus()
	{
		if (m_bIsInit && (!m_bIsFocus || !IsEnabled()))
			return;
		
		if (m_bIsInit)
			m_OnFocusChange.Invoke(false, m_vFocusPos, m_FocusEntity);
		
		m_fFocusDistance = -1;
		
		if (m_bIsInit)
			m_OnFocusDistanceChange.Invoke();
		
		m_Effect.SetParam("FocalLength", 0);
		m_Effect.SetParam("FocalLengthNear", 0);
		
		m_bIsFocus = false;
		m_bIsFocusRequsted = false;
		m_bIsFocusPosLocal = false;
		m_vFocusPos = vector.Zero;
		m_vFocusPosWorld = vector.Zero;
		m_FocusEntity = null;
		if (m_Widget)
			m_Widget.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Get event called when focus is set or cleared.
	//! \return Script invoker
	ScriptInvoker GetOnFocusChange()
	{
		return m_OnFocusChange;
	}

	//------------------------------------------------------------------------------------------------
	//! Get event called when focus distance changes.
	//! \return Script invoker
	ScriptInvoker GetOnFocusDistanceChange()
	{
		return m_OnFocusDistanceChange;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraReset()
	{
		ResetFocus();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled)
		{
			if (m_Widget)
				m_Widget.SetVisible(false);
			
			return;
		}
		
		if (m_bIsFocusRequsted)
		{
			m_bIsFocusRequsted = false;
			
			m_bIsFocus = true;
			m_bIsFocusPosLocal = m_FocusEntity != null;
			if (m_bIsFocusPosLocal)
				m_vFocusPos = m_FocusEntity.CoordToLocal(m_vFocusPos);
			
			m_Effect.SetParam("FocalLength", 500);
			m_Effect.SetParam("FocalLengthNear", 500);
			
			if (!m_sSoundEvent.IsEmpty())
				SCR_UISoundEntity.SoundEvent(m_sSoundEvent);
			
			m_OnFocusChange.Invoke(true, m_vFocusPos, m_FocusEntity);
		}
		
		//--- Ignore when no custom focus was set
		if (!m_bIsFocus)
			return;
	
		//--- Get focus position
		m_vFocusPosWorld = m_vFocusPos;
		if (m_bIsFocusPosLocal)
		{
			if (m_FocusEntity)
			{
				//--- Set focus to position in target's coordinate space
				m_vFocusPosWorld = m_FocusEntity.CoordToParent(m_vFocusPos);
			}
			else
			{
				//--- Local target was deleted, reset focus
				ResetFocus();
				return;
			}
		}
		
		//--- Update post-process
		m_fFocusDistance = vector.Distance(m_vFocusPosWorld, CoordFromCamera(param.transform[3]));
		m_Effect.SetParam("FocusDistance", m_fFocusDistance);
		//m_Effect.SetParam("FocalChange", m_fFocusDistance);
		m_OnFocusDistanceChange.Invoke();
		
		//--- Update GUI
		if (m_Widget)
		{
			m_Widget.SetVisible(true);
			vector screenPos = m_Widget.GetWorkspace().ProjWorldToScreen(m_vFocusPosWorld, param.world);
			if (screenPos[2] > 0)
				FrameSlot.SetPos(m_Widget, screenPos[0], screenPos[1]);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		if (!m_bIsFocus)
			return;
		
		data.m_aValues = {m_vFocusPosWorld[0], m_vFocusPosWorld[1], m_vFocusPosWorld[2]};
		data.m_Target = m_FocusEntity;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		SetFocus(Vector(data.m_aValues[0], data.m_aValues[1], data.m_aValues[2]), data.m_Target);
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		SCR_PostProcessCameraComponent postProcessManager = SCR_PostProcessCameraComponent.Cast(GetCameraEntity().FindComponent(SCR_PostProcessCameraComponent));
		if (!postProcessManager)
		{
			Debug.Error("SCR_FocusManualCameraComponent requires SCR_PostProcessCameraComponent!");
			return false;
		}
		
		m_Effect = postProcessManager.FindEffect(PostProcessEffectType.DepthOfFieldBokeh);
		if (!m_Effect)
		{
			Debug.Error("SCR_FocusManualCameraComponent requires DepthOfField post-process effect!");
			return false;
		}
		
		InputManager inputManager = GetInputManager();
		if (!inputManager)
			return false;
		
		inputManager.AddActionListener("ManualCameraFocus", EActionTrigger.DOWN, SetFocusToCursor);
		inputManager.AddActionListener("ManualCameraFocusReset", EActionTrigger.DOWN, ResetFocus);
		
		m_Widget = GetCameraEntity().CreateCameraWidget(m_Layout, false);
		
		//--- Disable blur by default
		m_bIsInit = false;
		ResetFocus();
		m_bIsInit = true;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		InputManager inputManager = GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.RemoveActionListener("ManualCameraFocus", EActionTrigger.DOWN, SetFocusToCursor);
		inputManager.RemoveActionListener("ManualCameraFocusReset", EActionTrigger.DOWN, ResetFocus);

		if (m_Widget)
			m_Widget.RemoveFromHierarchy();
	}
}

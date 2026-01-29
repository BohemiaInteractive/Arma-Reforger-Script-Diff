/*!
This component is meant for cases when content (expecially text) is too long horizontally/vertically and doesn't fit into UI.
The component animates content left-right when activated.

!!! WARNING !!!
The content widget (the one which will be animated) must be in a Frame Slot!
In most basic case you should create a Frame, attach the component to it, and and put a text widget inside the Frame.

Root (this component need to be here)
	Frame
		ContentWrapper (must be a Frame Slot - will be the one being moved)
			elementsBeingScrolled
			elementsBeingScrolled
			elementsBeingScrolled
*/

[EnumLinear()]
enum SCR_EScrollAnimationState
{
	NONE,

	MOVING_LEFT,
	MOVING_LEFT_DONE_WAITING,
	MOVING_RIGHT,
	MOVING_RIGHT_DONE_WAITING,

	MOVING_TOP,
	MOVING_TOP_DONE_WAITING,
	MOVING_BOTTOM,
	MOVING_BOTTOM_DONE_WAITING
}

class SCR_HorizontalScrollAnimationComponent : ScriptedWidgetComponent
{
	[Attribute("Content", UIWidgets.EditBox, "Name of widget which will be animated")]
	protected string m_sAnimationContentWidget;

	[Attribute("10", UIWidgets.EditBox, "Animation speed")]
	protected float m_fAnimationSpeedPxPerSec;

	[Attribute("0.5", UIWidgets.EditBox, "Wait time while animating when content is on the left")]
	protected float m_fWaitTimeLeftSec;

	[Attribute("0.5", UIWidgets.EditBox, "Wait time while animating when content is on the right")]
	protected float m_fWaitTimeRightSec;

	[Attribute("false", UIWidgets.CheckBox, "When true, content will animate to start after it has animated left. Otherwise it will jump to start instantly.")]
	protected bool m_bAnimateBack;

	[Attribute("0", UIWidgets.CheckBox, "Inwards offset from right edge of root widget to be used in calculations")]
	protected float m_fOffsetRight;

	[Attribute("0", UIWidgets.CheckBox, "Inwards offset from left edge of root widget to be used in calculations")]
	protected float m_fOffsetLeft;

	[Attribute(typename.EnumToString(SCR_EScrollAnimationState, SCR_EScrollAnimationState.NONE), UIWidgets.ComboBox, "Start state of animation", "", ParamEnumArray.FromEnum(SCR_EScrollAnimationState))]
	protected SCR_EScrollAnimationState m_eInitialState;

	protected SCR_EScrollAnimationState m_eState;
	protected Widget m_wContent;
	protected Widget m_wRoot;

	protected float m_fStartPosX;
	protected float m_fStartPosY;
	protected float m_fTimer;

	protected float m_fTargetPosX;
	protected float m_fTargetPosY;

	protected bool m_bIsVerticalAnimation;

	//------------------------------------------------------------------------------------------------
	// P U B L I C

	//------------------------------------------------------------------------------------------------
	void AnimationStop()
	{
		m_eState = SCR_EScrollAnimationState.NONE;

		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(OnFrame);
	}

	//------------------------------------------------------------------------------------------------
	void AnimationStart(bool IsVerticalAnimation = false)
	{
		// If not animating, start animation
		if (m_eState == SCR_EScrollAnimationState.NONE)
		{
			ResetPosition();
			GetGame().GetCallqueue().CallLater(SetupValues, 0);
			GetGame().GetCallqueue().CallLater(OnFrame, 0, true);

			m_bIsVerticalAnimation = IsVerticalAnimation;

			if (m_bIsVerticalAnimation)
				m_eState = SCR_EScrollAnimationState.MOVING_TOP;
			else
				m_eState = SCR_EScrollAnimationState.MOVING_LEFT;
		}

		// Otherwise it's already animating in some state, do nothing
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAnimating()
	{
		return m_eState != SCR_EScrollAnimationState.NONE;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupValues()
	{
		float contentw, contenth, contentx, contenty, rootw, rooth, rootx, rooty;
		m_wContent.GetScreenSize(contentw, contenth);
		m_wContent.GetScreenPos(contentx, contenty);
		m_wRoot.GetScreenSize(rootw, rooth);
		m_wRoot.GetScreenPos(rootx, rooty);
		
		m_fStartPosX = FrameSlot.GetPosX(m_wContent);
		m_fStartPosY = FrameSlot.GetPosY(m_wContent);

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		m_fTargetPosX = rootw - contentw - workspace.DPIScale(m_fOffsetLeft);
		m_fTargetPosY = rooth - contenth + workspace.DPIScale(m_fOffsetLeft);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets X position to its initial value when component was initialized
	void ResetPosition()
	{
		if (!m_wContent)
			return;

		if (m_bIsVerticalAnimation)
			FrameSlot.SetPosY(m_wContent, m_fStartPosY);
		else
			FrameSlot.SetPosX(m_wContent, m_fStartPosX);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when content fits the parent widget
	bool GetContentFitX()
	{
		if (!m_wContent)
			return true;

		float rootw, rooth, contentw, contenth;
		m_wContent.GetScreenSize(contentw, contenth);
		m_wRoot.GetScreenSize(rootw, rooth);

		return rootw > contentw + GetGame().GetWorkspace().DPIScale(m_fOffsetLeft + m_fOffsetRight);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when content fits the parent widget
	bool GetContentFitY()
	{
		if (!m_wContent)
			return true;

		float rootw, rooth, contentw, contenth;
		m_wContent.GetScreenSize(contentw, contenth);
		m_wRoot.GetScreenSize(rootw, rooth);

		return rooth > contenth + GetGame().GetWorkspace().DPIScale(m_fOffsetLeft + m_fOffsetRight);
	}

	//------------------------------------------------------------------------------------------------
	// P R O T E C T E D

	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;

		m_wRoot = w;
		m_wContent = w.FindAnyWidget(m_sAnimationContentWidget);

		if (!m_wContent)
		{
			PrintFormat("[SCR_HorizontalScrollAnimationComponent] Widget %1 was not found", m_sAnimationContentWidget, level: LogLevel.ERROR);
			return;
		}

		m_eState = m_eInitialState;

		//use CallLater, it seems that Call() might not have a loop/repeat option
		if (m_eState != SCR_EScrollAnimationState.NONE)
		{
			GetGame().GetCallqueue().CallLater(SetupValues, 0);
			GetGame().GetCallqueue().CallLater(OnFrame, 0, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(OnFrame);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFrame()
	{
		float tDelta = ftime * 0.001;

		switch (m_eState)
		{
			case SCR_EScrollAnimationState.NONE:
				// Doing nothing
				break;

			case SCR_EScrollAnimationState.MOVING_LEFT:
			{
				if (AnimateHorizontalPosition(tDelta, -m_fAnimationSpeedPxPerSec, m_fTargetPosX))
				{
					m_eState = SCR_EScrollAnimationState.MOVING_LEFT_DONE_WAITING;
					m_fTimer = m_fWaitTimeLeftSec;
				}

				break;
			}

			case SCR_EScrollAnimationState.MOVING_LEFT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					if (m_bAnimateBack)
					{
						m_eState = SCR_EScrollAnimationState.MOVING_RIGHT;
					}
					else
					{
						ResetPosition();
						m_fTimer = m_fWaitTimeRightSec;
						m_eState = SCR_EScrollAnimationState.MOVING_RIGHT_DONE_WAITING;
					}
				}

				break;
			}

			case SCR_EScrollAnimationState.MOVING_RIGHT:
			{
				WorkspaceWidget workspace = GetGame().GetWorkspace();
				if (AnimateHorizontalPosition(tDelta, m_fAnimationSpeedPxPerSec, workspace.DPIScale(m_fOffsetRight)))
				{
					m_eState = SCR_EScrollAnimationState.MOVING_RIGHT_DONE_WAITING;
					m_fTimer = m_fWaitTimeRightSec;
				}

				break;
			}

			case SCR_EScrollAnimationState.MOVING_RIGHT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					m_eState = SCR_EScrollAnimationState.MOVING_LEFT;
				}

				break;
			}

			case SCR_EScrollAnimationState.MOVING_TOP:
			{
				if (AnimateVerticalPosition(tDelta, -m_fAnimationSpeedPxPerSec, m_fTargetPosY))
				{
					m_eState = SCR_EScrollAnimationState.MOVING_TOP_DONE_WAITING;
					m_fTimer = m_fWaitTimeLeftSec;
				}

				break;
			}

			case SCR_EScrollAnimationState.MOVING_TOP_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					if (m_bAnimateBack)
					{
						m_eState = SCR_EScrollAnimationState.MOVING_BOTTOM;
					}
					else
					{
						ResetPosition();
						m_fTimer = m_fWaitTimeRightSec;
						m_eState = SCR_EScrollAnimationState.MOVING_BOTTOM_DONE_WAITING;
					}
				}

				break;
			}

			case SCR_EScrollAnimationState.MOVING_BOTTOM:
			{
				WorkspaceWidget workspace = GetGame().GetWorkspace();
				if (AnimateVerticalPosition(tDelta, -m_fAnimationSpeedPxPerSec, workspace.DPIScale(m_fOffsetRight)))
				{
					m_eState = SCR_EScrollAnimationState.MOVING_BOTTOM_DONE_WAITING;
					m_fTimer = m_fWaitTimeRightSec;
				}

				break;
			}

			case SCR_EScrollAnimationState.MOVING_BOTTOM_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					m_eState = SCR_EScrollAnimationState.MOVING_TOP;
				}

				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnimateHorizontalPosition(float tDelta, float speed, float target)
	{
		float currentPos = FrameSlot.GetPosX(m_wContent);
		float newPos = currentPos + tDelta * speed;

		if ((currentPos - target) * (newPos - target) <= 0)
		{
			FrameSlot.SetPosX(m_wContent, target);
			return true; // reached destination !
		}
		else
		{
			FrameSlot.SetPosX(m_wContent, newPos);
			return false;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnimateVerticalPosition(float tDelta, float speed, float target)
	{
		float currentPos = FrameSlot.GetPosY(m_wContent);
		float newPos = currentPos + tDelta * speed;

		if ((currentPos - target) * (newPos - target) <= 0)
		{
			FrameSlot.SetPosY(m_wContent, target);
			return true; // reached destination !
		}
		else
		{
			FrameSlot.SetPosY(m_wContent, newPos);
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	static SCR_HorizontalScrollAnimationComponent FindComponent(notnull Widget w)
	{
		return SCR_HorizontalScrollAnimationComponent.Cast(w.FindHandler(SCR_HorizontalScrollAnimationComponent));
	}
}

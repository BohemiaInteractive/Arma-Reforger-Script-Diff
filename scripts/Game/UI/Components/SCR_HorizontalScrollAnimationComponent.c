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
enum EScrollAnimationState
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

	[Attribute(typename.EnumToString(EScrollAnimationState, EScrollAnimationState.NONE), UIWidgets.ComboBox, "Start state of animation", "", ParamEnumArray.FromEnum(EScrollAnimationState))]
	protected EScrollAnimationState m_eInitialState;

	protected EScrollAnimationState m_State;
	protected Widget m_wContent;
	protected Widget m_wRoot;
	
	protected float m_fStartPosX;
	protected float m_fStartPosY;
	protected float m_fTimer;
	
	protected bool m_bIsVerticalAnimation;

	//------------------------------------------------------------------------------------------------
	// P U B L I C

	//------------------------------------------------------------------------------------------------
	void AnimationStop()
	{
		m_State = EScrollAnimationState.NONE;
		
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(OnFrame);
	}

	//------------------------------------------------------------------------------------------------
	void AnimationStart(bool IsVerticalAnimation = false)
	{
		// If not animating, start animation
		if (m_State == EScrollAnimationState.NONE)
		{
			GetGame().GetCallqueue().CallLater(OnFrame, 0, true);
			
			m_bIsVerticalAnimation = IsVerticalAnimation;
			
			if (m_bIsVerticalAnimation)
				m_State = EScrollAnimationState.MOVING_TOP;
			else
				m_State = EScrollAnimationState.MOVING_LEFT;
		}

		// Otherwise it's already animating in some state, do nothing
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

		float rootw, rooth;
		m_wRoot.GetScreenSize(rootw, rooth);

		float contentw, contenth;
		m_wContent.GetScreenSize(contentw, contenth);

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		return rootw > contentw + workspace.DPIScale(m_fOffsetLeft + m_fOffsetRight);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when content fits the parent widget
	bool GetContentFitY()
	{
		if (!m_wContent)
			return true;

		float rootw, rooth;
		m_wRoot.GetScreenSize(rootw, rooth);

		float contentw, contenth;
		m_wContent.GetScreenSize(contentw, contenth);

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		return rooth > contenth + workspace.DPIScale(m_fOffsetLeft + m_fOffsetRight);
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
			Print(string.Format("[SCR_HorizontalScrollAnimationComponent] Widget %1 was not found", m_sAnimationContentWidget), LogLevel.ERROR);
			return;
		}

		m_fStartPosX = FrameSlot.GetPosX(m_wContent);
		m_fStartPosY = FrameSlot.GetPosY(m_wContent);

		m_State = m_eInitialState;
		
		//use CallLater, it seems that Call() might not have a loop/repeat option
		if (m_State != EScrollAnimationState.NONE)
			GetGame().GetCallqueue().CallLater(OnFrame, 0, true);
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

		switch (m_State)
		{
			case EScrollAnimationState.NONE:
				// Doing nothing
				break;

			case EScrollAnimationState.MOVING_LEFT:
			{
				// Moving left
				AnimateHorizontalPosition(tDelta, -m_fAnimationSpeedPxPerSec);

				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootw, rooth, rootx, rooty;
				float contentw, contenth, contentx, contenty;
				m_wRoot.GetScreenSize(rootw, rooth);
				m_wRoot.GetScreenPos(rootx, rooty);
				m_wContent.GetScreenSize(contentw, contenth);
				m_wContent.GetScreenPos(contentx, contenty);
				
				float curr = contentx + contentw;
				float target = rootx + rootw - workspace.DPIScale(m_fOffsetRight);

				if (curr < target)
				{
					m_State = EScrollAnimationState.MOVING_LEFT_DONE_WAITING;
					m_fTimer = m_fWaitTimeLeftSec;
				}

				break;
			}

			case EScrollAnimationState.MOVING_LEFT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					if (m_bAnimateBack)
					{
						m_State = EScrollAnimationState.MOVING_RIGHT;
					}
					else
					{
						ResetPosition();
						m_fTimer = m_fWaitTimeRightSec;
						m_State = EScrollAnimationState.MOVING_RIGHT_DONE_WAITING;
					}
				}
				break;
			}

			case EScrollAnimationState.MOVING_RIGHT:
			{
				// Moving right
				AnimateHorizontalPosition(tDelta, m_fAnimationSpeedPxPerSec);

				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootx, rooty;
				float contentx, contenty;
				m_wRoot.GetScreenPos(rootx, rooty);
				m_wContent.GetScreenPos(contentx, contenty);

				if (contentx > rootx)
				{
					m_State = EScrollAnimationState.MOVING_RIGHT_DONE_WAITING;
					m_fTimer = m_fWaitTimeRightSec;
				}

				break;
			}

			case EScrollAnimationState.MOVING_RIGHT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					m_State = EScrollAnimationState.MOVING_LEFT;
				}
				break;
			}

			case EScrollAnimationState.MOVING_TOP:
			{
				// Moving Up
				AnimateVerticalPosition(tDelta, -m_fAnimationSpeedPxPerSec);

				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootw, rooth, rootx, rooty;
				float contentw, contenth, contentx, contenty;
				m_wRoot.GetScreenSize(rootw, rooth);
				m_wRoot.GetScreenPos(rootx, rooty);
				m_wContent.GetScreenSize(contentw, contenth);
				m_wContent.GetScreenPos(contentx, contenty);
				
				float curr = contenty + contenth;
				float target = rooty + rooth - workspace.DPIScale(m_fOffsetRight);

				if (curr < target)
				{
					m_State = EScrollAnimationState.MOVING_TOP_DONE_WAITING;
					m_fTimer = m_fWaitTimeLeftSec;
				}

				break;
			}

			case EScrollAnimationState.MOVING_TOP_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					if (m_bAnimateBack)
					{
						m_State = EScrollAnimationState.MOVING_BOTTOM;
					}
					else
					{
						ResetPosition();
						m_fTimer = m_fWaitTimeRightSec;
						m_State = EScrollAnimationState.MOVING_BOTTOM_DONE_WAITING;
					}
				}
				break;
			}

			case EScrollAnimationState.MOVING_BOTTOM:
			{
				// Moving down
				AnimateVerticalPosition(tDelta, m_fAnimationSpeedPxPerSec);

				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootx, rooty;
				float contentx, contenty;
				m_wRoot.GetScreenPos(rootx, rooty);
				m_wContent.GetScreenPos(contentx, contenty);

				if (contenty > rooty)
				{
					m_State = EScrollAnimationState.MOVING_BOTTOM_DONE_WAITING;
					m_fTimer = m_fWaitTimeRightSec;
				}

				break;
			}

			case EScrollAnimationState.MOVING_BOTTOM_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					m_State = EScrollAnimationState.MOVING_TOP;
				}
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AnimateHorizontalPosition(float tDelta, float speed)
	{
		float currentPos = FrameSlot.GetPosX(m_wContent);
		float newPos = currentPos + tDelta * speed;
		FrameSlot.SetPosX(m_wContent, newPos);
	}

	//------------------------------------------------------------------------------------------------
	protected void AnimateVerticalPosition(float tDelta, float speed)
	{
		float currentPos = FrameSlot.GetPosY(m_wContent);
		float newPos = currentPos + tDelta * speed;
		FrameSlot.SetPosY(m_wContent, newPos);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_HorizontalScrollAnimationComponent FindComponent(notnull Widget w)
	{
		return SCR_HorizontalScrollAnimationComponent.Cast(w.FindHandler(SCR_HorizontalScrollAnimationComponent));
	}
}

//! Base class for tasks which return some target from combat component
class SCR_AICinematicLookAt : AITaskScripted
{
	protected SlotManagerComponent m_SlotManager;
	protected SCR_CharacterControllerComponent m_ControllerComponent;
	protected EntitySlotInfo m_PointOfViewStart;
	protected EntitySlotInfo m_PointOfViewEnd;
	protected TAnimGraphVariable m_LookAtVariable;
	protected CharacterAnimationComponent m_CharacterAnimation;
	protected vector m_vLook[4];
	protected vector m_vLookEnd[4];
	protected vector m_vLookAxis;
	protected ChimeraCharacter m_Player;
	protected vector m_vPlayerPosition[4];
	protected vector m_vPlayerHeadPosition;
	
	//-------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_SlotManager = SlotManagerComponent.Cast(owner.GetControlledEntity().FindComponent(SlotManagerComponent));

		if (m_SlotManager)
		{
			m_PointOfViewStart = m_SlotManager.GetSlotByName("PointOfViewStart");
			m_PointOfViewEnd = m_SlotManager.GetSlotByName("PointOfViewEnd");
		}
		
		m_CharacterAnimation = CharacterAnimationComponent.Cast(owner.GetControlledEntity().FindComponent(CharacterAnimationComponent));
		
		if (m_CharacterAnimation)
			m_LookAtVariable = m_CharacterAnimation.BindVariableBool("Look");
		
		m_ControllerComponent = SCR_CharacterControllerComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_CharacterControllerComponent));
	}
	
	
	//-------------------------------------------------------------------------------------------
	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_ControllerComponent)
			return ENodeResult.FAIL;
		
		if (!m_ControllerComponent.IsLoitering())
			return ENodeResult.FAIL;
		
		m_Player = ChimeraCharacter.Cast(EntityUtils.GetPlayer());

		if (!m_Player)
			return ENodeResult.FAIL;

		m_Player.GetTransform(m_vPlayerPosition);	
		
		// Compute lookAt region origin
		if (m_PointOfViewStart)
			m_PointOfViewStart.GetWorldTransform(m_vLook);

		if (m_PointOfViewEnd)
			m_PointOfViewEnd.GetWorldTransform(m_vLookEnd);

		m_vLookAxis = vector.Direction(m_vLook[3], m_vLookEnd[3]);
		m_vLookAxis.Normalize();

		if (Math3D.IntersectionSphereCone(m_vPlayerPosition[3], 1, m_vLook[3], m_vLookAxis, 1.1))
		{			
			m_vPlayerHeadPosition = m_Player.EyePosition();
			
			m_CharacterAnimation.SetIKTarget("HeadLook", "HeadLook", owner.GetControlledEntity().CoordToLocal(m_vPlayerHeadPosition), {0, 0, 0});

			if (m_LookAtVariable)
				m_CharacterAnimation.SetVariableBool(m_LookAtVariable, true);
		}
		else
		{
			if (m_LookAtVariable)
				m_CharacterAnimation.SetVariableBool(m_LookAtVariable, false);
		}
		
		return ENodeResult.RUNNING;
	}
	
	//-------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
    {
        return true;
    }

};
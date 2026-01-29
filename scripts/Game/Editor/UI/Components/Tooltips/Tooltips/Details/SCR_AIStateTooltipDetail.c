[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_AIStateTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute("")]
	protected LocalizedString m_sFollowingOrderStateText;

	[Attribute("")]
	protected LocalizedString m_sIdleStateText;

	[Attribute("")]
	protected LocalizedString m_sAutonomousStateText;
	
	protected SCR_AIGroupInfoComponent m_GroupInfoComponent;
	protected EGroupControlMode m_CurrentState;
	
	protected TextWidget m_Text;
	
	//protected TextWidget m_Text;
	protected ProgressBarWidget m_Bar;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (m_Text && m_GroupInfoComponent.GetGroupControlMode() != m_CurrentState)
			SetAIStateText(m_GroupInfoComponent.GetGroupControlMode());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetAIStateText(EGroupControlMode state)
	{
		m_CurrentState = state;
		
		if (m_CurrentState == EGroupControlMode.IDLE || m_CurrentState == EGroupControlMode.NONE)
		{
			m_Text.SetText(m_sIdleStateText);
			return;
		}
		else if (m_CurrentState == EGroupControlMode.AUTONOMOUS)
		{
			m_Text.SetText(m_sAutonomousStateText);
			return;
		}
		else if (m_CurrentState == EGroupControlMode.FOLLOWING_WAYPOINT)
		{
			m_Text.SetText(m_sFollowingOrderStateText);
			return;
		}
		else 
		{
			m_Text.SetText("DEBUG UNKNOWN STATE!");
			Print(string.Format("AIStateTooltipDetail, EGroupControlMode %1 is an unsupported AI state", typename.EnumToString(EGroupControlMode, m_CurrentState)), LogLevel.NORMAL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		SCR_EditableGroupComponent group = SCR_EditableGroupComponent.Cast(entity);
		if (!group)
		{
			//If player return
			if (entity.GetPlayerID() > 0)
				return false;
			
			group = SCR_EditableGroupComponent.Cast(entity.GetParentEntity());
			
			if (!group)
				return false;
		}
		
		m_GroupInfoComponent = SCR_AIGroupInfoComponent.Cast(group.GetOwner().FindComponent(SCR_AIGroupInfoComponent));
		if (!m_GroupInfoComponent)
			return false;
		
		m_Text = TextWidget.Cast(widget);		
		if (!m_Text)
			return false;
		
		SetAIStateText( m_GroupInfoComponent.GetGroupControlMode());
		
		return true;
	}
}

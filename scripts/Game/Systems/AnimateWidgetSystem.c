/*
Helper system to call widget animations updates
*/

class AnimateWidgetSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(ESystemLocation.Client)
			.AddPoint(ESystemPoint.Frame);
	}

	protected ref AnimateWidget m_Animator;

	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{
		super.OnInit();
		
		m_Animator = AnimateWidget.GetInstance();
		if (!m_Animator)
			m_Animator = new AnimateWidget();

		m_Animator.m_OnAnimatingStarted.Insert(OnAnimatingStarted);
		m_Animator.m_OnAnimatingCompleted.Insert(OnAnimatingCompleted);
		
		Enable(m_Animator.IsActive())
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		super.OnUpdate(args.GetPoint());
		
		m_Animator.UpdateAnimations(args.GetTimeSliceSeconds());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ShouldBeEnabledInEditMode()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAnimatingCompleted()
	{
		Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnimatingStarted()
	{
		Enable(true);
	}
}
class SCR_InputButtonAnimations : AnimateWidget
{
	//------------------------------------------------------------------------------------------------
	static SCR_InputButtonAlphaMaskAnimation ButtonAlphaMask(Widget w, float targetValue, float speed, bool resetOnComplete = false)
	{
		if (!PrepareAnimation(w, speed, SCR_InputButtonAlphaMaskAnimation))
			return null;

		SCR_InputButtonAlphaMaskAnimation anim = new SCR_InputButtonAlphaMaskAnimation(w, speed, targetValue, resetOnComplete);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_InputButtonColorAnimation ButtonColor(Widget widget, Color color, float speed)
	{
		if (!PrepareAnimation(widget, speed, SCR_InputButtonColorAnimation))
			return null;

		SCR_InputButtonColorAnimation anim = new SCR_InputButtonColorAnimation(widget, speed, color);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}
}
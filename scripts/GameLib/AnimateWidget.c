//------------------------------------------------------------------------------------------------
class AnimateWidget
{
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Solving this with an array resulted in a lot of calls, as stopping/getting animations is bruteforced. In Mod Manager menu, it was reported we reached 26 million .Get() calls, as we recreate the Widgets to update the lists of mods
	
	protected ref array<ref WidgetAnimationBase> m_aAnimations = {};
		
//---- REFACTOR NOTE END ----
	
	protected static AnimateWidget s_Instance;

	ref ScriptInvoker m_OnAnimatingStarted = new ScriptInvoker();
	ref ScriptInvoker m_OnAnimatingCompleted = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	static sealed AnimateWidget GetInstance() 
	{
		return s_Instance;
	}

	/*!
	Stop either all animations on the widget, or an animation of a specific type
	*/
	//------------------------------------------------------------------------------------------------
	static bool StopAnimation(Widget w, typename typeName)
	{
		if (!s_Instance || !w)
			return false;
		
		bool success = false;
		WidgetAnimationBase current;
		for (int i = s_Instance.m_aAnimations.Count() - 1; i >= 0; i--)
		{
			current = s_Instance.m_aAnimations.Get(i);
			if (current.GetWidget() != w)
				continue;

			if (!current.IsInherited(typeName))
				continue;
			
			s_Instance.m_aAnimations.Remove(i);
			success = true;
		}
		
		return success;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Stop animation
	\param anim Animation class
	\return True if the animation was removed
	*/
	static bool StopAnimation(WidgetAnimationBase anim)
	{
		if (!s_Instance)
			return false;
		
		int index = s_Instance.m_aAnimations.Find(anim);
		if (index == -1)
			return false;
		
		s_Instance.m_aAnimations.Remove(index);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Stop all animation on given widget.
	\param w Target widget
	*/
	static void StopAllAnimations(Widget w)
	{
		if (!s_Instance)
			return;
		
		for (int i = s_Instance.m_aAnimations.Count() - 1; i >= 0; i--)
		{
			if (s_Instance.m_aAnimations[i].GetWidget() == w)
				s_Instance.m_aAnimations.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	static bool IsAnimating(Widget w)
	{
		if (!w || !s_Instance)
			return false;
		
		foreach (WidgetAnimationBase animation: s_Instance.m_aAnimations)
		{
			if (animation.GetWidget() == w)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsAnimatingType(Widget w, typename type)
	{
		if (!w || !s_Instance)
			return false;
		
		foreach (WidgetAnimationBase animation: s_Instance.m_aAnimations)
		{
			if (animation.GetWidget() == w && animation.Type() == type)
				return true;
		}
		
		return false;
	}	

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationBase GetAnimation(Widget w, typename type)
	{
		if (!w || !s_Instance)
			return null;
		
		foreach (WidgetAnimationBase animation: s_Instance.m_aAnimations)
		{
			if (animation.GetWidget() == w && animation.Type() == type)
				return animation;
		}
		
		return null;
	}	
	
	/// Is 
	//------------------------------------------------------------------------------------------------
	static bool IsActive()
	{
		if (!s_Instance)
			return false;

		return !s_Instance.m_aAnimations.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	static bool AddAnimation(notnull WidgetAnimationBase animation)
	{
		if (!s_Instance || !animation || !animation.GetWidget() || animation.GetSpeed() <= 0)
			return false;

		StopAnimation(animation.GetWidget(), animation.Type());
		if (!IsActive())
			s_Instance.m_OnAnimatingStarted.Invoke();
		
		s_Instance.m_aAnimations.Insert(animation);
		
		return true;
	}

	// Visibility/colorization animations
	static WidgetAnimationBlurIntensity BlurIntensity(Widget widget, float targetValue, float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationBlurIntensity))
			return null;
		
		WidgetAnimationBlurIntensity anim = new WidgetAnimationBlurIntensity(widget, speed, targetValue);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}
	
	//------------------------------------------------------------------------------------------------
	static WidgetAnimationOpacity Opacity(Widget widget, float targetValue, float speed, bool toggleVisibility = false)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationOpacity))
			return null;

		WidgetAnimationOpacity anim = new WidgetAnimationOpacity(widget, speed, targetValue, toggleVisibility);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationImageSaturation Saturation(Widget widget, float targetValue, float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationImageSaturation))
			return null;

		WidgetAnimationImageSaturation anim = new WidgetAnimationImageSaturation(widget, speed, targetValue);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationAlphaMask AlphaMask(Widget widget, float targetValue, float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationAlphaMask))
			return null;

		WidgetAnimationAlphaMask anim = new WidgetAnimationAlphaMask(widget, speed, targetValue);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}
	
	//------------------------------------------------------------------------------------------------
	static WidgetAnimationColor Color(Widget widget, Color color, float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationColor))
			return null;

		WidgetAnimationColor anim = new WidgetAnimationColor(widget, speed, color);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	// Size and position animations

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationFrameSize Size(Widget widget, float size[2], float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationFrameSize))
			return null;

		WidgetAnimationFrameSize anim = new WidgetAnimationFrameSize(widget, speed, size);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationPosition Position(Widget widget, float position[2], float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationPosition))
			return null;

		WidgetAnimationPosition anim = new WidgetAnimationPosition(widget, speed, position);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationPadding Padding(Widget widget, float padding[4], float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationPadding))
			return null;

		WidgetAnimationPadding anim = new WidgetAnimationPadding(widget, speed, padding);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationLayoutFill LayoutFill(Widget widget, float targetValue, float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationLayoutFill))
			return null;

		WidgetAnimationLayoutFill anim = new WidgetAnimationLayoutFill(widget, speed, targetValue);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}

	//------------------------------------------------------------------------------------------------
	static WidgetAnimationImageRotation Rotation(Widget widget, float targetValue, float speed)
	{
		if (!PrepareAnimation(widget, speed, WidgetAnimationImageRotation))
			return null;

		WidgetAnimationImageRotation anim = new WidgetAnimationImageRotation(widget, speed, targetValue);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}
	
	//------------------------------------------------------------------------------------------------
	static WidgetAnimationValue Value(float defaultValue, float targetValue, float speed)
	{
		if (!s_Instance || speed <= 0)
			return null;

		WidgetAnimationValue anim = new WidgetAnimationValue(null, speed, targetValue, defaultValue);
		s_Instance.m_aAnimations.Insert(anim);
		return anim;
	}
	
	//! Make sure that animation can be played, and animator is ready to run it
	//------------------------------------------------------------------------------------------------
	protected static bool PrepareAnimation(Widget w, float speed, typename typeName)
	{
		if (!g_Game || !(g_Game.InPlayMode() || g_Game.GetWorldEditor()))
			return false;
		
		if (!s_Instance)
		{
			Print("[AnimateWidget.PrepareAnimation] AnimateWidget entity instance is missing " + w + "." + w.GetName(), LogLevel.WARNING);
			return false;
		}
		
		if (!w)
		{
			Print("[AnimateWidget.PrepareAnimation] Widget to animate is missing", LogLevel.WARNING);
			return false;
		}
		
		if (speed <= 0)
		{
			Print("[AnimateWidget.PrepareAnimation] Animation speed must be > 0 " + w + "." + w.GetName(), LogLevel.WARNING);
			return false;
		}

		// Stop animation of the same type
		foreach (int i, WidgetAnimationBase anim : s_Instance.m_aAnimations)
		{
			if (anim.GetWidget() != w || !anim.IsInherited(typeName))
				continue;

			anim.Stop();
			s_Instance.m_aAnimations.Remove(i);
			break;
		}

		// If there is no animation running, invoke that new animation is ready to run
		if (!IsActive())
			s_Instance.m_OnAnimatingStarted.Invoke();

		return true;
	}

	// Events
	
	//------------------------------------------------------------------------------------------------
	void UpdateAnimations(float timeSlice)
	{
		if (m_aAnimations.IsEmpty())
			m_OnAnimatingCompleted.Invoke();

		// Tick all animations
		bool finished;
		for (int i = m_aAnimations.Count() - 1; i >=0; i--)
		{
			finished = m_aAnimations[i].OnUpdate(timeSlice);
			if (finished)
				m_aAnimations.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void AnimateWidget()
	{
		if (!s_Instance)
			s_Instance = this;
	}
};
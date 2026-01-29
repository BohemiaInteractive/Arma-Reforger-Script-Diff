//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Incomplete: lacks support for custom curves (workbench even has an editor to visualize them), looping, flip flop, stop & resume/revert from reached value

//------------------------------------------------------------------------------------------------
enum EAnimationCurve
{
	LINEAR,
	EASE_IN_SINE,
	EASE_IN_QUAD,
	EASE_IN_CUBIC,
	EASE_IN_QUART,
	EASE_IN_EXPO,
	EASE_IN_BACK,
	EASE_IN_CIRC,
	EASE_IN_ELASTIC,
	EASE_IN_BOUNCE,
	EASE_OUT_SINE,
	EASE_OUT_QUAD,
	EASE_OUT_CUBIC,
	EASE_OUT_QUART,
	EASE_OUT_EXPO,
	EASE_OUT_BACK,
	EASE_OUT_CIRC,
	EASE_OUT_ELASTIC,
	EASE_OUT_BOUNCE,
	EASE_IN_OUT_SINE,
	EASE_IN_OUT_QUAD,
	EASE_IN_OUT_CUBIC,
	EASE_IN_OUT_QUART,
	EASE_IN_OUT_EXPO,
	EASE_IN_OUT_BACK,
	EASE_IN_OUT_CIRC,
	EASE_IN_OUT_ELASTIC,
	EASE_IN_OUT_BOUNCE,
}

//------------------------------------------------------------------------------------------------
class WidgetAnimationBase
{
	// Constants for easing calculations
	protected static const float C1 = 1.70158;
	protected static const float C2 = C1 * 1.525;
	protected static const float C3 = C1 + 1;
	protected static const float C4 = (2 * Math.PI) / 3;
	protected static const float C5 = (2 * Math.PI) / 4.5;
	protected static const float N1 = 7.5625;
	protected static const float D1 = 2.75;

	protected bool m_bRepeat;
	protected float m_fSpeed;
	protected float m_fCurrentProgress;
	protected float m_fValue;

	protected Widget m_wWidget;
	protected EAnimationCurve m_eCurve;
	
	protected ref ScriptInvoker m_OnStopped;
	protected ref ScriptInvoker m_OnCompleted;
	protected ref ScriptInvoker m_OnCycleCompleted; // When repeat cycle is done

	//------------------------------------------------------------------------------------------------
	float GetProgressValue(float t)
	{
		switch (m_eCurve)
		{
			case EAnimationCurve.LINEAR:
				return t;
			case EAnimationCurve.EASE_IN_SINE:
				return 1 - Math.Cos((t * Math.PI) / 2);
			case EAnimationCurve.EASE_IN_QUAD:
				return t * t;
			case EAnimationCurve.EASE_IN_CUBIC:
				return t * t * t;
			case EAnimationCurve.EASE_IN_QUART:
				return t * t * t * t;
			case EAnimationCurve.EASE_IN_EXPO:
				if (t == 0)
					return 0;
				return Math.Pow(2, 10 * t - 10);
			case EAnimationCurve.EASE_IN_BACK:
				return C3 * t * t * t - C1 * t * t;
			case EAnimationCurve.EASE_IN_CIRC:
				return 1 - Math.Sqrt(1 - Math.Pow(t, 2));
			case EAnimationCurve.EASE_IN_ELASTIC:
				if (t == 0)
					return 0;
				if (t == 1)
					return 1;
				return -Math.Pow(2, 10 * t - 10) * Math.Sin((t * 10 - 10.75) * C4);
			case EAnimationCurve.EASE_IN_BOUNCE:
    			return Math.Pow(2, 6 * (t - 1)) * Math.AbsFloat(Math.Sin( t * Math.PI * 3.5 ));
			case EAnimationCurve.EASE_OUT_SINE:
				return Math.Sin((t * Math.PI) / 2);
			case EAnimationCurve.EASE_OUT_QUAD:
				return 1 - (1 - t) * (1 - t);
			case EAnimationCurve.EASE_OUT_CUBIC:
				return 1 - Math.Pow(1 - t, 3);
			case EAnimationCurve.EASE_OUT_QUART:
				return 1 - Math.Pow(1 - t, 4);
			case EAnimationCurve.EASE_OUT_EXPO:
				if (t == 1)
					return 1;
				return 1 - Math.Pow(2, -10 * t);
			case EAnimationCurve.EASE_OUT_BACK:
				return 1 + C3 * Math.Pow(t - 1, 3) + C1 * Math.Pow(t - 1, 2);
			case EAnimationCurve.EASE_OUT_CIRC:
				return Math.Sqrt(1 - Math.Pow(t - 1, 2));
			case EAnimationCurve.EASE_OUT_ELASTIC:
				if (t == 0)
					return 0;
				if (t == 1)
					return 1;
				return Math.Pow(2, -10 * t) * Math.Sin((t * 10 - 0.75) * C4) + 1;
			case EAnimationCurve.EASE_OUT_BOUNCE:
    			return 1 - Math.Pow(2, -6 * t) * Math.AbsFloat(Math.Cos(t * Math.PI * 3.5));
			case EAnimationCurve.EASE_IN_OUT_SINE:
				return -(Math.Cos(Math.PI * t) - 1) / 2;
			case EAnimationCurve.EASE_IN_OUT_QUAD:
				if (t < 0.5)
					return 2 * t * t;
				return 1 - Math.Pow(-2 * t + 2, 2) / 2;
			case EAnimationCurve.EASE_IN_OUT_CUBIC:
				if (t < 0.5)
					return 4 * t * t * t;
				return 1 - Math.Pow(-2 * t + 2, 3) / 2;
			case EAnimationCurve.EASE_IN_OUT_QUART:
				if (t < 0.5)
					return 8 * t * t * t * t;
				return 1 - Math.Pow(-2 * t + 2, 4) / 2;
			case EAnimationCurve.EASE_IN_OUT_EXPO:
				if (t == 0)
					return 0;
				if (t == 1)
					return 1;
				if (t < 0.5)
					return Math.Pow(2, 20 * t - 10) / 2;
				return (2 - Math.Pow(2, -20 * t + 10)) / 2;
			case EAnimationCurve.EASE_IN_OUT_BACK:
				if (t < 0.5)
					return (Math.Pow(2 * t, 2) * ((C2 + 1) * 2 * t - C2)) / 2;
				return (Math.Pow(2 * t - 2, 2) * ((C2 + 1) * (t * 2 - 2) + C2) + 2) / 2;
			case EAnimationCurve.EASE_IN_OUT_CIRC:
				if (t < 0.5)
					return (1 - Math.Sqrt(1 - Math.Pow(2 * t, 2))) / 2;
				return (Math.Sqrt(1 - Math.Pow(-2 * t + 2, 2)) + 1) / 2;
			case EAnimationCurve.EASE_IN_OUT_ELASTIC:
				if (t == 0)
					return 0;
				if (t == 1)
					return 1;
				if (t < 0.5)
					return -(Math.Pow(2, 20 * t - 10) * Math.Sin((20 * t - 11.125) * C5)) / 2;
				return (Math.Pow(2, -20 * t + 10) * Math.Sin((20 * t - 11.125) * C5)) / 2 + 1;
			case EAnimationCurve.EASE_IN_OUT_BOUNCE:
				if (t < 0.5)
			        return 8 * Math.Pow(2, 8 * (t - 1)) * Math.AbsFloat(Math.Sin(t * Math.PI * 7));
				else
					return 1 - 8 * Math.Pow(2, -8 * t) * Math.AbsFloat(Math.Sin(t * Math.PI * 7));
		}
		return t;
	}
	
	//------------------------------------------------------------------------------------------------
	// Overridden by other animations
	protected void ReverseDirection();

	// Public API

	//------------------------------------------------------------------------------------------------
	void SetRepeat(bool repeat)
	{
		m_bRepeat = repeat;
	}

	//------------------------------------------------------------------------------------------------
	bool IsRepeating()
	{
		return m_bRepeat;
	}

	//------------------------------------------------------------------------------------------------
	void Stop()
	{
		if (m_OnStopped)
			m_OnStopped.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return m_fValue;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnStopped()
	{
		if (!m_OnStopped)
			m_OnStopped = new ScriptInvoker();
		return m_OnStopped;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCompleted()
	{
		if (!m_OnCompleted)
			m_OnCompleted = new ScriptInvoker();
		return m_OnCompleted;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCycleCompleted()
	{
		if (!m_OnCycleCompleted)
			m_OnCycleCompleted = new ScriptInvoker();
		return m_OnCycleCompleted;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurve(EAnimationCurve curve)
	{
		m_eCurve = curve;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDelay(float delay)
	{
		m_fCurrentProgress -= delay * m_fSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDelay()
	{
		if (m_fCurrentProgress >= 0)
			return 0;
		
		return m_fCurrentProgress / m_fSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpeed(float speed)
	{
		if (speed > 0)
			m_fSpeed = speed;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSpeed()
	{
		return m_fSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetWidget()
	{
		return m_wWidget;
	}
	
	// Events
	//------------------------------------------------------------------------------------------------
	bool OnUpdate(float timeSlice)
	{
		m_fCurrentProgress += timeSlice * m_fSpeed;
		if (m_fCurrentProgress > 1)
			m_fCurrentProgress = 1;
		
		if (m_fCurrentProgress < 0)
			return false;

		m_fValue = GetProgressValue(m_fCurrentProgress);
		
		// Perform widget animation
		Animate(m_fCurrentProgress == 1);
		
		if (m_fCurrentProgress < 1)
			return false;

		if (m_bRepeat)
		{
			m_fCurrentProgress = 0;
			ReverseDirection();
			
			if (m_OnCycleCompleted)
			{
				m_OnCycleCompleted.Invoke(this);
				return false;
			}
			
			// simply stop - item is not supposed to hang here forever without callback
			m_bRepeat = false;		
		}
		
		// Invoke subscribed changes
		if (m_OnCompleted)
			m_OnCompleted.Invoke(this);
		if (m_OnStopped)
			m_OnStopped.Invoke(this);

		return true;
	}

	// Interface method for widget animation
	//------------------------------------------------------------------------------------------------
	protected void Animate(bool finished)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationBase(Widget w, float speed)
	{
		m_wWidget = w;
		m_fSpeed = speed;
	}
};

//---- REFACTOR NOTE END ----
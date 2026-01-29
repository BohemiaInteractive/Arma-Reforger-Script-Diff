/*!
	This object implements a pseudorandom camera shake.
*/
class SCR_NoisyCameraShakeProgress : SCR_GenericCameraShakeProgress
{
	protected float m_fCurrent;
	protected float m_fVelocity;
	protected float m_fMaxVelocity;

	protected float m_fLinearMagnitude;
	protected float m_fAngularMagnitude;

	protected float m_fInTime;
	protected float m_fSustainTime;
	protected float m_fOutTime;

	protected float m_fElapsedTime;
	protected float m_fDuration;
	
	override void Update(IEntity owner, float timeSlice)
	{
		if (!IsRunning())
			return;

		float smoothTime;
		float targetValue;
		// Check which state of transition this shake is in
		if (m_fElapsedTime <= m_fInTime)
		{
			// Blend in
			targetValue = 1.0;
			smoothTime = m_fInTime;
		}
		else if (m_fElapsedTime <= m_fInTime + m_fSustainTime)
		{
			// Sustain
			targetValue = 1.0;
			smoothTime = m_fSustainTime;
		}
		else
		{
			// Blend out
			targetValue = 0.0;
			smoothTime = m_fOutTime;
		}

		// Update elapsed time
		m_fElapsedTime += timeSlice;

		// Interpolate progress
		m_fCurrent = Math.SmoothCD(m_fCurrent, targetValue, m_fVelocity, smoothTime, m_fMaxVelocity, timeSlice);

		// Calculate new state
		m_vTranslation = m_fLinearMagnitude * m_fCurrent *
		Vector(
			Math.RandomFloat(-0.005, 0.005),
			Math.RandomFloat(-0.005, 0.005),
			0.0
		);
		m_vRotation = m_fAngularMagnitude * m_fCurrent *
		Vector(
			Math.RandomFloat(-0.25, 0.25),
			Math.RandomFloat(-0.25, 0.25),
			Math.RandomFloat(-1, 1)
		);

		m_fFovScale = 1.0;

		// Finished, clear state
		if (targetValue <= 0 && m_fCurrent <= 0.0001)
		{
			m_bIsRunning = false;
			m_fCurrent = 0.0;
			m_fVelocity = 0.0;
			m_vTranslation = vector.Zero;
			m_vRotation = vector.Zero;
			m_fFovScale = 1.0;
		}
	}
	
	void SetParams(float linearMagnitude, float angularMagnitude, float inTime, float sustainTime, float outTime)
	{
		m_fLinearMagnitude = linearMagnitude;
		m_fAngularMagnitude = angularMagnitude;
		m_fInTime = inTime;
		m_fSustainTime = sustainTime;
		m_fOutTime = outTime;
	}

	override void Start()
	{
		if (m_fInTime <= 0.0 || m_fOutTime <= 0.0)
		{
			Print("Camera Shake: In and out time can not be less or equal to zero!", LogLevel.ERROR);
			m_bIsRunning = false;
			return;
		}
		
		// Immediate values
		m_fCurrent = 0.0;
		m_fMaxVelocity = 10.0;
		m_fElapsedTime = 0.0;
		m_fDuration = m_fInTime + m_fSustainTime + m_fOutTime;

		// Start progress
		m_bIsRunning = true;
	}
	
	override void Clear()
	{
		super.Clear();
		m_bIsRunning = false;
	}
};
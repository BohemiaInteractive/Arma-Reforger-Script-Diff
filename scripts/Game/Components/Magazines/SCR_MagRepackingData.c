class SCR_MagRepackingData
{
	float m_fTimeSinceLastTransfer;

	protected float m_fTimePerRound;
	protected SCR_MagazineComponent m_SourceMagazine;
	protected SCR_MagazineComponent m_TargetMagazine;
	protected vector m_vLastCharacterPosition;
	protected ECharacterStance m_eLastCharacterStance;

	//------------------------------------------------------------------------------------------------
	void SCR_MagRepackingData(float timePerRound, SCR_MagazineComponent sourceMag = null, SCR_MagazineComponent targetMag = null, vector lastPos = vector.Zero, ECharacterStance lastStance = ECharacterStance.STAND)
	{
		m_fTimePerRound = timePerRound;
		m_SourceMagazine = sourceMag;
		m_TargetMagazine = targetMag;
		m_vLastCharacterPosition = lastPos;
		m_eLastCharacterStance = lastStance;
	}

	//------------------------------------------------------------------------------------------------
	void SetTimePerRound(float newTime)
	{
		m_fTimePerRound = newTime;
	}

	//------------------------------------------------------------------------------------------------
	float GetTimePerRound()
	{
		return m_fTimePerRound;
	}

	//------------------------------------------------------------------------------------------------
	//! This is not null only for the host
	SCR_MagazineComponent GetSourceMagazine()
	{
		return m_SourceMagazine;
	}

	//------------------------------------------------------------------------------------------------
	//! This is not null only for the host
	SCR_MagazineComponent GetTargetMagazine()
	{
		return m_TargetMagazine;
	}

	//------------------------------------------------------------------------------------------------
	bool CheckPosition(vector currentPosition)
	{
		foreach (int i, float part : m_vLastCharacterPosition)
		{
			if (!float.AlmostEqual(currentPosition[i], part, 0.1)) // character can slide around a little bit when stopped so ignore minor changes - ~10cm
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	ECharacterStance GetLastStance()
	{
		return m_eLastCharacterStance;
	}
}

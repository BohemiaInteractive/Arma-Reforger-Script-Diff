/*!
Holds data about a cover point.
Performs automatic release of cover point upon its destruction.

Work flow when searching for covers:
1. Find cover with SCR_AIFindCover node.

2. The node assigns the found cover to CombatMoveState.

3. SCR_AICoverLock automatically locks the cover through CoverManager.

4. Move the AI to cover, and once he arrives, set m_bInCover=true in CombatMoveState.

5. As long as m_bInCover is true, CombatMoveState.VerifyCurrentCover() will be actively validating that AI is still at that cover.
The cover will be released when the AI is physically no longer there.
Alternatively, SCR_AICoverLock will release the cover once it is destroyed.
*/

class SCR_AICoverLock
{
	vector m_vCoverPos;
	vector m_vCoverTallestPos;
	
	vector m_vCoverDirXZ; // Cover direction vector in XZ plane
	
	int m_iTileX;
	int m_iTileY;
	int m_iCoverId;
	
	//----------------------------------------------------------------
	vector GetPosition()
	{
		return m_vCoverPos;
	}
	
	//----------------------------------------------------------------
	//! Returns true if cover position is not zero
	bool IsValid()
	{
		return m_vCoverPos != vector.Zero;
	}
	
	//----------------------------------------------------------------
	//! Unlocks the cover and invalidates it
	void Release()
	{
		ChimeraCoverManagerComponent coverMgr = GetCoverManager();
		if (!coverMgr)
			return;
		
		// Release cover
		if (m_vCoverPos != vector.Zero)
			coverMgr.SetOccupiedCover(m_iTileX, m_iTileY, m_iCoverId, false);
		
		// This object is invalid from now on
		m_vCoverPos = vector.Zero;
	}
	
	//----------------------------------------------------------------
	void SCR_AICoverLock(int tilex, int tiley, int coverId, vector coverPos, vector coverTallestPos)
	{
		ChimeraCoverManagerComponent coverMgr = GetCoverManager();
		if (!coverMgr)
			return;
		
		m_iTileX = tilex;
		m_iTileY = tiley;
		m_iCoverId = coverId;
		m_vCoverPos = coverPos;
		m_vCoverTallestPos = coverTallestPos;
		
		// Occupy cover on construction
		if (m_vCoverPos != vector.Zero)
			coverMgr.SetOccupiedCover(m_iTileX, m_iTileY, m_iCoverId, true);
		
		// Calculate derived values
		vector coverDirXZ = m_vCoverTallestPos - m_vCoverPos;
		coverDirXZ[1] = 0;
		coverDirXZ.Normalize();
		m_vCoverDirXZ = coverDirXZ;
	}
	
	//----------------------------------------------------------------
	void ~SCR_AICoverLock()
	{
		Release();
	}
	
	//----------------------------------------------------------------
	//! Calculates cos of angle between cover direction and threat direction, in XZ plane
	float CosAngleToThreat(vector threatPos)
	{
		vector dirCoverToThreatXZ = threatPos - m_vCoverPos;
		dirCoverToThreatXZ[1] = 0;
		dirCoverToThreatXZ.Normalize();
		float cosAngleCoverDirThreatDir = vector.Dot(m_vCoverDirXZ, dirCoverToThreatXZ);
		return cosAngleCoverDirThreatDir;
	}
	
	//----------------------------------------------------------------
	protected static ChimeraCoverManagerComponent GetCoverManager()
	{
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return null;
		ChimeraCoverManagerComponent coverMgr = ChimeraCoverManagerComponent.Cast(aiWorld.FindComponent(CoverManagerComponent));
		return coverMgr;
	}
}
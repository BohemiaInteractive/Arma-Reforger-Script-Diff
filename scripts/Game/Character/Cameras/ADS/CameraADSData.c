//! Holds data for ADS camera which can be reused for different solving methods
class ADSCameraData
{
	float m_fDeltaTime;
	float m_fFOV;
	float m_fCamRecoilAmount;
	vector m_vLookAngles;
	vector m_vSightsOffset;
	vector m_mSightsLocalMat[4];
	bool m_bFreeLook;
}

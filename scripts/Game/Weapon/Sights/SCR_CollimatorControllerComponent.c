[ComponentEditorProps(category: "Weapon/Sights", description: "")]
class SCR_CollimatorControllerComponentClass : ScriptComponentClass
{
};

class SCR_CollimatorControllerComponent : ScriptComponent
{
	float m_fUCoord;
	float m_fVCoord;
	float m_fUScale;
	float m_fVScale;
	float m_fEmissiveLV;
	vector m_vEmissive;
	vector m_vColor;
	int m_ReticleMap;
	float m_fAlphaMul;
	
	
	void UpdateUVCoordinates(float u, float v, float uScale, float vScale)
	{
		m_fUCoord = u;
		m_fVCoord = v;
		m_fUScale = uScale;
		m_fVScale = vScale;
	}
	
	void SetReticleBrightness(float br)
	{
		m_fEmissiveLV = br;
	}
	
	void SetReticleColors(vector color, vector emissiveColor)
	{
		m_vColor = color;
		m_vEmissive = emissiveColor;
	}

	void SetReticleIndex(int i)
	{
		m_ReticleMap = i;
	}
	
	void SetReticleBrightnessScale(float scale)
	{
		m_fAlphaMul = scale;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		UpdateUVCoordinates(0.0, 0.0, 1.0, 1.0);
		SetReticleBrightness(9);
		SetReticleColors("0.318 0.561 0.071 1", "0.013 1 0.04 0");
		SetReticleIndex(0);
		SetReticleBrightnessScale(1.0);
	}
};

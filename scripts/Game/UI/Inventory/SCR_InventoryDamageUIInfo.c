[BaseContainerProps()]
class SCR_InventoryDamageUIInfo : UIInfo
{
	bool m_bDamageIconVisible;
	bool m_bDamageRegenerating;
	bool m_bBleedingIconVisible;
	bool m_bTourniquetIconVisible;
	bool m_bSalineBagIconVisible;
	bool m_bMorphineIconVisible;
	bool m_bFractureIconVisible;
	bool m_bFractureIcon2Visible;
	string m_sDamageIntensity;
	string m_sDamageText;
	string m_sBleedingText;
	
	void SCR_InventoryDamageUIInfo(bool damageIconVisible, bool regeneratingVisible, bool bleedingIconVisible, bool tourniquetIconVisible, bool salineBagIconVisible, bool morphineIconVisible, bool fractureIconVisible, bool fractureIcon2Visible, string damageIntensity, string damageText, string bleedingText)
	{
		m_bDamageIconVisible = damageIconVisible;
		m_bDamageRegenerating = regeneratingVisible;
		m_bBleedingIconVisible = bleedingIconVisible;
		m_bTourniquetIconVisible = tourniquetIconVisible;
		m_bSalineBagIconVisible = salineBagIconVisible;	
		m_bMorphineIconVisible = morphineIconVisible;	
		m_bFractureIconVisible = fractureIconVisible;
		m_bFractureIcon2Visible = fractureIcon2Visible;
		m_sDamageIntensity = damageIntensity;
		m_sDamageText = damageText;	
		m_sBleedingText = bleedingText;
	}
}
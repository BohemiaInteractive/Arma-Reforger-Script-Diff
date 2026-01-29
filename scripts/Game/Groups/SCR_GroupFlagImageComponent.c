class SCR_GroupFlagImageComponent : SCR_ButtonImageComponent
{
	protected int m_iPicID = -1;
	protected ResourceName m_sImageSet;
	protected bool m_bIsFromImageset;

	[Attribute("128 80 0")]
	protected vector m_vImageSize; //160 90 old values, caused stretching
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		Resize();	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name
	void SetFlagButtonFromImageSet(string name)
	{		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!playerFaction)
			return;		
		
		ResourceName res = playerFaction.GetGroupFlagImageSet();
		if (!res)
			return;
		
		m_wImage.LoadImageFromSet(0, res, name);
		
		Resize();	
	}
	
	//------------------------------------------------------------------------------------------------	
	//! \param[in] name
	void SetFlagButtonFromTexture(ResourceName name)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!playerFaction)
			return;
		
		ResourceName imageSetResource = playerFaction.GetGroupFlagImageSet();
		array<ResourceName> textures = {};
		playerFaction.GetGroupFlagTextures(textures);
		
		m_wImage.LoadImageTexture(0, textures[textures.Find(name)]);
		
		Resize();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] scale
	void Resize(float scale = 1)
	{
		m_wImage.SetSize(m_vImageSize[0] * scale, m_vImageSize[1] * scale);		
	}
	
	//------------------------------------------------------------------------------------------------		
	//! \param[in] value
	void SetIsFromImageset(bool value)
	{
		m_bIsFromImageset = value;
	}
	
	//------------------------------------------------------------------------------------------------		
	//! \return
	bool GetIsFromImageset()
	{
		return m_bIsFromImageset;
	}
	
	//------------------------------------------------------------------------------------------------		
	//! \return
	int GetImageID()
	{
		return m_iPicID;
	}
	
	//------------------------------------------------------------------------------------------------		
	//! \param[in] ID
	void SetImageID(int ID)
	{
		m_iPicID = ID;
	}
	
	//------------------------------------------------------------------------------------------------		
	//! \param[in] imageSet
	void SetImageSet(ResourceName imageSet)
	{
		m_sImageSet = imageSet;
	}

	//------------------------------------------------------------------------------------------------		
	//! \return
	ResourceName GetImageSet()
	{
		return m_sImageSet;
	}
}

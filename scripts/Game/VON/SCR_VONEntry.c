//------------------------------------------------------------------------------------------------
//! Voice over network entry data class, used for management of communication methods
class SCR_VONEntry : SCR_SelectionMenuEntry
{
	protected bool m_bIsUsable = true; 	// whether this entry is allowed to be used (f.e. radio turned off)
	protected bool m_bIsActive;		// this entry will be used in case of outgoing transmission
	protected bool m_bIsMuted; 		// this transceiver entry is muted
	protected bool m_bIsSelected; 	// this entry is currently selected/hovered in a menu
	protected string m_sText;  	 	// current display text
			
	//------------------------------------------------------------------------------------------------
	// Init entry data
	void InitEntry()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Usable entry 
	void SetUsable(bool state)
	{
		m_bIsUsable = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is usable entry 
	bool IsUsable()
	{
		return m_bIsUsable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate entry 
	void SetActive(bool state)
	{
		m_bIsActive = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is active entry 
	bool IsActive()
	{
		return m_bIsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate entry 
	void SetMuted(bool state)
	{
		m_bIsMuted = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry selection state 
	void SetSelected(bool state)
	{
		m_bIsSelected = state;
	}
	
	//------------------------------------------------------------------------------------------------ 
	//! Adjust entry configuration such as radio frequency
	//! \param modifier is custom logic modifier 
	void AdjustEntry(int modifier)
	{}
	
	//------------------------------------------------------------------------------------------------ 
	//! Adjust entry configuration such as radio frequency, modified input
	//! \param modifier is custom logic modifier 
	void AdjustEntryModif(int modifier)
	{}
	
	//------------------------------------------------------------------------------------------------ 
	//! Toggle entry such as radio on/off
	void ToggleEntry()
	{}
			
	//------------------------------------------------------------------------------------------------
	//! Get entry display text such as current frequency
	string GetDisplayText()
	{
		return m_sText;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get icon resource path string
	string GetIconResource()
	{
		return string.Empty;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! VON method type
	ECommMethod GetVONMethod()
	{
		return ECommMethod.DIRECT;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Called only when update is requested
	override void Update()
	{		
	}
};

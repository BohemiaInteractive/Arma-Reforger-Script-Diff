enum EInventoryButton
{
	BUTTON_PAGE_PREV,
	BUTTON_PAGE_NEXT,
	BUTTON_TRAVERSE_BACK,
	BUTTON_SHOW_DEFAULT
};


//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory button for various UI storages operations.
class SCR_InventoryButton : BasicButtonComponent
{
	
	protected SCR_InventoryMenuUI							m_pMenuHandler;
	protected SCR_InventoryStorageBaseUI					m_pStorageUI = null;
	protected EInventoryButton								m_EButtonType;
	protected Widget										m_widget;
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
		
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		switch ( m_EButtonType )
		{
			case EInventoryButton.BUTTON_PAGE_PREV: 
			{ 
				if ( m_pStorageUI )
					m_pStorageUI.Action_PrevPage(); 
			} break;
			case EInventoryButton.BUTTON_PAGE_NEXT: 
			{ 
				if ( m_pStorageUI )
					m_pStorageUI.Action_NextPage(); 
			} break;
			
			case EInventoryButton.BUTTON_TRAVERSE_BACK: 
			{ 
				if ( !m_pStorageUI )
					return false;
				if ( !m_pMenuHandler )
					return false;
			//	m_pMenuHandler.SetActiveStorage( m_pStorageUI );
				m_pMenuHandler.Action_StepBack();
			} break;
			
			/*case EInventoryButton.BUTTON_SHOW_DEFAULT:
			{
				if ( m_pMenuHandler )
					m_pMenuHandler.ShowDefault( );
			} break;*/
			
		}
		return true;
	}
		
	//------------------------------------------------------------------------------------------------	
	override bool OnFocus(Widget w, int x, int y)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool OnFocusLost(Widget w, int x, int y)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool OnChange(Widget w, bool finished)
	{
	}
		
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		m_widget = w;
	}
			
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryButton( EInventoryButton EButtonType, SCR_InventoryMenuUI pMenuHandler = null, SCR_InventoryStorageBaseUI pStorageUI = null )
	{
		m_pStorageUI = pStorageUI;
		m_pMenuHandler = pMenuHandler;
		//store the item into the slot
		m_EButtonType = EButtonType;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryButton()
	{
	}
};
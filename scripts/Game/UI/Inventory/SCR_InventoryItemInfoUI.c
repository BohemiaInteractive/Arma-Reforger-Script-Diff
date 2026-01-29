[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory Item Info UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventoryItemInfoUI : ScriptedWidgetComponent
{
	protected Widget						m_infoWidget;
	protected VerticalLayoutWidget			m_wHintWidget;
	protected TextWidget					m_wTextName;
	protected TextWidget					m_wTextDescription;
	protected TextWidget					m_wTextWeight;
	protected ImageWidget 					m_wItemIcon;
	protected SCR_SlotUIComponent			m_pFrameSlotUI;
	protected Widget 						m_wWidgetUnderCursor;
	protected bool 							m_bForceShow;

	protected ResourceName					m_sAmmoTypeConfig = "{8D3E102893955B15}Configs/Inventory/ItemHints/MagazineAmmoType.conf";
	protected ref SCR_AmmoTypeInfoConfig	m_AmmoTypeConf;
	
	protected const string 					m_sHintLayout = "{9996B50BE8DFED5F}UI/layouts/Menus/Inventory/InventoryItemHintElement.layout";
	
	protected const string 					m_sIdentityItemLayout = "{9168128FD3D1FBC2}UI/layouts/Menus/Inventory/InventoryIdentityItem.layout";
	protected const string					m_sIdentityWidgetHolderName = "IdentityHolder";
	
					
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
	//------------------------------------------------------------------------------------------------
	protected void ShowInfoWidget( bool bShow )
	{
		if (m_bForceShow)
		{
			m_infoWidget.SetVisible( true );
			return;
		}

		if ( !m_wWidgetUnderCursor )
			return;
		if ( WidgetManager.GetWidgetUnderCursor() != m_wWidgetUnderCursor )
			return; //the cursor is on different position already
		m_infoWidget.SetVisible( true );
	}

	//------------------------------------------------------------------------------------------------
	void Show( float fDelay = 0.0, Widget w = null, bool forceShow = false )
	{
		m_bForceShow = forceShow;
		m_wWidgetUnderCursor = w;
		if ( fDelay == 0 )
			ShowInfoWidget( true );
		else
		{
			GetGame().GetCallqueue().Remove( ShowInfoWidget );
			GetGame().GetCallqueue().CallLater( ShowInfoWidget, fDelay*1000, false, true );
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetIcon(ResourceName iconPath, Color color = null)
	{
		if (iconPath.IsEmpty())
			return;
		
		m_wItemIcon.SetVisible(m_wItemIcon.LoadImageTexture(0, iconPath));
		if (color)
			m_wItemIcon.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void ShowIcon(bool isVisible)
	{
		m_wItemIcon.SetVisible(isVisible);
	}
	
	//------------------------------------------------------------------------------------------------
	void Hide( float fDelay = 1.0 )
	{
		m_infoWidget.SetVisible( false );
		m_infoWidget.SetEnabled( false );
		m_wItemIcon.SetVisible(false);
		
		Widget childWidget = m_wHintWidget.GetChildren();
		while (childWidget)
		{
			Widget next = childWidget.GetSibling();
			m_wHintWidget.RemoveChild(childWidget);
			childWidget = next;
		}

		GetGame().GetCallqueue().Remove(ShowInfoWidget);
	}
		
	//------------------------------------------------------------------------------------------------
	void Move( float x, float y )
	{
		if ( !m_pFrameSlotUI )
			return;
		m_pFrameSlotUI.SetPosX( x );
		m_pFrameSlotUI.SetPosY( y );
	}

	//------------------------------------------------------------------------------------------------
	void SetName( string sName )
	{
		m_wTextName.SetText( sName );
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDescription( string sDescr )
	{
		if( sDescr != "" )
		{
			m_wTextDescription.SetEnabled( true );
			m_wTextDescription.SetVisible( true );
			m_wTextDescription.SetText( sDescr );
		}
		else
		{
			m_wTextDescription.SetEnabled( false );
			m_wTextDescription.SetVisible( false );
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	void SetItemHints(InventoryItemComponent item, array<SCR_InventoryItemHintUIInfo> itemHintArray, SCR_InventorySlotUI focusedSlot)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		//~ Clear existing hints if any
		Widget hintChild = m_wHintWidget.GetChildren();
		Widget deleteHint;
		while(hintChild)
		{
			deleteHint = hintChild;
			hintChild = deleteHint.GetSibling();
		
			delete deleteHint;
		}
		
		foreach (SCR_InventoryItemHintUIInfo hintUIInfo : itemHintArray)
		{
			if (!hintUIInfo.CanBeShown(item, focusedSlot))
				continue;
			
			Widget createdWidget = workspace.CreateWidgets(m_sHintLayout, m_wHintWidget);
			if (!createdWidget)
				return;
			
			hintUIInfo.SetItemHintNameTo(item, RichTextWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintText")));
			hintUIInfo.SetIconTo(ImageWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintIcon")));
		}

		SetItemHints(item, false);
	}
	
	void SetItemHints(InventoryItemComponent item, bool deleteSlots = true)
	{
		if (!m_AmmoTypeConf)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_sAmmoTypeConfig);
			if (!holder || !holder.IsValid())
				return;	
	
			BaseContainer container = holder.GetResource().ToBaseContainer();
			if (!container)
				return;
			
			m_AmmoTypeConf = SCR_AmmoTypeInfoConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			if (!m_AmmoTypeConf)
				return;
		}
		
		if (deleteSlots)
		{
			Widget hintChild = m_wHintWidget.GetChildren();
			Widget deleteHint;
			while(hintChild)
			{
				deleteHint = hintChild;
				hintChild = deleteHint.GetSibling();
			
				delete deleteHint;
			}		
		}
		
		if (!item || !item.GetOwner())
			return;

		EAmmoType flags;
		
		MagazineComponent mag = MagazineComponent.Cast(item.GetOwner().FindComponent(MagazineComponent));
		if (mag)
		{
			MagazineUIInfo ammoInfo = MagazineUIInfo.Cast(mag.GetUIInfo());
			if (!ammoInfo)
				return;

			flags = ammoInfo.GetAmmoTypeFlags();
		}

		WeaponComponent weapon = WeaponComponent.Cast(item.GetOwner().FindComponent(WeaponComponent));
		if (weapon)
		{
			GrenadeUIInfo ammoInfo = GrenadeUIInfo.Cast(weapon.GetUIInfo());
			if (!ammoInfo)
				return;
			
			flags = ammoInfo.GetAmmoTypeFlags();
		}

		array<EAmmoType> values = {};
		SCR_Enum.GetEnumValues(EAmmoType, values);
		foreach (EAmmoType flag : values)
		{
			if (flags & flag)
			{
				Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_sHintLayout, m_wHintWidget);
				if (!createdWidget)
					return;
		
				RichTextWidget text = RichTextWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintText"));
				ImageWidget icon = ImageWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintIcon"));		
				
				m_AmmoTypeConf.SetIconAndDescriptionTo(flag, icon, text);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetWeight( string sWeight )
	{
		if (!sWeight.IsEmpty())
		{
			m_wTextWeight.SetEnabled( true );
			m_wTextWeight.SetVisible( true );
			m_wTextWeight.SetTextFormat("#AR-ValueUnit_Short_Kilograms", sWeight);
		}
		else
		{
			m_wTextWeight.SetEnabled( false );
			m_wTextWeight.SetVisible( false );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWeight( float fWeight )
	{
		if( fWeight <= 0.0 )
			SetWeight( "" );						
		else
			SetWeight( fWeight.ToString() );
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates the identity UI when hovering over the identity item in the player inventory
	//! \param[in] identityItemComponent Identity component of the linked character
	void CreateIdentityUI(SCR_IdentityInventoryItemComponent identityItemComponent)
	{
		if (!m_infoWidget)
			return;
		
		Widget identityHolder = m_infoWidget.FindAnyWidget(m_sIdentityWidgetHolderName);
		if (!identityHolder)
			return;
		
		//~ Clear existing identity if any
		SCR_WidgetHelper.RemoveAllChildren(identityHolder);
		
		//~ Check if can create Identity
		if (!identityItemComponent)
			return;
		
		SCR_ExtendedIdentityComponent linkedIdentity = identityItemComponent.GetLinkedExtendedIdentity();
		if (!linkedIdentity)
			return;
		
		SCR_IdentityBio identityBio = linkedIdentity.GetIdentityBio();
		if (!identityBio)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		//~ Create Identity
		Widget createdWidget = workspace.CreateWidgets(m_sIdentityItemLayout, identityHolder);
		if (!createdWidget)
			return;
		
		SCR_InventoryIdentityItemWidgetComponent identityItemWidgetComponent = SCR_InventoryIdentityItemWidgetComponent.Cast(createdWidget.FindHandler(SCR_InventoryIdentityItemWidgetComponent));
		if (!identityItemWidgetComponent)
		{
			delete createdWidget;
			return;
		}
		
		identityItemWidgetComponent.SetIdentityData(identityItemComponent, linkedIdentity, identityBio);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		if( !w )
			return;
		
		m_infoWidget		= w;
		m_wHintWidget		= VerticalLayoutWidget.Cast( w.FindAnyWidget( "VerticalHintParent" ) );
		m_wTextName 		= TextWidget.Cast( w.FindAnyWidget( "ItemInfo_name" ) );
		m_wTextDescription 	= TextWidget.Cast( w.FindAnyWidget( "ItemInfo_description" ) );
		m_wTextWeight 		= TextWidget.Cast( w.FindAnyWidget( "ItemInfo_weight" ) );
		m_wItemIcon 		= ImageWidget.Cast(w.FindAnyWidget("ItemInfo_icon"));
		
		Widget wItemInfo	= m_infoWidget.FindAnyWidget( "ItemInfo" );
		if ( !wItemInfo )
			return;
		m_pFrameSlotUI 		= SCR_SlotUIComponent.Cast( wItemInfo.FindHandler( SCR_SlotUIComponent ) );
	}
	
	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		if ( m_infoWidget )
		{		
			GetGame().GetCallqueue().Remove( ShowInfoWidget );
			m_infoWidget.RemoveHandler( m_pFrameSlotUI );
			m_infoWidget.RemoveHandler( this );
			m_infoWidget.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetInfoWidget()
	{
		return m_infoWidget.FindAnyWidget("size");
	}

	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
			
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryItemInfoUI()
	{
	}	

	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryItemInfoUI()
	{
	}
};
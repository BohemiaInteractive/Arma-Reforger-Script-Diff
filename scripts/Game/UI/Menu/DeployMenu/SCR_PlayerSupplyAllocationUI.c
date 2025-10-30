//PlayerSupplyAllocation Widget Handler

class SCR_PlayerSupplyAllocationUI : ScriptedWidgetComponent
{

	//Supplies text widget for showing up maximum amount of supplies Allocation and current supplies amount
	[Attribute("MSARSuppliesText", desc: "Text for displaying supplies related info to player")]	
	protected ResourceName m_sSuppliesText;
	
	//Number of currently available allocated supplies
	[Attribute("0", desc: "Available supplies Count")]	
	protected int m_iAvailableSuppliesAmount;
	
	//Current allocated supplies limit
	[Attribute("0", desc: "Allocated supplies limit")]	
	protected int m_iMaxAvailableSuppliesAmount;
		
	//Switch for allowing visible top limit of supplies, if set to true - there will be also top limit visible in inventory at all times no matter this setting
	[Attribute("0", desc: "Bool for switching between supplies amount display modes")]	
	protected bool m_bVisibleTopLimit;
	
	protected RichTextWidget m_wSuppliesText;

	protected string m_sSuppliesAmount;
	
	[Attribute("#AR-supplies_MSAR_Availability")]
	protected string m_sAvailableText;
	
	[Attribute("#AR-supplies_MSAR_Availability_Long")]
	protected string m_sAvailableTextLong;
	
	protected PlayerController m_PlayerController = GetGame().GetPlayerController();
	
	protected SCR_PlayerSupplyAllocationComponent m_PlayerSupplyAllocationComponent;

	//------------------------------------------------------------------------------------------------
	//! Calls superclass method, subscribes to events and sets player current and limit allocation.
	//! \param[in] w Widget attached to the method, representing the root of the layout.
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (SCR_Global.IsEditMode() || !m_PlayerController)
		{
			w.SetVisible(false);
			return;
		}

		m_PlayerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (!m_PlayerSupplyAllocationComponent || !SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
		{
			w.SetVisible(false);
			return;
		}

		w.SetVisible(true);
		m_wSuppliesText = RichTextWidget.Cast(w.GetParent().FindAnyWidget(m_sSuppliesText));

		m_PlayerSupplyAllocationComponent.GetOnAvailableAllocatedSuppliesChanged().Insert(OnAvailableAllocatedSuppliesChanged);
		m_PlayerSupplyAllocationComponent.GetOnMilitarySupplyAllocationChanged().Insert(OnMilitarySupplyAllocationChanged);

		SetPlayerCurrentAndLimitAllocation(w);
	}

	//------------------------------------------------------------------------------------------------
	//! Calls superclass method and unsubscribes from events
	//! \param[in] w Widget attached to the method, representing the root of the layout.
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_PlayerSupplyAllocationComponent)
		{
			w.SetVisible(false);
			m_PlayerSupplyAllocationComponent.GetOnAvailableAllocatedSuppliesChanged().Remove(OnAvailableAllocatedSuppliesChanged);
			m_PlayerSupplyAllocationComponent.GetOnMilitarySupplyAllocationChanged().Remove(OnMilitarySupplyAllocationChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPlayerCurrentAndLimitAllocation(Widget w)
	{
		if (!m_PlayerSupplyAllocationComponent)
			return;

		m_iAvailableSuppliesAmount = m_PlayerSupplyAllocationComponent.GetPlayerAvailableAllocatedSupplies();
		m_iMaxAvailableSuppliesAmount = m_PlayerSupplyAllocationComponent.GetPlayerMilitarySupplyAllocation();
		
		UpdateSupplyAmountText();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the Supplies text to current amount of available and allocated supplies
	protected void UpdateSupplyAmountText()
	{
		if (!m_wSuppliesText)
			return;

		if (m_bVisibleTopLimit)
			//Formating String to display to player Available supplies/Max Available Supplies, 100/500 Available
			m_wSuppliesText.SetTextFormat(m_sAvailableTextLong, m_iAvailableSuppliesAmount, m_iMaxAvailableSuppliesAmount);	
		else
			//Formating String to display to player Available supplies only 100 Available
			m_wSuppliesText.SetTextFormat(m_sAvailableText, m_iAvailableSuppliesAmount);
	}

	//------------------------------------------------------------------------------------------------
	//! Available Allocated Supplies amount changed, set the new value and update Supplies text
	//! \param[in] amount
	protected void OnAvailableAllocatedSuppliesChanged(int amount)
	{
		if (m_iAvailableSuppliesAmount == amount)
			return;

		m_iAvailableSuppliesAmount = amount;
		UpdateSupplyAmountText();
	}

	//------------------------------------------------------------------------------------------------
	//! Military Supply Allocation amount changed, set the new value and update Supplies text
	//! \param[in] amount
	protected void OnMilitarySupplyAllocationChanged(int amount)
	{
		if (m_iMaxAvailableSuppliesAmount == amount)
			return;

		m_iMaxAvailableSuppliesAmount = amount;
		UpdateSupplyAmountText();
	}
}
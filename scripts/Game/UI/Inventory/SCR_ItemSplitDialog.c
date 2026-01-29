class SCR_ItemSplitDialog : SCR_ConfigurableDialogUi
{
	protected SCR_SliderComponent m_Slider;
	protected IEntity m_EntityTo;
	protected IEntity m_EntityFrom;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		Widget slider = w.FindAnyWidget("Slider");
		m_Slider = SCR_SliderComponent.Cast(slider.FindHandler(SCR_SliderComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSliderMax(float max)
	{
		m_Slider.SetSliderSettings(0, max, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSliderValue(float value)
	{
		m_Slider.SetValue(value);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSliderValue()
	{
		return m_Slider.GetValue();
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntityTo()
	{
		return m_EntityTo;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntityFrom()
	{
		return m_EntityFrom;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetEntityTo(IEntity entityTo)
	{
		m_EntityTo = entityTo;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetEntityFrom(IEntity entityFrom)
	{
		m_EntityFrom = entityFrom;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ItemSplitDialog Create(float max = 1, IEntity entityTo = null, IEntity entityFrom = null)
	{
		SCR_ItemSplitDialog dialogUI = new SCR_ItemSplitDialog();
		SCR_ConfigurableDialogUi.CreateFromPreset("{A4F929B76A255B4F}Configs/Inventory/InventoryDialogPreset.conf", "ITEM_SPLIT", dialogUI);
		dialogUI.SetSliderMax(max);
		dialogUI.SetSliderValue(max);
		dialogUI.SetEntityTo(entityTo);
		dialogUI.SetEntityFrom(entityFrom);
		
		return dialogUI;
	}
}
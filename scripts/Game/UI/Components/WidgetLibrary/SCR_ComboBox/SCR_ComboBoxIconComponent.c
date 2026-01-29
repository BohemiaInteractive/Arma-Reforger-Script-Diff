class SCR_ComboBoxIconComponent: SCR_ComboBoxComponent
{
	[Attribute()]
	protected ref array<ResourceName> m_aElementIcons;
	protected ref array<float> m_aElementIconRotations;
	
	[Attribute("ComboBoxIcon")]
	protected string m_sIconWidgetName;
	
	protected ImageWidget m_IconWidget;
	protected ref ScriptInvoker Event_OnCreateElement = new ScriptInvoker;
	protected ref Color m_cIconDisabledColor = new Color (0.8, 0.8, 0.8, 1);
	
	protected override void UpdateName()
	{
		super.UpdateName();
		
		if (!m_IconWidget)
			return;
		
		if (m_aElementIcons && m_iSelectedItem >= 0 && m_iSelectedItem < m_aElementIcons.Count() && m_aElementIcons[m_iSelectedItem] != string.Empty)
		{
			m_IconWidget.LoadImageTexture(0, m_aElementIcons[m_iSelectedItem]);
			m_IconWidget.SetImage(0);
			m_IconWidget.SetRotation(m_aElementIconRotations[m_iSelectedItem]);
			m_IconWidget.SetVisible(true);
		}
		else {
			m_IconWidget.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int AddItemAndIcon(string item, ResourceName icon, float rotation = 0, Managed data = null)
	{
		if (!m_aElementIcons)
			m_aElementIcons = new array<ResourceName>;
		
		if (!m_aElementIconRotations)
			m_aElementIconRotations = new array<float>;
		
		m_aElementIcons.Insert(icon);
		m_aElementIconRotations.Insert(rotation);
		
		int i = super.AddItem(item, false, data);
		return i;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetIconWidget()
	{
		return m_IconWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	override int AddItem(string item, bool last = false, Managed data = null)
	{
		m_aElementIcons.Insert(string.Empty);
		int i = super.AddItem(item, last, data);
		return i;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ClearAll()
	{
		m_aElementIcons.Clear();
		super.ClearAll();
	}
	
	void AllowFocus(bool allowFocus)
	{
		if (allowFocus)
			m_wContentRoot.ClearFlags(WidgetFlags.NOFOCUS);
		else
			m_wContentRoot.SetFlags(WidgetFlags.NOFOCUS);
	}
	
	//------------------------------------------------------------------------------------------------	
	ScriptInvoker GetOnCreateElement()
	{
		return Event_OnCreateElement;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnCreateElement(Widget elementWidget, int index)
	{		
		ImageWidget iconWidget = ImageWidget.Cast(elementWidget.FindAnyWidget("Icon"));
		
		SCR_ButtonTextComponent textComp = SCR_ButtonTextComponent.Cast(elementWidget.GetHandler(0));
		
		if (!iconWidget)
			return;
		
		if (m_aElementIcons && index >= 0 && index < m_aElementIcons.Count() && m_aElementIcons[index] != string.Empty)
		{
			iconWidget.LoadImageTexture(0, m_aElementIcons[index]);
			iconWidget.SetImage(0);
			iconWidget.SetRotation(m_aElementIconRotations[m_iSelectedItem]);
			iconWidget.SetVisible(true);
		}
		else 
		{
			iconWidget.SetVisible(false);
		}
		
		
		Event_OnCreateElement.Invoke(elementWidget, index);
	}
	
	void SetElementEnabled(int index, bool enable)
	{
		if (index >= 0 && index < m_aElementWidgets.Count())
		{
			m_aElementWidgets[index].SetEnabled(enable);
			
			ImageWidget iconWidget = ImageWidget.Cast(m_aElementWidgets[index].FindAnyWidget("Icon"));
			if (!iconWidget)
				return;
			
			if (enable)
				iconWidget.SetColor(Color.FromInt(Color.WHITE));
			else
				iconWidget.SetColor(m_cIconDisabledColor);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void RemoveItem(int item, bool last = false)
	{
		if (item < m_aElementIcons.Count())
			m_aElementIcons.Remove(item);
		
		super.RemoveItem(item, last);
	}
	
	override void HandlerAttached(Widget w)
	{
		m_IconWidget = ImageWidget.Cast(w.FindAnyWidget(m_sIconWidgetName));
		super.HandlerAttached(w);
	}
};
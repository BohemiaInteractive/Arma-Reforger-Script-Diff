class SCR_RowTooltipEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("{694896EC99960920}UI/layouts/Editor/Tooltips/TooltipPrefabs/DynamicRow/TooltipPrefab_RowEntry.layout")]
	protected ResourceName m_sRowWidget;
	
	[Attribute("{199C5BF81F86C23A}UI/layouts/Editor/Tooltips/TooltipPrefabs/TooltipPrefab_TextAndIcon.layout")]
	protected ResourceName m_sTextWidget;
	
	[Attribute("{FA27A94694E01C73}UI/layouts/Editor/Tooltips/TooltipPrefabs/DynamicRow/TooltipPrefab_RowIcon.layout")]
	protected ResourceName m_sIconWidget;
	
	protected Widget m_wRoot;
	protected ref array<Widget> m_aRows = {};
	
	//------------------------------------------------------------------------------------------------
	bool AddText(int rowIndex, string text, Color color = null, string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty, string param4 = string.Empty, string param5 = string.Empty)
	{
		if (text.IsEmpty())
			return false;
		
		Widget row = GetRow(rowIndex);
		Widget newWidget = GetGame().GetWorkspace().CreateWidgets(m_sTextWidget, row);
		
		TextWidget newTextWidget = TextWidget.Cast(newWidget);
		if (!newTextWidget)
		{
			newTextWidget = TextWidget.Cast(newWidget.FindAnyWidget("Text"));
			if (!newTextWidget)
				return false;
		}

		newTextWidget.SetTextFormat(text, param1, param2, param3, param4, param5);
		
		if (color)
		{
			newTextWidget.SetColor(color);
			newWidget.SetColor(color);
		}
		
		Widget imageWidget = newWidget.FindAnyWidget("Icon");
		if (imageWidget)
			imageWidget.SetVisible(false);
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool AddIcon(int rowIndex, ResourceName icon, Color color = null, string setName = string.Empty)
	{
		if (icon.IsEmpty())
			return false;
		
		Widget row = GetRow(rowIndex);
		Widget newWidget = GetGame().GetWorkspace().CreateWidgets(m_sIconWidget, row);
		if (!newWidget)
			return false;
		
		ImageWidget imageWidget = ImageWidget.Cast(newWidget.FindAnyWidget("Icon"));
		if (!imageWidget)
			return false;
		
		if (setName.IsEmpty())
			imageWidget.LoadImageTexture(0, icon);
		else 
			imageWidget.LoadImageFromSet(0, icon, setName);
		
		if (color)
			imageWidget.SetColor(color);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget GetRow(int rowIndex)
	{
		if (rowIndex >= m_aRows.Count())
			CreateRows((rowIndex - m_aRows.Count()) +1);
		
		return m_aRows[rowIndex];
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateRows(int rowsToAdd = 1)
	{
		Widget newRow;
		
		while (rowsToAdd > 0)
		{
			rowsToAdd--;
			newRow = GetGame().GetWorkspace().CreateWidgets(m_sRowWidget, m_wRoot);
			if (!newRow)
				Print("SCR_RowTooltipEditorUIComponent could not create new row!", LogLevel.ERROR);
			
			m_aRows.Insert(newRow);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}
}

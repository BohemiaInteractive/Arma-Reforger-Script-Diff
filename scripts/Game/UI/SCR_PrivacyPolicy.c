class SCR_PrivacyPolicy: ChimeraMenuBase
{
	protected const string POLICY_ROW_LAYOUT = "{AB4838F67BA5293B}Configs/System/PolicyRow.layout";
	protected const int TABLE_1_ROWS = 20;
	protected const int TABLE_1_COLUMNS = 3;
	protected const int TABLE_2_ROWS = 9;
	protected const int TABLE_2_COLUMNS = 3;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		protected Widget footer;
		
		footer = GetRootWidget().FindAnyWidget("Footer");
		
		super.OnMenuOpen();
		// Subscribe to buttons
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, footer);
		if (back)
			back.m_OnActivated.Insert(EndPrivacyPolicy);
		
		UpdateTable(1, TABLE_1_COLUMNS, TABLE_1_ROWS);
		UpdateTable(2, TABLE_2_COLUMNS, TABLE_2_ROWS);		
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void UpdateTable(int tableIndex, int columns, int rows)
	{
		Widget table = GetRootWidget().FindAnyWidget("PolicyTable" + tableIndex);
		if (!table)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		Widget tableRow;
		RichTextWidget rowText;
		for (int i = 0; i < rows; i++)
		{
			tableRow = workspace.CreateWidgets(POLICY_ROW_LAYOUT, table);
			if (!tableRow)
				continue;
			
			for (int j = 0; j < columns; j++)
			{
				rowText = RichTextWidget.Cast(tableRow.FindAnyWidget("PolicyColumn" + j));
				if (!rowText)
					continue;
				
				rowText.SetText("#AR-PrivacyPolicy_" + tableIndex + "_" + j + "_" + i);
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	void EndPrivacyPolicy()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.PrivacyPolicyMenu)
	}
};

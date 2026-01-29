class SCR_AttributesButtonEditorUIComponent: SCR_BaseEditorUIComponent
{
	//---- REFACTOR NOTE START: Enum could be used for better clarity
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Attribute target type.", enums: { ParamEnum("Global", "0"), ParamEnum("Camera", "1"), ParamEnum("FactionsOnly", "2") })]
	protected int m_iTarget;
	
	//---- REFACTOR NOTE END ----
	
	[Attribute(desc: "Config of the category that should be selected upon opening attribute window.", params: "conf class=SCR_EditorAttributeCategory")]
	protected ResourceName m_Category;
	
	protected void OnButtonAction()
	{		
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (!attributesManager)
			return;
		
		if (m_Category)
			attributesManager.SetCurrentCategory(m_Category);
		
		//---- REFACTOR NOTE START: Enum could be used for better clarity
		
		switch (m_iTarget)
		{
			case 0:
				attributesManager.StartEditing(GetGame().GetGameMode());
				break;
			case 1:
				attributesManager.StartEditing(SCR_CameraEditorComponent.GetCameraInstance());
				break;
			case 2:
				attributesManager.StartEditing(SCR_TaskSystem.GetInstance());
				break;
		}
		
		//---- REFACTOR NOTE END ----
	}
	
	override bool IsUnique()
	{
		return false;
	}
	
	override void HandlerAttachedScripted(Widget w)
	{		
		SCR_InputButtonComponent navButton = SCR_InputButtonComponent.Cast(w.FindHandler(SCR_InputButtonComponent));
		if (navButton)
			navButton.m_OnActivated.Insert(OnButtonAction);
		else
			ButtonActionComponent.GetOnAction(w, true).Insert(OnButtonAction);
	}
};
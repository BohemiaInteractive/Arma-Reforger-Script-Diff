/** @ingroup Editor_UI Editor_UI_Menus
*/
class EditorAttributesDialogUI: EditorMenuBase
{
	private SCR_AttributesManagerEditorComponent m_AttributesManager;
	private ref array<ResourceName> m_CategoryConfigs;
	private ref array<ref SCR_EditorAttributeCategory> m_Categories;
	
	protected void InitCategories()
	{
		array<SCR_BaseEditorAttribute> attributes = new array<SCR_BaseEditorAttribute>;
		m_AttributesManager.GetEditedAttributes(attributes);
		
		m_CategoryConfigs = new array<ResourceName>;
		m_Categories = new array<ref SCR_EditorAttributeCategory>;
		array<int> priorities = new array<int>;
		
		//--- Get categories from all attributes
		foreach (SCR_BaseEditorAttribute attribute: attributes)
		{
			ResourceName categoryConfig = attribute.GetCategoryConfig();
			if (m_CategoryConfigs.Find(categoryConfig) == -1)
			{
				//--- Get category container
				Resource categoryContainer = BaseContainerTools.LoadContainer(categoryConfig);
				if (!categoryContainer)
				{
					Print(string.Format("Error loading category '%1'!", categoryConfig), LogLevel.ERROR);
					continue;
				}
					
				SCR_EditorAttributeCategory category = SCR_EditorAttributeCategory.Cast(BaseContainerTools.CreateInstanceFromContainer(categoryContainer.GetResource().ToBaseContainer()));
				if (!category)
				{
					Print(string.Format("SCR_EditorAttributeCategory not found in %1!", categoryConfig), LogLevel.ERROR);
					continue;
				}
				
				//--- Add to the array sorted by priority
				int priority = category.GetPriority();
				int index = 0;
				int count = m_CategoryConfigs.Count();
				bool breakCalled = false;
				
				for (int c = 0; c < count; c++)
				{					
					if (priority > priorities[c])
					{
						index = c;
						breakCalled = true;
						break;
					}
				}
				
				if (breakCalled)
				{
					m_CategoryConfigs.InsertAt(categoryConfig, index);
					m_Categories.InsertAt(category, index);
					priorities.InsertAt(category.GetPriority(), index);
				}
				else {
					m_CategoryConfigs.Insert(categoryConfig);
					m_Categories.Insert(category);
					priorities.Insert(category.GetPriority());
				}
			}
		}
	}
	
	void RemoveAutoClose()
	{
		if (m_AttributesManager)
		{
			m_AttributesManager.GetOnAttributesConfirm().Remove(CloseSelf);
			m_AttributesManager.GetOnAttributesCancel().Remove(CloseSelf);
		}
	}
	
	override void OnMenuOpen()
	{		
		m_AttributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent, true));
		if (!m_AttributesManager) 
			return;
		
		m_AttributesManager.GetOnAttributesConfirm().Insert(CloseSelf);
		m_AttributesManager.GetOnAttributesCancel().Insert(CloseSelf);
		
		InitCategories();
		SCR_AttributesEditorUIComponent attributesUI = SCR_AttributesEditorUIComponent.Cast(GetRootWidget().FindHandler(SCR_AttributesEditorUIComponent));
		if (attributesUI)
		{
			attributesUI.InitWidget(m_CategoryConfigs, m_Categories);
		}
	}
	
	override void OnMenuClose()
	{
		RemoveAutoClose();
	}
};
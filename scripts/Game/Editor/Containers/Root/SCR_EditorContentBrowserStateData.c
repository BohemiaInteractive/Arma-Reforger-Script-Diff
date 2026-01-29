/*!
Editor Content browser State data (Eg: Saved active filters, Search string and pagination) as well as tab UI info of content browser states.
This is used for persistence states between games. Saving it in Editor Settings
*/
class SCR_EditorContentBrowserSaveStateData
{
	[Attribute(desc: "List of labels that are saved", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_iSavedLabels;
	
	[Attribute(desc: "Index that is set when loaded.")]
	protected int m_iPageIndex = 0;
	
	[Attribute(desc: "Search string that is set when state is loaded")]
	protected string m_sSearchString;
	
	/*!
	Get saved Label filters count
	\return Saved label count
	*/
	int GetSavedLabelCount()
	{
		return m_iSavedLabels.Count();
	}
	
	/*!
	Set saved Label filters
	\param savedLabels labels to save
	\param If true will check if saved label is still valid
	*/
	void SetLabels(notnull array<EEditableEntityLabel> savedLabels, bool checkForInvalid = false)
	{
		//~ Create list
		if (!m_iSavedLabels)
			m_iSavedLabels = new array<EEditableEntityLabel>;
		else
			m_iSavedLabels.Clear();
		
		array<EEditableEntityLabel> validLabels = {};
		
		if (checkForInvalid)
			SCR_Enum.GetEnumValues(EEditableEntityLabel, validLabels);
		
		foreach (EEditableEntityLabel label: savedLabels)
		{
			//~ Check if invalid in cause the enums changed or a mod was changed
			if (checkForInvalid && !validLabels.Contains(label))
				continue;

			m_iSavedLabels.Insert(label);
		}
	}
	
	/*!
	Add or remove the given label
	\param label Label to add or remove
	\param addLabel If true will add the label. Else will remove
	*/
	void AddRemoveLabel(EEditableEntityLabel label, bool addLabel)
	{
		if (addLabel)
		{
			if (m_iSavedLabels.Contains(label))
				return;
			
			m_iSavedLabels.Insert(label);
		}
		else 
		{
			int count = m_iSavedLabels.Count();
			
			for(int i = 0; i < count; i++)
			{
				if (m_iSavedLabels[i] == label)
				{
					m_iSavedLabels.RemoveOrdered(i);
					return;
				}
			}
		}
		
	}
	
	/*!
	Get saved Label filters
	\param[out] labels Saved labels
	\return Saved label count
	*/
	int GetLabels(out notnull array<EEditableEntityLabel> labels)
	{
		if (m_iSavedLabels)
			labels.Copy(m_iSavedLabels);
		else
			return 0;
		
		return labels.Count();
	}
	
	/*!
	Check if provided label is on the list of saved labels
	\return true if given label is on the list
	*/
	bool ContainsLabel(EEditableEntityLabel label)
	{
		return m_iSavedLabels.Contains(label);
	}
	
	/*!
	Set saved page index
	\param pageIndex Page index to save
	*/
	void SetPageIndex(int pageIndex)
	{
		m_iPageIndex = pageIndex;
	}
	
	/*!
	Get saved page index
	\return Saved page index
	*/
	int GetPageIndex()
	{
		return m_iPageIndex;
	}
	
	/*!
	Set saved search string
	\param searchString Search string to save
	*/
	void SetSearchString(string searchString)
	{
		m_sSearchString = searchString;
	}
	
	
	/*!
	Get saved search string
	\return Saved search string
	*/
	string GetSearchString()
	{
		return m_sSearchString;
	}
	
	/*!
	Get has search string saved
	\return True if search string is saved
	*/
	bool HasSearchSaved()
	{
		return !m_sSearchString.IsEmpty();
	}
}
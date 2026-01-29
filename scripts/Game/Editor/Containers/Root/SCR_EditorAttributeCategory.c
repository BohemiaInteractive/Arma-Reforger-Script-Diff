[BaseContainerProps(configRoot: true)]
/** @ingroup Editor_Containers_Root
*/

/*!
Visual description of attribute category.
*/
class SCR_EditorAttributeCategory
{
	[Attribute()]
	protected ref SCR_UIInfo m_UIInfo;
	
	[Attribute(defvalue: "", desc: "When the texture is an image set, define the quad name here")]
	private string m_sImageName;
	
	[Attribute(defvalue: "0", desc: "Priority when showing up the category list. Higher value = higher priority")]
	private int m_iPriority;
	
	[Attribute(defvalue: "0", desc: "Whether or not the category is global. If true it means global attributes are opened which saves the tab index when closed for each mode.")]
	private bool m_bIsGlobalAttributeCategory;
	
	/*!
	Get attribute's UI info
	\return UI info
	*/
	SCR_UIInfo GetInfo()
	{
		return m_UIInfo;
	}
	/*!
	Get priority.
	\return Priority
	*/
	int GetPriority()
	{
		return m_iPriority;
	}
	/*!
	Is the category a global category? If ture it will save the last opened index
	\return m_bIsGlobalAttributeCategory
	*/
	bool GetIsGlobalAttributeCategory()
	{
		return m_bIsGlobalAttributeCategory;
	}
};
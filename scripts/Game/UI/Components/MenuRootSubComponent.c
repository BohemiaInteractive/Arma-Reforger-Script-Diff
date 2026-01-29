/*!
Widget component which is linked to menu it's part of.
Linking is done through MenuRootComponent.
*/
class MenuRootSubComponent: ScriptedWidgetComponent
{
	private Widget m_Widget;
	private MenuRootComponent m_Root;
	
	/*!
	Get widget to which the component belongs to.
	\return Widget
	*/
	Widget GetWidget()
	{
		return m_Widget;
	}
	
	/*!
	Get menu of which the widget is part of.
	\return Menu
	*/
	MenuRootBase GetMenu()
	{
		if (!m_Root)
			return null;
		
		return m_Root.GetMenu();
	}
	
	/*!
	Get root component of this subcomponent,
	\return Menu
	*/
	MenuRootComponent GetRootComponent()
	{
		return m_Root;
	}
	
	/*!
	Is the component to exist only in one instance? If so, it will be registered in the list of components and can be searched for.
	To be overwritten by inherited classes.
	\return True if unique
	*/
	protected bool IsUnique()
	{
		return true;
	}
	
	/*!
	Event called when subcomponent is initialized.
	Called only after all other subcomponents were registered as well and can be searched for. 
	To be overwritten by inherited classes.
	\param Widget
	*/
	void HandlerAttachedScripted(Widget w);
	
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return; //--- Run-time only
		
		m_Widget = w;
		
		m_Root = MenuRootComponent.GetRootOf(m_Widget);
		if (IsUnique())
		{
			if (!m_Root.FindComponent(Type()))
			{
				m_Root.AddComponent(this);
			}
			else
			{
				Print(string.Format("Duplicate instance of UI component %1 found on widget '%2'! The component is marked as unique, only one instance is allowed.", Type(), w.GetName()), LogLevel.WARNING);
				return;
			}
		}
		
		if (m_Root.GetMenu())
		{
			//--- Execute after delay to make sure all new components are registered first
			GetGame().GetCallqueue().CallLater(HandlerAttachedScripted, 0, false, w);
		}
	}
	
	override void HandlerDeattached(Widget w)
	{
		if (m_Root)
			m_Root.RemoveComponent(this);
	}
};
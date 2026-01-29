[BaseContainerProps(configRoot: true)]
class SCR_HUDLayout
{
	protected ref array<ref Widget> m_aAllGroups = {};
	protected ref array<ref SCR_HUDElement> m_aElements = {};

	[Attribute()]
	protected string m_sIdentifier;

	[Attribute(params: "layout")]
	protected ResourceName m_sLayout;

	protected Widget m_wRoot;

	//------------------------------------------------------------------------------------------------
	void SetIdentifier(string id)
	{
		if (id == string.Empty)
			return;
		
		PlayerController pController = GetGame().GetPlayerController();
		if (!pController)
			return;
		
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(pController.FindComponent(SCR_HUDManagerComponent));
		if (!hudManager)
			return;
		
		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(hudManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (!layoutHandler)
			return;
		
		array <string> allIdentifiers = layoutHandler.GetAllIdentifiers();
		
		if (allIdentifiers.Contains(id))
		{
			Print("HUDManager: Identifier: " + id + " is already used! Check if your Identifier is Unique!", LogLevel.ERROR);
			return;
		}
		
		m_sIdentifier = id;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetHUDElements(notnull out array<SCR_HUDElement> hudElements)
	{
		foreach (SCR_HUDElement element : m_aElements)
		{
			hudElements.Insert(element);
		}

		return hudElements.Count();
	}

	//------------------------------------------------------------------------------------------------
	string GetIdentifier()
	{
		return m_sIdentifier;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetLayout()
	{
		return m_sLayout;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	void SetRootWidget(notnull Widget widget)
	{
		m_wRoot = widget;

		Widget iteratedWidget = m_wRoot.GetChildren();
		while (iteratedWidget)
		{
			Widget iteratedChildWidget = iteratedWidget.GetChildren();
			while (iteratedChildWidget)
			{
				SCR_HUDGroupUIComponent groupComponent = SCR_HUDGroupUIComponent.Cast(iteratedChildWidget.FindHandler(SCR_HUDGroupUIComponent));
				if (!groupComponent)
				{
					iteratedChildWidget = iteratedChildWidget.GetSibling();
					continue;
				}

				m_aAllGroups.Insert(iteratedChildWidget);
				iteratedChildWidget = iteratedChildWidget.GetSibling();
			}

			iteratedWidget = iteratedWidget.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ResizeLayout()
	{
		array<SCR_HUDGroupUIComponent> groupsInLayout = {};
		GetAllGroupComponents(groupsInLayout);

		foreach (SCR_HUDGroupUIComponent groupComponent : groupsInLayout)
		{
			groupComponent.ResizeGroup();
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddHudElement(notnull SCR_HUDElement element, bool replaceParent = false)
	{
		if (!m_wRoot)
			return;
		
		Widget slotWidget = element.GetWidget();
		if (!slotWidget)
			return;

		Widget parentWidget = m_wRoot.FindAnyWidget(element.GetParentWidgetName());
		if (!parentWidget)
			return;

		if (replaceParent)
			parentWidget.AddChild(slotWidget);

		m_aElements.Insert(element);
		element.SetParentLayout(this);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveHudElement(notnull SCR_HUDElement element, bool replaceParent = false)
	{
		Widget elementWidget = element.GetWidget();
		if (!elementWidget)
			return;

		Widget parentWidget = elementWidget.GetParent();
		if (replaceParent && parentWidget)
			parentWidget.RemoveChild(elementWidget);
		
		m_aElements.RemoveItem(element);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns a Group Widget with the given name.
	\param groupName Name of the group to look for.
	*/
	Widget GetGroupWidgetByName(string groupName)
	{
		if (!m_wRoot)
			return null;
		
		return m_wRoot.FindAnyWidget(groupName);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns the Component responsible for managing and controlling a Group with the given name.
	\param groupName Name of the group to look for.
	*/
	SCR_HUDGroupUIComponent GetGroupComponent(string groupName)
	{
		Widget group = GetGroupWidgetByName(groupName);
		if (!group)
			return null;
		return SCR_HUDGroupUIComponent.Cast(group.FindHandler(SCR_HUDGroupUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Fills the provided array with all the GroupComponents this HUD Layout owns.
	\param groupComponents the array to be filled.
	*/
	int GetAllGroupComponents(notnull out array<SCR_HUDGroupUIComponent> groupComponents)
	{
		groupComponents.Clear();
		foreach (Widget groupWidget : m_aAllGroups)
		{
			SCR_HUDGroupUIComponent groupComponent = SCR_HUDGroupUIComponent.Cast(groupWidget.FindHandler(SCR_HUDGroupUIComponent));
			if (groupComponent)
				groupComponents.Insert(groupComponent);
		}

		return groupComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Fills the provided array with all the Slots that exist within this HUD Layout.
	\param slotComponents the array to be filled.
	*/
	int GetAllSlotComponents(notnull out array<SCR_HUDSlotUIComponent> slotComponents)
	{
		slotComponents.Clear();

		foreach (Widget groupWidget : m_aAllGroups)
		{
			Widget childWidget = groupWidget.GetChildren();
			while (childWidget)
			{
				SCR_HUDSlotUIComponent slotComponent = SCR_HUDSlotUIComponent.Cast(childWidget.FindHandler(SCR_HUDSlotUIComponent));
				if (!slotComponent)
				{
					childWidget = childWidget.GetSibling();
					continue;
				}

				slotComponents.Insert(slotComponent);
				childWidget = childWidget.GetSibling();
			}
		}

		return slotComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDSlotUIComponent FindSlotComponent(string slotName)
	{
		Widget slotWidget = m_wRoot.FindAnyWidget(slotName);
		if (!slotWidget)
			return null;

		return SCR_HUDSlotUIComponent.Cast(slotWidget.FindHandler(SCR_HUDSlotUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	Widget FindWidgetByName(string widgetName)
	{
		return m_wRoot.FindAnyWidget(widgetName);
	}
}

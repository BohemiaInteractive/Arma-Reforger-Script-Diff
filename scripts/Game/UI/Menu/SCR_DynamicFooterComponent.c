/*
Component for menu footers that handles navigation buttons, allowing quick and easy setup.
TODO: ideally, this should support different types of buttons, once we make them derive from a common class
*/

//------------------------------------------------------------------------------------------------
class SCR_DynamicFooterComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("20", desc: "Padding between buttons")]
	protected float m_fPadding;

	[Attribute("Footer", desc: "Left footer name")]
	protected string m_sLeftFooter;

	[Attribute("FooterRight", desc: "Right footer name")]
	protected string m_sRightFooter;

	protected ref map<string, SCR_InputButtonComponent> m_Buttons = new map<string, SCR_InputButtonComponent>();

	protected Widget m_wLeftFooter;
	protected Widget m_wRightFooter;

	protected ref ScriptInvokerString m_OnButtonActivated;

	// ---- OVERRIDES ----
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		//! Start searching from the widget above in case the component is inserted in the footer wrapper
		Widget root = w.GetParent();
		if (!root)
			root = w;
		
		m_wLeftFooter = root.FindAnyWidget(m_sLeftFooter);
		m_wRightFooter = root.FindAnyWidget(m_sRightFooter);

		array<SCR_InputButtonComponent> buttons = SCR_DynamicFooterComponent.FindChildrenButtons(w, false);

		foreach (SCR_InputButtonComponent button : buttons)
		{
			RegisterButton(button);
		}

		UpdateAllButtonsPadding(m_wLeftFooter);
		UpdateAllButtonsPadding(m_wRightFooter, SCR_EDynamicFooterButtonAlignment.RIGHT);
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// It is questionable if this component should care at all about giving invokers for the buttons, and instead just focus on setting the paddings/looks. The menu/dialog can then find the buttons and bind the proper events

	// ---- PRIVATE ----
	//------------------------------------------------------------------------------------------------
	//! Provides the button's name as a tag to easily identify which button was pressed
	// TODO: provide the input button's action as well
	private void OnButtonActivated(SCR_InputButtonComponent button)
	{
		if(m_OnButtonActivated)
			m_OnButtonActivated.Invoke(button.GetRootWidget().GetName());
	}

//---- REFACTOR NOTE END ----	
	
	//------------------------------------------------------------------------------------------------
	private void UpdateAllButtonsPadding(Widget wrapper, SCR_EDynamicFooterButtonAlignment alignment = 0)
	{
		if (!wrapper)
			return;

		array<SCR_InputButtonComponent> buttons = SCR_DynamicFooterComponent.FindChildrenButtons(wrapper);

		foreach (SCR_InputButtonComponent button : buttons)
		{
			UpdateButtonPadding(button, alignment);
		}
	}

	//------------------------------------------------------------------------------------------------
	private void UpdateButtonPadding(SCR_InputButtonComponent button, SCR_EDynamicFooterButtonAlignment alignment = 0)
	{
		if (!button)
			return;

		float paddingRight, paddingLeft;
		float left, top, right, bottom;

		switch (alignment)
		{
			case SCR_EDynamicFooterButtonAlignment.LEFT:
				paddingRight = m_fPadding;
				break;

			case SCR_EDynamicFooterButtonAlignment.RIGHT:
				paddingLeft = m_fPadding;
				break;
		}

		Widget widget = button.GetRootWidget();
		AlignableSlot.GetPadding(widget, left, top, right, bottom);

		AlignableSlot.SetPadding(widget, paddingLeft, top, paddingRight, bottom);
	}

	// ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	SCR_InputButtonComponent CreateButton(ResourceName layout, string tag, string label, string action, SCR_EDynamicFooterButtonAlignment alignment = 0, bool visible = true)
	{
		//! Decide wrapper layout
		Widget wrapper;
		switch (alignment)
		{
			case SCR_EDynamicFooterButtonAlignment.LEFT:
				wrapper = m_wLeftFooter;
				break;

			case SCR_EDynamicFooterButtonAlignment.RIGHT:
				wrapper = m_wRightFooter;
				break;
		}

		//! Create new button
		Widget buttonWidget = GetGame().GetWorkspace().CreateWidgets(layout, wrapper);
		if (!buttonWidget)
			return null;

		SCR_InputButtonComponent button = SCR_InputButtonComponent.FindComponent(buttonWidget);
		if (!button)
			return null;

		//! Button setup
		RegisterButton(button);
		UpdateButtonPadding(button, alignment);

		buttonWidget.SetName(tag);

		button.SetVisible(visible, false);
		button.SetLabel(label);
		button.SetAction(action);

		//! Make sure the right footer is in the correct position if the layout has it as a child of the left footer
		if (m_wRightFooter && wrapper == m_wLeftFooter)
			m_wRightFooter.SetZOrder(buttonWidget.GetZOrder() + 1);

		return button;
	}

	//------------------------------------------------------------------------------------------------
	SCR_InputButtonComponent CreateButtonAtPosition(ResourceName layout, string tag, string label, string action, int position, SCR_EDynamicFooterButtonAlignment alignment = 0, bool visible = true)
	{
		//! Decide wrapper layout
		Widget wrapper;
		switch (alignment)
		{
			case SCR_EDynamicFooterButtonAlignment.LEFT:
				wrapper = m_wLeftFooter;
				break;

			case SCR_EDynamicFooterButtonAlignment.RIGHT:
				wrapper = m_wRightFooter;
				break;
		}

		//! Adjust existing buttons' z order to insert new one
		array<ref Widget> existingButtons = {};
		SCR_WidgetHelper.GetAllChildren(wrapper, existingButtons);
		int count;

		foreach (Widget existingButton : existingButtons)
		{
			if (count == position)
				count++; //Skip what will be the position of the new button

			existingButton.SetZOrder(count);
			count++;
		}

		//! Create new button and set it's position
		SCR_InputButtonComponent button = CreateButton(layout, tag, label, action, alignment, visible);
		if (button)
			button.GetRootWidget().SetZOrder(position);

		//! Make sure the right footer is in the correct position if the layout has it as a child of the left footer
		if (wrapper == m_wLeftFooter)
			m_wRightFooter.SetZOrder(button.GetRootWidget().GetZOrder() + 1);

		UpdateButtonPadding(button, alignment);

		return button;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Caches a button and binds the invoker. Uses the button widget's name as a tag
	void RegisterButton(SCR_InputButtonComponent button)
	{
		Widget w = button.GetRootWidget();
		m_Buttons.Insert(w.GetName(), button);

		button.m_OnActivated.Insert(OnButtonActivated);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a button with given tag
	SCR_InputButtonComponent FindButton(string tag)
	{
		return m_Buttons.Get(tag);
	}

	//----------------------------------------------------------------------------------------
	//! Returns a button's tag
	string GetButtonTag(SCR_InputButtonComponent button)
	{
		if(!button)
			return string.Empty;
		
		return SCR_MapHelper<string, SCR_InputButtonComponent>.GetKeyByValue(m_Buttons, button);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes an existing button
	bool RemoveButton(string tag)
	{
		SCR_InputButtonComponent button = FindButton(tag);

		if (!button)
			return false;

		m_Buttons.Remove(tag);
		m_wRoot.RemoveChild(button.GetRootWidget());

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes all buttons
	void ClearButtons()
	{
		foreach (SCR_InputButtonComponent comp : m_Buttons)
		{
			m_wRoot.RemoveChild(comp.GetRootWidget());
		}
		m_Buttons.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns all the buttons that are direct children of the left or right footer
	array<SCR_InputButtonComponent> GetButtonsInFooter(SCR_EDynamicFooterButtonAlignment alignment)
	{
		Widget wrapper;
		switch (alignment)
		{
			case SCR_EDynamicFooterButtonAlignment.LEFT:
				wrapper = m_wLeftFooter;
				break;

			case SCR_EDynamicFooterButtonAlignment.RIGHT:
				wrapper = m_wRightFooter;
				break;
		}

		if (!wrapper)
			return null;

		return SCR_DynamicFooterComponent.FindChildrenButtons(wrapper);
	}

	//------------------------------------------------------------------------------------------------
	Widget GetLeftFooter()
	{
		return m_wLeftFooter;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetRightFooter()
	{
		return m_wRightFooter;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Provides the button's name as a tag to easily identify which button was pressed
	ScriptInvokerString GetOnButtonActivated()
	{
		if (!m_OnButtonActivated)
			m_OnButtonActivated = new ScriptInvokerString();

		return m_OnButtonActivated;
	}

	
	// ---- STATIC ----
	//------------------------------------------------------------------------------------------------
	static array<SCR_InputButtonComponent> FindChildrenButtons(notnull Widget w, bool searchImmediateChildrenOnly = true)
	{
		array<SCR_InputButtonComponent> buttons = {};
		array<ref Widget> children = {};

		SCR_WidgetHelper.GetAllChildren(w, children, !searchImmediateChildrenOnly);

		foreach (Widget child : children)
		{
			SCR_InputButtonComponent comp = SCR_InputButtonComponent.FindComponent(child);
			if (comp)
				buttons.Insert(comp);
		}

		return buttons;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_DynamicFooterComponent FindComponentInHierarchy(notnull Widget root)
	{
		SCR_DynamicFooterComponent comp;

		ScriptedWidgetEventHandler handler = SCR_WidgetTools.FindHandlerInChildren(root, SCR_DynamicFooterComponent);
		if (handler)
			comp = SCR_DynamicFooterComponent.Cast(handler);

		return comp;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_DynamicFooterComponent FindComponent(notnull Widget w)
	{
		return SCR_DynamicFooterComponent.Cast(w.FindHandler(SCR_DynamicFooterComponent));
	}
}

enum SCR_EDynamicFooterButtonAlignment
{
	LEFT = 0,
	RIGHT
}
class SCR_TaskIconComponent : ScriptedWidgetComponent
{
	[Attribute("1", UIWidgets.ComboBox, "Icon type", "", ParamEnumArray.FromEnum(ETaskIconType) )]
	protected ETaskIconType m_TaskIconType;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_BackgroundColor;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker)]
	protected ref Color m_IconColor;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker)]
	protected ref Color m_OutlineColor;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker)]
	protected ref Color m_HoverColor;

	[Attribute("1", UIWidgets.CheckBox)]
	protected bool m_BackgroundShown;

	[Attribute("1", UIWidgets.CheckBox)]
	protected bool m_IconShown;

	[Attribute("1", UIWidgets.CheckBox)]
	protected bool m_OutlineShown;

	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_HoverShown;

	protected const string TASK_ICON_IMAGESET = "{10C0A9A305E8B3A4}UI/Imagesets/Tasks/Task_Icons.imageset";

	protected Widget m_wRoot;
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		SetBackgroundColor(m_BackgroundColor);
		SetIconColor(m_IconColor);
		SetOutlineColor(m_OutlineColor);
		SetHoverColor(m_HoverColor);
		BackgroundShown(m_BackgroundShown);
		IconShown(m_IconShown);
		OutlineShown(m_OutlineShown);
		HoverShown(m_HoverShown);
		SetIconType(m_TaskIconType);
	}

	//------------------------------------------------------------------------------------------------
	void SetBackgroundColor(Color color)
	{
		ImageWidget background = ImageWidget.Cast(m_wRoot.FindAnyWidget("Background"));
		if (!background)
			return;
		background.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void SetIconColor(Color color)
	{
		ImageWidget icon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		if (!icon)
			return;
		icon.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void SetOutlineColor(Color color)
	{
		ImageWidget outline = ImageWidget.Cast(m_wRoot.FindAnyWidget("Outline"));
		if (!outline)
			return;
		outline.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void SetHoverColor(Color color)
	{
		ImageWidget hover = ImageWidget.Cast(m_wRoot.FindAnyWidget("Hover"));
		if (!hover)
			return;
		hover.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void BackgroundShown(bool shown)
	{
		ImageWidget background = ImageWidget.Cast(m_wRoot.FindAnyWidget("Background"));
		if (!background)
			return;
		if (shown)
			background.SetOpacity(1);
		else
			background.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	void IconShown(bool shown)
	{
		ImageWidget icon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		if (!icon)
			return;
		if (shown)
			icon.SetOpacity(1);
		else
			icon.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	void OutlineShown(bool shown)
	{
		ImageWidget outline = ImageWidget.Cast(m_wRoot.FindAnyWidget("Outline"));
		if (!outline)
			return;
		if (shown)
			outline.SetOpacity(1);
		else
			outline.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	void HoverShown(bool shown)
	{
		ImageWidget hover = ImageWidget.Cast(m_wRoot.FindAnyWidget("Hover"));
		if (!hover)
			return;
		if (shown)
			hover.SetOpacity(1);
		else
			hover.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	void SetIconType(ETaskIconType type)
	{
		ImageWidget icon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		if (!icon)
			return;
		string iconName;
		int x,y;
		//TODO:Marian change this to not use enum but strings to become modder friendly
		switch (type)
		{
			case ETaskIconType.CUSTOM:
				iconName = "Icon_Task_Custom";
				break;
			case ETaskIconType.EVACUATE:
				iconName = "Icon_Task_Evacuate";
				break;
			case ETaskIconType.GUARD:
				iconName = "Icon_Task_Guard";
				break;
			case ETaskIconType.MOVE:
				iconName = "Icon_Task_Move";
				break;
			case ETaskIconType.SEIZE_RELAY:
				iconName = "Icon_Task_Seize_Relay";
				break;
			case ETaskIconType.REFUEL:
				iconName = "Icon_Task_Refuel";
				break;
			case ETaskIconType.SEIZE:
				iconName = "Icon_Task_Seize";
				break;
			case ETaskIconType.TRANSPORT:
				iconName = "Icon_Task_Transport";
		}
		icon.LoadImageFromSet(0, TASK_ICON_IMAGESET, iconName);
		icon.GetImageSize(0, x, y);
		icon.SetSize(x, y);
	}
};

enum ETaskIconType
{
	CUSTOM,
	EVACUATE,
	GUARD,
	MOVE,
	SEIZE_RELAY,
	REFUEL,
	SEIZE,
	TRANSPORT
};
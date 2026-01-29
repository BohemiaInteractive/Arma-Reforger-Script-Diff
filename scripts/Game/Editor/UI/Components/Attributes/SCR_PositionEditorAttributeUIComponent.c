class SCR_PositionEditorAttributeUIComponent: SCR_BaseEditorAttributeUIComponent
{
	[Attribute("EditboxX")]
	protected string m_sEditBoxXName;
	[Attribute("EditboxY")]
	protected string m_sEditBoxYName;
	[Attribute("EditboxZ")]
	protected string m_sEditBoxZName;
	[Attribute("EditBoxRoot0")]
	protected string m_sEditBoxComponentName;
	
	protected BaseWorld m_World;
	protected vector m_vMinWorldBounding;
	protected vector m_vMaxWorldBounding;
	
	protected SCR_EditBoxComponent m_EditBoxX;
	protected SCR_EditBoxComponent m_EditBoxY;
	protected SCR_EditBoxComponent m_EditBoxZ;
	
	protected Widget m_root;
	protected vector m_vStartingValue;
	protected vector m_vHideValue;
	
	protected bool m_bSnapToSurface;
	protected float m_fPositionY;
	
	protected int m_iInitCalledAmount = 0;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		m_root = w;
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		m_World = GetGame().GetWorld();
		if (!m_World)
			return;
		
		m_World.GetBoundBox(m_vMinWorldBounding, m_vMaxWorldBounding);
		
		Widget editBoxX = w.FindAnyWidget(m_sEditBoxXName);
		Widget editBoxY = w.FindAnyWidget(m_sEditBoxYName);
		Widget editBoxZ = w.FindAnyWidget(m_sEditBoxZName);
		
		if (!editBoxX || !editBoxY || !editBoxZ)
			return;
		
		Widget editBoxComponentX = editBoxX.FindAnyWidget(m_sEditBoxComponentName);
		Widget editBoxComponentY = editBoxY.FindAnyWidget(m_sEditBoxComponentName);
		Widget editBoxComponentZ = editBoxZ.FindAnyWidget(m_sEditBoxComponentName);
		
		if (!editBoxComponentX || !editBoxComponentY || !editBoxComponentZ)
			return;
		
		m_EditBoxX = SCR_EditBoxComponent.Cast(editBoxComponentX.FindHandler(SCR_EditBoxComponent));
		m_EditBoxY = SCR_EditBoxComponent.Cast(editBoxComponentY.FindHandler(SCR_EditBoxComponent));
		m_EditBoxZ = SCR_EditBoxComponent.Cast(editBoxComponentZ.FindHandler(SCR_EditBoxComponent));
		
		if (!m_EditBoxX || !m_EditBoxY || !m_EditBoxZ)
			return;
		
		m_EditBoxX.m_OnConfirm.Insert(OnEditBoxValueChanged);
		m_EditBoxY.m_OnConfirm.Insert(OnEditBoxValueChanged);
		m_EditBoxZ.m_OnConfirm.Insert(OnEditBoxValueChanged);
		
		//Enable disable Y
		SCR_BaseEditorAttribute snapAattribute;
		if (m_AttributeManager.GetActiveAttribute(SCR_SnapYPositionEditorAttribute, snapAattribute))
		{
			SCR_SnapYPositionEditorAttribute snapYAttribute = SCR_SnapYPositionEditorAttribute.Cast(snapAattribute);
			if (snapYAttribute)
			{
				snapYAttribute.GetOnChanged().Insert(OnSnapYChanged);
				OnSnapYChanged(snapYAttribute.GetVariableOrCopy().GetBool());
			}
		}
		//Always snap if snap attribute not found
		else 
		{
			OnSnapYChanged(true);
		}
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		
		if (var)
			m_vStartingValue = var.GetVector();

		//Hide if gamepad
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		super.Init(w, attribute);
	}
	
	protected void OnSnapYChanged(bool snapEnabled)
	{
		m_bSnapToSurface = snapEnabled;
		m_EditBoxY.SetEnabled(!snapEnabled);
		
		if (snapEnabled)
		{
			m_fPositionY = m_EditBoxY.GetValue().ToFloat();
			m_EditBoxY.SetValue(" ");
		}
		else 
		{
			m_EditBoxY.SetValue(CapValueToString(m_fPositionY));
		}
		
		OnChangeInternal(null, 1, 0, false);
	}

	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		SetVisible(!isGamepad);
	}
	
	protected void SetVisible(bool newVisible)
	{
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute)
			return;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		if (!newVisible)
		{
			m_vHideValue = var.GetVector();
			var.SetVector(m_vStartingValue);
		}
		else
		{
			var.SetVector(m_vHideValue);
		}
		
		m_root.SetVisible(newVisible);
	}
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{	
		m_EditBoxX.SetValue(CapValueToString(m_vMinWorldBounding[0]));
		m_EditBoxZ.SetValue(CapValueToString(m_vMinWorldBounding[2]));
		
		if (!m_bSnapToSurface)
			m_EditBoxY.SetValue(CapValueToString(m_vMinWorldBounding[1]));
		else
			m_fPositionY = m_vMinWorldBounding[1];
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{						
		super.SetFromVar(var);
		
		if (!var)
			return;
		
		vector value = var.GetVector();
		m_EditBoxX.SetValue(CapValueToString(value[0]));
		m_EditBoxZ.SetValue(CapValueToString(value[2]));
		
		if (!m_bSnapToSurface)
			m_EditBoxY.SetValue(CapValueToString(value[1]));
		else
			m_fPositionY = value[1];
	}
	
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{	
		if (x != 1)
			return false;
		
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute) 
			return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariable(true);
		if (!var) 
			return false;
		
		float xValue = m_EditBoxX.GetValue().ToFloat();
		float yValue;
		float zValue = m_EditBoxZ.GetValue().ToFloat();
		
		if (m_bSnapToSurface)
			yValue = m_World.GetSurfaceY(xValue, zValue);
		else
			yValue = m_EditBoxY.GetValue().ToFloat();
		
		vector Value = Vector(xValue,  yValue,  zValue);
		
		var.SetVector(Value);
		return false;
	}
	
	protected void OnEditBoxValueChanged(SCR_EditBoxComponent editBox, string stringValue)
	{
		float value = stringValue.ToFloat();
		
		//X box
		if (editBox == m_EditBoxX)
		{
			if (value < m_vMinWorldBounding[0])
				value = m_vMinWorldBounding[0];
			else if (value > m_vMaxWorldBounding[0])
				value = m_vMaxWorldBounding[0];
		}
		//Y box
		else if (editBox == m_EditBoxY)
		{
			if (!m_bSnapToSurface)
			{
				if (value < m_vMinWorldBounding[1])
					value = m_vMinWorldBounding[1];
				else if (value > m_vMaxWorldBounding[1])
					value = m_vMaxWorldBounding[1];
			}
		}
		//Z box
		else
		{
			if (value < m_vMinWorldBounding[2])
				value = m_vMinWorldBounding[2];
			else if (value > m_vMaxWorldBounding[2])
				value = m_vMaxWorldBounding[2];
		}
		
		editBox.SetValue(CapValueToString(value));
		
		OnChangeInternal(null, 1, 0, false);
	}
	
	protected string CapValueToString(float value)
	{
		const int decimals = 2;
		
		float coef = Math.Pow(10, decimals);
		value = Math.Round(value * coef);
		string valueText = value.ToString();
		
		for (int i = 0, count = decimals - valueText.Length() + 1; i < count; i++)
		{
			valueText = "0" + valueText;
		}
		int length = valueText.Length();
		valueText = valueText.Substring(0, length - decimals) + "." + valueText.Substring(length - decimals, decimals);
		
		
		return valueText;
	}
	
	override void HandlerDeattached(Widget w)
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		if (m_AttributeManager && m_EditBoxY)
		{
			SCR_BaseEditorAttribute attribute;
			if (m_AttributeManager.GetActiveAttribute(SCR_SnapYPositionEditorAttribute, attribute))
			{
				SCR_SnapYPositionEditorAttribute snapYAttribute = SCR_SnapYPositionEditorAttribute.Cast(attribute);
				if (snapYAttribute)
				{
					snapYAttribute.GetOnChanged().Remove(OnSnapYChanged);
				}
			}
		}
		
	}
};


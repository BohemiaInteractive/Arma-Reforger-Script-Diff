[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class LocParserRule_Custom : LocParserRule
{
	[Attribute(category: "ID", desc: "Structure of the ID.\n\Params:\n%1 - file name\n%2 - variable name\n%3 - parent class\n%4 - parent name\n%5, %6, %7 - custom params (see Param attributes)")]
	protected string m_sIdFormat;

	[Attribute(category: "ID", desc: "Strings to be removed from the ID.\nTo mention multiple strings, separate them by a comma.")]
	protected string m_sRemove;

	[Attribute(desc: "Name of the variable which holds the string.\nTo mention multiple variables, separate them by a comma.")]
	protected string m_sVarName;

	[Attribute(desc: "Name of the parent (see 'Parent Type' for information which name is used).")]
	protected string m_sParentName;

	[Attribute(desc: "Depth of the queried parent.\n 0 means the class where the variable is defined, 1 is its parent, and so on.")]
	protected int m_iParentDepth;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Class name (inherited)", "0", ""), ParamEnum("Class name (exact)", "1", ""), ParamEnum("Name", "2", "") }, desc: "What information from the queried parent will be used.")]
	protected int m_iParentType;

	[Attribute(category: "ID", desc: "Parameter read from parent's variable and inserted in the ID format as %6.")]
	protected ref LocParserRule_Custom_BaseParam m_IdParam1;

	[Attribute(category: "ID", desc: "Parameter read from parent's variable and inserted in the ID format as %7.")]
	protected ref LocParserRule_Custom_BaseParam m_IdParam2;

	[Attribute(category: "ID", desc: "Parameter read from parent's variable and inserted in the ID format as %8.")]
	protected ref LocParserRule_Custom_BaseParam m_IdParam3;

	protected ref array<string> m_aVarName = {};
	protected ref array<string> m_aRemove = {};

	//------------------------------------------------------------------------------------------------
	protected void RemoveExpression(out string text)
	{
		foreach (string remove : m_aRemove)
		{
			text.Replace(remove, string.Empty);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool Evaluate(string fileName, string varName, array<BaseContainer> objects)
	{
		//--- Check variable name
		if (m_aVarName.Find(varName) == -1)
			return false;

		//--- Check parent class
		if (m_sParentName.IsEmpty())
			return true;

		if (objects.Count() > m_iParentDepth)
		{
			switch (m_iParentType)
			{
				//--- Class name (inherited)
				case 0:
				{
					typename parentType = objects[m_iParentDepth].GetClassName().ToType();
					if (parentType && parentType.IsInherited(m_sParentName.ToType()))
						return true;
					break;
				}
				//--- Class name (exact)
				case 1:
				{
					if (objects[m_iParentDepth].GetClassName() == m_sParentName)
						return true;
					break;
				}
				case 2:
				{
					//--- Name
					if (objects[m_iParentDepth].GetName() == m_sParentName)
						return true;
					break;
				}
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override string GetID(string fileName, string varName, array<BaseContainer> objects, array<int> indexes)
	{
		//--- Get parent params
		string parentClass, parentName, param1, param2, param3;
		if (m_sParentName.IsEmpty() || objects.Count() > m_iParentDepth)
		{
			BaseContainer parent = objects[m_iParentDepth];
			int index = indexes[m_iParentDepth];
			parentClass = parent.GetClassName();
			if (parentClass == "TextWidgetClass" || parentClass == "RichTextWidgetClass")
				parent.Get("Name", parentName);
			else
				parentName = parent.GetName();

			RemoveExpression(parentClass);
			RemoveExpression(parentName);

			BaseContainer paramObject = parent;
			int paramIndex = index;
			if (m_IdParam1 && m_IdParam1.GetArguments(objects, paramObject, indexes, paramIndex))
			{
				param1 = m_IdParam1.GetParam(paramObject, paramIndex);
				RemoveExpression(param1);
			}
			paramObject = parent;
			paramIndex = index;
			if (m_IdParam2 && m_IdParam2.GetArguments(objects, paramObject, indexes, paramIndex))
			{
				param2 = m_IdParam2.GetParam(paramObject, paramIndex);
				RemoveExpression(param2);
			}
			paramObject = parent;
			paramIndex = index;
			if (m_IdParam3 && m_IdParam3.GetArguments(objects, paramObject, indexes, paramIndex))
			{
				param3 = m_IdParam3.GetParam(paramObject, paramIndex);
				RemoveExpression(param3);
			}
		}

		//--- Format the ID
		RemoveExpression(fileName);
		RemoveExpression(varName);
		string id = string.Format(m_sIdFormat, fileName, varName, parentClass, parentName, param1, param2, param3);

		return id;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void LocParserRule_Custom()
	{
		m_sVarName.Split(",", m_aVarName, true);
		m_sRemove.Split(",", m_aRemove, true);
	}
}

[BaseContainerProps()]
class LocParserRule_Custom_BaseParam
{
	[Attribute("-1", desc: "Depth of the param parent.\n 0 means the class where the variable is defined, 1 is its parent, and so on.")]
	protected int m_iParentDepth;

	//------------------------------------------------------------------------------------------------
	bool GetArguments(notnull array<BaseContainer> objects, out BaseContainer outObject, notnull array<int> indexes, out int outIndex)
	{
		if (objects.IsEmpty() || objects.Count() <= m_iParentDepth)
			return false;

		if (m_iParentDepth < 0)
			return true;

		outObject = objects[m_iParentDepth];
		outIndex = indexes[m_iParentDepth];
		return true;
	}
	string GetParam(BaseContainer object, int index)
	{
		return string.Empty;
	}
}

[BaseContainerProps()]
class LocParserRule_Custom_IndexParam : LocParserRule_Custom_BaseParam
{
	//------------------------------------------------------------------------------------------------
	override string GetParam(BaseContainer object, int index)
	{
		return index.ToString();
	}
}

[BaseContainerProps()]
class LocParserRule_Custom_StringParam : LocParserRule_Custom_BaseParam
{
	[Attribute()]
	protected string m_sParamName;

	//------------------------------------------------------------------------------------------------
	override string GetParam(BaseContainer object, int index)
	{
		string text;
		object.Get(m_sParamName, text);

		if (text.StartsWith(LocParserManager.LOCALIZED_PREFIX))
			text = WidgetManager.Translate(text);

		string allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789";
		for (int i = text.Length() - 1; i >= 0; i--)
		{
			if (!allowedChars.Contains(text[i]))
				text.Replace(text[i], string.Empty);
		}
		return text;
	}
}

[BaseContainerProps()]
class LocParserRule_Custom_EnumParam : LocParserRule_Custom_BaseParam
{
	[Attribute()]
	protected string m_sParamName;

	[Attribute()]
	protected string m_sEnumName;

	[Attribute()]
	protected bool m_bIsFlag;

	//------------------------------------------------------------------------------------------------
	override string GetParam(BaseContainer object, int index)
	{
		int value;
		if (object.Get(m_sParamName, value))
		{
			typename type = m_sEnumName.ToType();
			if (m_bIsFlag)
				return SCR_Enum.FlagsToString(type, value, "_", "NONE");
			else
				return typename.EnumToString(type, value);
		}
		else
		{
			return string.Empty;
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class LocParserRule_Custom_ResourceNameParam : LocParserRule_Custom_BaseParam
{
	[Attribute()]
	protected string m_sParamName;

	override string GetParam(BaseContainer object, int index)
	{
		ResourceName path;
		object.Get(m_sParamName, path);
		return FilePath.StripExtension(FilePath.StripPath(path));
	}
}

[BaseContainerProps()]
class LocParserRule_Custom_NotificationParam : LocParserRule_Custom_BaseParam
{
	[Attribute()]
	protected string m_sParamName;

	//------------------------------------------------------------------------------------------------
	override string GetParam(BaseContainer object, int index)
	{
		ENotification notification;
		object.Get(m_sParamName, notification);
		return typename.EnumToString(ENotification, notification);
	}
}

[BaseContainerProps()]
class LocParserRule_Custom_HintParam : LocParserRule_Custom_BaseParam
{
	[Attribute()]
	protected string m_sParamName;

	//------------------------------------------------------------------------------------------------
	override string GetParam(BaseContainer object, int index)
	{
		EHint hint;
		object.Get(m_sParamName, hint);
		return typename.EnumToString(EHint, hint);
	}
}

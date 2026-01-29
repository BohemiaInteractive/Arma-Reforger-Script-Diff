/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class typename
{
	private void typename();
	private void ~typename();

	const static typename Empty;

	//!Returns type name of variable as string
	proto external string ToString();
	/*!
	Returns true when type is the same as 'baseType', or inherited one.
	\param baseType typename
	\returns \p bool true when type is the same as 'baseType', or inherited one.
	*/
	proto external bool IsInherited(typename baseType);
	/*!
	Dynamic variant to `new` keyword. It creates new instance of class.
	\returns \p instance of class

	*/
	proto external ref Managed Spawn();
	proto external int GetVariableCount();
	proto external owned string GetVariableName(int vIdx);
	proto external typename GetVariableType(int vIdx);
	proto external bool GetVariableValue(Class inst, int vIdx, out void val);
	/*!
	Return all attributes on class member.
	\code
	[BaseContainerCustomTitleField("m_Prop")]
	class ConfigExample
	{
		[Attribute(desc: "Some bla bla for m_Prop property")]
		int m_Prop;
	}

	void Usage()
	{
		typename t = ConfigExample;
		array<Class> attributes = {};
		t.GetVariableAttributes(0, attributes);
		foreach (Class a: attributes)
		{
			Attribute att = Attribute.Cast(a);
			if (att)
			{
				Print(att.m_Desc); // output: string m_Desc = 'Some bla bla for m_Prop property'
				break;
			}
		}
	}
	\endcode
	*/
	proto external void GetVariableAttributes(int vIdx, out notnull array<Class> attributes);
	/*!
	Return all attributes on class.
	\code
	[BaseContainerCustomTitleField("m_Prop")]
	class ConfigExample
	{
		[Attribute(desc: "Some bla bla for m_Commands property")]
		int m_Prop;
	}

	void Usage()
	{
		typename t = ConfigExample;
		array<Class> attributes = {};
		t.GetAttributes(attributes);
		foreach (Class a: attributes)
		{
			BaseContainerCustomTitleField att = BaseContainerCustomTitleField.Cast(a);
			if (att)
			{
				Print(att.m_PropertyName); // output:  string m_PropertyName = 'm_Prop'
				break;
			}
		}
	}
	\endcode
	*/
	proto external void GetAttributes(out notnull array<Class> attributes);
	/*!
	Return string name of enum value.
	\code
		DialogPriority prio = DialogPriority.WARNING;
		Print( typename.EnumToString(DialogPriority, prio) );
	\endcode
	*/
	static proto string EnumToString(typename e, int enumValue);
	/*!
	Return enum value from string name.
	\code
		Print( typename.StringToEnum(DialogPriority, "WARNING") );
	\endcode
	*/
	static proto int StringToEnum(typename e, string enumName);
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Attributes
\{
*/

class Attribute: UniqueAttribute
{
	string m_DefValue;
	string m_UiWidget; //!< use values from UIWidgets. Each widget type contains a description of possible parameters which you can put into the m_Params variable bellow
	/*!

				\brief Parameters that specify how the chosen UI widget (defined in m_UiWidget attribute) behaves together with the attribute that can be of several data types.

				The parameters must be separated by spaces and you can only use those that are compatible with the widget and attribute data type.
				These parameters bellow are for specific data types. They work with most of widgets although some combinations don't make sense. Beside them you can use also other parameters described within widget types in UIWidgets.

				For numbers (integers, scalars):
				- "minValue maxValue stepValue" - These three parameters are identified only by their position from left to right. It means that:
					1. first parameter is always be treated as minValue
					2. second parameter is always be treated as maxValue
					3. third parameter is always be treated as stepValue

					\n This simple rule with fixed order/position describes when they must be defined and when they don't. For example if you want to define maxValue then you must define also minValue parameter due to it's correct position.
					\n The stepValue is deprecated and it's also not functional at the moment but it becomes mandatory when you have more than these three parameters!

					Examples:
						- "5 20" - An integer in the range from 5 to 20
						- "inf inf" - An unlimited number (integer or scalar) in the maximum possible range
						- "-8.2 12.6" - Scalar number in the range from -8.2 to 12.6
						- "5 20 0 sortAlphabet" - An integer in the range from 5 to 20 with extra sortAlphabet parameter which affects sorting in combobox. In this case the minValue, maxValue and stepValue are mandatory!


				For vectors:
				- "minValue maxValue step" - For these parameters apply the same rules as to the numbers above. It means that first three parameters can be only these!
				- purpose=coords - Specifies that the attribute holds 3D coordinates. It allows editing the value in scene through an axis gizmo.
				- purpose=angles - Specifies that the attribute holds rotation angles. It allows editing the value in scene through a rotation gizmo.
				- purpose=sizes - Specifies that the attribute holds 3D volume sizes. It allows editing the value in scene through a bounding volume gizmo. This is not generally functional yet (works only in very specific cases). Ask for it if you need it
				- purpose=direction - Specifies that the attribute holds a normalized vector. It allows editing the value in scene through a rotation gizmo and visualized vector line.
				- space=entity - Specifies that the attribute value is for use in entity/model space. Thanks to that editing the value through a gizmo in scene will be in local space of selected entity
				- space=world - Specifies that the attribute value is for use in world space. Thanks to that editing the value through a gizmo in scene will be in world space
				- space=custom - Specifies that the attribute value is for use in generally unknown (thus custom) space. This ensures that editor will ask an entity and its components for the unknown space (matrix) through calling _WB_GetKeySpaceMatrixWorld callback event to them. Thanks to this editing values through editor's gizmos in scene is possible also in any other space (e.g space of a specific bone)
				- anglesVar=anglesVariableName - Specifies which another vector attribute holds rotation angles that are good for a gizmo in scene when editing the attribute value itself. It makes sense together with purpose=coords
				- coordsVar=coordsVariableName - Specifies which another vector attribute holds coordinates that are good for a gizmo in scene when editing the attribute value itself. It makes sense together with purpose=angles

					Examples:
						- "inf inf 0 purpose=coords space=entity anglesVar=anglesVariableName" - vector that holds coordinates which will be editable through axis gizmo in scene
						- "inf inf 0 purpose=angles space=entity coordsVar=coordsVariableName" - vector that holds rotation angles which will be editable through rotation gizmo in scene
						- "inf inf 0 purpose=direction space=entity" - vector that holds normalized direction which will be editable through bounding volume gizmo

				For resource names:
					- "ext1 ext2 ext3" - Specifies that the attribute can hold only file resource names of these types (extensions). A file dialog will offer only these file types.
					- folders - Specifies that the attribute can hold only a resource name of registered folder (with a meta-file). This is usual when the folder is a part of game data.
					- unregFolders - Specifies that the attribute can hold only a resource name of unregistered folder (without a meta-file). This is usual in properies of editing tools/plugins which are not a part of game data
					- class=MyClassName - Specifies that only config based file types of this class and also of all inherited classes will be offered in the file dialog
					- inheritedClasses=false - Specifies that the file dialog will not offer inherited classes of the class specified in class=MyClassName

				For arrays (of all types):
				- MaxSize=10 - Limits an amount of possible elements
				*/
	string m_Params;
	string m_Desc;
	string m_Category;
	int m_Precision;
	bool m_Prefabbed;
	/*!
	Only integers and floats are currently supported. Array can be defined this
	way:
	\code
		{ ParamEnum("Choice 1", "1"), ParamEnum("Choice 2", "2") }
	\endcode
	*/
	ref ParamEnumArray m_Enums;
	typename m_EnumType;
	/*!
	*/
	void Attribute(string defvalue = "", string uiwidget = "auto"/*use UIWidgets*/, string desc = "", string params = "", ParamEnumArray enums = NULL, string category = "", int precision = 3, typename enumType = void, bool prefabbed = false)
	{
		m_DefValue = defvalue;
		m_UiWidget = uiwidget;
		m_Params = params;
		m_Desc = desc;
		m_Category = category;
		m_Enums = enums;
		m_Precision = precision;
		m_EnumType = enumType;
		m_Prefabbed = prefabbed;
	}

}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
Base class for attribute for setting custom title in property grid
(Workbench). Works only on classes defined by BaseContainerProps attribute.
Inherit this attribute to set your own custom title.
\code
	class MyCustomTitle: BaseContainerCustomTitle
	{
		override bool _WB_GetCustomTitle(BaseContainer source, out string title)
		{
			title = "My Title";
			return true;
		}
	}

	[BaseContainerProps(), MyCustomTitle()]
	class TestConfigClass
	{
		[Attribute()]
		string m_ID;
	};
\endcode
*/
class BaseContainerCustomTitle
{
	bool _WB_GetCustomTitle(BaseContainer source, out string title);
	bool _WB_GetCustomClassTitle(string className, out string title);

}

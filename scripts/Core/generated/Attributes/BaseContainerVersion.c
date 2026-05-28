/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Attributes
\{
*/

/*!
Attribute for defining version of config class and setting callback for container migration to actual version.

\code
	[BaseContainerProps(configRoot:true), BaseContainerVersion(version: 3, onVersionChangeFn: OnVersionChange)]
	class TestingClass
	{
		[Attribute()]
		float m_float1;

		static void OnVersionChange(BaseContainer cont, BaseContainer hierParent, int version)
		{
			Print("TestingClass.OnVersionChange");
			Print(cont);
			Print(hierParent);
			Print(version);
		}
	};
\endcode
*/
class BaseContainerVersion: UniqueAttribute
{
	/*!
	\param version Config class version.
	\param onVersionChangeFn Callback method which is called when loaded container has different version.
	*/
	proto void BaseContainerVersion(
		int version = 1,
		BaseContainerVersionCallback onVersionChangeFn = null
	);

}

/*!
\}
*/

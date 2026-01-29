/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Containers
\{
*/

sealed class BaseContainerTools
{
	private void BaseContainerTools();
	private void ~BaseContainerTools();

	static proto bool WriteToWidget(BaseContainer cont, Widget w, string varName);
	static proto bool ReadFromWidget(BaseContainer cont, Widget w, string varName);
	static proto void WriteToInstance(notnull Class inst, BaseContainer src);
	static proto void ReadFromInstance(notnull Class inst, BaseContainer src);
	/*!
	Load BaseContainer from file.
	\code
		// load config
		Resource holder = BaseContainerTools.LoadContainer("{86C36D99914265F8}test.conf");
		if (holder)
		{
			BaseContainer cont = holder.GetResource().ToBaseContainer();
			Print(cont.GetClassName());
		}
	\endcode
	*/
	static proto ref Resource LoadContainer(ResourceName resourceName);
	/*!
	Create BaseContainer from object.
	\code
		[BaseContainerProps()]
		class TestingClass
		{
			[Attribute()]
			float m_float1;
			[Attribute()]
			float m_float2;
		};

		Resource holder = BaseContainerTools.CreateContainer("TestingClass");
		if (holder)
		{
			BaseContainer cont = holder.GetResource().ToBaseContainer();
			Print(cont.GetClassName());
		}
	\endcode
	*/
	static proto ref Resource CreateContainer(string typeClass);
	/*!
	Save BaseContainer to file.
	\code
		[BaseContainerProps()]
		class TestingClass
		{
			[Attribute()]
			float m_float1;
			[Attribute()]
			float m_float2;
		};

		// save config
		TestingClass obj = new TestingClass();
		obj.m_float1 = 654.1;
		obj.m_float2 = 996.9;

		// save config
		Resource holder = BaseContainerTools.CreateContainerFromInstance(obj);
		if (holder)
		{
			BaseContainerTools.SaveContainer(holder.GetResource().ToBaseContainer(), "{86C36D99914265F8}test.conf");
		}
	\endcode
	*/
	static proto bool SaveContainer(BaseContainer cont, ResourceName resourceName, string fileName = "");
	/*!
	Create BaseContainer from object.
	\code
		[BaseContainerProps()]
		class TestingClass
		{
			[Attribute()]
			float m_float1;
			[Attribute()]
			float m_float2;
		};

		// save config
		TestingClass obj = new TestingClass();
		obj.m_float1 = 654.1;
		obj.m_float2 = 996.9;

		// save config
		Resource holder = BaseContainerTools.CreateContainerFromInstance(obj);
		if (holder)
		{
			BaseContainerTools.SaveContainer(holder.GetResource().ToBaseContainer(), "{86C36D99914265F8}test.conf");
		}
	\endcode
	*/
	static proto ref Resource CreateContainerFromInstance(notnull Managed instance);
	/*!
	Create object from BaseContainer.
	\code
		[BaseContainerProps()]
		class TestingClass
		{
			[Attribute()]
			float m_float1;
			[Attribute()]
			float m_float2;
		};
		// load config
		Resource holder = BaseContainerTools.LoadContainer("{86C36D99914265F8}test.conf");
		if (holder)
		{
			TestingClass obj = TestingClass.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			if (obj)
			{
				Print(obj.m_float1);
				Print(obj.m_float2);
			}
		}
	\endcode
	*/
	static proto ref Managed CreateInstanceFromContainer(BaseContainer src);
}

/*!
\}
*/

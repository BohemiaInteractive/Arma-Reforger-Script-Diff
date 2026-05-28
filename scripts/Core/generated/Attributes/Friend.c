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
Attribute allows other classes to access protected methods. Can be added on specific method or whole class.
\code
[Friend(ClassB)]
class ClassA
{
	static protected void MethodA()
	{
	}

	[Friend(ClassC)]
	static protected void MethodB()
	{
	}
}


class ClassB
{
	static void Do()
	{
		ClassA.MethodA();
		ClassA.MethodB();
	}
}

class ClassC
{
	static void Do()
	{
		ClassA.MethodB();
	}
}
\endcode
*/
class Friend
{
	typename m_FriendClass;
	void Friend(typename friendClass)
	{
		m_FriendClass = friendClass;
	}

}

/*!
\}
*/

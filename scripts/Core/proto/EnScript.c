/*!
\defgroup Enforce Enforce script essentials
\{
*/

/*!
Marks method as obsolete. When is the method used, compiler just throw a compile-time
warning, but method is called normally.
\code
	[Obsolete("Use different method!")]
	void Hello()
	{
	}

	void Test()
	{
		Hello(); // throws compile warning on this line: 'Hello' is obsolete: use different method!
	}
\endcode
*/
class Obsolete
{
	string m_Msg;
	void Obsolete(string msg = "")
	{
		m_Msg = msg;
	}
}

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

enum EAccessLevel
{
	ANY = 0,
	LEVEL_0, //!< core module
	LEVEL_1, //!< game lib module
	LEVEL_2, //!< game module
	LEVEL_3,
	LEVEL_4,
	LEVEL_5,
	LEVEL_6,
	/* stored in 4bits, max is 15 */
}

/*!
Limit access to method only to script modules within some access level. If user
tries to call this method from script module with improper access level, compilation
error is thrown.
\code
	[Restrict(EAccessLevel.LEVEL_1, false)] // just throws warning
	void DangerousMethod1();

	[Restrict(EAccessLevel.LEVEL_1)] // throw an error and break compilation
	void DangerousMethod2();
\endcode
*/
class Restrict
{
	private EAccessLevel m_Level;
	private bool m_IsError; //!< If true, throws compilation error, else it throw just warning

	void Restrict(EAccessLevel level, bool isError = true)
	{
		m_Level = level;
		m_IsError = isError;
	}
}

//!Helper for printing out string expression. Example: PrintString("Hello " + var);
void PrintString(string s)
{
	Print(s);
}

/*!
\}
*/

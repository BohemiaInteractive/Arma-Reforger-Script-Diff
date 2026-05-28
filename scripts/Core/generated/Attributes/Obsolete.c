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
\}
*/

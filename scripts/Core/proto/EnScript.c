/*!
\defgroup Enforce Enforce script essentials
\{
*/

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
class Restrict: Managed
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

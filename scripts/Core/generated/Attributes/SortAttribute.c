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
Attribute to mark member variable of class as hint for sorting arrays of objects (both dynamic and static).
Supported member types for sorting are int, float, string and object

\code
class SampleObject
{
	[SortAttribute()]
	int m_iMember;

	void SampleObject(int i)
	{
		m_iMember = i;
	}
}

void Test()
{
	array<ref SampleObject> a = {SampleObject(5), SampleObject(2), SampleObject(7)};
	Print("Before:");
	foreach (auto o: a) Print(o.m_iMember); // 5,2,7

	a.Sort();

	Print("After:");
	foreach (auto o: a) Print(o.m_iMember); // 2,5,7
}

\endcode
*/
class SortAttribute: UniqueAttribute
{
}

/*!
\}
*/

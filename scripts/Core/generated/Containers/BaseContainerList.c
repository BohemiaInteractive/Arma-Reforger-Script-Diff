/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Containers
\{
*/

sealed class BaseContainerList: Managed
{
	private void BaseContainerList();

	proto external BaseContainer Get(int index);
	proto external bool Set(int index, BaseContainer cont);
	proto external bool Insert(BaseContainer cont, int index = -1);
	proto external bool Remove(BaseContainer cont);
	proto external int Count();
}

/*!
\}
*/

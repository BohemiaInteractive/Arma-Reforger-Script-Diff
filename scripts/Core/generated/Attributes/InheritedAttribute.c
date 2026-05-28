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
Attribute used for specify inheritance behavior within the attribute list.
A special, unique attribute class should be used to control how attributes are inherited.
This attribute can be placed anywhere in the attribute list, Be at the beginning, end, or between other attributes in order to specify where inherited attributes should be inserted.
This allows you to override specific attributes or establish dependencies as needed. It can be used in a same way for inherited and modded classes.

\code
// Attributes declaration:
class AttributeA : UniqueAttribute;
class AttributeB : AttributeA;
class AttributeC : UniqueAttribute;

// Attributes usage:
[AttributeA(), AttributeB(), AttributeC()]
class SomeClass;

[InheritedAttribute(useVanilla : false), AttributeC()]
class InheritedClass : SomeClass;

// AttributeB is inherited from SomeClass thanks to InheritedAttribute; AttributeC is declared in both SomeClass and InheritedClass,
// but due to it is UniqueAttribute, just the last one from InheritedClass is used. This mechanics can be used to override attributes from base class
// The final result after compilation:
[AttributeB(), AttributeC()]
class InheritedClass : SomeClass;

\endcode
*/
sealed class InheritedAttribute
{
	/*!
	\param useVanilla is used for modded classes to specify if source of inherited attributes is directly the vanilla class or eventually other previously used "modded class"
	*/
	void InheritedAttribute(bool useVanilla = false);

}

/*!
\}
*/

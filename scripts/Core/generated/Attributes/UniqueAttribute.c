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
An attribute used to specify that an attribute class is unique.
If some attribute is inherited from UniqueAttribute, only the last attribute of the kind is taken into account.
\code
// Attributes declaration:
class AttributeA : UniqueAttribute;
class AttributeB : AttributeA;
class AttributeC : UniqueAttribute;

// Attributes usage:
[AttributeA(), AttributeB(), AttributeC()]
class SomeClass;

// AttributeA and AttributeB are of the same kind and inherits from UniqueAttribute, so only the last one is used.
// The final result after compilation:
[AttributeB(), AttributeC()]
class SomeClass;
\endcode
*/
class UniqueAttribute
{
}

/*!
\}
*/

/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

/*!
Structure to store or generate UUIDs.

- Null UUID "" is default value without initialized string.
- Null UUID "00000000-0000-0000-0000-000000000000" is default value with initialized string.
\note If string which is not in valid UUID format is stored in UUID then it is considered as Null UUID.
\warning UUID in Script still operates as string so "" is not equal to "00000000-0000-0000-0000-000000000000" even when both are considered as Null.
         Use IsNull() method to verify if value is null instead.

- Generated Version 4 is fully random UUID.
- Generated Version 8 is custom variant of UUID based on first 128 bits of SHA-256 generated from namespace and name.

Based on RFC-9562 for Universally Unique IDentifiers (UUIDs).
[RFC 9562 Documentation](https://www.rfc-editor.org/rfc/rfc9562)
*/
sealed class UUID: string
{
	//! Null UUID with initialized string. To check if UUID is null use IsNull() method.
	static const UUID NULL_UUID = "00000000-0000-0000-0000-000000000000";

	/*!
	Standardized UUID namespaces by RFC.
	[RFC Namespace](https://www.rfc-editor.org/rfc/rfc9562#name-namespace-id-usage-and-allo)
	*/
	static const UUID NAMESPACE_DNS = "6ba7b810-9dad-11d1-80b4-00c04fd430c8";
	static const UUID NAMESPACE_URL = "6ba7b811-9dad-11d1-80b4-00c04fd430c8";
	static const UUID NAMESPACE_OID = "6ba7b812-9dad-11d1-80b4-00c04fd430c8";
	static const UUID NAMESPACE_X500 = "6ba7b814-9dad-11d1-80b4-00c04fd430c8";

	/*!
	Returns true if this UUID is Null("00000000-0000-0000-0000-000000000000" or "").

	\code
		UUID uuid1;
		UUID uuid2 = "";
		UUID uuid3 = UUID.NULL_UUID;
		UUID uuid4 = "5c146b14"; // not valid UUID format will be Null
		UUID uuid5 = "5c146b14-3c52-8afd-938a-375d0df1fbf6";

		Print(uuid1.IsNull());
		Print(uuid2.IsNull());
		Print(uuid3.IsNull());
		Print(uuid4.IsNull());
		Print(uuid5.IsNull());

		>> 1
		>> 1
		>> 1
		>> 1
		>> 0
	\endcode
	*/
	proto external bool IsNull();
	/*!
	Returns true if provided string is in valid UUID format.

	\code
		bool good = UUID.IsUUID("5c146b14-3c52-8afd-938a-375d0df1fbf6");
		bool bad = UUID.IsUUID("5c146b14");

		Print(good);
		Print(bad);

		>> good = 1
		>> bad = 0
	\endcode
	*/
	static proto bool IsUUID(string uuid);
	//! Returns UUID version
	proto external int GetVersion();
	/*!
	Will generate UUID of version 4 (random).
	Every bit is random except those used for version and variant.

	\code
		UUID uuid = UUID.GenV4();
		Print(uuid);

		// uuid will be random
		>> uuid = "f1b7954f-5aeb-41fc-86c0-3af76b7bf30f"
	\endcode
	*/
	static proto UUID GenV4();
	/*!
	Will generate UUID of version 8 (custom).
	For our custom UUID we use first 128 bits from SHA-256 hash generated from
	bytes of namespace UUID and name. Bits for version and variant are then
	replaced with proper values. V8 follows example specification defined in
	[RFC UUIDv8 Example](https://www.rfc-editor.org/rfc/rfc9562#name-example-of-a-uuidv8-value-n).
	\param namespaceUUID Namespace UUID used to prevent collisions or to give context to name. Use one of standardized or custom UUID.
	\param name String value used for hashing.

	\code
		UUID uuid = UUID.GenV8(UUID.NAMESPACE_DNS, "www.example.com");
		Print(uuid);

		// uuid will be always same
		>> uuid = "5c146b14-3c52-8afd-938a-375d0df1fbf6"
	\endcode
	*/
	static proto UUID GenV8(UUID namespaceUUID, string name);
	//! Retrieve the internal 128 bit ID into 4 integers for advanced handling. Primary purpose is to pass them into FromInts() later.
	proto external void GetInts(out int i1, out int i2, out int i3, out int i4);
	//! Combine back the 4 ints retrieved from GetInts
	static proto UUID FromInts(int i1, int i2, int i3, int i4);
}

/*!
\}
*/

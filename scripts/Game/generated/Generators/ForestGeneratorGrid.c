/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Generators
\{
*/

class ForestGeneratorGrid: Managed
{
	void ForestGeneratorGrid(float gridSize);

	proto external void Clear();
	proto external bool IsInBounds(int x, int y);
	proto external void Resize(float x, float y);
	proto external void SetPointOffset(float x, float y);
	proto external bool IsColliding(vector pos, ForestGeneratorTreeBase tree, array<int> entriesCollideWith = null);
	proto external void CountEntriesAround(vector pos, float radius, out array<float> treesAround);
	proto external int GetEntryCount();
	proto SCR_ForestGeneratorTreeBase GetEntry(int index, out vector position);
	proto external void AddEntry(SCR_ForestGeneratorTreeBase tree, vector position);
	proto external void RemoveEntry(ForestGeneratorTreeBase tree);
	proto external void RemoveEntryByIndex(int index);
}

/*!
\}
*/

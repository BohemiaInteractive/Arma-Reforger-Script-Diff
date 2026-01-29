/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

/*! Simple solver for assignment problem. Optimizing selection of jobs for set of workers
Basically brought from https://www.geeksforgeeks.org/job-assignment-problem-using-branch-and-bound/
*/
sealed class AssignmentSolver
{
	private void AssignmentSolver();
	private void ~AssignmentSolver();

	/*! Solves basic assignment problem by branch and bound defined by
	array of cost per worker in costs. Result is list of workers ids res
	Result doesn't guarantee optimal solution since B&B is used.
	*/
	static proto void Solve(array<ref array<int>> costs, out array<int> res);
}

/*!
\}
*/

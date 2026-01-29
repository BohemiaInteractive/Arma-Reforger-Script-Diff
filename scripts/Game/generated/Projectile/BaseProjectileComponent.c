/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile
\{
*/

class BaseProjectileComponentClass: GameComponentClass
{
}

class BaseProjectileComponent: GameComponent
{
	proto external Instigator GetInstigator();
	proto external void SetInstigator(notnull Instigator instigator);
	//Gets projectile entity
	proto external IEntity GetParentProjectile();
}

/*!
\}
*/

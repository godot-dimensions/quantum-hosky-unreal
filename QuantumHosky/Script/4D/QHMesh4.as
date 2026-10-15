//______________________________________________________________________________
namespace QH
{
	UMesh4D GetUnitBoxMesh4D()
	{
		return Cast<UMesh4D>(LoadObject(nullptr, "/Game/Mesh/UnitRadiusBox4D.UnitRadiusBox4D"));
	}
}

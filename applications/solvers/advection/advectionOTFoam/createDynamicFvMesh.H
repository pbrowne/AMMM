Info<< "Create mesh for time = "
    << runTime.timeName() << nl << endl;

autoPtr<dynamicFvMesh> meshPtr
(
    dynamicFvMesh::New
    (
        IOobject
	(
	    dynamicFvMesh::defaultRegion,
	    runTime.timeName(),
	    runTime,
	    IOobject::MUST_READ,
	    IOobject::AUTO_WRITE
        )
    )
);

dynamicFvMesh& mesh = meshPtr();

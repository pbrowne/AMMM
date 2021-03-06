// Create the mesh displacement potential, Phi
volScalarField Phi
(
    IOobject("Phi",runTime.timeName(),mesh,
             IOobject::MUST_READ, IOobject::AUTO_WRITE),
    mesh
);
Phi.oldTime();

// Create the perturbation from Phi
volScalarField phi("phi", Phi);
phi == dimensionedScalar("phi", dimArea, scalar(0));

// The gradient of Phi on faces
surfaceVectorField gradPhif("gradPhif", fvc::interpolate(fvc::grad(Phi)));
gradPhif += (fvc::snGrad(Phi) - (gradPhif & mesh.Sf())/mesh.magSf())
            *mesh.Sf()/mesh.magSf();

// Hessian of Phi and its determinant
volTensorField Hessian("Hessian", fvc::grad(gradPhif));
volScalarField detHess("detHess", fvc::det(Hessian + tensor::I));

// The matrix A of co-factors
volTensorField cofacA("cofacA", fvc::posDefCof(Hessian + tensor::I));

// The original monitor function on the old mesh
const volScalarField monitor
(
    IOobject(monitorFunc().name(), runTime.timeName(), mesh,
             IOobject::READ_IF_PRESENT),
    mesh,
    dimensionedScalar("monitor", dimensionSet(0,0,0,0,0), scalar(1))
);

// The monitor function on the old mesh calculated from the new mesh
volScalarField monitorNew("monitorNew", monitor);

// The monitor function on the new mesh
volScalarField monitorR("monitorR", monitorFunc().map(rMesh, monitor));
setInternalAndBoundaryValues(monitorNew, monitorR);

// The equidistribution
volScalarField equiDist("equiDist", monitorR*detHess);

volScalarField PABequi("PABequi", monitorR*detHess);

// The global equidistribution
dimensionedScalar Vtot = sum(mesh.V());
dimensionedScalar PABem = fvc::domainIntegrate(equiDist)/Vtot;
dimensionedScalar PABe = sqrt(fvc::domainIntegrate(sqr(equiDist - PABem)))
                         /(Vtot*PABem);

// The mean equidistribution, c
dimensionedScalar equiDistMean = fvc::domainIntegrate(detHess)
                                 /fvc::domainIntegrate(1/monitorNew);

// Source term
volScalarField source("source", detHess - equiDistMean/monitorNew);

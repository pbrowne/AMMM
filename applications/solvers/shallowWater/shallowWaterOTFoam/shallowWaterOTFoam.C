/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2016 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    shallowWaterOTFoam

Description
    Transient solver for shallow water equations on a moving mesh.
    Mesh calculated using optimal transport.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "dynamicFvMesh.H"
#include "pisoControl.H"
#include "CorrectPhi.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addBoolOption("reMeshOnly", "Re-mesh then stop, no fluid flow");
    argList::addBoolOption("fixedMesh", "run on rMesh and do not modify");
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createDynamicFvMesh.H"
    const Switch reMeshOnly = args.optionFound("reMeshOnly");
    const Switch fixedMesh = args.optionFound("fixedMesh");

    const scalar ACblend
    (
        readScalar(mesh.schemesDict().lookup("ACblend"))
    );
    const scalar CoriRecon = readScalar
    (
        mesh.schemesDict().lookup("CoriRecon")
    );
    const scalar ocCoeff
    (
        readScalar(mesh.schemesDict().lookup("ocCoeff"))
    );
    #include "createFields.H"

    if (reMeshOnly)
    {
        mesh.update();
        runTime.writeAndEnd();
    }

    #include "readEnvironmentalProperties.H"
    const dimensionedScalar H(envProperties.lookup("H"));
    #include "CourantNo.H"

    pisoControl piso(mesh);

    Info << "Mesh has normal direction" << flush;
    vector meshNormal = 0.5*(Vector<label>(1,1,1)-mesh.geometricD());
    meshNormal -= 2*meshNormal[1]*vector(0.,1.,0.);
    Info << meshNormal << endl;

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    #include "initDiags.H"

    Info<< "\nStarting time loop\n" << endl;
    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;
        #include "CourantNo.H"

        if (!fixedMesh)
        {
            mesh.update();
        }
        #include "fluidEqns.H"
        #include "diags.H"

        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //

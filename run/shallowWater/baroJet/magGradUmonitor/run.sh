#!/bin/bash -ve

# Create the case, run and post-process

# clear out old stuff
rm -rf [0-9]* constant/polyMesh constant/*/polyMesh core log* legends gmt.history *.dat

# Create initial mesh
blockMesh
mkdir -p constant/rMesh
cp -r constant/polyMesh constant/rMesh

# Createt initial conditions on the rMesh and plot h and the mesh
cp -r init0 0
setPlaneJet -region rMesh
gmtFoam -time 0 -region rMesh hMesh
evince 0/hMesh.pdf &

# Iterate, creating an adapted mesh and initial conditions on the mesh
meshIter=0
until [ $meshIter -ge 5 ]; do
    echo Mesh generation iteration $meshIter
    
    # Calculate the rMesh based on the monitor function derived from Uf
    movingshallowWaterFoamH -reMeshOnly

    # Re-create the initial conditions and re-plot
    setPlaneJet -region rMesh
    gmtFoam -time 0 -region rMesh hMesh
    
    let meshIter+=1
done

# Solve the shallow water equations
movingshallowWaterFoamH >& log & sleep 0.01; tail -f log

# Plot the final solutions
time=1e+06
gmtFoam -time $time hMesh -region rMesh
evince $time/hMesh.pdf &

time=1e+06
gmtFoam -time $time monitor -region rMesh
gv $time/monitor.pdf &

# Plot animation of solutions
gmtFoam hMesh -region rMesh
eps2gif hMesh.gif 0/hMesh.pdf [1-9]00000/hMesh.pdf 1e+06/hMesh.pdf &

# Plot change between initial and final solutions
time=1e+06
sumFields $time hDiff $time h 0 h -scale0 1 -scale1 -1
sumFields $time UDiff $time U 0 U -scale0 1 -scale1 -1
gmtFoam -time $time hUDiff
evince $time/hUDiff.pdf &

# If we have an analytic solution in directory ref, then we can calculate 
# error norms relative to this solution. For demonstration, I assume that
# the analytic solution is the initial condition. This is not the case here
# as this is not a steady state problem
ref=0
time=1e+06
globalSum h -time $ref
globalSum hDiff -time $time
l1=`tail -1 globalSumh.dat | awk '{print $2}'`
l2=`tail -1 globalSumh.dat | awk '{print $3}'`
li=`tail -1 globalSumh.dat | awk '{print $4}'`
echo "time l1 l2 linf" > hErrors.dat
awk '{if ($1 == 1*$1) print $1, $2/'$l1', $3/'$l2', $4/'$li'}' \
    globalSumhDiff.dat >> hErrors.dat

# Calculate and plot the vorticity
time=1e+06
postProcess -func rMesh/vorticity2D -time $time -region rMesh
gmtFoam -time $time vorticity -region rMesh
evince $time/vorticity.pdf &


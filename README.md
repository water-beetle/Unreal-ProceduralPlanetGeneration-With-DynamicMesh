# SpaceCamper

SpaceCamper is an Unreal Engine project focused on procedural planet generation.
It contains a custom module that demonstrates how to build dynamic planet meshes
and add foliage such as trees and grass through code.

## Unreal Engine Version

This project targets **Unreal Engine 5.4** as defined in the target files.

## Building and Running

1. Ensure Unreal Engine 5.4 (or newer) is installed.
2. Generate project files with Unreal's *GenerateProjectFiles* script.
3. Open the generated solution in your IDE and build the `SpaceCamperEditor`
   target, or build from the command line using `UnrealBuildTool`.
4. Launch the editor with the built target to run the project.

A minimal command line example on Windows might look like:

```bat
"<UnrealEnginePath>/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun ^
  -project="Path/To/SpaceCamper.uproject" ^
  -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development ^
  -cook -build -stage -pak -archive
```

## Folder Overview

- `SpaceCamper/` – Main module source code.
  - `Planet/` – Actor definitions for planets.
  - `Utility/Generators/` – Classes to generate planet meshes and foliage.
- `SpaceCamper.Target.cs` – Build target for the game.
- `SpaceCamperEditor.Target.cs` – Build target for the editor.

## Example Usage

The planet generation logic uses Perlin noise to displace mesh vertices.
Below is a snippet from `PlanetGenerator.cpp` showing how vertex positions are
modified:

```cpp
UE::Geometry::FMeshNormals Normals(&EditMesh);
auto GetDisplacedPosition = [&EditMesh, &Offsets, &Normals, this](int32 VertexID)
{
    FVector3d Pos = EditMesh.GetVertex(VertexID);
    float Magnitude = PlanetRadius * 0.1f;
    FVector3d Displacement;
    for (int32 k = 0; k < 3; ++k)
    {
        FVector NoisePos = (FVector)((double)NoiseFrequency * (Pos + Offsets[k]));
        Displacement[k] = Magnitude * FMath::PerlinNoise3D(NoiseFrequency * NoisePos);
    }
    Pos += Displacement;
    return Pos;
};
```

With this as a starting point, you can extend the planet generator or integrate
it into your own projects.


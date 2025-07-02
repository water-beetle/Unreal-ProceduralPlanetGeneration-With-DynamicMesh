# Unreal - Procedural Planet Generation With Dynamic Mesh

 Dynamic Mesh를 사용하여 절차적 행성 구현해보기

## Intro

Unreal Engine 5의 Dynamic Mesh를 이용해
행성을 생성하고, Foliage들을 배치하는 프로젝트

절차적 행성에 필요한 파라미터 값을 입력
1. Planet Radius : 행성 반지름
2. Mountain Height, Oceah Height : 산, 바다의 높이값 -> 해당 값으로 산과 바다의 Material 설정
3. Planet Params(Noise) : Perlin Noise에 적용할 파라미터 값
![image](https://github.com/user-attachments/assets/174f6815-d0aa-4a93-b88c-24a1f2c8f393)

파라미터 값을 입력 후, Generate Planet Mesh Event -> Bake Planet Mesh Event를 누르면
Dynamic Mesh를 이용해 절차적으로 행성이 생성되고, Static Mesh로 Bake된다.
![image](https://github.com/user-attachments/assets/58c18925-0e77-4342-b091-9eb2c777b17a)

![image](https://github.com/user-attachments/assets/660a326c-425e-4bd8-9f18-b2d685c50b99)


게임을 실행하게 되면 플레이어 주위의 3x3 Chunk 크기로 Foliage들이 배치된다.
![image](https://github.com/user-attachments/assets/89fcbbbd-e64b-48df-9456-137d33d22256)
![image](https://github.com/user-attachments/assets/2da556cf-3900-4982-94e3-9889b68cad8d)


## Dynamic Mesh를 사용한 절차적 행성 생성 방법

![image](https://github.com/user-attachments/assets/719e8f20-ddcb-4563-aca4-8a19ae2569b5)

Dynamic Mesh 관련 함수들이 Blueprint로 잘 구현이 되어 있어서
기본적인 구 생성과 Smoothing 같은 것들은 이미 구현되어 있는 블루프린트 함수를 사용하였다.

Foliage들을 행성 표면위에 적절하게 배치하기 위해서는 어떤 Noise값을 적용하였는지 정보가 필요해서
Noise를 적용하는 함수는 엔진의 코드를 참고하여 조금 커스텀하여 PlanetMeshGenerator.cpp에서 다시 구현하였다.
```
// Get Actor(Planet)'s Params
	float PlanetRadius = PlanetActor->PlanetRadius;
	FVector NoiseFrequencyShift = PlanetActor->NoiseFrequencyShift * 10000.0f;
	float NoiseFrequency = PlanetActor->NoiseFrequency;
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


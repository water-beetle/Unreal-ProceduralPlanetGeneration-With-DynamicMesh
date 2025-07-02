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
언리얼 엔진의 GeometryScript의 ApplyPlanetPerlinNoiseToMesh에 행성의 파라미터 값을 사용하도록 함수를 수정  

// Get Actor(Planet)'s Params
float PlanetRadius = PlanetActor->PlanetRadius;
FVector NoiseFrequencyShift = PlanetActor->NoiseFrequencyShift * 10000.0f;
float NoiseFrequency = PlanetActor->NoiseFrequency;
```

## 행성 높이에 따른 Material 적용

Planet Actor의 Planet Radius, Mountain Height, Ocean Height 변수값을 읽어와  
높이에 따른 material을 적용하도록 구현하였다.  
![image](https://github.com/user-attachments/assets/0638da1e-31a7-4af6-bb52-4e1fd2a9c624)


## Foliage 배치

처음에는 수십만개의 Foliage들을 행성 모든곳에 배치하려니까 너무 느리고 OOM이 발생하였다.  
그래서 아래와 같은 방법을 사용함.  

1. 행성을 여러개의 Chunk로 나누고, 캐릭터 주위에만 Foliage를 배치하도록 구현  
```
//[PlanetFoliageBase.cpp]
void UPlanetFoliageBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

        AActor* Owner = GetOwner();
        if (!Owner) return;

        APlayerController* PC = Owner->GetWorld()->GetFirstPlayerController();
        if (!PC) return;

        FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
        FVector FromCenter = (CameraLocation - Owner->GetActorLocation()).GetSafeNormal();

        FIntPoint CurrentChunk = GetChunkCoordFromOctahedral(FromCenter, NumChunks);

        if (CurrentChunk != LastCameraChunk)
        {
                LastCameraChunk = CurrentChunk;
                UpdateFoliageChunks(CurrentChunk);
        }
}
```

2. 행성을 Lat, Lon 기반으로 Chunk를 나누었더니 극점에서 균일하게 배치되지 않음  
-> 구글링해보니 Octahedral Mapping이라고 구의 표면의 점들을 균일하게 근사하는 방법이 있어 해당 방법을 사용  
```
//[PlanetFoliageBase.cpp]
FVector2D UPlanetFoliageBase::OctahedralEncode(const FVector& N)
{
        FVector n = N / (FMath::Abs(N.X) + FMath::Abs(N.Y) + FMath::Abs(N.Z));
        if (n.Z < 0)
        {
                float x = (1.0f - FMath::Abs(n.Y)) * (n.X >= 0.0f ? 1.0f : -1.0f);
                float y = (1.0f - FMath::Abs(n.X)) * (n.Y >= 0.0f ? 1.0f : -1.0f);
                return FVector2D(x, y);
        }
        return FVector2D(n.X, n.Y);
}

FVector UPlanetFoliageBase::OctahedralDecode(const FVector2D& UV)
{
        FVector2D Oct = UV * 2.0f - FVector2D(1.0f, 1.0f);

        FVector N;
        if (1.0f - FMath::Abs(Oct.X) - FMath::Abs(Oct.Y) >= 0.0f)
        {
                N = FVector(Oct.X, Oct.Y, 1.0f - FMath::Abs(Oct.X) - FMath::Abs(Oct.Y));
        }
        else
        {
                N = FVector(
                        Oct.X >= 0.0f ? 1.0f - FMath::Abs(Oct.Y) : -1.0f + FMath::Abs(Oct.Y),
                        Oct.Y >= 0.0f ? 1.0f - FMath::Abs(Oct.X) : -1.0f + FMath::Abs(Oct.X),
                        -1.0f);
        }

        return N.GetSafeNormal();
}
```

### Reference
1. Sebastian Lague - Procedural Planets  
https://www.youtube.com/watch?v=QN39W020LqU&list=PLFt_AvWsXl0cONs3T0By4puYy6GM22ko8  
2. CodeLikeMe - Unreal Planet Generator #1 - Generate a Planet with Dynamic Mesh and Geometry Scripts  
https://www.youtube.com/watch?v=S7FbUNQsXts  
3. ChatGPT  

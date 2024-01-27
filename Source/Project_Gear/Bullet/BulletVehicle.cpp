// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletVehicle.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletManager.h"
#include "DrawDebugHelpers.h"

#include "Net/UnrealNetwork.h"
#include "Project_GearCharacter.h"
#include "Kismet/GameplayStatics.h"

ABulletVehicle::ABulletVehicle()
{
	VehicleBody = CreateDefaultSubobject<UStaticMeshComponent>(FName("VehicleBody"));
	SetRootComponent(VehicleBody);

	Wheel_FL = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_FL"));
	Wheel_FL->SetupAttachment(VehicleBody);
	
	Wheel_FR = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_FR"));
	Wheel_FR->SetupAttachment(VehicleBody);

	Wheel_RL = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_RL"));
	Wheel_RL->SetupAttachment(VehicleBody);

	Wheel_RR = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_RR"));
	Wheel_RR->SetupAttachment(VehicleBody);

	SimulationMode = EBulletPhysicSimMode::Dynamic;
}

void ABulletVehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void ABulletVehicle::Tick(float DeltaTime)
{
	FVector CurrentLocation = GetActorLocation();
	Velocity = (CurrentLocation - LastLocation) / DeltaTime;
	LastLocation = CurrentLocation;

	if (GetLocalRole() == ROLE_Authority)
	{
		ValidateAndSortVehicleState();
		VehicleBufferedState = ConsumeStateBuffer();
		FrameIndex = VehicleBufferedState.VehicleTimeStamp.FrameIndex;
	}

	Super::Tick(DeltaTime);
	
	FVehicleState State = GetVehiclePhysicState(DeltaTime);

	if (GetLocalRole() == ROLE_Authority)
	{
		bool bDiSync =  State.IsInDisync(VehicleBufferedState, 100);
		if (bDiSync)
		{
			SendVehiclePhyiscStateToClient(VehicleBufferedState);
		}
	}
	else
	{
		AddStateBuffer(State);
		SendVehiclePhyiscStateToServer(State);
	}
}

void ABulletVehicle::AddStateBuffer(const FVehicleState& State)
{
	VehicleStateBuffer.Enqueue(State);
	VehicleStateBufferLength++;
}

void ABulletVehicle::SendVehiclePhyiscStateToServer_Implementation(const FVehicleState& State)
{
	AddStateBuffer(State);
}

void ABulletVehicle::SendVehiclePhyiscStateToClient_Implementation(const FVehicleState& State)
{
	while (!VehicleStateBuffer.IsEmpty() && VehicleStateBuffer.Peek()->VehicleTimeStamp.FrameIndex < FrameIndex)
	{
		VehicleStateBuffer.Pop();
	}

	if (VehicleStateBuffer.Peek()->VehicleTimeStamp.FrameIndex == FrameIndex)
	{
		DoCorrection();
	}
}

FVector ABulletVehicle::GetBulletVehicleVelocity()
{
	return Velocity;
}


FVehicleState ABulletVehicle::ConsumeStateBuffer()
{
	if (VehicleStateBufferLength > 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has %i input buffered. consuming 2 inputs."), *GetName(), VehicleStateBufferLength);
		VehicleStateBufferLength -= 2;

		FVehicleState Input_1;
		FVehicleState Input_2;

		VehicleStateBuffer.Dequeue(Input_1);
		VehicleStateBuffer.Dequeue(Input_2);

		return Input_1 > Input_2 ? Input_1 : Input_2;
	}

	else if (VehicleStateBufferLength > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has %i input buffered. consuming 1 inputs."), *GetName(), VehicleStateBufferLength);
		VehicleStateBufferLength -= 1;

		FVehicleState Input_1;
		VehicleStateBuffer.Dequeue(Input_1);

		return Input_1;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s has %i input buffered. server is starving .consuming 0 inputs."), *GetName(), VehicleStateBufferLength);
	return VehicleBufferedState;
}

FVehicleState ABulletVehicle::GetVehiclePhysicState(float DeltaTime)
{
	FVehicleState PhysicState;

	PhysicState.VehicleTimeStamp.FrameIndex = ++FrameIndex;
	PhysicState.VehicleTimeStamp.FrameDeltaTime = DeltaTime;

	PhysicState.VehicleInput.MovementInput = VehicleInput;

	PhysicState.VehicleTransform = GetActorTransform();

	PhysicState.WheelState_FL = Wheel_FL->GetWheelState();
	PhysicState.WheelState_FR = Wheel_FR->GetWheelState();
	PhysicState.WheelState_RL = Wheel_RL->GetWheelState();
	PhysicState.WheelState_RR = Wheel_RR->GetWheelState();

	return PhysicState;
}

void ABulletVehicle::ValidateAndSortVehicleState()
{
	TArray<FVehicleState> StateArray;

	while (!VehicleStateBuffer.IsEmpty())
	{
		FVehicleState State;
		VehicleStateBuffer.Dequeue(State);
		
		if (State.VehicleTimeStamp.FrameIndex > FrameIndex)
		{
			StateArray.Add(State);
		}
	}

	VehicleStateBufferLength = StateArray.Num();

	StateArray.StableSort([](const FVehicleState& L, const FVehicleState& R) { return L.VehicleTimeStamp.FrameIndex < R.VehicleTimeStamp.FrameIndex; });

	for (auto& a : StateArray)
	{
		VehicleStateBuffer.Enqueue(a);
	}
}

void ABulletVehicle::DoCorrection()
{
	TArray<FVehicleState> StateArray;

	while (!VehicleStateBuffer.IsEmpty())
	{
		FVehicleState State;
		VehicleStateBuffer.Dequeue(State);

		StateArray.Add(State);

		{
			SetActorTransform(State.VehicleTransform);
		
			Wheel_FL->SetWheelState(State.WheelState_FL);
			Wheel_FR->SetWheelState(State.WheelState_FR);
			Wheel_RL->SetWheelState(State.WheelState_RL);
			Wheel_RR->SetWheelState(State.WheelState_RR);
		}

		Tick(State.VehicleTimeStamp.FrameDeltaTime);

		ABulletManager::Get()->Tick(State.VehicleTimeStamp.FrameDeltaTime);
	}

	for (auto& a : StateArray)
	{
		VehicleStateBuffer.Enqueue(a);
	}
}

void ABulletVehicle::BeginPlay()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		//SimulationMode = EBulletPhysicSimMode::Ignore;
	}

	//if (GetLocalRole() == ROLE_Authority)
	//{
	//	SetReplicates(true);
	//	SetReplicateMovement(true);
	//}

	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		RigidBody->setActivationState(DISABLE_DEACTIVATION);
	}

	LastLocation = GetActorLocation();
}
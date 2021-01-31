// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include <Editor/AudioEditor/Private/SoundCueEditor.h>

ASDTCollectible::ASDTCollectible()
{
    //create sound object to be played upon collectible pickup
    static ConstructorHelpers::FObjectFinder<USoundBase> Soundf(TEXT("/Game/StarterContent/Audio/Explosion01"));
    Sound = Soundf.Object;

    //create particlue object to be played upon collectible pickup
    static ConstructorHelpers::FObjectFinder<UParticleSystem> Particlesf(TEXT("/Game/StarterContent/Particles/P_Explosion"));
    Particles = Particlesf.Object;
}

void ASDTCollectible::Collect()
{
    GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);
    
    //play the sound and animation
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particles, GetActorLocation());

    GetStaticMeshComponent()->SetVisibility(false);
}

void ASDTCollectible::OnCooldownDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_CollectCooldownTimer);

    GetStaticMeshComponent()->SetVisibility(true);
}

bool ASDTCollectible::IsOnCooldown()
{
    return m_CollectCooldownTimer.IsValid();
}

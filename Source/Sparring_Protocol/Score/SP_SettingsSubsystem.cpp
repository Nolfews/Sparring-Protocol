#include "Score/SP_SettingsSubsystem.h"

#include "AkAudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	const TCHAR* SettingsSlot = TEXT("Settings");
	constexpr int32 SettingsUserIndex = 0;
	constexpr int32 VolumeInterpolationMs = 100;
}

void USP_SettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameplayStatics::DoesSaveGameExist(SettingsSlot, SettingsUserIndex))
	{
		Data = Cast<USP_SettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SettingsSlot, SettingsUserIndex));
	}
	if (!Data)
	{
		Data = Cast<USP_SettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(USP_SettingsSaveGame::StaticClass()));
	}

	// Wwise may not be ready yet when the game instance starts: apply again after each map load.
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USP_SettingsSubsystem::HandlePostLoadMap);
	ApplyAudio();
}

void USP_SettingsSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	Super::Deinitialize();
}

void USP_SettingsSubsystem::HandlePostLoadMap(UWorld* World)
{
	ApplyAudio();
}

float USP_SettingsSubsystem::GetMouseSensitivity() const
{
	return Data ? Data->MouseSensitivity : 1.f;
}

void USP_SettingsSubsystem::SetMouseSensitivity(float Value)
{
	if (Data)
	{
		Data->MouseSensitivity = FMath::Clamp(Value, MinSensitivity, MaxSensitivity);
	}
}

bool USP_SettingsSubsystem::GetInvertY() const
{
	return Data && Data->bInvertY;
}

void USP_SettingsSubsystem::SetInvertY(bool bValue)
{
	if (Data)
	{
		Data->bInvertY = bValue;
	}
}

float USP_SettingsSubsystem::GetVolume(ESPVolumeChannel Channel) const
{
	if (!Data)
	{
		return 100.f;
	}
	switch (Channel)
	{
	case ESPVolumeChannel::Music: return Data->MusicVolume;
	case ESPVolumeChannel::Effects: return Data->EffectsVolume;
	default: return Data->MasterVolume;
	}
}

void USP_SettingsSubsystem::SetVolume(ESPVolumeChannel Channel, float Value)
{
	if (!Data)
	{
		return;
	}

	const float Clamped = FMath::Clamp(Value, 0.f, 100.f);
	switch (Channel)
	{
	case ESPVolumeChannel::Music: Data->MusicVolume = Clamped; break;
	case ESPVolumeChannel::Effects: Data->EffectsVolume = Clamped; break;
	default: Data->MasterVolume = Clamped; break;
	}

	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->SetRTPCValue(GetRtpcName(Channel), Clamped, VolumeInterpolationMs, nullptr);
	}
}

void USP_SettingsSubsystem::ApplyAudio() const
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice)
	{
		return;
	}
	for (const ESPVolumeChannel Channel : { ESPVolumeChannel::Master, ESPVolumeChannel::Music, ESPVolumeChannel::Effects })
	{
		AudioDevice->SetRTPCValue(GetRtpcName(Channel), GetVolume(Channel), 0, nullptr);
	}
}

void USP_SettingsSubsystem::SaveSettings()
{
	if (Data)
	{
		UGameplayStatics::SaveGameToSlot(Data, SettingsSlot, SettingsUserIndex);
	}
}

const TCHAR* USP_SettingsSubsystem::GetRtpcName(ESPVolumeChannel Channel)
{
	switch (Channel)
	{
	case ESPVolumeChannel::Music: return TEXT("Volume_Music");
	case ESPVolumeChannel::Effects: return TEXT("Volume_Effects");
	default: return TEXT("Volume_Master");
	}
}

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SP_SettingsSubsystem.generated.h"

UENUM(BlueprintType)
enum class ESPVolumeChannel : uint8
{
	Master,
	Music,
	Effects
};

/** Player settings written on disk (Saved/SaveGames/Settings.sav). Video settings live in GameUserSettings. */
UCLASS()
class SPARRING_PROTOCOL_API USP_SettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float MouseSensitivity = 1.f;

	UPROPERTY()
	bool bInvertY = false;

	/** Half volume on a first launch: the game is loud at 100. */
	UPROPERTY()
	float MasterVolume = 50.f;

	UPROPERTY()
	float MusicVolume = 100.f;

	UPROPERTY()
	float EffectsVolume = 100.f;
};

/**
 * Mouse and audio settings, loaded when the game starts and applied everywhere:
 *  - the player character reads the sensitivity / invert Y
 *  - the volumes are sent to Wwise as global Game Parameters (RTPC), 0 to 100:
 *    Volume_Master, Volume_Music, Volume_Effects (to create in the Wwise project and
 *    connect to the bus volumes).
 */
UCLASS()
class SPARRING_PROTOCOL_API USP_SettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Settings")
	float GetMouseSensitivity() const;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMouseSensitivity(float Value);

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool GetInvertY() const;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetInvertY(bool bValue);

	/** 0 to 100. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	float GetVolume(ESPVolumeChannel Channel) const;

	/** 0 to 100, sent to Wwise right away. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetVolume(ESPVolumeChannel Channel, float Value);

	/** Writes the settings on disk (called when the settings screen closes). */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

	/** Sends the three volumes to Wwise. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAudio() const;

	static constexpr float MinSensitivity = 0.1f;
	static constexpr float MaxSensitivity = 3.f;

private:
	void HandlePostLoadMap(UWorld* World);
	static const TCHAR* GetRtpcName(ESPVolumeChannel Channel);

	UPROPERTY(Transient)
	TObjectPtr<USP_SettingsSaveGame> Data;

	FDelegateHandle PostLoadMapHandle;
};

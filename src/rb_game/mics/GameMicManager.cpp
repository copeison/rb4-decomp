#include "rb_game/mics/GameMicManager.h"

#include "os/debug/Debug.h"
#include "rb_game/mics/GameMic.h"

GameMicManager* TheGameMicManager;

// Reconstructed from eboot.elf at 0x9BF490.
void GameMicManager::Init() {
    TheGameMicManager = new GameMicManager();
    TheDebug.AddExitCallback(Terminate);
    theMicClientMapper->SetMicManager(TheGameMicManager);
}

// Reconstructed from eboot.elf at 0x9BF600.
void GameMicManager::Terminate() {
    delete TheGameMicManager;
    TheGameMicManager = nullptr;
}

// Reconstructed from eboot.elf at 0x9BF630.
bool GameMicManager::ToggleMicSensitivity() {
    mHighSensitivity = !mHighSensitivity;
    return mHighSensitivity;
}

// Reconstructed from eboot.elf at 0x9BF640.
GameMicManager::GameMicManager()
    : mFxFlags{false, false, false, true},
      mHighSensitivity(false),
      mFxValues{0.5F, -1.0F, -1.0F},
      mMicCount(0),
      mPlayback(false),
      mPlaybackPending(false) {
    mMics.resize(4);
}

// Reconstructed from eboot.elf at 0x9BF740.
GameMicManager::~GameMicManager() {}

// Reconstructed from eboot.elf at 0x9BF8F0.
void GameMicManager::HandleMicsChanged() {
    eastl::vector<int> connected;
    theMicClientMapper->GetAllConnectedMics(connected);
    for (unsigned long id = 0; id < mMics.size(); id++) {
        bool found = false;
        for (const int mic : connected) {
            if (static_cast<unsigned int>(mic) == id) {
                found = true;
                break;
            }
        }
        if (found) {
            CreateMic(static_cast<int>(id));
        } else {
            DeleteMic(static_cast<int>(id));
        }
    }
    GameMicsChangedMsg msg;
    Export(msg.mData);
}

// Reconstructed from eboot.elf at 0x9BFB50.
void GameMicManager::DeleteMic(int micID) {
    GameMic* const mic = mMics[micID];
    if (mic == nullptr) {
        return;
    }
    delete mic;
    mMics[micID] = nullptr;
    mMicCount--;
}

// Reconstructed from eboot.elf at 0x9BFBA0. The binary inlines
// ApplyPlayback here, in HandleMicsChanged and in SetPlayback.
void GameMicManager::CreateMic(int micID) {
    if (mMics[micID] != nullptr) {
        return;
    }
    GameMic* const mic = new GameMic(micID);
    mMics[micID] = mic;
    mMicCount++;
    ApplyPlayback(mPlayback, false, mic);
}

// Reconstructed from eboot.elf at 0x9BFC30.
bool GameMicManager::HasMic(const MicClientID& client) const {
    return theMicClientMapper->GetMicIDForClientID(client) != -1;
}

// Reconstructed from eboot.elf at 0x9BFC50.
GameMic* GameMicManager::GetMic(const MicClientID& client) {
    const int micID = theMicClientMapper->GetMicIDForClientID(client);
    if (micID == -1) {
        return nullptr;
    }
    return mMics[micID];
}

// Reconstructed from eboot.elf at 0x9BFC90.
float GameMicManager::GetEnergyForMic(const MicClientID& client) {
    GameMic* const mic = GetMic(client);
    if (mic == nullptr) {
        return 0.0F;
    }
    mic->Update();
    return mic->mEnergy;
}

// Reconstructed from eboot.elf at 0x9BFCE0.
void GameMicManager::SetPlayback(bool playback) {
    if (mPlayback == playback && !mPlaybackPending) {
        return;
    }
    mPlayback = playback;
    mPlaybackPending = false;
    for (GameMic* const mic : mMics) {
        if (mic != nullptr) {
            ApplyPlayback(playback, false, mic);
        }
    }
}

// Reconstructed from eboot.elf at 0x9BFDD0.
void GameMicManager::SetPlaybackForMic(bool playback, bool unused, const MicClientID& client) {
    GameMic* const mic = GetMic(client);
    if (mic != nullptr) {
        ApplyPlayback(playback, unused, mic);
    }
}

// Reconstructed from eboot.elf at 0x9BFE30.
void GameMicManager::SetRecordFileName(const char* name) {
    mRecordFileName = name;
}

// Reconstructed from eboot.elf at 0x9BFEA0.
int GameMicManager::GetMicCount() const {
    return mMicCount;
}

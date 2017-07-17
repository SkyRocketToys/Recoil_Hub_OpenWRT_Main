//.cpp file code:

#include "GameState.h"
#include "IGameNetworkEvent.h"

static TangibleEvent<IGameNetworkEvent::CreateLobbyFn> *ReceiveCreateLobby = new TangibleEvent<IGameNetworkEvent::CreateLobbyFn>();

static TangibleEvent<IGameNetworkEvent::CreateLobbyFailedFn> *ReceiveCreateLobbyFailed = new TangibleEvent<IGameNetworkEvent::CreateLobbyFailedFn>();

static TangibleEvent<IGameNetworkEvent::JoinLobbyFn> *ReceiveJoinLobby = new TangibleEvent<IGameNetworkEvent::JoinLobbyFn>();

static TangibleEvent<IGameNetworkEvent::PlayerConnectFn> *ReceivePlayerConnect = new TangibleEvent<IGameNetworkEvent::PlayerConnectFn>();

static TangibleEvent<IGameNetworkEvent::OtherPlayerConnectFn> *ReceiveOtherPlayerConnect = new TangibleEvent<IGameNetworkEvent::OtherPlayerConnectFn>();

static TangibleEvent<IGameNetworkEvent::PlayerReconnectFn> *ReceivePlayerReconnect = new TangibleEvent<IGameNetworkEvent::PlayerReconnectFn>();

static TangibleEvent<IGameNetworkEvent::PlayerDisconnectFn> *ReceivePlayerDisconnect = new TangibleEvent<IGameNetworkEvent::PlayerDisconnectFn>();

static TangibleEvent<IGameNetworkEvent::SetGameModeFn> *ReceiveSetGameMode = new TangibleEvent<IGameNetworkEvent::SetGameModeFn>();
static TangibleEvent<IGameNetworkEvent::ChangeGameModeFn> *ReceiveChangeGameMode = new TangibleEvent<IGameNetworkEvent::ChangeGameModeFn>();
static TangibleEvent<IGameNetworkEvent::ChangeTeamFn> *ReceiveChangeTeam = new TangibleEvent<IGameNetworkEvent::ChangeTeamFn>();
static TangibleEvent<IGameNetworkEvent::StartFn> *ReceiveStart = new TangibleEvent<IGameNetworkEvent::StartFn>();
static TangibleEvent<IGameNetworkEvent::AbortFn> *ReceiveAbort = new TangibleEvent<IGameNetworkEvent::AbortFn>();
static TangibleEvent<IGameNetworkEvent::PlayerRankChangeFn> *ReceivePlayerRankChange = new TangibleEvent<IGameNetworkEvent::PlayerRankChangeFn>();

/// <summary>
/// The following are functions that receive EVENTS during a GAME MODE
/// </summary>
static TangibleEvent<IGameNetworkEvent::UpdateGameStateFn> *ReceiveUpdateGameState = new TangibleEvent<IGameNetworkEvent::UpdateGameStateFn>();
static TangibleEvent<IGameNetworkEvent::GoToAmmoFn> *ReceiveGoToAmmo = new TangibleEvent<IGameNetworkEvent::GoToAmmoFn>();
static TangibleEvent<IGameNetworkEvent::GoToBaseFn> *ReceiveGoToBase = new TangibleEvent<IGameNetworkEvent::GoToBaseFn>();
static TangibleEvent<IGameNetworkEvent::FireFn> *ReceiveFire = new TangibleEvent<IGameNetworkEvent::FireFn>();
static TangibleEvent<IGameNetworkEvent::PlayerWasHitFn> *ReceivePlayerWasHit = new TangibleEvent<IGameNetworkEvent::PlayerWasHitFn>();
static TangibleEvent<IGameNetworkEvent::OtherPlayerWasHitFn> *ReceiveOtherPlayerWasHit = new TangibleEvent<IGameNetworkEvent::OtherPlayerWasHitFn>();

static TangibleEvent<IGameNetworkEvent::UpdatePlayerStateFn> *ReceiveUpdatePlayerState = new TangibleEvent<IGameNetworkEvent::UpdatePlayerStateFn>();
static TangibleEvent<IGameNetworkEvent::UpdateOtherPlayersStateFn> *ReceiveUpdateOtherPlayersState = new TangibleEvent<IGameNetworkEvent::UpdateOtherPlayersStateFn>();

static TangibleEvent<IGameNetworkEvent::ReloadClipFn> *ReceiveReloadClip = new TangibleEvent<IGameNetworkEvent::ReloadClipFn>();
static TangibleEvent<IGameNetworkEvent::PickedUpItemFn> *ReceivePickedUpItem = new TangibleEvent<IGameNetworkEvent::PickedUpItemFn>();

static TangibleEvent<IGameNetworkEvent::PlayerDeathFn> *ReceivePlayerDeath = new TangibleEvent<IGameNetworkEvent::PlayerDeathFn>();
static TangibleEvent<IGameNetworkEvent::OtherPlayerDiedFn> *ReceiveOtherPlayerDied = new TangibleEvent<IGameNetworkEvent::OtherPlayerDiedFn>();
static TangibleEvent<IGameNetworkEvent::RespawnPlayerFn> *ReceiveRespawnPlayer = new TangibleEvent<IGameNetworkEvent::RespawnPlayerFn>();
static TangibleEvent<IGameNetworkEvent::SetBasePointFn> *ReceiveSetBase = new TangibleEvent<IGameNetworkEvent::SetBasePointFn>();
static TangibleEvent<IGameNetworkEvent::SetRespawnPointFn> *ReceiveSetRespawn = new TangibleEvent<IGameNetworkEvent::SetRespawnPointFn>();
static TangibleEvent<IGameNetworkEvent::SetAmmoPointFn> *ReceiveSetAmmo = new TangibleEvent<IGameNetworkEvent::SetAmmoPointFn>();
static TangibleEvent<IGameNetworkEvent::SetBombPointFn> *ReceiveSetBomb = new TangibleEvent<IGameNetworkEvent::SetBombPointFn>();
static TangibleEvent<IGameNetworkEvent::GameOverFn> *ReceiveGameOver = new TangibleEvent<IGameNetworkEvent::GameOverFn>();

static TangibleEvent<IGameNetworkEvent::FlagPickedUpFn> *ReceiveFlagPickedUp = new TangibleEvent<IGameNetworkEvent::FlagPickedUpFn>();
static TangibleEvent<IGameNetworkEvent::FlagDroppedFn> *ReceiveFlagDropped = new TangibleEvent<IGameNetworkEvent::FlagDroppedFn>();
static TangibleEvent<IGameNetworkEvent::FlagReturnedFn> *ReceiveFlagReturned = new TangibleEvent<IGameNetworkEvent::FlagReturnedFn>();

/// <summary>
/// The following are functions that receives EVENTS during the results.
/// </summary>
static TangibleEvent<IGameNetworkEvent::RetryFn> *ReceiveRetry = new TangibleEvent<IGameNetworkEvent::RetryFn>();

/// <summary>
/// The following are functions that receive EVENTS for features not yet implemented
/// </summary>
static TangibleEvent<IGameNetworkEvent::NewMapFn> *ReceiveMap = new TangibleEvent<IGameNetworkEvent::NewMapFn>();
static TangibleEvent<IGameNetworkEvent::NewPlayerProfileFn> *ReceivePlayerProfile = new TangibleEvent<IGameNetworkEvent::NewPlayerProfileFn>();
static TangibleEvent<IGameNetworkEvent::NewVoiceDataFn> *ReceiveVoiceData = new TangibleEvent<IGameNetworkEvent::NewVoiceDataFn>();
static TangibleEvent<IGameNetworkEvent::NewExplosionFn> *ReceiveExplosion = new TangibleEvent<IGameNetworkEvent::NewExplosionFn>();

static TangibleEvent<IGameNetworkEvent::NewSpawnFireFn> *ReceiveNewSpawnFire = new TangibleEvent<IGameNetworkEvent::NewSpawnFireFn>();

static TangibleEvent<IGameNetworkEvent::NewSpawnMineFn> *ReceiveNewSpawnMine = new TangibleEvent<IGameNetworkEvent::NewSpawnMineFn>();
static TangibleEvent<IGameNetworkEvent::NewSpawnHomingMineFn> *ReceiveNewSpawnHomingMine = new TangibleEvent<IGameNetworkEvent::NewSpawnHomingMineFn>();

static TangibleEvent<IGameNetworkEvent::NewScrambleMapOnFn> *ReceiveNewScrambleMapOn = new TangibleEvent<IGameNetworkEvent::NewScrambleMapOnFn>();
static TangibleEvent<IGameNetworkEvent::NewScrambleMapOffFn> *ReceiveNewScrambleMapOff = new TangibleEvent<IGameNetworkEvent::NewScrambleMapOffFn>();

void IGameNetworkEvent::SendUpdatePlayerStatePositionAndHeading(GameState::HotGenVec3 *Pos, float rot)
{
} //override not required

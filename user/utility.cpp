#include "pch-il2cpp.h"
#include "utility.h"
#include "state.hpp"
#include "game.h"
#include "gitparams.h"
#include "logger.h"
#include "profiler.h"
#include <random>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

int randi(int lo, int hi) {
	int n = hi - lo + 1;
	int i = rand() % n;
	if (i < 0) i = -i;
	return lo + i;
}

RoleRates::RoleRates(GameOptionsData__Fields gameOptionsDataFields, int playerAmount) {
	this->ImposterCount = gameOptionsDataFields._.numImpostors;
	auto maxImpostors = GetMaxImposterAmount(playerAmount);
	if(this->ImposterCount > maxImpostors)
		this->ImposterCount = maxImpostors;

		for (auto& kvp : il2cpp::Dictionary(gameOptionsDataFields.RoleOptions->fields.roleRates))
		{
			if (kvp.key == RoleTypes__Enum::Engineer)
			{
				this->EngineerChance = kvp.value.Chance;
				this->EngineerCount = kvp.value.MaxCount;
			}
			else if (kvp.key == RoleTypes__Enum::Scientist)
			{
				this->ScientistChance = kvp.value.Chance;
				this->ScientistCount = kvp.value.MaxCount;
			}
			else if (kvp.key == RoleTypes__Enum::Shapeshifter)
			{
				this->ShapeshifterChance = kvp.value.Chance;
				this->ShapeshifterCount = kvp.value.MaxCount;
			}
			else if (kvp.key == RoleTypes__Enum::GuardianAngel)
			{
				this->GuardianAngelChance = kvp.value.Chance;
				this->GuardianAngelCount = kvp.value.MaxCount;
			}
		}
}

int RoleRates::GetRoleCount(RoleTypes__Enum role) {
	switch (role)
	{
		case RoleTypes__Enum::Shapeshifter:
			return this->ShapeshifterCount;
		case RoleTypes__Enum::Impostor:
			return this->ImposterCount;
		case RoleTypes__Enum::Scientist:
			return this->ScientistCount;
		case RoleTypes__Enum::Engineer:
			return this->EngineerCount;
		case RoleTypes__Enum::GuardianAngel:
			return this->GuardianAngelCount;
		case RoleTypes__Enum::Crewmate:
			return this->MaxCrewmates;
		default:
#ifdef _DEBUG
			assert(false);
#endif
			return 0;
	}
}

void RoleRates::SubtractRole(RoleTypes__Enum role) {
	if (role == RoleTypes__Enum::Shapeshifter)
	{
		if (this->ShapeshifterCount < 1)
			return;
		this->ShapeshifterCount--;
		this->ImposterCount--;
	}
	else if (role == RoleTypes__Enum::Impostor)
	{
		if (this->ImposterCount < 1)
			return;
		this->ImposterCount--;
		this->ShapeshifterCount--;
	}
	else if (role == RoleTypes__Enum::Scientist)
	{
		if (this->ScientistCount < 1)
			return;
		this->ScientistCount--;
	}
	else if (role == RoleTypes__Enum::Engineer)
	{
		if (this->EngineerCount < 1)
			return;
		this->EngineerCount--;
	}
	else if (role == RoleTypes__Enum::GuardianAngel)
	{
		if (this->GuardianAngelCount < 1)
			return;
		this->GuardianAngelCount--;
	}
}

int GetMaxImposterAmount(int playerAmount)
{
	if(playerAmount >= 9)
		return 3;
	if(playerAmount >= 7)
		return 2;
	return 1;
}

int GenerateRandomNumber(int min, int max)
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
	return dist(rng);
}

PlayerSelection::PlayerSelection()
{
	this->hasValue = false;
	this->clientId = 0;
	this->playerId = 0;
}

PlayerSelection::PlayerSelection(PlayerControl* playerControl)
{
	if (playerControl != NULL) {
		this->hasValue = true;
		this->clientId = playerControl->fields._.OwnerId;
		this->playerId = playerControl->fields.PlayerId;
	} else {
		*this = PlayerSelection();
	}
}

PlayerSelection::PlayerSelection(GameData_PlayerInfo* playerData)
{
	if (playerData != NULL) {
		*this = PlayerSelection(playerData->fields._object);
	} else {
		*this = PlayerSelection();
	}
}

bool PlayerSelection::equals(PlayerControl* playerControl)
{
	if (!this->has_value()) return false;
	return this->get_PlayerControl() == playerControl;
}

Vector2 GetTrueAdjustedPosition(PlayerControl* playerControl)
{
	Vector2 playerVector2 = PlayerControl_GetTruePosition(playerControl, NULL);
	playerVector2.y += 0.36f;
	return playerVector2;
}

bool PlayerSelection::equals(GameData_PlayerInfo* playerData)
{
	if (!this->has_value()) return false;
	return this->get_PlayerData() == playerData;
}

bool PlayerSelection::equals(PlayerSelection selectedPlayer)
{
	if (!this->has_value() || !selectedPlayer.has_value()) return false;
	return (this->get_PlayerId() == selectedPlayer.get_PlayerId() && this->get_PlayerId() == selectedPlayer.get_PlayerId());
}

PlayerControl* PlayerSelection::get_PlayerControl()
{
	if (!this->hasValue) return NULL;

	if (clientId == -2) {
		auto playerControl = GetPlayerControlById(this->playerId);
		if (playerControl != NULL)
			return playerControl;
	}

	for (auto client : GetAllClients()) {
		if (client->fields.Id == this->clientId) {
			return client->fields.Character;
		}
	}

	*this = PlayerSelection();
	return NULL;
}

GameData_PlayerInfo* PlayerSelection::get_PlayerData()
{
	if (!this->hasValue) return NULL;

	auto playerControl = this->get_PlayerControl();
	if (playerControl != NULL)
		return playerControl->fields._cachedData;

	*this = PlayerSelection();
	return NULL;
}

bool PlayerSelection::has_value()
{
	if (!this->hasValue)
		return false;

	if (this->get_PlayerControl() == NULL)
		return false;

	return true;
}

uint8_t PlayerSelection::get_PlayerId()
{
	if (!this->has_value()) return 0;
	return this->playerId;
}

int32_t PlayerSelection::get_ClientId()
{
	if (!this->has_value()) return 0;
	return this->clientId;
}

bool PlayerSelection::is_LocalPlayer()
{
	if (!this->has_value()) return false;
	return this->get_PlayerControl() == *Game::pLocalPlayer;
}

bool PlayerSelection::is_Disconnected()
{
	// This is a sanity check, I don't think this is necessary as when a player.
	// When a player disconnects their PlayerControl is destroyed and as such has_value() should return false
	if (!this->has_value()) return true;
	return this->get_PlayerData()->fields.Disconnected;
}

ImVec4 AmongUsColorToImVec4(const Color& color) {
	return ImVec4(color.r, color.g, color.b, color.a);
}

ImVec4 AmongUsColorToImVec4(const CorrectedColor32& color) {
	return ImVec4(color.r / 255.0F, color.g / 255.0F, color.b / 255.0F, color.a / 255.0F);
}

#define LocalInGame (((*Game::pAmongUsClient)->fields._.GameMode == GameModes__Enum::LocalGame) && ((*Game::pAmongUsClient)->fields._.GameState == InnerNetClient_GameStates__Enum::Started))
#define OnlineInGame (((*Game::pAmongUsClient)->fields._.GameMode == GameModes__Enum::OnlineGame) && ((*Game::pAmongUsClient)->fields._.GameState == InnerNetClient_GameStates__Enum::Started))
#define OnlineInLobby (((*Game::pAmongUsClient)->fields._.GameMode == GameModes__Enum::OnlineGame) && ((*Game::pAmongUsClient)->fields._.GameState == InnerNetClient_GameStates__Enum::Joined))
#define TutorialScene (State.CurrentScene.compare("Tutorial") == 0)

bool IsInLobby() {
	if (Object_1_IsNull((Object_1*)*Game::pAmongUsClient)) return false;
	return OnlineInLobby && Object_1_IsNotNull((Object_1*)*Game::pLocalPlayer);
}

bool IsHost() {
	if (Object_1_IsNull((Object_1*)*Game::pAmongUsClient)) return false;
	return app::InnerNetClient_get_AmHost((InnerNetClient*)(*Game::pAmongUsClient), NULL);
}

bool IsInGame() {
	if (Object_1_IsNull((Object_1*)*Game::pAmongUsClient)) return false;
	return (LocalInGame || OnlineInGame || TutorialScene) && Object_1_IsNotNull((Object_1*)*Game::pShipStatus) && Object_1_IsNotNull((Object_1*)*Game::pLocalPlayer);
}

bool IsInMultiplayerGame() {
	if (Object_1_IsNull((Object_1*)*Game::pAmongUsClient)) return false;
	return (LocalInGame || OnlineInGame) && Object_1_IsNotNull((Object_1*)*Game::pShipStatus) && Object_1_IsNotNull((Object_1*)*Game::pLocalPlayer);
}

GameData_PlayerInfo* GetPlayerData(PlayerControl* player) {
	if (player) return app::PlayerControl_get_Data(player, NULL);
	return NULL;
}

GameData_PlayerInfo* GetPlayerDataById(uint8_t id) {
	return app::GameData_GetPlayerById((*Game::pGameData), id, NULL);
}

PlayerControl* GetPlayerControlById(uint8_t id) {
	for (auto player : GetAllPlayerControl()) {
		if (player->fields.PlayerId == id) return player;
	}

	return NULL;
}

PlainDoor* GetPlainDoorByRoom(SystemTypes__Enum room) {
	for (auto door : il2cpp::Array((*Game::pShipStatus)->fields.AllDoors))
	{
		if (door->fields.Room == room)
		{
			return door;
		}
	}

	return nullptr;
}

il2cpp::Array<PlainDoor__Array> GetAllPlainDoors() {
	return (*Game::pShipStatus)->fields.AllDoors;
}

il2cpp::List<List_1_PlayerControl_> GetAllPlayerControl() {
	return *Game::pAllPlayerControls;
}

il2cpp::List<List_1_GameData_PlayerInfo_> GetAllPlayerData() {
	return (*Game::pGameData)->fields.AllPlayers;
}

il2cpp::Array<DeadBody__Array> GetAllDeadBodies() {
	static std::string deadBodyType = translate_type_name("DeadBody, Assembly-CSharp");

	Type* deadBody_Type = app::Type_GetType(convert_to_string(deadBodyType), NULL);
	return (DeadBody__Array*)app::Object_1_FindObjectsOfType(deadBody_Type, NULL);
}

il2cpp::List<List_1_PlayerTask_> GetPlayerTasks(PlayerControl* player) {
	return player->fields.myTasks;
}

std::vector<NormalPlayerTask*> GetNormalPlayerTasks(PlayerControl* player) {
	static std::string normalPlayerTaskType = translate_type_name("NormalPlayerTask");

	auto playerTasks = GetPlayerTasks(player);
	std::vector<NormalPlayerTask*> normalPlayerTasks;
	normalPlayerTasks.reserve(playerTasks.size());

	for (auto playerTask : playerTasks)
		if (normalPlayerTaskType == playerTask->klass->_0.name || normalPlayerTaskType == playerTask->klass->_0.parent->name)
			normalPlayerTasks.push_back((NormalPlayerTask*)playerTask);

	return normalPlayerTasks;
}

SabotageTask* GetSabotageTask(PlayerControl* player) {
	static std::string sabotageTaskType = translate_type_name("SabotageTask");

	auto playerTasks = GetPlayerTasks(player);

	for (auto playerTask : playerTasks)
		if (sabotageTaskType == playerTask->klass->_0.name || sabotageTaskType == playerTask->klass->_0.parent->name)
			return (SabotageTask*)playerTask;

	return NULL;
}

void RepairSabotage(PlayerControl* player) {
	static std::string electricTaskType = translate_type_name("ElectricTask");
	static std::string hqHudOverrideTaskType = translate_type_name("HqHudOverrideTask");
	static std::string hudOverrideTaskType = translate_type_name("HudOverrideTask");
	static std::string noOxyTaskType = translate_type_name("NoOxyTask");
	static std::string reactorTaskType = translate_type_name("ReactorTask");

	auto sabotageTask = GetSabotageTask(player);
	if (sabotageTask == NULL) return;

	if (electricTaskType == sabotageTask->klass->_0.name) {
		auto electricTask = (ElectricTask*)sabotageTask;

		auto switchSystem = electricTask->fields.system;
		auto actualSwitches = switchSystem->fields.ActualSwitches;
		auto expectedSwitches = switchSystem->fields.ExpectedSwitches;

		if (actualSwitches != expectedSwitches) {
			for (auto i = 0; i < 5; i++) {
				auto switchMask = 1 << (i & 0x1F);

				if ((actualSwitches & switchMask) != (expectedSwitches & switchMask))
					State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Electrical, i));
			}
		}
	}

	if (hqHudOverrideTaskType == sabotageTask->klass->_0.name) {
		State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Comms, 16));
		State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Comms, 17));
	}

	if (hudOverrideTaskType == sabotageTask->klass->_0.name) {
		State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Comms, 0));
	}

	if (noOxyTaskType == sabotageTask->klass->_0.name) {
		State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::LifeSupp, 64));
		State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::LifeSupp, 65));
	}

	if (reactorTaskType == sabotageTask->klass->_0.name) {
		if (State.mapType == Settings::MapType::Ship || State.mapType == Settings::MapType::Hq) {
			State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Reactor, 64));
			State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Reactor, 65));
		}

		if (State.mapType == Settings::MapType::Pb) {
			State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Laboratory, 64));
			State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Laboratory, 65));
		}
		if (State.mapType == Settings::MapType::Airship) {
			State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Reactor, 16));
			State.rpcQueue.push(new RpcRepairSystem(SystemTypes__Enum::Reactor, 17));
		}
	}
}

void CompleteTask(NormalPlayerTask* playerTask) {
	if (playerTask->fields._._Owner_k__BackingField == (*Game::pLocalPlayer)) {
		while (playerTask->fields.taskStep < playerTask->fields.MaxStep)
			app::NormalPlayerTask_NextStep(playerTask, NULL);
	}
}

#pragma warning(suppress:26812)
const char* TranslateTaskTypes(TaskTypes__Enum taskType) {
	static const char* const TASK_TRANSLATIONS[] = { "Submit Scan", "Prime Shields", "Fuel Engines", "Chart Course", "Start Reactor", "Swipe Card", "Clear Asteroids", "Upload Data",
		"Inspect Sample", "Empty Chute", "Empty Garbage", "Align Engine Output", "Fix Wiring", "Calibrate Distributor", "Divert Power", "Unlock Manifolds", "Reset Reactor",
		"Fix Lights", "Clean O2 Filter", "Fix Communications", "Restore Oxygen", "Stabilize Steering", "Assemble Artifact", "Sort Samples", "Measure Weather", "Enter ID Code",
		"Buy Beverage", "Process Data", "Run Diagnostics", "Water Plants", "Monitor Oxygen", "Store Artifacts", "Fill Canisters", "Activate Weather Nodes", "Insert Keys",
		"Reset Seismic Stabilizers", "Scan Boarding Pass", "Open Waterways", "Replace Water Jug", "Repair Drill", "Align Telecopse", "Record Temperature", "Reboot Wifi",
		"Polish Ruby", "Reset Breakers", "Decontaminate", "Make Burger", "Unlock Safe", "Sort Records", "Put Away Pistols", "Fix Shower", "Clean Toilet", "Dress Mannequin",
		"Pick Up Towels", "Rewind Tapes", "Start Fans", "Develop Photos", "Get Biggol Sword", "Put Away Rifles", "Stop Charles", "Vent Cleaning"};
	return TASK_TRANSLATIONS[(uint8_t)taskType];
}

#pragma warning(suppress:26812)
const char* TranslateSystemTypes(SystemTypes__Enum systemType) {
	static const char* const SYSTEM_TRANSLATIONS[] = { "Hallway", "Storage", "Cafeteria", "Reactor", "Upper Engine", "Navigation", "Admin", "Electrical", "Oxygen", "Shields",
		"MedBay", "Security", "Weapons", "Lower Engine", "Communications", "Ship Tasks", "Doors", "Sabotage", "Decontamination", "Launchpad", "Locker Room", "Laboratory",
		"Balcony", "Office", "Greenhouse", "Dropship", "Decontamination", "Outside", "Specimen Room", "Boiler Room", "Vault Room", "Cockpit", "Armory", "Kitchen", "Viewing Deck",
		"Hall Of Portraits", "Cargo Bay", "Ventilation", "Showers", "Engine Room", "The Brig", "Meeting Room", "Records Room", "Lounge Room", "Gap Room", "Main Hall", "Medical" };
	return SYSTEM_TRANSLATIONS[(uint8_t)systemType];
}

CorrectedColor32 GetPlayerColor(uint8_t colorId) {
	CorrectedColor32* colorArray = (CorrectedColor32*)app::Palette__TypeInfo->static_fields->PlayerColors->vector;
	return colorArray[colorId];
}

std::filesystem::path getModulePath(HMODULE hModule) {
	TCHAR buff[MAX_PATH];
	GetModuleFileName(hModule, buff, MAX_PATH);
	return std::filesystem::path(buff);
}

std::string getGameVersion() {
	if (app::Application_get_version != nullptr)
		return convert_from_string(app::Application_get_version(NULL));
	else
		return "unavailable";
}

SystemTypes__Enum GetSystemTypes(const Vector2& vector) {
	if (*Game::pShipStatus) {
		auto shipStatus = *Game::pShipStatus;
		for (auto room : il2cpp::Array(shipStatus->fields._AllRooms_k__BackingField))
			if (room->fields.roomArea != nullptr && app::Collider2D_OverlapPoint(room->fields.roomArea, vector, NULL)) 
				return room->fields.RoomId;
	}
	return SystemTypes__Enum::Outside;
}

std::optional<EVENT_PLAYER> GetEventPlayer(GameData_PlayerInfo* playerInfo)
{
	if (!playerInfo) return std::nullopt;
	return EVENT_PLAYER(playerInfo);
}

std::optional<EVENT_PLAYER> GetEventPlayerControl(PlayerControl* player)
{
	GameData_PlayerInfo* playerInfo = player->fields._cachedData;

	if (!playerInfo) return std::nullopt;
	return EVENT_PLAYER(playerInfo);
}

std::optional<Vector2> GetTargetPosition(GameData_PlayerInfo* playerInfo)
{
	if (!playerInfo) return std::nullopt;
	if (Object_1_IsNull((Object_1*)playerInfo->fields._object)) {
		// TODO: Replace all "playerInfo->fields._object" with "GameData_PlayerInfo_get_Object(playerInfo)"
		return std::nullopt;
	}
	return PlayerControl_GetTruePosition(playerInfo->fields._object, NULL);
}

il2cpp::Array<Camera__Array> GetAllCameras() {
	int32_t cameraCount = app::Camera_get_allCamerasCount(nullptr);
	il2cpp::Array cameraArray = (Camera__Array*)il2cpp_array_new((Il2CppClass*)app::Camera__TypeInfo, cameraCount);
	int32_t returnedCount = app::Camera_GetAllCameras(cameraArray.get(), nullptr);
	assert(returnedCount == cameraCount);
	return cameraArray;
}

il2cpp::List<List_1_InnerNet_ClientData_> GetAllClients()
{
	return (*Game::pAmongUsClient)->fields._.allClients;
}

Vector2 GetSpawnLocation(int32_t playerId, int32_t numPlayer, bool initialSpawn)
{
	if (State.mapType == Settings::MapType::Ship || State.mapType != Settings::MapType::Pb || initialSpawn)
	{
		Vector2 vector = { 0, 1 };
		vector = Rotate(vector, (float)(playerId - 1) * (360.f / (float)numPlayer));
		float radius = (*Game::pShipStatus)->fields.SpawnRadius;
		vector = { vector.x * radius, vector.y * radius };
		Vector2 spawncenter = (initialSpawn ? (*Game::pShipStatus)->fields.InitialSpawnCenter : (*Game::pShipStatus)->fields.MeetingSpawnCenter);
		return { spawncenter.x + vector.x, spawncenter.y + vector.y + 0.3636f };
	}
	if (playerId < 5)
	{
		Vector2 spawncenter = (*Game::pShipStatus)->fields.MeetingSpawnCenter;
		return { (spawncenter.x + 1) * (float)playerId, spawncenter.y * (float)playerId };
	}
	Vector2 spawncenter = (*Game::pShipStatus)->fields.MeetingSpawnCenter2;
	return { (spawncenter.x + 1) * (float)(playerId - 5), spawncenter.y * (float)(playerId - 5) };
}

bool IsAirshipSpawnLocation(const Vector2& vec)
{
	return (State.mapType == Settings::MapType::Airship);
}

Vector2 Rotate(const Vector2& vec, float degrees)
{
	float f = 0.017453292f * degrees;
	float num = cos(f);
	float num2 = sin(f);
	return { vec.x * num - num2 * vec.y, vec.x * num2 + num * vec.y };
}

bool Equals(const Vector2& vec1, const Vector2& vec2) {
	return vec1.x == vec2.x && vec1.y == vec2.y;
}

std::string ToString(Object* object) {
	std::string type = convert_from_string(Object_ToString(object, NULL));
	if (type == "System.String") {
		return convert_from_string((String*)object);
	}
	return type;
}

#define ADD_QUOTES_HELPER(s) #s
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s)

std::string GetGitCommit()
{
#ifdef GIT_CUR_COMMIT
	return ADD_QUOTES(GIT_CUR_COMMIT);
#endif
	return "unavailable";
}

std::string GetGitBranch()
{
#ifdef GIT_BRANCH
	return ADD_QUOTES(GIT_BRANCH);
#endif
	return "unavailable";
}

void ImpersonateName(PlayerSelection player)
{
	app::GameData_PlayerOutfit* outfit = GetPlayerOutfit(player.get_PlayerData());
	if (!(IsInGame() || IsInLobby() || outfit)) return;
	if (convert_from_string(outfit->fields._playerName).length() < 10) {
		if (IsInGame())
			State.rpcQueue.push(new RpcSetName(convert_from_string(outfit->fields._playerName) + " "));
		else if (IsInLobby())
			State.lobbyRpcQueue.push(new RpcSetName(convert_from_string(outfit->fields._playerName) + " "));
	}
	else {
		if (IsInGame())
			State.rpcQueue.push(new RpcSetName(convert_from_string(outfit->fields._playerName)));
		else if (IsInLobby())
			State.lobbyRpcQueue.push(new RpcSetName(convert_from_string(outfit->fields._playerName)));
	}
}

int GetRandomColorId()
{
	int colorId = 0;
	auto PlayerColors = il2cpp::Array(app::Palette__TypeInfo->static_fields->PlayerColors);
	assert(PlayerColors.size() > 0);
	if (IsInGame() || IsInLobby())
	{
		auto players = GetAllPlayerControl();
		std::vector<int> availableColors = { };
		for (size_t i = 0; i < PlayerColors.size(); i++)
		{
			bool colorAvailable = true;
			for (PlayerControl* player : players)
			{
				app::GameData_PlayerOutfit* outfit = GetPlayerOutfit(GetPlayerData(player));
				if (outfit == NULL) continue;
				if (i == outfit->fields.ColorId)
				{
					colorAvailable = false;
					break;
				}
			}

			if (colorAvailable)
				availableColors.push_back((int)i);
		}
		assert(availableColors.size() > 0);
		colorId = availableColors.at(randi(0, (int)availableColors.size() - 1));
	}
	else
	{
		colorId = randi(0, (int)PlayerColors.size() - 1);
	}
	return colorId;
}

void SaveOriginalAppearance()
{
	PlayerSelection player = *Game::pLocalPlayer;
	app::GameData_PlayerOutfit* outfit = GetPlayerOutfit(player.get_PlayerData());
	if (outfit == NULL) return;
	LOG_DEBUG("Set appearance values to current player");
	State.originalName = convert_from_string(outfit->fields._playerName);
	State.originalSkin = outfit->fields.SkinId;
	State.originalHat = outfit->fields.HatId;
	State.originalPet = outfit->fields.PetId;
	State.originalColor = outfit->fields.ColorId;
	State.activeImpersonation = false;
	State.originalVisor = outfit->fields.VisorId; 
	State.originalNamePlate = outfit->fields.NamePlateId;
}

void ResetOriginalAppearance()
{
	LOG_DEBUG("Reset appearance values to invalid");
	State.originalSkin = nullptr;
	State.originalHat = nullptr;
	State.originalPet = nullptr;
	State.originalColor = 0xFF;
	State.originalVisor = nullptr;
	State.originalNamePlate = nullptr;
}

GameData_PlayerOutfit* GetPlayerOutfit(GameData_PlayerInfo* player, bool includeShapeshifted /* = false */) {
	if (!player) return nullptr;
	const il2cpp::Dictionary dic(player->fields.Outfits);
	if (includeShapeshifted) {
		auto playerOutfit = dic[PlayerOutfitType__Enum::Shapeshifted];
		if (playerOutfit && !convert_from_string(playerOutfit->fields._playerName).empty()) {
			return playerOutfit;
		}
	}
	return dic[PlayerOutfitType__Enum::Default];
}

bool PlayerIsImpostor(GameData_PlayerInfo* player) {

	if (player->fields.Role == nullptr) return false;

	RoleBehaviour* role = player->fields.Role;
	return role->fields.TeamType == RoleTeamTypes__Enum::Impostor;
}


Color GetRoleColor(RoleBehaviour* roleBehaviour) {
	if (roleBehaviour == nullptr) return Palette__TypeInfo->static_fields->White;

	app::Color c;
	switch (roleBehaviour->fields.Role) {
	default:
	case RoleTypes__Enum::Crewmate:
		c = Palette__TypeInfo->static_fields->White;
		break;
	case RoleTypes__Enum::Engineer:
	case RoleTypes__Enum::GuardianAngel:
	case RoleTypes__Enum::Scientist:
		c = Palette__TypeInfo->static_fields->CrewmateBlue;
		break;
	case RoleTypes__Enum::Impostor:
	case RoleTypes__Enum::Shapeshifter:
		c = Palette__TypeInfo->static_fields->ImpostorRed;
		break;
	}
	return c;
}

std::string GetRoleName(RoleBehaviour* roleBehaviour, bool abbreviated /* = false */)
{
	if (roleBehaviour == nullptr) return (abbreviated ? "Unk" : "Unknown");

	switch (roleBehaviour->fields.Role)
	{
		case RoleTypes__Enum::Engineer:
			return (abbreviated ? "Eng" : "Engineer");
		case RoleTypes__Enum::GuardianAngel:
			return (abbreviated ? "GA" : "GuardianAngel");
		case RoleTypes__Enum::Impostor:
			return (abbreviated ? "Imp" : "Impostor");
		case RoleTypes__Enum::Scientist:
			return (abbreviated ? "Sci" : "Scientist");
		case RoleTypes__Enum::Shapeshifter:
			return (abbreviated ? "SH" : "Shapeshifter");
		case RoleTypes__Enum::Crewmate:
			return (abbreviated ? "Crew" : "Crewmate");
		default:
			return (abbreviated ? "Unk" : "Unknown");
	}
}

RoleTypes__Enum GetRoleTypesEnum(RoleType role)
{
	if (role == RoleType::Shapeshifter) {
		return RoleTypes__Enum::Shapeshifter;
	}
	else if (role == RoleType::Impostor) {
		return RoleTypes__Enum::Impostor;
	}
	else if (role == RoleType::Engineer) {
		return RoleTypes__Enum::Engineer;
	}
	else if (role == RoleType::Scientist) {
		return RoleTypes__Enum::Scientist;
	}
	return RoleTypes__Enum::Crewmate;
}

float GetDistanceBetweenPoints_Unity(const Vector2& p1, const Vector2& p2)
{
	float dx = p1.x - p2.x, dy = p1.y - p2.y;
	return sqrtf(dx * dx + dy * dy);
}

float GetDistanceBetweenPoints_ImGui(const ImVec2& p1, const ImVec2& p2)
{
	float dx = p1.x - p2.x, dy = p1.y - p2.y;
	return sqrtf(dx * dx + dy * dy);
}

void DoPolylineSimplification(std::vector<ImVec2>& inPoints, std::vector<std::chrono::system_clock::time_point>& inTimeStamps, std::vector<ImVec2>& outPoints, std::vector<std::chrono::system_clock::time_point>& outTimeStamps, float sqDistanceThreshold, bool clearInputs)
{
	sqDistanceThreshold = sqDistanceThreshold - FLT_EPSILON;
	size_t numPendingPoints = inPoints.size();
	if (numPendingPoints < 2)
		return;

	Profiler::BeginSample("PolylineSimplification");
	ImVec2 prevPoint = inPoints[0], point = inPoints[0];
	std::chrono::system_clock::time_point timestamp = inTimeStamps[0];
	size_t numNewPointsAdded = 0;

	// always add the first point
	outPoints.push_back(point);
	outTimeStamps.push_back(timestamp);
	numNewPointsAdded++;
	for (size_t index = 1; index < numPendingPoints; index++)
	{
		point = inPoints[index];
		timestamp = inTimeStamps[index];
		float diffX = point.x - prevPoint.x, diffY = point.y - prevPoint.y;
		if ((diffX * diffX + diffY * diffY) >= sqDistanceThreshold)
		{
			prevPoint = point;
			// add the point if it's beyond the distance threshold of prev point.
			outPoints.push_back(point);
			outTimeStamps.push_back(timestamp);
			numNewPointsAdded++;
		}
	}
	// add the last point if it's not also the first point nor has already been added as the last point
	if ((point.x != prevPoint.x) || (point.y != prevPoint.y))
	{
		outPoints.push_back(point);
		outTimeStamps.push_back(timestamp);
		numNewPointsAdded++;
	}

	if (clearInputs)
	{
		inPoints.clear();
		inTimeStamps.clear();
	}
	Profiler::EndSample("PolylineSimplification");
}

float getMapXOffsetSkeld(float x)
{
	return (State.mapType == Settings::MapType::Ship && State.FlipSkeld) ? x - 50.0f : x;
}

bool Object_1_IsNotNull(app::Object_1* obj)
{
	return app::Object_1_op_Implicit(obj, nullptr);
}

bool Object_1_IsNull(app::Object_1* obj)
{
	return !Object_1_IsNotNull(obj);
}

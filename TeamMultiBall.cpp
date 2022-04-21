#include "pch.h"
#include "TeamMultiBall.h"
#include "RenderingTools/RenderingTools.h"
#include <sstream>

BAKKESMOD_PLUGIN(TeamMultiBall, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void TeamMultiBall::onLoad()
{
	_globalCvarManager = cvarManager;
	//cvarManager->log("Plugin loaded!");

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	cvarManager->log("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	cvarManager->log("the cvar with name: " + cvarName + " changed");
	//	cvarManager->log("the new value is:" + newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&TeamMultiBall::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);
	gameWrapper->HookEventPost("Function TAGame.Car_TA.SetVehicleInput", [this](...) {onTick(); });
	cvarManager->registerNotifier("spawnBalls", [this](std::vector<std::string> params){createAndIdentifyBalls(true);}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&TeamMultiBall::FUNCTION, this, _1, _2, _3));
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {renderGame(canvas);});


	gameWrapper->HookEvent("Function TAGame.MatchType_TA.Init", [this](std::string eventName) {reloadPlugin(); });
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&TeamMultiBall::YourPluginMethod, this);
	Netcode = std::make_shared<NetcodeManager>(cvarManager, gameWrapper, exports, [this](const std::string& Message, PriWrapper Sender) {OnMessageReceived(Message, Sender); });
	CVarWrapper netcodeLogCvar = cvarManager->getCvar("NETCODE_Log_Level");
	if (netcodeLogCvar) {
		netcodeLogCvar.setValue("2");
	}
}

void TeamMultiBall::bounceBall(BallWrapper ball) {
	auto ballLoc = ball.GetLocation();
	auto velocity = ball.GetVelocity();
	auto speed = velocity.magnitude();
	// converts speed to km/h from cm/s
	speed *= 0.036f;
	speed += 0.5f;

	//  5028
	if (ballLoc.Y > backWall && velocity.Y > 0) {
		// ball is going in orange net
		cvarManager->log("shot taken at orange net " + std::to_string(speed));
		velocity.Y = -velocity.Y;
		ball.SetVelocity(velocity);
	} else if (ballLoc.Y < -backWall && velocity.Y < 0) {
		// ball is going in blue net
		cvarManager->log("shot taken at blue net " + std::to_string(speed));

		velocity.Y = -velocity.Y;
		ball.SetVelocity(velocity);
	}
}

void TeamMultiBall::onTick() {
	if (gameWrapper->IsInOnlineGame()) {
		return; // only do this if we are host
	}
	if (!ballsValid()) {
		createAndIdentifyBalls(true);
	}
	BallWrapper blueBall = BallWrapper(blueBallAddr);
	if (!blueBall) {
		LOG("blueBall NULL");
		return;
	}
	if (blueBall.GetLocation().Y < -5130 /*field length*/ + blueBall.GetRadius()) {
		LOG("bouncing blue");
		bounceBall(blueBall);
	}
	BallWrapper orangeBall = BallWrapper(orangeBallAddr);
	if (!orangeBall) {
		LOG("orangeBall NULL");
		return;
	}
	if (orangeBall.GetLocation().Y > 5130 /*field length*/ - orangeBall.GetRadius()) {
		LOG("bouncing orange");
		bounceBall(orangeBall);
	}
}

void TeamMultiBall::createAndIdentifyBalls(bool allowRecursion) {
	if (!gameWrapper->IsInOnlineGame()) {
		// only do this if we're the host
		ServerWrapper sw = gameWrapper->GetCurrentGameState();
		if (!sw) {
			LOG("NULL SERVER");
			return;
		}
		ArrayWrapper<BallWrapper> balls = sw.GetGameBalls();
		LOG("{} balls found, game expects {}", balls.Count(), sw.GetTotalGameBalls());
		if (balls.Count() != 2) {
			LOG("Incorrect number of balls ({})", balls.Count());
			if (allowRecursion) {
				LOG("Spawning balls...");
				sw.SetTotalGameBalls(2);
				sw.ResetBalls();
				createAndIdentifyBalls(false);
			}
			return;
		}
		BallWrapper firstBall = balls.Get(0);
		if (!firstBall) {
			LOG("NULL firstBall, LENGTH 1");
			return;
		}
		blueBallAddr = firstBall.memory_address;
		BallWrapper secondBall = balls.Get(1);
		if (!secondBall) {
			LOG("NULL secondBall, LENGTH 1");
			return;
		}
		orangeBallAddr = secondBall.memory_address;
	}
	else {
		// we are the client and need to ask the server for help if we're not already waiting
		if (!clientIsWaitingOnLocation) {
			Netcode->SendNewMessage("ball_loc");
			clientIsWaitingOnLocation = true;
		}
	}
}

void TeamMultiBall::onUnload()
{
}

void TeamMultiBall::reloadPlugin()
{
	blueBallAddr = NULL;
	orangeBallAddr = NULL;
	clientIsWaitingOnLocation = false;
}

bool TeamMultiBall::ballsValid() {
	ServerWrapper sw = gameWrapper->GetCurrentGameState();
	if (!sw) {
		LOG("NULL sERVER");
		return false;
	}
	ArrayWrapper<BallWrapper> balls = sw.GetGameBalls();
	if (balls.Count() != 2) {
		return false;
	}

	bool isValid = true;
	for (BallWrapper ball : balls) {
		if (!ball) {
			LOG("nuLL ball");
			return false;
		}
		isValid = isValid && (ball.memory_address == blueBallAddr || ball.memory_address == orangeBallAddr);
	}
	return isValid;
}

void TeamMultiBall::renderGame(CanvasWrapper canvas) {
	
	int playlistID = 0;
	ServerWrapper sw = gameWrapper->GetCurrentGameState();
	if (!sw) {
		LOG("Null server");
		return;
	} else {
		GameSettingPlaylistWrapper playlist = sw.GetPlaylist();
		if (playlist) playlistID = playlist.GetPlaylistId();
		
	}
	
	if (!gameWrapper->IsInOnlineGame() || playlistID == 24) {
		
		auto camera = gameWrapper->GetCamera();
		if (camera.IsNull()) { LOG("Null camera"); return; }
		RT::Frustum frust{ canvas, camera };
		if (!ballsValid()) {
			createAndIdentifyBalls(true);
		}
		BallWrapper blueBall = BallWrapper(blueBallAddr);
		if (!blueBall) {
			LOG("blueBall NULL");
			return;
		}
		LinearColor blue{ 0, 0, 255, 255 };
		canvas.SetColor(blue);
		RT::Sphere(blueBall.GetLocation(), RotatorToQuat(blueBall.GetRotation()), blueBall.GetRadius()).Draw(canvas, frust, camera.GetLocation(), 30);
		BallWrapper orangeBall = BallWrapper(orangeBallAddr);
		if (!orangeBall) {
			LOG("Orange ball null");
			return;
		}
		LinearColor orange{ 255, 165, 0, 255 };
		canvas.SetColor(orange);
		RT::Sphere(orangeBall.GetLocation(), RotatorToQuat(orangeBall.GetRotation()), orangeBall.GetRadius()).Draw(canvas, frust, camera.GetLocation(), 30);
	}
}

void TeamMultiBall::OnMessageReceived(const std::string& Message, PriWrapper Sender)
{
	LOG("Got Netcode message: {}", Message);
	if (!gameWrapper->IsInOnlineGame()) {
		// if we're the host
		if (Message == "ball_loc") {
			LOG("Host got ball request");
			if (!ballsValid()) {
				createAndIdentifyBalls(true);
			}
			BallWrapper blueBall = BallWrapper(blueBallAddr);
			if (!blueBall) {
				LOG("REQ'd BALL NULL!");
				Netcode->SendNewMessage("ball_failed");
				return;
			}
			Vector ballLoc = blueBall.GetLocation();
			Netcode->SendNewMessage(std::to_string((int)ballLoc.X) + "," + std::to_string((int)ballLoc.Y) + "," + std::to_string((int)ballLoc.Z));
		} else {
			LOG("Got unexpected message {}", Message);
		}

	}
	else {
		// if we're the client
		if (Message == "ball_failed") {
			LOG("SERVER SAYS BALL FAILED");
			clientIsWaitingOnLocation = false;
			return;
		}
		std::vector<std::string> ballCoords = splitOnChar(Message, ',');
		if(ballCoords.size() != 3){
			LOG("Got non-conforming ({} splits) message {}", ballCoords.size(), Message);
			clientIsWaitingOnLocation = false;
			return;
		}
		Vector blueBallLoc{ 0, 0, 0 };
		try{
			blueBallLoc.X = std::stoi(ballCoords.at(0));
			blueBallLoc.Y = std::stoi(ballCoords.at(1));
			blueBallLoc.Z = std::stoi(ballCoords.at(2));
		}
		catch (const std::invalid_argument& ia) {
			LOG("Invalid argument converting string to int: {}", ia.what());
			clientIsWaitingOnLocation = false;
			return;
		}
		// find the ball closest to the coords
		ServerWrapper sw = gameWrapper->GetCurrentGameState();
		if (!sw) {
			LOG("NUlL SERVER");
			clientIsWaitingOnLocation = false;
			return;
		}
		ArrayWrapper<BallWrapper> balls = sw.GetGameBalls();
		LOG("{} balls found, game expects {}", balls.Count(), sw.GetTotalGameBalls());
		if (balls.Count() != 2) {
			LOG("Incorrect number of balls ({})", balls.Count());
			clientIsWaitingOnLocation = false;
			return;
		}
		BallWrapper firstBall = balls.Get(0);
		if (!firstBall) {
			LOG("NULL firstBall, LENGTH 1");
			clientIsWaitingOnLocation = false;
			return;
		}
		BallWrapper secondBall = balls.Get(1);
		if (!secondBall) {
			LOG("NULL secondBall, LENGTH 1");
			clientIsWaitingOnLocation = false;
			return;
		}
		float firstBallDist = getDistance(blueBallLoc, firstBall.GetLocation());
		float secondBallDist = getDistance(blueBallLoc, secondBall.GetLocation());
		LOG("Ball distances: 1st: {}, 2nd: {}", round(firstBallDist), round(secondBallDist));
		if(firstBallDist < secondBallDist){
			blueBallAddr = firstBall.memory_address;
			orangeBallAddr = secondBall.memory_address;
		} else {
			blueBallAddr = secondBall.memory_address;
			orangeBallAddr = firstBall.memory_address;
		}
		clientIsWaitingOnLocation = false;
	}
}
std::vector<std::string> TeamMultiBall::splitOnChar(std::string inString, char delimiter){
	std::stringstream inStream(inString);
	std::string segment;
	std::vector<std::string> seglist;

	while(std::getline(inStream, segment, delimiter))
	{
	seglist.push_back(segment);
	}	
	return seglist;
}
float TeamMultiBall::getDistance(Vector a, Vector b){
	return sqrt(std::pow(a.X - b.X, 2) + std::pow(a.Y - b.Y, 2) + std::pow(a.Z - b.Z, 2));
}
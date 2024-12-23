#include "pch.h"
#include <fstream>
#include <iostream>
#include <string>
#include "AICoachBakkesPlugin.h"
#include "bakkesmod/wrappers/GameEvent/TutorialWrapper.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/BallWrapper.h"

BAKKESMOD_PLUGIN(AICoachBakkesPlugin, "AI Coach", plugin_version, PLUGINTYPE_THREADED)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

//float waitTime = 5; // Timer to track logging interval
//float nextTime = 0.0f;
bool isDribble = false;

void AICoachBakkesPlugin::onLoad()
{
    _globalCvarManager = cvarManager;
    cvarManager->registerNotifier("ai_dribble", std::bind(&AICoachBakkesPlugin::OnCommand, this, std::placeholders::_1), "Starts/stops recording of macro", PERMISSION_FREEPLAY);

    LOG("AI Coach Plugin loaded!");
    //server_thread = std::thread(std::bind(&AICoachBakkesPlugin::RunTracking, this));
    gameWrapper->HookEvent("Function TAGame.Ball_TA.OnRigidBodyCollision", std::bind(&AICoachBakkesPlugin::OnDroppedBall, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.Mutator_Freeplay_TA.Init", bind(&AICoachBakkesPlugin::OnFreeplayLoad, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&AICoachBakkesPlugin::OnFreeplayDestroy, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", bind(&AICoachBakkesPlugin::OnRecordTick, this, std::placeholders::_1));

    //server_thread.detach();
}
void AICoachBakkesPlugin::OnFreeplayLoad(std::string eventName)
{
    LOG("AI Coach Plugin loaded!");
}

void AICoachBakkesPlugin::OnFreeplayDestroy(std::string eventName)
{
    gameWrapper->UnregisterDrawables();
    gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
}

void AICoachBakkesPlugin::OnDroppedBall(std::string eventName)
{
    if (isDribble) {
        if (!gameWrapper->IsInFreeplay())
            return;
        ServerWrapper tutorial = gameWrapper->GetGameEventAsServer();
        if (tutorial.GetGameBalls().Count() == 0)
            return;

        BallWrapper ball = tutorial.GetGameBalls().Get(0);
        CarWrapper car = tutorial.GetGameCar();
        if (ball.IsNull() || car.IsNull())
            return;

        Vector ballLocation = ball.GetLocation();
        Vector ballVelocity = ball.GetVelocity();

        if (ballLocation.Z <= 94) {
            // Ball is near or on the ground
            LOG("dropped the ball");
            isDribble = false;

            std::ofstream myfile;
            myfile.open("C:/OneDriveTemp/data.csv", std::ios::out | std::ios::app);  // Open in append mode (to not overwrite the file)
            for (unsigned int i = 0; i < playbackData.size(); i++)
            {
                myfile << playbackData.at(i) + "\n";
            }
            myfile.close();
            playbackData.clear();

        }      
    }
}

void AICoachBakkesPlugin::OnRecordTick(std::string eventName)
{
    if (!isDribble)
        return;

    ServerWrapper server = gameWrapper->GetGameEventAsServer();
    auto players = gameWrapper->GetGameEventAsServer().GetCars();


    if (!gameWrapper->IsInFreeplay())
        return;
    ServerWrapper tutorial = gameWrapper->GetGameEventAsServer();
    if (tutorial.GetGameBalls().Count() == 0)
        return;

    BallWrapper ball = tutorial.GetGameBalls().Get(0);
    CarWrapper car = tutorial.GetGameCar();
    if (ball.IsNull() || car.IsNull())
        return;

    std::string data_string = std::to_string(server.GetSecondsElapsed()) + "," + std::to_string(car.GetLocation().X) + "," + std::to_string(car.GetLocation().Y) + "," + std::to_string(car.GetLocation().Z) + "," + std::to_string(ball.GetLocation().X) + "," + std::to_string(ball.GetLocation().Y) + "," + std::to_string(ball.GetLocation().Z);
    playbackData.push_back(data_string);

}

void AICoachBakkesPlugin::OnCommand(std::vector<std::string> params)
{
    std::string command = params.at(0);
    if (command.compare("ai_dribble") == 0) {
        if (!gameWrapper->IsInFreeplay())
            return;
        ServerWrapper tutorial = gameWrapper->GetGameEventAsServer();


        if (tutorial.GetGameBalls().Count() == 0)
            return;

        BallWrapper ball = tutorial.GetGameBalls().Get(0);
        CarWrapper car = tutorial.GetGameCar();
        if (ball.IsNull() || car.IsNull())
            return;
        Vector playerVelocity = car.GetVelocity();
        Vector addToBall = Vector(playerVelocity.X, playerVelocity.Y, 170);

        addToBall.X = std::max(std::min(20.0f, addToBall.X), -20.0f);
        addToBall.Y = std::max(std::min(30.0f, addToBall.Y), -30.0f);

        ball.SetLocation(car.GetLocation() + addToBall);
        ball.SetVelocity(playerVelocity);
        isDribble = true;

        std::ofstream myfile;
        myfile.open("C:/OneDriveTemp/data.csv");
        myfile << "time,car_x,car_y,car_z,ball_x,ball_y,ball_z\n";
        myfile.close();

    }
}
//void AICoachBakkesPlugin::RunTracking()
//{
//    gameWrapper->SetTimeout([this](GameWrapper* gw) {
//        if (gw->IsInGame() || gw->IsInOnlineGame() || gw->IsInFreeplay()) {
//            LOG(getCarStates());
//            ServerWrapper tutorial = gw->GetGameEventAsServer();
//            BallWrapper ball = tutorial.GetGameBalls().Get(0);
//            LOG(std::to_string(ball.GetLocation().Z));
//
//        }
//        if (gw->IsInReplay()) {
//            LOG(getCarStates());
//        }
//        RunTracking();
//        }, waitTime);
//}


//// Function to get the state of all cars in a JSON-like format
//std::string AICoachBakkesPlugin::getCarStates()
//{
//    std::string carStates = "";
//    auto game = gameWrapper->GetGameEventAsServer();
//    if (!game) return carStates; 
//
//    auto cars = game.GetCars(); 
//
//
//    if (cars.Count() == 0) return carStates;
//
//    bool firstCar = true;
//    for (auto car : cars) 
//    {
//        if (!firstCar) carStates += ",";
//        firstCar = false;
//
//        Vector position = car.GetLocation();
//        float boost = 0.0f; 
//
//        if (car.GetbOverrideBoostOn())
//        {
//            boost = car.GetMaxDriveForwardSpeed();
//        }
//        carStates += car.GetOwnerName()+":"+ std::to_string(boost) + ":" + std::to_string(position.X) + ":" + std::to_string(position.Y) + ":" + std::to_string(position.Z);
//
//    }
//
//    return carStates;
//}

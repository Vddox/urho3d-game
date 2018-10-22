

#include "Game.h"
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Scene/ValueAnimation.h>



#include "../Scenes/World/World.hpp"

using namespace Urho3D;

/**
* This happens before the engine has been initialized
* so it's usually minimal code setting defaults for
* whatever instance variables you have.
* You can also do this in the Setup method.
*/
Game::Game(Context* context)
    : Application(context)

{
}

void Game::Setup()
{
    // These parameters should be self-explanatory.
    // See http://urho3d.github.io/documentation/1.5/_main_loop.html
    // for a more complete list.
    engineParameters_["FullScreen"] = false;
    engineParameters_["WindowWidth"] = 1920;
    engineParameters_["WindowHeight"] = 1080;
    engineParameters_["WindowResizable"] = true;
}

void Game::Start()
{


    // Create MainMenu scene, start the scene and set it to be the current viewport
    world_ = std::make_unique<World>(context_);
    world_->start();

    GetSubsystem<Input>()->SetMouseVisible(true);

    // Setup logic update callback to handle networking
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));


}

void Game::Stop()
{
}


void Game::HandleUpdate(StringHash eventType, VariantMap& eventData)
{

    io_context_.poll();
}

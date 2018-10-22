//
// Created by ahodges on 20/10/18.
//

#pragma once



#include <string>
#include <sstream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>

#include <Urho3D/IO/Log.h>


#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/ToolTip.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/View3D.h>

#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/RenderPath.h>


#include "../../ObjectHandlers/MissileController.hpp"
#include "../../ObjectHandlers/AlertController.hpp"
#include "../../UI/AlertMaker.hpp"

using namespace Urho3D;

struct World : Object{

    URHO3D_OBJECT(World, Object);

    World(Context * context);

    void start();

    void CreateAlert(const std::string & text, const float lifeTime);

    void SetWorldColour(Scene* scene);

    void CreateOverlayCamera();

    void CreateMissilePreview(ResourceCache* cache);

    void CreatePlane(ResourceCache* cache);

    void CreateText(ResourceCache* cache);

    void CreateButton();

    void CreateAmbientLigthing();

    void CreateMushrooms(ResourceCache* cache);

    void CreateBoxes(ResourceCache* cache);

    void CreateDirectionLight();

    void CreateParticleEmmitter(ResourceCache* cache);

    void CreateCamera();

    void CreateSpotLight();

    void SetupViewport();

    void SubscribeToEvents();

    void UnSubscribeFromAllEvents();



    void HandleClosePressed(StringHash eventType,VariantMap& eventData);

    void HandleClick(StringHash eventType,VariantMap& eventData);


    void HandleMouseWheel(StringHash eventType,VariantMap& eventData);



    void HandleKeyDown(StringHash eventType,VariantMap& eventData);

    void HandleUpdate(StringHash eventType,VariantMap& eventData);



private:
    int framecount_;
    float time_;

    SharedPtr<Context> context_;

    SharedPtr<Scene> scene_;
    SharedPtr<Scene> overlayScene_;

    SharedPtr<Node> cameraNode_;
    SharedPtr<Node> overlayCameraNode_;

    Camera* camera_;
    Camera* overlayCamera_;

    SharedPtr<ResourceCache> cache_;

    SharedPtr<UIElement> uiRoot_;

    SharedPtr<Text> text_;

    std::unique_ptr<MissileController> missileController_;
    std::unique_ptr<AlertController> alertController_;
    std::unique_ptr<AlertMaker> alertMaker_;

    float camera_zoom_ = 1;
};



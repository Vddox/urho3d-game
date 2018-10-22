//
// Created by ahodges on 20/10/18.
//

#include "World.hpp"
#include <iostream>



World::World(Context *context)
                :Object(context),
                context_(context),
                framecount_(0),
                time_(0)
                {
    // Init cache and UI
    cache_ = context_->GetSubsystem<ResourceCache>();
    uiRoot_ = context_->GetSubsystem<UI>()->GetRoot();

    // Set default style
    auto* style = cache_->GetResource<XMLFile>("UI/DefaultStyle.xml");
    uiRoot_->SetDefaultStyle(style);

    // Init scene
    scene_ = new Scene(context_);
    overlayScene_ = new Scene(context_);
}

void World::start() {


    // Let's use the default style that comes with Urho3D.
    auto* style = cache_->GetResource<XMLFile>("UI/DefaultStyle.xml");
    uiRoot_->SetDefaultStyle(style);

    // Let's create some text to display.
    CreateText(cache_);

    // Add a button, just as an interactive UI sample.
    CreateButton();

    // Setup Controllers
    missileController_ = std::make_unique<MissileController>(scene_);
    alertController_ = std::make_unique<AlertController>(context_);

    // Create an alert maker
    alertMaker_ = std::make_unique<AlertMaker>(context_,*alertController_);

    // Let the scene have an Octree component!
    scene_->CreateComponent<Octree>();
    overlayScene_->CreateComponent<Octree>();

    // Set the overlayScenes colour
    SetWorldColour(overlayScene_);

    // Let's add an additional scene component for fun.
    scene_->CreateComponent<DebugRenderer>();

    // Create plane node & StaticModel component for showing a static plane
    CreatePlane(cache_);

    // Zone component for ambient lighting and fog control
    CreateAmbientLigthing();

    // Create some mushrooms
    CreateMushrooms(cache_);

    // Create randomly sized boxes. If boxes are big enough, make them occluders
    CreateBoxes(cache_);

    // Create a directional light to the world. Enable cascaded shadows on it
    CreateDirectionLight();

    // Create a particle emmitter
    CreateParticleEmmitter(cache_);


    // We need a camera from which the viewport can render.
    CreateCamera();

    // Create overlay camera to render missile preview from.
    CreateOverlayCamera();

    // Create missile preview
    CreateMissilePreview(cache_);



    // add a green spot light to the camera node
    CreateSpotLight();

    // Now we setup the viewport. Of course, you can have more than one!
    SetupViewport();

    SubscribeToEvents();




}



///////////////// Setup stuff ///////////////////////
void World::CreateAlert(const std::string & text, const float lifeTime) {
    alertController_->CreateAlert(text, lifeTime);
}

void World::CreateMissilePreview(ResourceCache* cache){

    auto* missilePreviewNode = overlayScene_->CreateChild("missilePreview");

    auto* missilePreview = missilePreviewNode->CreateComponent<StaticModel>();
    missilePreview->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    missilePreview->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
    missilePreviewNode->SetPosition(Vector3(0,0,2.5));



    auto* missileDisplay = new Button(context_);
    missileDisplay->SetSize(128,128);
    missileDisplay->SetAlignment(HA_RIGHT,VA_TOP);
    //missileDisplay->SetTexture(cache->GetResource<Texture2D>("Textures/UrhoDecal.dds"));
    missileDisplay->SetBlendMode(BLEND_ALPHA);

    uiRoot_->AddChild(missileDisplay);


    // Create a renderable texture (1024x768, RGB format), enable bilinear filtering on it
    SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
    renderTexture->SetSize(128, 128, Graphics::GetRGBAFormat(), TEXTURE_RENDERTARGET);
    renderTexture->SetFilterMode(FILTER_BILINEAR);

    missileDisplay->SetTexture(renderTexture);


    RenderSurface* surface = renderTexture->GetRenderSurface();

    SharedPtr<Viewport> OverlayViewport(new Viewport(context_, overlayScene_, overlayCamera_));
    surface->SetViewport(0, OverlayViewport);
    surface->SetUpdateMode(RenderSurfaceUpdateMode::SURFACE_UPDATEALWAYS);
//    surface->QueueUpdate();

    auto rp = OverlayViewport->GetRenderPath()->Clone();
    for ( int i = 0; i < rp->GetNumCommands(); i++ )
    {
        RenderPathCommand * cmd = rp->GetCommand( i );
        if ( cmd->type_ == RenderCommandType::CMD_CLEAR )
        {
            cmd->useFogColor_ = false;
            cmd->clearColor_ = Color( 0.0f, 0.0f, 0.0f, 0.0f );
        }
    }
    OverlayViewport->SetRenderPath(rp);










}

void World::CreatePlane(ResourceCache* cache){
    Node* planeNode = scene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(100.0f, 1.0f, 100.0f));
    auto* planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));
}

void World::CreateText(ResourceCache* cache){
    text_ = new Text(context_);
    // Text will be updated later in the E_UPDATE handler. Keep readin'.
    text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\nWait a bit to see FPS.");
    // If the engine cannot find the font, it comes with Urho3D.
    // Set the environment variables URHO3D_HOME, URHO3D_PREFIX_PATH or
    // change the engine parameter "ResourcePrefixPath" in the Setup method.
    text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
    text_->SetColor(Color(.3,0,.3));
    text_->SetHorizontalAlignment(HA_CENTER);
    text_->SetVerticalAlignment(VA_TOP);
    uiRoot_->AddChild(text_);
}

void World::CreateButton(){
    Button* button=new Button(context_);
    // Note, must be part of the UI system before SetSize calls!
    uiRoot_->AddChild(button);
    button->SetName("Button Quit");
    button->SetStyle("Button");
    button->SetSize(32,32);
    button->SetPosition(16,116);


    // Subscribe to button release (following a 'press') events
    scene_->SubscribeToEvent(button,E_RELEASED,URHO3D_HANDLER(World,HandleClosePressed));

}

void World::CreateAmbientLigthing(){

    Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

}

void World::SetWorldColour(Scene* scene){
    Node* zoneNode = scene->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(1.0f, 1.0f, 1.0f,0.0f));
}

void World::CreateMushrooms(ResourceCache* cache){

    auto* mushroomMat = cache->GetResource<Material>("Materials/Mushroom.xml");
    // Apply shader parameter animation to material
    SharedPtr<ValueAnimation> specColorAnimation(new ValueAnimation(context_));
    specColorAnimation->SetKeyFrame(0.0f, Color(0.1f, 0.1f, 0.1f, 16.0f));
    specColorAnimation->SetKeyFrame(1.0f, Color(1.0f, 0.0f, 0.0f, 2.0f));
    specColorAnimation->SetKeyFrame(2.0f, Color(1.0f, 1.0f, 0.0f, 2.0f));
    specColorAnimation->SetKeyFrame(3.0f, Color(0.1f, 0.1f, 0.1f, 16.0f));
    // Optionally associate material with scene to make sure shader parameter animation respects scene time scale
    mushroomMat->SetScene(scene_);
    mushroomMat->SetShaderParameterAnimation("MatSpecColor", specColorAnimation);





    const unsigned NUM_MUSHROOMS = 240;
    for (unsigned i = 0; i < NUM_MUSHROOMS; ++i)
    {
        Node* mushroomNode = scene_->CreateChild("Mushroom");
        mushroomNode->SetPosition(Vector3(Random(90.0f) - 45.0f, 0.0f, Random(90.0f) - 45.0f));
        mushroomNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
        mushroomNode->SetScale(0.5f + Random(2.0f));
        auto* mushroomObject = mushroomNode->CreateComponent<StaticModel>();
        mushroomObject->SetModel(cache->GetResource<Model>("Models/Mushroom.mdl"));
        mushroomObject->SetMaterial(mushroomMat);
        mushroomObject->SetCastShadows(true);
    }
}

void World::CreateBoxes(ResourceCache* cache){
    const unsigned NUM_BOXES = 20;
    for (unsigned i = 0; i < NUM_BOXES; ++i)
    {
        Node* boxNode = scene_->CreateChild("Box");
        float size = 1.0f + Random(10.0f);
        boxNode->SetPosition(Vector3(Random(80.0f) - 40.0f, size * 0.5f, Random(80.0f) - 40.0f));
        boxNode->SetScale(size);

        auto* boxObject = boxNode->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
        boxObject->SetCastShadows(true);
        if (size >= 3.0f)
            boxObject->SetOccluder(true);
    }
}

void World::CreateDirectionLight(){
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
}

void World::CreateParticleEmmitter(ResourceCache* cache){
    assert(scene_);
    Node* particle_emmitter = scene_->CreateChild("Particle");
    particle_emmitter->SetPosition(Vector3(0,10,10));
    particle_emmitter->SetScale(Vector3(5,6,6));
    ParticleEffect* effect = cache->GetResource<ParticleEffect>("Particle/Fire.xml");
    assert(effect);
    ParticleEmitter* emitter=particle_emmitter->CreateComponent<ParticleEmitter>();
    assert(emitter);
    emitter->SetEffect(effect);
    emitter->SetEmitting(true);
    emitter->SetNumParticles(100);
    particle_emmitter->SetAnimationEnabled(true);
    particle_emmitter->SetEnabled(true);

}

void World::CreateCamera(){
    cameraNode_=scene_->CreateChild("Camera");
    camera_=cameraNode_->CreateComponent<Camera>();
    camera_->SetFarClip(300.0f);

    // Set an initial position for the front camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
    auto direction = Vector3(1.0f,-1.0f,0.0f);
    direction.Normalize();
    cameraNode_->SetDirection(direction);
}

void World::CreateOverlayCamera(){

    overlayCameraNode_ = overlayScene_->CreateChild("Camera");
    overlayCamera_ = overlayCameraNode_->CreateComponent<Camera>();
    overlayCamera_->SetFarClip(300.0f);

    overlayCameraNode_->SetPosition(Vector3(0,0,0));

}

void World::CreateSpotLight(){
    {
        Node* node_light=cameraNode_->CreateChild();
        Light* light=node_light->CreateComponent<Light>();
        node_light->Pitch(15);  // point slightly downwards
        light->SetLightType(LIGHT_SPOT);
        light->SetRange(20);
        light->SetColor(Color(.6,1,.6,1.0));
        light->SetBrightness(2.8);
        light->SetFov(25);
    }

}

void World::SetupViewport(){

    auto* graphics = context_->GetSubsystem<Graphics>();
    Renderer* renderer= context_->GetSubsystem<Renderer>();

    renderer->SetNumViewports(1);

    SharedPtr<Viewport> viewport(new Viewport(context_,scene_,cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0,viewport);



}

void World::SubscribeToEvents(){

    scene_->SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(World,HandleUpdate));
    scene_->SubscribeToEvent(E_MOUSEBUTTONUP,URHO3D_HANDLER(World,HandleClick));
    scene_->SubscribeToEvent(E_KEYDOWN,URHO3D_HANDLER(World,HandleKeyDown));
    scene_->SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(World,HandleMouseWheel));
}

void World::UnSubscribeFromAllEvents() {

    scene_->UnsubscribeFromAllEvents();
}



///////////////////////////////////////// Event Callbacks ////////////////////////////////////////////////////////

void World::HandleUpdate(StringHash eventType, VariantMap &eventData) {
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
    framecount_++;
    time_+=timeStep;
    // Movement speed as world units per second
    float MOVE_SPEED=10.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY=0.1f;

    if(time_ >=1)
    {
        std::string str;
        str.append("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n");
        {
            std::ostringstream ss;
            ss<<framecount_;
            std::string s(ss.str());
            str.append(s.substr(0,6));
        }
        str.append(" frames in ");
        {
            std::ostringstream ss;
            ss<<time_;
            std::string s(ss.str());
            str.append(s.substr(0,6));
        }
        str.append(" seconds = ");
        {
            std::ostringstream ss;
            ss<<(float)framecount_/time_;
            std::string s(ss.str());
            str.append(s.substr(0,6));
        }
        str.append(" fps");
        String s(str.c_str(),str.size());
        text_->SetText(s);
        URHO3D_LOGINFO(s);     // this show how to put stuff into the log
        framecount_=0;
        time_=0;
    }


    if(!context_->GetSubsystem<Input>()->IsMouseVisible())
    {
        Input* input= context_->GetSubsystem<Input>();
        if(input->GetKeyDown(KEY_SHIFT))
            MOVE_SPEED*=10;
        if(input->GetKeyDown(KEY_W))
            cameraNode_->Translate(Vector3(0,0, 1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown(KEY_S))
            cameraNode_->Translate(Vector3(0,0,-1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown(KEY_A))
            cameraNode_->Translate(Vector3(-1,0,0)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown(KEY_D))
            cameraNode_->Translate(Vector3( 1,0,0)*MOVE_SPEED*timeStep);



        // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
        IntVector2 mouseMove=input->GetMouseMove();
        static float yaw_=0;
        static float pitch_=0;
        yaw_+=MOUSE_SENSITIVITY*mouseMove.x_;
        pitch_+=MOUSE_SENSITIVITY*mouseMove.y_;
        pitch_=Clamp(pitch_,-90.0f,90.0f);
        // Reset rotation and set yaw and pitch again
        cameraNode_->SetDirection(Vector3::FORWARD);
        cameraNode_->Yaw(yaw_);
        cameraNode_->Pitch(pitch_);
    }

    //Update Controllers
    missileController_->MoveMissiles(timeStep);
    alertController_->CheckAlerts();



    // Rotate the overlayScenes preview object
    Node* missilePreviewNode = overlayScene_->GetChild("missilePreview");
    missilePreviewNode->Rotate(Quaternion(8*timeStep,16*timeStep,0));
}


void World::HandleClosePressed(StringHash eventType,VariantMap& eventData)
{
    context_->GetSubsystem<Engine>()->Exit();
}

void World::HandleClick(StringHash eventType, VariantMap &eventData)
{
    if (!context_->GetSubsystem<Input>()->IsMouseVisible()){
        missileController_->CreateMissile(cameraNode_->GetPosition() ,cameraNode_->GetDirection());
    }

}


void World::HandleMouseWheel(StringHash eventType,VariantMap& eventData)
{
    using namespace MouseWheel;
    int roll = eventData[P_WHEEL].GetInt();
    camera_zoom_ += roll*0.1;
    camera_->SetZoom(camera_zoom_);
}

void World::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        context_->GetSubsystem<Engine>()->Exit();

    if(key==KEY_TAB)    // toggle mouse cursor when pressing tab
    {
        context_->GetSubsystem<Input>()->SetMouseVisible(!context_->GetSubsystem<Input>()->IsMouseVisible());
    }
    if(key==KEY_G){
        CreateAlert("G was pressed!", 3.0);
    }
}

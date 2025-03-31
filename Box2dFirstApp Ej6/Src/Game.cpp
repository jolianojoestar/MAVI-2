#include "Game.h"
#include "Box2DHelper.h"
#include <iostream>
#include <list>

// Constructor
Game::Game(int ancho, int alto, std::string titulo)
{
    // Inicialización de propiedades
    this->alto = alto;
    this->ancho = ancho;
    cannonAngle = 0.0f;
    clearColor = Color::Black;

    // Configuración de la ventana
    wnd = new RenderWindow(VideoMode(ancho, alto), titulo);
    wnd->setVisible(true);
    wnd->setFramerateLimit(60);
    frameTime = 1.0f / 60.0f;

    // Inicializar sistemas
    SetZoom();
    InitPhysics();
}

// Bucle principal
void Game::Loop()
{
    while (wnd->isOpen())
    {
        wnd->clear(clearColor);
        DoEvents();
        CheckCollitions();
        UpdatePhysics();
        DrawGame();
        wnd->display();
    }
}

// Actualizar física
void Game::UpdatePhysics()
{
    // Paso de simulación física
    phyWorld->Step(frameTime, 8, 8);
    phyWorld->ClearForces();

    // Eliminar proyectiles fuera de pantalla
    auto it = projectiles.begin();
    while (it != projectiles.end())
    {
        b2Vec2 pos = (*it)->GetPosition();
        if (pos.x < 0.0f || pos.x > 100.0f || pos.y < 0.0f || pos.y > 100.0f)
        {
            phyWorld->DestroyBody(*it);
            it = projectiles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// Dibujar elementos
void Game::DrawGame()
{
    // Suelo
    RectangleShape groundShape(Vector2f(500.0f, 5.0f));
    groundShape.setFillColor(Color::Red);
    groundShape.setPosition(0.0f, 95.0f);
    wnd->draw(groundShape);

    // Paredes
    RectangleShape leftWall(Vector2f(10.0f, static_cast<float>(alto)));
    leftWall.setFillColor(Color::Cyan);
    leftWall.setPosition(0.0f, -50.0f);
    wnd->draw(leftWall);

    RectangleShape rightWall(Vector2f(10.0f, static_cast<float>(alto)));
    rightWall.setFillColor(Color::Cyan);
    rightWall.setPosition(90.0f, -50.0f);
    wnd->draw(rightWall);

    // Cañón
    RectangleShape cannonShape(Vector2f(25.0f, 5.0f));
    cannonShape.setFillColor(Color::Magenta);
    cannonShape.setOrigin(12.5f, 2.5f);
    cannonShape.setPosition(controlBody->GetPosition().x, controlBody->GetPosition().y);
    cannonShape.setRotation(controlBody->GetAngle() * 180.0f / b2_pi);
    wnd->draw(cannonShape);

    // Proyectiles
    for (b2Body* bullet : projectiles)
    {
        CircleShape bulletShape(2.0f);
        bulletShape.setFillColor(Color::Yellow);
        bulletShape.setOrigin(2.0f, 2.0f);
        bulletShape.setPosition(bullet->GetPosition().x, bullet->GetPosition().y);
        wnd->draw(bulletShape);
    }
}

// Procesar eventos
void Game::DoEvents()
{
    Event evt;
    while (wnd->pollEvent(evt))
    {
        switch (evt.type)
        {
        case Event::Closed:
            wnd->close();
            break;

        case Event::MouseButtonPressed:
        {
            // Crear cuerpo estático en posición del mouse
            b2Body* triangle = Box2DHelper::CreateTriangularStaticBody(
                phyWorld,
                b2Vec2(0.0f, 0.0f),
                10.0f
            );

            Vector2f mousePos = wnd->mapPixelToCoords(
                Vector2i(evt.mouseButton.x, evt.mouseButton.y)
            );

            triangle->SetTransform(b2Vec2(mousePos.x, mousePos.y), 0.0f);
            break;
        }

        default:
            break;
        }
    }

    // Rotación del cañón
    controlBody->SetAwake(true);
    if (Keyboard::isKeyPressed(Keyboard::Up))
        cannonAngle -= 0.05f;
    if (Keyboard::isKeyPressed(Keyboard::Down))
        cannonAngle += 0.05f;

    controlBody->SetTransform(controlBody->GetPosition(), cannonAngle);

    // Disparo
    static bool spacePressed = false;
    if (Keyboard::isKeyPressed(Keyboard::Space) && !spacePressed)
    {
        // Calcular posición de disparo
        b2Vec2 tipPosition = controlBody->GetWorldPoint(b2Vec2(12.5f, 0.0f));

        // Crear proyectil
        b2Body* bullet = Box2DHelper::CreateCircularDynamicBody(
            phyWorld,
            2.0f,   // Radio
            1.0f,   // Densidad
            0.1f,   // Fricción
            0.0f    // Restitución
        );
        bullet->SetTransform(tipPosition, 0.0f);

        // Aplicar impulso
        b2Vec2 fireDirection(cos(cannonAngle), sin(cannonAngle));
        bullet->ApplyLinearImpulse(150.0f * fireDirection, bullet->GetWorldCenter(), true);
        
        projectiles.push_back(bullet);
        spacePressed = true;
    }
    else if (!Keyboard::isKeyPressed(Keyboard::Space))
    {
        spacePressed = false;
    }
}

// Colisiones (a implementar)
void Game::CheckCollitions()
{
    // Lógica de colisiones aquí
}

// Configurar vista
void Game::SetZoom()
{
    View gameView;
    gameView.setSize(100.0f, 100.0f);
    gameView.setCenter(50.0f, 50.0f);
    wnd->setView(gameView);
}

// Inicializar física
void Game::InitPhysics()
{
    // Mundo físico
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));

    // Debug renderer
    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(UINT_MAX);
    phyWorld->SetDebugDraw(debugRender);

    // Crear cuerpos estáticos
    b2Body* ground = Box2DHelper::CreateRectangularStaticBody(phyWorld, 100.0f, 10.0f);
    ground->SetTransform(b2Vec2(50.0f, 100.0f), 0.0f);

    b2Body* leftWall = Box2DHelper::CreateRectangularStaticBody(phyWorld, 10.0f, 100.0f);
    leftWall->SetTransform(b2Vec2(0.0f, 50.0f), 0.0f);

    b2Body* rightWall = Box2DHelper::CreateRectangularStaticBody(phyWorld, 10.0f, 100.0f);
    rightWall->SetTransform(b2Vec2(150.0f, 50.0f), 0.0f);

    // Crear cañón (cinemático)
    controlBody = Box2DHelper::CreateRectangularKinematicBody(phyWorld, 25.0f, 5.0f);
    controlBody->SetTransform(b2Vec2(12.5f, 50.0f), 0.0f); // Centrado en la pared izquierda
    controlBody->SetFixedRotation(false);
}

// Destructor
Game::~Game(void)
{
    // Limpiar proyectiles
    for (b2Body* bullet : projectiles)
    {
        phyWorld->DestroyBody(bullet);
    }
    projectiles.clear();

    // Limpiar otros recursos
    delete phyWorld;
    delete debugRender;
    delete wnd;
}
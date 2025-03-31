#pragma once
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include "SFMLRenderer.h"
#include <list>

using namespace sf;

class Game
{
private:
    // Propiedades de la ventana
    int ancho;
    int alto;
    RenderWindow* wnd;
    Color clearColor;
    // Propiedades físicas
    b2World* phyWorld;
    SFMLRenderer* debugRender;
    float frameTime;
    int fps;
    // Control del cañón
    b2Body* controlBody;
    float cannonAngle;
    // Administración de proyectiles
    std::list<b2Body*> projectiles;

public:
    // Constructor/Destructor
    Game(int ancho, int alto, std::string titulo);
    ~Game(void);
    // Funciones principales
    void Loop();
    void InitPhysics();
    void SetZoom();
    // Funciones del game loop
    void DoEvents();
    void UpdatePhysics();
    void DrawGame();
    void CheckCollitions();
    // Funciones auxiliares (opcional)
    void CreateEnemy(int x, int y); // Si la necesitas para futuras implementaciones
};
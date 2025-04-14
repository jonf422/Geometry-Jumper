#include "engine.h"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

enum state {start, play, over};
state screen = start;

const color skyBlue(77/255.0, 213/255.0, 240/255.0);
const color grassGreen(26/255.0, 176/255.0, 56/255.0);
const color darkGreen(27/255.0, 81/255.0, 45/255.0);
const color white(1, 1, 1);
const color brickRed(201/255.0, 20/255.0, 20/255.0);
const color darkBlue(1/255.0, 110/255.0, 214/255.0);
const color purple(119/255.0, 11/255.0, 224/255.0);
const color black(0, 0, 0);
const color magenta(1, 0, 1);
const color orange(1, 163/255.0, 22/255.0);
const color cyan (0, 1, 1);

float velocityY = 0.0f;
int isJumping = 0;
int maxJumps = 2;
static bool jumpKeyWasPressed = false;
float jumpVelocity = 200.0f;
float gravity = -180.0f;

int points = 0;

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
    this->initPlatforms();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Set uniforms that never change
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);

    //Configure text/font
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    //text/font uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}
void Engine::initPlatforms()
{
    int totalPlatformWidth = 0;
    vec2 platformSize;
    vec2 platformPos;
    bool firstPlatform = true;


    while (totalPlatformWidth < width + 100) {
        // platform height = 5
        platformSize.y = 15;
        if (firstPlatform)
        {
            platformSize.x = width / 2;
            firstPlatform = false;
            platformPos.y = user->getBottom() - 10;
            platformPos.x = totalPlatformWidth + (platformSize.x / 2.0) + 5;
            platforms.push_back(make_unique<Rect>(shapeShader, platformPos,
                                               platformSize, brickRed));
            totalPlatformWidth = platformPos.x + platformSize.x / 2;
            firstPlatform = false;
            continue;
        }
        // platform width between 60-100
        else
        {
            platformSize.x = rand() % 41 + 100;
        }
        if (rand() % 2 == 0)
        {
            platformPos.x = totalPlatformWidth + (platformSize.x / 2.0) + 5;
            platformPos.y = height/2 + ((rand() % 20) + 20);
            platforms.push_back(make_unique<Rect>(shapeShader, platformPos,
                                               platformSize, brickRed));
        }
        else
        {
            platformPos.x = totalPlatformWidth + (platformSize.x / 2.0) + 5;
            platformPos.y = height/2 - ((rand() % 20) + 20);
            platforms.push_back(make_unique<Rect>(shapeShader, platformPos,
                                               platformSize, brickRed));
        }

        totalPlatformWidth += platformSize.x + 50;
    }
}

void Engine::initShapes() {
    //Initialize the user to be a 20x20 white block
    //centered at (0, 0)
    user = make_unique<Rect>(shapeShader, vec2(50, height/2), vec2(20, 20), white); // placeholder for compilation

    // Init grass
    grass = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height / 3), grassGreen);

    // Init mountains
    mountains.push_back(make_unique<Triangle>(shapeShader, vec2(width/4, 300), vec2(width, 400), darkGreen));
    mountains.push_back(make_unique<Triangle>(shapeShader, vec2(2*width/3, 300), vec2(width, 500), darkGreen));

    // Init Cloud
    clouds.push_back(Cloud(shapeShader, vec2(200, 500)));
    clouds.push_back(Cloud(shapeShader, vec2(400, 520)));
    clouds.push_back(Cloud(shapeShader, vec2(325, 480)));

    //init score
    score = make_unique<Rect>(shapeShader, vec2(width-25, height-15), vec2(50, 30), white);
}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    if (screen == start && keys[GLFW_KEY_SPACE])
    {
        screen = play;
        points = 0;
        velocityY = 0.0f;
        isJumping = 0;
        user->setPosY(height / 2);
        platforms.clear();
        initPlatforms();
    }
    else if (screen == play)
    {
        if (keys[GLFW_KEY_UP]) {
            if (!jumpKeyWasPressed && isJumping < maxJumps) {
                velocityY = jumpVelocity;
                isJumping++;
                jumpKeyWasPressed = true;
            }
        } else {
            jumpKeyWasPressed = false;
        }
    }
    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped



/*
    for (const unique_ptr<Rect>& r : buildings1) {
        if (r->isOverlapping(*user)) {
            r->setColor(orange);
        } else {
            r->setColor(brickRed);
        }
    }*/
    // TODO: Once you are confident your isOverlapping method
    //  works, uncomment this code to have the program exit
    //  when the user overlaps with the clouds.

    if (grass->isOverlapping(*user)) {
        screen = over;
    }
}

void Engine::update()
{
    if (screen == play) points++;
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float prevUserBottom = user->getBottom();

    //jump
    velocityY += gravity * deltaTime;
    user->moveY(velocityY * deltaTime);




    // Update clouds
    for (Cloud& c : clouds) {
        c.moveXWithinBounds(-1, width);
    }

    float rightmostPlatformX = platforms.back()->getPosX() + platforms.back()->getSize().x / 2.0f;
    if (rightmostPlatformX < width + 100) {  // You can adjust this threshold as needed
        spawnPlatform();
    }

    // Update platforms
    for (int i = 0; i < platforms.size(); ++i) {
        //
        float userBottom = user->getBottom();

        if (user->isOverlapping(*platforms[i]) && velocityY < 0.0f && prevUserBottom >= platforms[i]->getTop() - 1)
        {
            velocityY = 0;
            user->setPosY(platforms[i]->getPosY() + platforms[i]->getSize().y / 2 + user->getSize().y / 2);
            isJumping = 0;
        }

        // Move all the red buildings to the left
        platforms[i]->moveX(-1.5);
        // If a building has moved off the screen
        if (platforms[i]->getPosX() < -(platforms[i]->getSize().x/2)) {
            platforms.erase(platforms.begin() + i);
            i--; // Adjust index after removal
            spawnPlatform();
        }
    }
}

void Engine::spawnPlatform() {
    vec2 platformPos;
    vec2 platformSize;

    // Set the platform's position relative to the rightmost platform
    platformPos.x = platforms.back()->getPosX() + platforms.back()->getSize().x + rand() % 50 + 50;  // Add a gap

    // Random height for the platform
    if (rand() % 2 == 0) platformPos.y = height / 2 + rand() % (height / 4);
    else platformPos.y = height / 2 - rand() % (height / 4);

    // Set random platform size (width)
    platformSize.x = rand() % 41 + 60;
    platformSize.y = 15;

    platforms.push_back(make_unique<Rect>(shapeShader, platformPos, platformSize, brickRed));
}

void Engine::render() {
    glClearColor(skyBlue.red,skyBlue.green, skyBlue.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch (screen)
    {
    case start: {
            string title = "Geometry Jumper";
            string message1 = "Press space to start";
            string message2 = "Press the up arrow to jump";
            string message3 = "You can double jump";
            // (12 * message.length()) is the offset to center text.
            // 12 pixels is the width of each character scaled by 1.
            // NOTE: This line changes the shader being used to the font shader.
            //  If you want to draw shapes again after drawing text,
            //  you'll need to call shapeShader.use() again first.
            this->fontRenderer->renderText(title, width/2 - (24 * title.length()), height/2 + 150, projection, 2, vec3{0, 0, 0});
            this->fontRenderer->renderText(message1, width/2 - (12 * message1.length()), height/2, projection, 1, vec3{0, 0, 0});
            this->fontRenderer->renderText(message2, width/2 - (12 * message2.length()), height/2-25, projection, 1, vec3{0, 0, 0});
            this->fontRenderer->renderText(message3, width/2 - (12 * message3.length()), height/2-50, projection, 1, vec3{0, 0, 0});
            break;
    }
    case play:
        {
            for (const unique_ptr<Triangle>& m : mountains) {
                m->setUniforms();
                m->draw();
            }

            for (Cloud& c : clouds) {
                c.setUniformsAndDraw();
            }

            grass->setUniforms();
            grass->draw();

            for (int i = 0; i < platforms.size(); ++i)
            {
                platforms[i]->setUniforms();
                platforms[i]->draw();
            }

            user->setUniforms();
            user->draw();

            score->setUniforms();
            score->draw();

            fontRenderer->renderText(std::to_string(points), score->getPos().x-20, score->getPos().y, projection, 0.5, vec3{0, 0, 0});
            shapeShader.use();

            break;
        }
    case over: {
            string message = "Game Over";
            string pointStr = std::to_string(points);
            fontRenderer->renderText(message, width/2 - (9.6 * message.length()), height/2, projection, 0.8, vec3{0, 0, 0});
            fontRenderer->renderText(pointStr, width/2 - (9.6 * pointStr.length()), height/2 - 25, projection, 0.8, vec3{0, 0, 0});
            break;
        }
    }
    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}
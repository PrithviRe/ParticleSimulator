#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Particle.hpp"
#include "Shader.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

constexpr int DEFAULT_WIDTH = 1920;
constexpr int DEFAULT_HEIGHT = 1080;
constexpr int PARTICLE_COUNT = 100000;

constexpr float FORCE_RADIUS = 0.5f;
constexpr float FORCE_STRENGTH = 2.0f;

constexpr float MIN_LIFETIME = 3.0f;
constexpr float MAX_LIFETIME = 7.0f;

static std::mt19937& getRNG() {
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

static float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(getRNG());
}

void initializeParticle(Particle& p) {
    p.x = randomFloat(-1.0f, 1.0f);
    p.y = randomFloat(-1.0f, 1.0f);

    p.velocityX = randomFloat(-0.25f, 0.25f);
    p.velocityY = randomFloat(-0.25f, 0.25f);

    p.red = randomFloat(0.2f, 0.5f);
    p.green = randomFloat(0.5f, 1.0f);
    p.blue = 1.0f;

    p.maxLifetime = randomFloat(MIN_LIFETIME, MAX_LIFETIME);
    p.lifetime = p.maxLifetime;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Particle Laboratory", nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Uncapped FPS for high-performance benchmarking

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });

    // Required for gl_PointSize in vertex shader to take effect in modern OpenGL core profile
    glEnable(GL_PROGRAM_POINT_SIZE);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << '\n';
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';

    // Heap allocation prevents stack overflow with 100K particles
    std::vector<Particle> particles(PARTICLE_COUNT);
    for (Particle& p : particles)
        initializeParticle(p);

    // Each vertex contains: position (2 floats), color (3 floats), alpha (1 float)
    constexpr int FLOATS_PER_PARTICLE = 6;
    std::vector<float> vertices(PARTICLE_COUNT * FLOATS_PER_PARTICLE);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    constexpr GLsizei STRIDE = FLOATS_PER_PARTICLE * sizeof(float);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, STRIDE, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE,
                          reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, STRIDE,
                          reinterpret_cast<void*>(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    Shader shader(SHADER_DIR "/particle.vs", SHADER_DIR "/particle.fs");

    if (!shader.ID) {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    float previousTime = static_cast<float>(glfwGetTime());
    double fpsTimer = 0.0;
    int frameCount = 0;

    float currentFPS = 0.0f;
    float frameTimeMs = 0.0f;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << '\n';

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';

    std::cout << "Version: " << glGetString(GL_VERSION) << '\n';
    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = std::min(currentTime - previousTime, 0.05f);
        frameTimeMs = deltaTime * 1000.0f;

        fpsTimer += deltaTime;
        frameCount++;

        if (fpsTimer >= 1.0) {
            currentFPS = static_cast<float>(frameCount) / static_cast<float>(fpsTimer);

            std::cout << "FPS: " << currentFPS << " | Frame time: " << frameTimeMs << " ms"
                      << " | Particles: " << PARTICLE_COUNT << '\n';

            fpsTimer = 0.0;
            frameCount = 0;
        }
        previousTime = currentTime;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        float normalizedMouseX = 0.0f;
        float normalizedMouseY = 0.0f;

        if (windowWidth > 0 && windowHeight > 0) {
            normalizedMouseX = static_cast<float>(mouseX / windowWidth) * 2.0f - 1.0f;

            normalizedMouseY = 1.0f - static_cast<float>(mouseY / windowHeight) * 2.0f;
        }

        bool repel = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        // Update particles and construct GPU vertex data.
        for (int i = 0; i < PARTICLE_COUNT; ++i) {
            Particle& p = particles[i];

            if (p.lifetime <= 0.0f)
                initializeParticle(p);

            float dx = normalizedMouseX - p.x;
            float dy = normalizedMouseY - p.y;

            float distanceSquared = dx * dx + dy * dy;

            if (distanceSquared < FORCE_RADIUS * FORCE_RADIUS && distanceSquared > 0.0001f) {
                float distance = std::sqrt(distanceSquared);

                float direction = repel ? -1.0f : 1.0f;

                float force = (1.0f - distance / FORCE_RADIUS) * FORCE_STRENGTH;

                p.velocityX += direction * (dx / distance) * force * deltaTime;

                p.velocityY += direction * (dy / distance) * force * deltaTime;
            }
            p.limitSpeed(1.5f);
            p.update(deltaTime);

            int offset = i * FLOATS_PER_PARTICLE;

            vertices[offset] = p.x;
            vertices[offset + 1] = p.y;

            vertices[offset + 2] = p.red;
            vertices[offset + 3] = p.green;
            vertices[offset + 4] = p.blue;

            vertices[offset + 5] = p.getAlpha();
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

        glClearColor(0.01f, 0.01f, 0.025f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

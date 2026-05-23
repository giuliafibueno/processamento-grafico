#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

using namespace std;
using namespace glm;

const int WIDTH  = 800;
const int HEIGHT = 600;


const int SHEET_W    = 1280;
const int SHEET_H    = 480;
const int FRAME_W    = 160;
const int FRAME_H    = 150;
const int IDLE_ROW   = 0;
const int FLY_ROW    = 1;
const int FLY_COUNT  = 8;


const char* bgVertSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 UV;

uniform mat4 projection; 
uniform float offsetX;   
uniform float offsetY;   

void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    UV = vec2(aUV.x + offsetX, aUV.y + offsetY);
}
)";

const char* spriteVertSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 UV;

uniform mat4 projection;
uniform mat4 model;
uniform vec2 uvOffset; 
uniform vec2 uvScale;  

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    UV = uvOffset + aUV * uvScale;
}
)";

const char* fragSrc = R"(
#version 330 core
in  vec2 UV;
out vec4 FragColor;
uniform sampler2D tex;
void main() {
    FragColor = texture(tex, UV);
}
)";


static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[512]; glGetShaderInfoLog(s, 512, nullptr, log); cerr << log; }
    return s;
}

static GLuint linkProgram(const char* vert, const char* frag) {
    GLuint vs = compileShader(GL_VERTEX_SHADER,   vert);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, frag);
    GLuint p  = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs);
    glLinkProgram(p);
    int ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) { char log[512]; glGetProgramInfoLog(p, 512, nullptr, log); cerr << log; }
    glDeleteShader(vs); glDeleteShader(fs);
    return p;
}

static GLuint loadTexture(const char* path, GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_CLAMP_TO_EDGE) {
    GLuint id; glGenTextures(1, &id);
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true); // PNG tem y=0 no topo; OpenGL espera y=0 na base
    unsigned char* data = stbi_load(path, &w, &h, &ch, STBI_rgb_alpha);
    if (!data) { cerr << "Falha ao carregar: " << path << "\n"; return id; }
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // NEAREST preserva pixel art
    stbi_image_free(data); // libera memória RAM; a textura já está na GPU
    return id;
}

static GLuint makeFullscreenQuad() {
    //         x              y            u     v
    float verts[] = {
        0.f,           (float)HEIGHT,  0.f,  1.f,  // topo-esquerda
        0.f,           0.f,            0.f,  0.f,  // base-esquerda
        (float)WIDTH,  0.f,            2.f,  0.f,  // base-direita

        0.f,           (float)HEIGHT,  0.f,  1.f,  // topo-esquerda
        (float)WIDTH,  0.f,            2.f,  0.f,  // base-direita
        (float)WIDTH,  (float)HEIGHT,  2.f,  1.f,  // topo-direita
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    // atributo 0: posição (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // atributo 1: UV (u, v)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return VAO;
}

static GLuint makeSpriteQuad() {
    float verts[] = {
        0.f, 1.f,  0.f, 1.f,
        0.f, 0.f,  0.f, 0.f,
        1.f, 0.f,  1.f, 0.f,

        0.f, 1.f,  0.f, 1.f,
        1.f, 0.f,  1.f, 0.f,
        1.f, 1.f,  1.f, 1.f,
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return VAO;
}

// ── Estruturas ────────────────────────────────────────────────────────────────

struct Layer {
    GLuint tex;    // ID da textura na GPU
    float  speedX; // fator de parallax horizontal
    float  speedY; // fator de parallax vertical
    float  offX;   // deslocamento UV horizontal acumulado
    float  offY;   // deslocamento UV vertical acumulado
};

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    const char* ASSETS =
        "/Users/i761522/Unisinos/processamentoGrafico/giulia/"
        "processamento-grafico/game_background_1/layers/";
    const char* BIRD_SHEET =
        "/Users/i761522/Unisinos/processamentoGrafico/giulia/"
        "processamento-grafico/BirdSpriteBig.png";

    // Inicializa GLFW (biblioteca que cria a janela e trata o teclado)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
        "Vivencial 2 – Parallax Scrolling", nullptr, nullptr);
    if (!window) { cerr << "Falha ao criar janela\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    // Inicializa GLEW (biblioteca que dá acesso às funções modernas do OpenGL)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { cerr << "Falha no GLEW\n"; return -1; }

    // Em telas Retina o framebuffer pode ser maior que a janela — usa o tamanho real
    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    // Habilita transparência: pixels com alpha < 1 deixam ver o que está atrás
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Projeção ortográfica: mapeia o espaço [0,800]x[0,600] para a tela sem perspectiva
    mat4 projection = ortho(0.f, (float)WIDTH, 0.f, (float)HEIGHT);

    // Programas de shader: um para o fundo (parallax), outro para o sprite
    GLuint bgProg     = linkProgram(bgVertSrc,     fragSrc);
    GLuint spriteProg = linkProgram(spriteVertSrc, fragSrc);

    glUseProgram(bgProg);
    glUniform1i(glGetUniformLocation(bgProg, "tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(bgProg, "projection"), 1, GL_FALSE, value_ptr(projection));

    glUseProgram(spriteProg);
    glUniform1i(glGetUniformLocation(spriteProg, "tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(spriteProg, "projection"), 1, GL_FALSE, value_ptr(projection));

    // Um único quad full-screen reutilizado para desenhar todas as camadas de fundo
    GLuint bgVAO     = makeFullscreenQuad();
    // Quad unitário para o sprite do pássaro (escalonado via model matrix)
    GLuint spriteVAO = makeSpriteQuad();

    // As 7 camadas, ordenadas do mais distante ao mais próximo.
    // speedX e speedY crescentes garantem que camadas próximas movam mais rápido.
    const int NUM_LAYERS = 7;
    Layer layers[NUM_LAYERS];

    auto path = [&](const char* file) -> string {
        return string(ASSETS) + file;
    };

    layers[0] = { loadTexture(path("sky.png").c_str()),      0.02f, 0.01f, 0.f, 0.f }; // céu — quase estático
    layers[1] = { loadTexture(path("clouds_4.png").c_str()), 0.05f, 0.02f, 0.f, 0.f }; // nuvens distantes
    layers[2] = { loadTexture(path("clouds_3.png").c_str()), 0.10f, 0.04f, 0.f, 0.f };
    layers[3] = { loadTexture(path("clouds_2.png").c_str()), 0.18f, 0.07f, 0.f, 0.f };
    layers[4] = { loadTexture(path("clouds_1.png").c_str()), 0.28f, 0.11f, 0.f, 0.f }; // nuvens próximas
    layers[5] = { loadTexture(path("rocks_1.png").c_str()),  0.45f, 0.18f, 0.f, 0.f }; // rochas de fundo
    layers[6] = { loadTexture(path("rocks_2.png").c_str()),  0.70f, 0.28f, 0.f, 0.f }; // rochas de frente

    // Carrega o spritesheet do pássaro.
    // GL_CLAMP_TO_EDGE nos dois eixos evita que pixels de frames vizinhos vazem nas bordas.
    GLuint birdTex = loadTexture(BIRD_SHEET, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    // Tamanho de um frame em coordenadas UV (0..1)
    const float UV_FRAME_W = (float)FRAME_W / SHEET_W; // 128/1280 = 0.1
    const float UV_FRAME_H = (float)FRAME_H / SHEET_H; // 160/480  = 0.333

    // Posição do pássaro na tela em pixels (canto inferior-esquerdo)
    // Começa no centro, escalado 2x para ficar visível
    const float BIRD_SCALE = 0.4f;
    const float BIRD_W = FRAME_W * BIRD_SCALE;
    const float BIRD_H = FRAME_H * BIRD_SCALE;
    // Pássaro fixo no centro da tela — quem se move é o cenário
    const float birdX = (WIDTH  - BIRD_W) / 2.f;
    const float birdY = (HEIGHT - BIRD_H) / 2.f;

    // Estado da animação
    int   animFrame    = 0;      // frame atual dentro da animação
    float animTimer    = 0.f;    // acumulador de tempo para troca de frame
    bool  facingRight  = true;   // controla espelhamento horizontal
    const float FLY_FPS  = 12.f; // velocidade da animação de voo (frames por segundo)

    const float MOVE_SPEED = 200.f; // pixels por segundo

    float lastTime = (float)glfwGetTime();

    // ── Loop principal ────────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float now = (float)glfwGetTime();
        float dt  = now - lastTime;
        lastTime  = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Calcula o deslocamento deste frame baseado nas teclas pressionadas.
        float dx = 0.f, dy = 0.f;
        if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) dx -= MOVE_SPEED * dt;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) dx += MOVE_SPEED * dt;
        if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) dy += MOVE_SPEED * dt;
        if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) dy -= MOVE_SPEED * dt;

        // Atualiza o offset de cada camada proporcionalmente ao seu speed.
        // Sinal negativo: o cenário rola na direção oposta ao movimento do pássaro.
        // Camadas mais próximas (speed maior) rolam mais rápido — efeito parallax.
        for (int i = 0; i < NUM_LAYERS; ++i) {
            layers[i].offX -= dx * layers[i].speedX * 0.003f;
            layers[i].offY -= dy * layers[i].speedY * 0.003f;
        }

        // ── Animação do pássaro ───────────────────────────────────────────────
        bool moving = (dx != 0.f || dy != 0.f);

        // Atualiza direção apenas no movimento horizontal
        if (dx < 0.f) facingRight = false;
        if (dx > 0.f) facingRight = true;

        // Avança o frame da animação FLY apenas quando o pássaro está se movendo
        if (moving) {
            animTimer += dt;
            if (animTimer >= 1.f / FLY_FPS) {
                animTimer -= 1.f / FLY_FPS;
                animFrame = (animFrame + 1) % FLY_COUNT;
            }
        } else {
            animFrame = 0;
            animTimer = 0.f;
        }

        // Calcula o UV do frame atual no spritesheet.
        int row = FLY_ROW;
        float uvX = animFrame * UV_FRAME_W;
        float uvY = (2 - row) * UV_FRAME_H;

        // ── Renderização ──────────────────────────────────────────────────────
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenha as camadas de fundo de trás para frente.
        glUseProgram(bgProg);
        glBindVertexArray(bgVAO);
        for (int i = 0; i < NUM_LAYERS; ++i) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layers[i].tex);
            glUniform1f(glGetUniformLocation(bgProg, "offsetX"), layers[i].offX);
            glUniform1f(glGetUniformLocation(bgProg, "offsetY"), layers[i].offY);
            glDrawArrays(GL_TRIANGLES, 0, 6); // 2 triângulos = 6 vértices
        }

        // Desenha o pássaro sobre o fundo
        glUseProgram(spriteProg);
        glBindVertexArray(spriteVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, birdTex);

        mat4 model = mat4(1.f);
        if (facingRight) {
            model = translate(model, vec3(birdX, birdY, 0.f));
            model = scale(model,     vec3(BIRD_W, BIRD_H, 1.f));
        } else {
            model = translate(model, vec3(birdX + BIRD_W, birdY, 0.f));
            model = scale(model,     vec3(-BIRD_W, BIRD_H, 1.f));
        }

        glUniformMatrix4fv(glGetUniformLocation(spriteProg, "model"),    1, GL_FALSE, value_ptr(model));
        glUniform2f(glGetUniformLocation(spriteProg, "uvOffset"), uvX, uvY);
        glUniform2f(glGetUniformLocation(spriteProg, "uvScale"),  UV_FRAME_W, UV_FRAME_H);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

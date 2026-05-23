# Processamento Gráfico

## Vivencial 1

### Descrição

Programa interativo em OpenGL que permite criar triângulos coloridos clicando na tela. A cada clique, um vértice é registrado. A cada 3 vértices registrados, um triângulo é formado e desenhado com uma cor aleatória. O exercício revisa a criação de buffers de geometria (VAO/VBO) e o envio de informações para shaders.

### Dependências

- [GLFW](https://www.glfw.org/) — janela e contexto OpenGL
- [GLEW](http://glew.sourceforge.net/) — carregamento de extensões OpenGL
- [GLM](https://github.com/g-truc/glm) — matemática (vetores, matrizes)
- OpenGL 3.3+

Instale via Homebrew:

```bash
brew install glfw glew glm
```

### Compilação

**macOS:**

```bash
g++ -std=c++11 vivencial1.cpp -o vivencial1 \
  -framework Cocoa -framework OpenGL -framework IOKit \
  -I/opt/homebrew/include -L/opt/homebrew/lib \
  -lGLEW -lglfw
```

### Execução

**macOS:**

```bash
./vivencial1
```

### Como usar

1. Execute o programa — uma janela 800×600 será aberta
2. Clique em 3 pontos da tela para criar um triângulo
3. O triângulo aparece com uma cor aleatória
4. Repita para criar quantos triângulos quiser
5. Feche a janela para encerrar

## Vivencial 2

### Descrição

Cena 2D com parallax scrolling e sprite animado. O cenário é composto por 7 camadas independentes (céu, nuvens e rochas) que se deslocam em velocidades diferentes ao pressionar as teclas direcionais, criando a ilusão de profundidade. Um pássaro centralizado na tela é animado a partir de um spritesheet (8 frames de voo) e espelha horizontalmente conforme a direção do movimento. O pássaro permanece fixo no centro — quem se move é o cenário ao redor.

**Camadas de parallax (mais distante → mais próxima):**

| Camada | Arquivo | Velocidade X |
|--------|---------|-------------|
| Céu | `sky.png` | 0.02 |
| Nuvens distantes | `clouds_4.png` | 0.05 |
| Nuvens | `clouds_3.png` | 0.10 |
| Nuvens | `clouds_2.png` | 0.18 |
| Nuvens próximas | `clouds_1.png` | 0.28 |
| Rochas (fundo) | `rocks_1.png` | 0.45 |
| Rochas (frente) | `rocks_2.png` | 0.70 |

### Dependências

- [GLFW](https://www.glfw.org/) — janela, contexto OpenGL e input de teclado
- [GLEW](http://glew.sourceforge.net/) — carregamento de extensões OpenGL
- [GLM](https://github.com/g-truc/glm) — matemática (vetores, matrizes)
- [stb_image](https://github.com/nothings/stb) — carregamento de texturas PNG (incluso: `stb_image.h`)
- OpenGL 3.3+

Instale via Homebrew:

```bash
brew install glfw glew glm
```

### Compilação

**macOS:**

```bash
g++ -std=c++17 vivencial2.cpp -o vivencial2 \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  -lGLEW -lglfw \
  -framework OpenGL
```

### Execução

**macOS:**

```bash
./vivencial2
```

### Como usar

| Tecla | Ação |
|-------|------|
| `←` `→` | Move o cenário horizontalmente; o pássaro vira na direção do movimento |
| `↑` `↓` | Move o cenário verticalmente |
| `ESC` | Encerra o programa |
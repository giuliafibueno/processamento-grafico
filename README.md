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
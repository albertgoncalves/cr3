#include "math.h"

#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 768
#define WINDOW_NAME   __FILE__

#define FRAME_BUFFER_SCALE 8

#define FRAME_BUFFER_WIDTH  (WINDOW_WIDTH / FRAME_BUFFER_SCALE)
#define FRAME_BUFFER_HEIGHT (WINDOW_HEIGHT / FRAME_BUFFER_SCALE)

#define BACKGROUND_COLOR 0.125f, 0.125f, 0.125f, 1.0f

#define TIME_INTERVAL 4

#define FRAME_DURATION ((u64)((1.0 / (60.0 + 1.0)) * NANO_PER_SECOND))

#define EXIT_IF_GL_ERROR()                                 \
    do {                                                   \
        switch (glGetError()) {                            \
        case GL_INVALID_ENUM: {                            \
            EXIT_WITH("GL_INVALID_ENUM");                  \
        }                                                  \
        case GL_INVALID_VALUE: {                           \
            EXIT_WITH("GL_INVALID_VALUE");                 \
        }                                                  \
        case GL_INVALID_OPERATION: {                       \
            EXIT_WITH("GL_INVALID_OPERATION");             \
        }                                                  \
        case GL_INVALID_FRAMEBUFFER_OPERATION: {           \
            EXIT_WITH("GL_INVALID_FRAMEBUFFER_OPERATION"); \
        }                                                  \
        case GL_OUT_OF_MEMORY: {                           \
            EXIT_WITH("GL_OUT_OF_MEMORY");                 \
        }                                                  \
        case GL_NO_ERROR: {                                \
            break;                                         \
        }                                                  \
        }                                                  \
    } while (FALSE)

ATTRIBUTE(noreturn) static void callback_error(i32 code, const char* error) {
    printf("%d: %s\n", code, error);
    _exit(ERROR);
}

static void callback_key(GLFWwindow* window, i32 key, i32, i32 action, i32) {
    if (action != GLFW_PRESS) {
        return;
    }
    switch (key) {
    case GLFW_KEY_ESCAPE: {
        glfwSetWindowShouldClose(window, TRUE);
        break;
    }
    }
}

static void compile_shader(const char* path, u32 shader) {
    const MemMap map = path_to_map(path);
    const char*  source = map_to_buffer(map);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    i32 status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(shader,
                           (i32)(CAP_BUFFER - LEN_BUFFER),
                           NULL,
                           &BUFFER[LEN_BUFFER]);
        printf("%s", &BUFFER[LEN_BUFFER]);
        EXIT();
    }
    EXIT_IF(munmap(map.address, map.len));
}

i32 main(i32 n, const char** args) {
    EXIT_IF(n < 3);
    printf("GLFW version : %s\n", glfwGetVersionString());

    EXIT_IF(!glfwInit());
    glfwSetErrorCallback(callback_error);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, FALSE);
    GLFWwindow* window =
        glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
    EXIT_IF(!window);

    glfwSetKeyCallback(window, callback_key);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glClearColor(BACKGROUND_COLOR);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    EXIT_IF_GL_ERROR();

    const u32 program = glCreateProgram();
    {
        const u32 shader_vert = glCreateShader(GL_VERTEX_SHADER);
        const u32 shader_frag = glCreateShader(GL_FRAGMENT_SHADER);
        compile_shader(args[1], shader_vert);
        compile_shader(args[2], shader_frag);
        glAttachShader(program, shader_vert);
        glAttachShader(program, shader_frag);
        glLinkProgram(program);
        {
            i32 status = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &status);
            if (!status) {
                glGetProgramInfoLog(program,
                                    (i32)(CAP_BUFFER - LEN_BUFFER),
                                    NULL,
                                    &BUFFER[LEN_BUFFER]);
                printf("%s", &BUFFER[LEN_BUFFER]);
                EXIT_IF(!status);
            }
        }
        glDeleteShader(shader_vert);
        glDeleteShader(shader_frag);
        glUseProgram(program);
        EXIT_IF_GL_ERROR();
    }

    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    EXIT_IF_GL_ERROR();

    u32 vbo;
    {
        Vec2f vertices[] = {
            {1.0f, 1.0f},
            {1.0f, -1.0f},
            {-1.0f, -1.0f},
            {-1.0f, 1.0f},
        };
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertices),
                     vertices,
                     GL_STATIC_DRAW);
        EXIT_IF_GL_ERROR();
    }

    u32 ebo;
    {
        Vec3u indices[] = {
            {0, 1, 3},
            {1, 2, 3},
        };
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(indices),
                     indices,
                     GL_STATIC_DRAW);
        EXIT_IF_GL_ERROR();
    }

    {
        i32 index = glGetAttribLocation(program, "VERT_IN_POSITION");
        glVertexAttribPointer((u32)index,
                              2,
                              GL_FLOAT,
                              FALSE,
                              sizeof(Vec2f),
                              0);
        glEnableVertexAttribArray((u32)index);
        EXIT_IF_GL_ERROR();
    }

    u32 render_buffer_color;
    glGenRenderbuffers(1, &render_buffer_color);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_color);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_RGB,
                          FRAME_BUFFER_WIDTH,
                          FRAME_BUFFER_HEIGHT);
    EXIT_IF_GL_ERROR();

    // u32 render_buffer_depth;
    // glGenRenderbuffers(1, &render_buffer_depth);
    // glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_depth);
    // glRenderbufferStorage(GL_RENDERBUFFER,
    //                       GL_DEPTH_COMPONENT,
    //                       FRAME_BUFFER_WIDTH,
    //                       FRAME_BUFFER_HEIGHT);
    // EXIT_IF_GL_ERROR();

    u32 frame_buffer;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER,
                              render_buffer_color);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER,
    //                           GL_DEPTH_ATTACHMENT,
    //                           GL_RENDERBUFFER,
    //                           render_buffer_depth);
    EXIT_IF_GL_ERROR();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        EXIT_IF_GL_ERROR();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glEnable(GL_DEPTH_TEST);

    glUniform2f(glGetUniformLocation(program, "WINDOW"),
                FRAME_BUFFER_WIDTH,
                FRAME_BUFFER_HEIGHT);

    Vec3f position = {0.0f, 3.0f, 15.0f};
    Vec3f aim = {0.0f, -1.5f, 0.0f};

    i32 uniform_time = glGetUniformLocation(program, "TIME");
    i32 uniform_position = glGetUniformLocation(program, "POSITION");
    i32 uniform_aim = glGetUniformLocation(program, "AIM");

    u64 time = now_ns();
    u64 frames = 0;
    printf("\n\n");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const u64 start = now_ns();
        ++frames;
        // NOTE: See `http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/`.
        if (NANO_PER_SECOND <= (start - time)) {
            printf("\033[2A"
                   "%7.4f ms/f\n"
                   "%7lu f/s\n",
                   (NANO_PER_SECOND / (f64)frames) / NANO_PER_MILLI,
                   frames);
            time += NANO_PER_SECOND;
            frames = 0;
        }

#define SPEED 0.25f
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            position.x += SPEED;
            aim.x += SPEED;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            position.x -= SPEED;
            aim.x -= SPEED;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            position.z -= SPEED;
            aim.z -= SPEED;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            position.z += SPEED;
            aim.z += SPEED;
        }
#undef SPEED

        glUniform1f(uniform_time,
                    ((f32)(start % (NANO_PER_SECOND * TIME_INTERVAL))) /
                        (NANO_PER_SECOND * TIME_INTERVAL));

        glUniform3f(uniform_position, position.x, position.y, position.z);
        glUniform3f(uniform_aim, aim.x, aim.y, aim.z);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBlitFramebuffer(0,
                          0,
                          FRAME_BUFFER_WIDTH,
                          FRAME_BUFFER_HEIGHT,
                          0,
                          0,
                          WINDOW_WIDTH,
                          WINDOW_HEIGHT,
                          // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                          GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        EXIT_IF_GL_ERROR();
        glfwSwapBuffers(window);

        const u64 elapsed = now_ns() - start;
        if (elapsed < FRAME_DURATION) {
            EXIT_IF(
                usleep((u32)((FRAME_DURATION - elapsed) / NANO_PER_MICRO)));
        }
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteRenderbuffers(1, &render_buffer_color);
    // glDeleteRenderbuffers(1, &render_buffer_depth);
    glDeleteProgram(program);
    glfwTerminate();
    return OK;
}

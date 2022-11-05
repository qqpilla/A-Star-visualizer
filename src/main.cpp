#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "constants.h"
#include "grid.h"
#include "shader_program.h"

Grid grid;
bool is_placing_main_cells = true;

bool left_click = false;
bool right_click = false;
double cursor_x;
double cursor_y;

void ErrorCallback(int errorCode, const char *message)
{
    std::cout << "ERROR: " << message << "\nERROR CODE: " << errorCode << std::endl;
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (!(left_click || right_click) && key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        is_placing_main_cells = !is_placing_main_cells;

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        grid.ClearAll();
}

void CursorPositionCallback(GLFWwindow *window, double x_pos, double y_pos)
{
    cursor_x = x_pos;
    cursor_y = double(W_Side) - y_pos;

    if (!is_placing_main_cells && (left_click || right_click))
    {
        int i_ind, j_ind;
        Cell *cell = grid.FindCellAround(cursor_x, cursor_y, i_ind, j_ind);

        if (left_click)
            grid.PlaceBlockedCell(cell, i_ind, j_ind);
        else
            grid.RemoveBlockedCell(cell, i_ind, j_ind);
    }
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    left_click = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS);
    right_click = (button == GLFW_MOUSE_BUTTON_RIGHT) && (action == GLFW_PRESS);

    if (left_click || right_click)
    {
        int i_ind, j_ind;
        Cell *cell = grid.FindCellAround(cursor_x, cursor_y, i_ind, j_ind);

        if (is_placing_main_cells)
        {
            if (left_click)
                grid.SetStartCell(cell, i_ind, j_ind);
            else
                grid.SetDestinationCell(cell, i_ind, j_ind);
        }
        else
        {
            if (left_click)
                grid.PlaceBlockedCell(cell, i_ind, j_ind);
            else
                grid.RemoveBlockedCell(cell, i_ind, j_ind);
        }
    }
}

int main()
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        std::cout << "ERROR: Glfw initialization failed" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(W_Side, W_Side, "A* Visualizer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "ERROR: Window creation failed" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR: Glad initialization failed" << std::endl;
        glfwTerminate();
        return 1;
    }

    glViewport(0, 0, W_Side, W_Side);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    ShaderProgram vertical_grid_shader("../shaders/v_grid.vs", "../shaders/grid.fs");
    ShaderProgram horizontal_grid_shader("../shaders/h_grid.vs", "../shaders/grid.fs");
    ShaderProgram main_cells_shader("../shaders/main_cells.vs", "../shaders/cells.fs");
    ShaderProgram blocked_cells_shader("../shaders/blocked_cells.vs", "../shaders/cells.fs");

    grid.InitializeGrid();
    grid.InitializeMainCells();
    grid.InitializeBlockedCells();

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(blocked_cells_shader.ID());
        grid.DrawBlockedCells();

        glUseProgram(main_cells_shader.ID());
        grid.DrawStart();
        grid.DrawDestination();

        glUseProgram(vertical_grid_shader.ID());
        grid.DrawSetOfGridLines();
        glUseProgram(horizontal_grid_shader.ID());
        grid.DrawSetOfGridLines();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
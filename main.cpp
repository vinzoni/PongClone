#include <iostream>
#include <SDL2/SDL.h>

using namespace std;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
bool game_is_running = false;
float last_frame_time = 0;
bool isGoalScored;
int consecutive_shots;

struct Ball {
    float x;
    float y;
    float width;
    float height;
    float x_direction;
    float y_direction;
    float x_speed;
    float y_speed;
} ball;

struct Paddle {
    float x;
    float y;
    float width;
    float height;
    float y_direction;
} paddle_left, paddle_right;

int leftScore, rightScore;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int FPS = 30;
const int FRAME_TARGET_TIME = 1000 / FPS;
const int HORIZONTAL_SPEED = WINDOW_WIDTH / 4.4;
const int VERTICAL_SPEED = WINDOW_HEIGHT / 5;
const int PADDLE_SPEED = HORIZONTAL_SPEED * 2;

const int BALL_SIZE = WINDOW_WIDTH / 60;
const int PADDLE_WIDTH = WINDOW_WIDTH / 80;
const int PADDLE_HEIGHT = WINDOW_WIDTH / 10;
const int MIDDLE_LINE_SEGMENT_HEIGHT = WINDOW_WIDTH / 40;
const int MIDDLE_LINE_SEGMENT_WIDTH = MIDDLE_LINE_SEGMENT_HEIGHT / 5;
const int MIDDLE_LINE_SEGMENT_SPACING = MIDDLE_LINE_SEGMENT_WIDTH;
const float LEFT_SCORE_X = WINDOW_WIDTH / 3;
const float RIGHT_SCORE_X = WINDOW_WIDTH / 1.65;
const int SCORE_Y = WINDOW_HEIGHT / 12;
const int SCORE_SEGMENT_LEN = WINDOW_WIDTH / 20;
const int SCORE_THIKNESS = SCORE_SEGMENT_LEN / 5;

bool initialize_window() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        cerr << "Error initializing SDL." << endl;
        return false;
    }

    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED_DISPLAY(1),
        SDL_WINDOWPOS_CENTERED_DISPLAY(1),
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_BORDERLESS
    );

    if (!window) {
        cerr << "Error creating SDL window." << endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        cerr << "Error creating SDL renderer." << endl;
        return false;
    }

    return true;
}

void process_input(float delta_time) {

    SDL_PumpEvents();
    const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

    if (keyboardState[SDL_SCANCODE_ESCAPE]) {
        game_is_running = false;
        return;
    }
    if (keyboardState[SDL_SCANCODE_DOWN]) {
        if (paddle_right.y + PADDLE_HEIGHT / 2 >= WINDOW_HEIGHT) {
            paddle_right.y = WINDOW_HEIGHT - PADDLE_HEIGHT / 2;
            return;
        }
        paddle_right.y += PADDLE_SPEED * delta_time;
        paddle_right.y_direction = 1;
    }
    if (keyboardState[SDL_SCANCODE_UP]) {
        if (paddle_right.y - PADDLE_HEIGHT / 2 <= 0) {
            paddle_right.y = PADDLE_HEIGHT / 2;
            return;
        }
        paddle_right.y -= PADDLE_SPEED * delta_time;
        paddle_right.y_direction = -1;
    }
    if (keyboardState[SDL_SCANCODE_A]) {
        if (paddle_left.y - PADDLE_HEIGHT / 2 <= 0) {
            paddle_left.y = PADDLE_HEIGHT / 2;
            return;
        }
        paddle_left.y -= PADDLE_SPEED * delta_time;
        paddle_left.y_direction = -1;
    }
    if (keyboardState[SDL_SCANCODE_Z]) {
        if (paddle_left.y + PADDLE_HEIGHT / 2 >= WINDOW_HEIGHT) {
            paddle_left.y = WINDOW_HEIGHT - PADDLE_HEIGHT / 2;
            return;
        }
        paddle_left.y += PADDLE_SPEED * delta_time;
        paddle_left.y_direction = 1;
    }
}

bool collision(Ball ball, Paddle paddle, float delta_time) {

    if (delta_time == 0)
        return false;

    if ((abs(ball.x - paddle.x) <= paddle.width / 2)
        &&
        (abs(ball.y - paddle.y) <= paddle.height / 2))
        return true;

    return false;
}

void kick_off() {
    if (ball.x <= 0)
    {
        ball.x = paddle_left.x + paddle_left.width + ball.width;
        ball.y = BALL_SIZE;
        ball.x_direction = 1;
        ball.y_direction = 1;
    }
    else
    {
        ball.x = paddle_right.x - paddle_right.width - ball.width;
        ball.y = BALL_SIZE;
        ball.x_direction = -1;
        ball.y_direction = 1;
    }
}

void update(float delta_time) {

    float delta_x = ball.x_speed * delta_time;
    float delta_y = ball.y_speed * delta_time;

    ball.x += delta_x * ball.x_direction;
    ball.y += delta_y * ball.y_direction;

    if (collision(ball, paddle_right, delta_time))
    {
        ball.x_direction = -1;
        if (ball.y_direction == paddle_right.y_direction)
            ball.y_direction = -ball.y_direction;
        consecutive_shots++;
    }
    else if (collision(ball, paddle_left, delta_time))
    {
        ball.x_direction = 1;
        if (ball.y_direction == paddle_left.y_direction)
            ball.y_direction = -ball.y_direction;
        consecutive_shots++;
    }
    else if (ball.y >= WINDOW_HEIGHT)
    {
        ball.y_direction = -1;
    }
    else if (ball.y <= 0)
    {
        ball.y_direction = +1;
    }
    else if (ball.x <= 0)
    {
        rightScore++;
        kick_off();
        isGoalScored = true;
        consecutive_shots = 0;
    }
    else if (ball.x >= WINDOW_WIDTH)
    {
        leftScore++;
        kick_off();
        isGoalScored = true;
        consecutive_shots = 0;
    }

    // grow speed and shrink paddle if players manage to hit the ball
    // ten times in a row.
    if ((consecutive_shots > 0) && (consecutive_shots % 10 == 0)) {
        ball.x_speed += ball.x_speed * 0.2;
        ball.x_speed += ball.y_speed * 0.2;
        paddle_left.height -= paddle_left.height * 0.1;
        paddle_right.height -= paddle_right.height * 0.1;
        consecutive_shots = 0;
    }
}

void render_middle_line(SDL_Renderer* renderer) {

    for (int y = 2;
            y < WINDOW_WIDTH - MIDDLE_LINE_SEGMENT_HEIGHT;
            y += MIDDLE_LINE_SEGMENT_HEIGHT + MIDDLE_LINE_SEGMENT_SPACING)
    {
        SDL_Rect middle_line = {
            WINDOW_WIDTH / 2,
            y,
            MIDDLE_LINE_SEGMENT_WIDTH,
            MIDDLE_LINE_SEGMENT_HEIGHT
        };
        SDL_RenderFillRect(renderer, &middle_line);
    }
}

void render_score(SDL_Renderer* renderer, int x, int y, int number) {

            SDL_Rect top = {
                x,
                y,
                SCORE_SEGMENT_LEN + SCORE_THIKNESS,
                SCORE_THIKNESS
            };
            SDL_Rect bottom = {
                x,
                y + (SCORE_SEGMENT_LEN - SCORE_THIKNESS) * 2,
                SCORE_SEGMENT_LEN + SCORE_THIKNESS,
                SCORE_THIKNESS
            };
            SDL_Rect middle = {
                x,
                y + (SCORE_SEGMENT_LEN - SCORE_THIKNESS),
                SCORE_SEGMENT_LEN + SCORE_THIKNESS,
                SCORE_THIKNESS
            };
            SDL_Rect left_top = {
                x,
                y,
                SCORE_THIKNESS,
                SCORE_SEGMENT_LEN
            };
            SDL_Rect left_bottom = {
                x,
                y + SCORE_SEGMENT_LEN,
                SCORE_THIKNESS,
                SCORE_SEGMENT_LEN - SCORE_THIKNESS
            };
            SDL_Rect right_top = {
                x + SCORE_SEGMENT_LEN,
                y,
                SCORE_THIKNESS,
                SCORE_SEGMENT_LEN
            };
            SDL_Rect right_bottom = {
                x + SCORE_SEGMENT_LEN,
                y + SCORE_SEGMENT_LEN,
                SCORE_THIKNESS,
                SCORE_SEGMENT_LEN - SCORE_THIKNESS
            };

        switch (number % 10) {
            case 9:
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &bottom);
                SDL_RenderFillRect(renderer, &left_top);
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &right_bottom);
                SDL_RenderFillRect(renderer, &middle);
                break;
            case 8:
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &bottom);
                SDL_RenderFillRect(renderer, &left_top);
                SDL_RenderFillRect(renderer, &left_bottom);
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &right_bottom);
                SDL_RenderFillRect(renderer, &middle);
                break;
            case 7:
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &right_bottom);
                break;
            case 6:
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &left_top);
                SDL_RenderFillRect(renderer, &middle);
                SDL_RenderFillRect(renderer, &right_bottom);
                SDL_RenderFillRect(renderer, &left_bottom);
                SDL_RenderFillRect(renderer, &bottom);
                break;
            case 5:
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &left_top);
                SDL_RenderFillRect(renderer, &middle);
                SDL_RenderFillRect(renderer, &right_bottom);
                SDL_RenderFillRect(renderer, &bottom);
                break;
            case 4:
                SDL_RenderFillRect(renderer, &left_top);
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &middle);
                SDL_RenderFillRect(renderer, &right_bottom);
                break;
            case 3:
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &middle);
                SDL_RenderFillRect(renderer, &right_bottom);
                SDL_RenderFillRect(renderer, &bottom);
                break;
            case 2:
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &middle);
                SDL_RenderFillRect(renderer, &left_bottom);
                SDL_RenderFillRect(renderer, &bottom);
                break;
            case 1:
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &right_bottom);
                break;
            case 0:
            default: // 0
                SDL_RenderFillRect(renderer, &top);
                SDL_RenderFillRect(renderer, &bottom);
                SDL_RenderFillRect(renderer, &left_top);
                SDL_RenderFillRect(renderer, &left_bottom);
                SDL_RenderFillRect(renderer, &right_top);
                SDL_RenderFillRect(renderer, &right_bottom);
                break;
        }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect ball_rect = {
        (int) (ball.x - ball.width / 2),
        (int) (ball.y - ball.height / 2),
        (int) ball.width,
        (int) ball.height
        };

    SDL_Rect paddle1_rect = {
        (int) (paddle_left.x - paddle_left.width / 2),
        (int) (paddle_left.y - paddle_left.height / 2),
        (int) paddle_left.width,
        (int) paddle_left.height
        };

    SDL_Rect paddle2_rect = {
        (int) (paddle_right.x - paddle_right.width / 2),
        (int) (paddle_right.y - paddle_right.height / 2),
        (int) paddle_right.width,
        (int) paddle_right.height
        };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    render_middle_line(renderer);
    render_score(renderer, LEFT_SCORE_X, SCORE_Y, leftScore);
    render_score(renderer, RIGHT_SCORE_X, SCORE_Y, rightScore);
    SDL_RenderFillRect(renderer, &ball_rect);
    SDL_RenderFillRect(renderer, &paddle1_rect);
    SDL_RenderFillRect(renderer, &paddle2_rect);
    SDL_RenderPresent(renderer);

}

void setup() {
    paddle_left.x = WINDOW_WIDTH / 7;
    paddle_left.y = WINDOW_HEIGHT / 2;
    paddle_left.width = PADDLE_WIDTH;
    paddle_left.height = PADDLE_HEIGHT;
    paddle_left.y_direction = 0;

    paddle_right.x = WINDOW_WIDTH - WINDOW_WIDTH / 7;
    paddle_right.y = WINDOW_HEIGHT / 2;
    paddle_right.width = PADDLE_WIDTH;
    paddle_right.height = PADDLE_HEIGHT;
    paddle_right.y_direction = 0;

    ball.width = BALL_SIZE;
    ball.height = BALL_SIZE;
    ball.x_speed = HORIZONTAL_SPEED;
    ball.y_speed = VERTICAL_SPEED;
    ball.x = 0;
    ball.y = 0;

    kick_off();
    isGoalScored = false;
    consecutive_shots = 0;
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}

int main()
{
    game_is_running = initialize_window();

    setup();

    while (game_is_running) {

        float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
        last_frame_time = SDL_GetTicks();

        process_input(delta_time);
        update(delta_time);
        render();

        if (isGoalScored) {
            SDL_Delay(1000);
            isGoalScored = false;
            last_frame_time = SDL_GetTicks();
        }
    }

    destroy_window();

    return 0;
}

#include <math.h>
#include "demo.h"
#include "gfx.h"

#define MAP_WIDTH                       (10)
#define MAP_HEIGHT                      (20)
#define MAX_LIGHT                       (10)

/* Camera variables */
static float camera_x = 5.0f, camera_y = 5.0f;
static float camera_dir_x = -1.0f, camera_dir_y = 0.0f;
static float plane_x = 0.0f, plane_y = 0.66f;
static int bricks = 0, steel = 0;

static int map[MAP_WIDTH][MAP_HEIGHT] = {
    { 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 2, 0, 0, 1 },
    { 2, 0, 0, 0, 0, 1, 1, 1, 0, 1, 2, 0, 0, 0, 0, 0, 2, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1 },
    { 2, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 2, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

void demo_init(void) {
    camera_x = 2.0f;
    camera_y = 2.0f;
    camera_dir_x = -1.0f;
    camera_dir_y = 0.0f;
    plane_x = 0.0f;
    plane_y = 0.66f;

    steel = gfx_generate_bitmap("steel.bmp");
    bricks = gfx_generate_bitmap("bricks.bmp");
}

void demo_free(void) {
    gfx_free_bitmap(bricks);
    gfx_free_bitmap(steel);
}

void demo_tick(float delta_time, int* keys) {
    float walk = 5.0f * delta_time;
    float turn = 3.0f * delta_time;

    if (keys[DEMO_INPUT_UP] == 1)
    {
        /* Move forward in the direction we are facing but clamp to the walls. */
        if (map[(int)(camera_x + camera_dir_x * walk)][(int)camera_y] == 0)
            camera_x += camera_dir_x * walk;
        if (map[(int)camera_x][(int)(camera_y + camera_dir_y * walk)] == 0)
            camera_y += camera_dir_y * walk;
    }
    if (keys[DEMO_INPUT_DOWN] == 1)
    {
        /* Move backwards in the direction we are facing but clamp to the walls. */
        if (map[(int)(camera_x - camera_dir_x * walk)][(int)camera_y] == 0)
            camera_x -= camera_dir_x * walk;
        if (map[(int)camera_x][(int)(camera_y - camera_dir_y * walk)] == 0)
            camera_y -= camera_dir_y * walk;
    }
    if (keys[DEMO_INPUT_LEFT] == 1 || keys[DEMO_INPUT_RIGHT] == 1)
    {
        float prev_dir_x, prev_plane_x;
        float final_turn = 0.0f;

        if (keys[DEMO_INPUT_LEFT] == 1)
            final_turn = turn;
        else
            final_turn = -turn;

        /* Rotate to the left, both camera direction and the entire camera plane need to be rotated. */
        prev_dir_x = camera_dir_x;
        prev_plane_x = plane_x;
        camera_dir_x = camera_dir_x * cosf(final_turn) - camera_dir_y * sinf(final_turn);
        camera_dir_y = prev_dir_x * sinf(final_turn) + camera_dir_y * cosf(final_turn);
        plane_x = plane_x * cosf(final_turn) - plane_y * sinf(final_turn);
        plane_y = prev_plane_x * sinf(final_turn) + plane_y * cosf(final_turn);
    }
}

void demo_draw(void) {
    int x = 0;
    int y = 0;

    /* We are going to need to send out a ray along each column of
     * the screen. We do not need to loop every pixel. We are going to
     * look at each column of the screen and figure out where we draw the
     * wall for that line. */
    for (x = 0; x < DEMO_WIDTH; x++) {
        /* Get the x-coordinate on the camera plane. This is the traditional camera plane
         * where 0 is in the center, -1 on the left, etc. We will use this to figure out
         * where our ray is. */
        float x_in_camera = (2 * x) / (float)(DEMO_WIDTH) - 1;
        /* Calculate the position of the ray and the direction. */
        float ray_dir_x = camera_dir_x + plane_x * x_in_camera;
        float ray_dir_y = camera_dir_y + plane_y * x_in_camera;
        /* Figure out the distances between blocks in the map that our ray has to travel throuh. This
         * is done to prevent stepping along the ray which has the chance of missing a wall. To prevent,
         * that we are using DDA ray collision detection. */
        float delta_dist_x;
        float delta_dist_y;
        /* Which block on the map we are in. */
        int map_x = (int)camera_x;
        int map_y = (int)camera_y;
        /* Length of the ray from the current position to the next x or y side of the next block in the map. */
        float side_dist_x;
        float side_dist_y;
        /* The length of the ray. */
        float perp_wall_dist;
        /* The direction that the ray will be stepping in. */
        int step_x;
        int step_y;
        /* Did the ray hit a wall? */
        int hit = 0;
        /* Was it a horizontal or vertcal face of the wall that we hit? */
        int side;
        /* The position of the wall on the column that we are drawing. */
        int line_height, line_start, line_end;
        /* The coloring of the wall. */
        int c_r, c_g, c_b, bitmap;
        /* Used to sample the wall bitmap's horizontal offset. */
        float wall_x;
        int bitmap_x;
        /* Scaling factor to use when stepping through the bitmap during the drawing. */
        float bitmap_step;
        float bitmap_current;

        if (ray_dir_x == 0) {
            delta_dist_x = 0;
            delta_dist_y = 0;
        } else if (ray_dir_y == 0) {
            delta_dist_x = 1;
            delta_dist_y = 1;
        } else {
            delta_dist_x = fabsf(1.0f / ray_dir_x);
            delta_dist_y = fabsf(1.0f / ray_dir_y);
        }

        /* Calculate the step direction and the starting side distance. */
        if (ray_dir_x < 0.0f) {
            step_x = -1;
            side_dist_x = (camera_x - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0f - camera_x) * delta_dist_x;
        }

        if (ray_dir_y < 0.0f) {
            step_y = -1;
            side_dist_y = (camera_y - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0f - camera_y) * delta_dist_y;
        }

        /* This loop is the actuall DDA collision algorithm. It will move the ray through
         * all the blocks in the map until it hits a wall. It is important that the level
         * always have walls on the edge to prevent an infinite loop here. */
        while (hit == 0) {
            /* Jump to the next block in the map in the x direction or y direction. */
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }

            /* Check to see if we hit a solid block. */
            if (map[map_x][map_y] > 0)
                hit = 1;
        }

        /* Calculate the distance projected in the camera direction. We will be using the camera
         * plane to avoid the fisheye effect. If we use the real-world distance from the camera
         * position in the scene to the wall, the distance will be longer on the sides since the
         * ray on the side is not travelling in a straight line. The distance change will cause the
         * wall to be drawn at a smaller size near the edges then it will in the center and cause the
         * rounded fisheye effect. */
        if (side == 0)
            perp_wall_dist = (map_x - camera_x + (1 - step_x) / 2) / ray_dir_x;
        else
            perp_wall_dist = (map_y - camera_y + (1 - step_y) / 2) / ray_dir_y;

        /* Recall how we are drawing the screen based on columns. We now need to figure out where the sliver
         * of wall that is on our column goes. */
        line_height = (int)((float)(DEMO_HEIGHT) / perp_wall_dist);

        line_start = -line_height / 2 + DEMO_HEIGHT / 2;
        if (line_start < 0)
            line_start = 0;

        line_end = line_height / 2 + DEMO_HEIGHT / 2;
        if (line_end > DEMO_HEIGHT)
            line_end = DEMO_HEIGHT;

        /* Pick the color of the line. We do this by sampling the correct bitmap.
         * We subtract one to account for the fact that 0 is an empty space in the
         * map but it is a legal bitmap index. */
        bitmap = map[map_x][map_y] - 1;
        if (side == 0)
            wall_x = camera_y + perp_wall_dist * ray_dir_y;
        else
            wall_x = camera_x + perp_wall_dist * ray_dir_x;
        wall_x -= floorf(wall_x);
        /* Convert wall coordinate to bitmap. */
        bitmap_x = (int)(wall_x * (float)gfx_get_bitmap_width(bitmap));
        if ((side == 0 && ray_dir_x > 0) || (side == 1 && ray_dir_y < 0))
            bitmap_x = gfx_get_bitmap_width(bitmap) - bitmap_x - 1;

        /* Figure out how much to increase the bitmap offset by per screen pixel. */
        bitmap_step = 1.0f * (float)gfx_get_bitmap_height(bitmap) / (float)line_height;
        bitmap_current = ((float)line_start - (float)DEMO_HEIGHT / 2.0f + (float)line_height / 2.0f) * bitmap_step;

        /* Figure out how far the wall is and adjust the lighting to darken things that are further away. */
        float light = (1.0f - (perp_wall_dist / MAX_LIGHT));
        light = fmaxf(light, 0.0f);
        light = fminf(light, 1.0f);

        for (y = 0; y < DEMO_HEIGHT; y++) {
            /* This means that the wall intersects the floor and ceiling and gets drawn instead. */
            if (y >= line_start && y <= line_end) {
                /* Convert to integers to allow for indexing of the color and mask with the height
                 * to make sure that we don't end up rouning up. */
                int bitmap_y = (int)bitmap_current & (gfx_get_bitmap_height(bitmap) - 1);

                /* Get the color of that pixel on the texture. */
                gfx_get_bitmap_color(bitmap, bitmap_x, bitmap_y, &c_r, &c_g, &c_b);
                /* Place the pixel on screen and apply lighting. */
                gfx_putpixel(x, y, (int)(c_r * light), (int)(c_g * light), (int)(c_b * light));

                /* Move on to the next pixel. */
                bitmap_current += bitmap_step;
            }
            /* We are on the top half which means we should draw the ceiling. */
            else if (y < (DEMO_HEIGHT / 2))
                gfx_putpixel(x, y, 80, 80, 80);
            /* We are drawing the bottom half so we want to draw the floor. */
            else
                gfx_putpixel(x, y, 10, 10, 10);
        }
    }
}
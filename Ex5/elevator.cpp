#include "elevator.h"
#include "channels.h"
#include "io.h"

#define N_FLOORS

elevator::elevator(QObject *parent) :
    QObject(parent)
{
}

int elevator::elevator_init(void)
{
    // Init hardware
    if (!io_init())
        return 0;

    // Zero all floor button lamps
    /*for (int i = 0; i < N_FLOORS; ++i) {
        if (i != 0)
            elev_set_button_lamp(BUTTON_CALL_DOWN, i, 0);

        if (i != N_FLOORS-1)
            elev_set_button_lamp(BUTTON_CALL_UP, i, 0);

        elev_set_button_lamp(BUTTON_COMMAND, i, 0);
    }

    // Clear stop lamp, door open lamp, and set floor indicator to ground floor.
    elev_set_stop_lamp(0);
    elev_set_door_open_lamp(0);
    elev_set_floor_indicator(0);
*/
    // Return success.
    return 1;
}

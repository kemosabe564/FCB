import argparse

from drone.serial import Serial
from drone.cli import CLI, CLIAction
from drone.command import Command, CommandType, SerialCommandDecoder
from drone.drone import Drone
from drone.controller import Controller
from drone.gui import GUI
from drone.joystick import Joystick, JoystickAxis
from drone.eventloop import Eventloop
from drone.crc8 import crc8

ser = None
cli = None
controller = None
gui = None
running = True


def new_cmd_handler(data):
    if type(data) == Command:
        if data.type == CommandType.DebugMessage:
            cli.to_cli("[drone debug] {}".format(data.get_data("message")))
    elif type(data) == bytearray:
        cli.to_cli(data)
    else:
        cli.to_cli(data)


def on_quit():
    ser.stop()
    cli.stop()
    controller.stop()

    global running
    running = False


def new_action_handler(action, data=None):
    if action == CLIAction.SendCommand:
        ser.send_command(data)

    if action == CLIAction.Exit:
        on_quit()

    if action == CLIAction.SetProtocol:
        ser.set_protocol(data)

    if action == CLIAction.SetTraffic:
        ser.set_print_traffic(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('--port', type=str, default='/dev/ttyUSB0', help='COM port of the drone')
    parser.add_argument('--baud', type=int, default=115200, help='Baudrate for drone')
    parser.add_argument('--no-gui', action='store_true', default=False, help='Only use CLI without GUI')

    args = parser.parse_args()

    eventloop = Eventloop()

    joystick = eventloop.joystick()
    keyboard = eventloop.keyboard()

    ser = Serial(port=args.port, baud=args.baud, command_handler=new_cmd_handler)
    cli = CLI(action_handler=new_action_handler)

    drone = Drone(ser)
    controller = Controller(drone, joystick, keyboard)

    if not args.no_gui:
        gui = GUI((800, 480), drone, controller)

    while running:
        if gui:
            gui.draw()

        eventloop.update()

    cli.join()
    ser.join()
    controller.join()

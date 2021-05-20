import argparse

from drone.serial import Serial
from drone.cli import CLI, CLIAction
from drone.command import Command
from drone.drone import Drone
from drone.controller import Controller
from drone.gui import GUI

ser = None
cli = None
controller = None
gui = None


def new_cmd_handler(data):
    if type(data) != Command:
        cli.to_cli(data)


def on_quit():
    ser.stop()
    cli.stop()
    controller.stop()
    if gui:
        gui.stop()


def new_action_handler(action, data=None):
    if action == CLIAction.SendCommand:
        ser.send_command(data)

    if action == CLIAction.Exit:
        on_quit()

    if action == CLIAction.SetProtocol:
        ser.set_protocol(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('--port', type=str, default='/dev/ttyUSB0', help='COM port of the drone')
    parser.add_argument('--baud', type=int, default=115200, help='Baudrate for drone')
    parser.add_argument('--no-gui', action='store_true', default=False, help='Only use CLI without GUI')

    args = parser.parse_args()

    ser = Serial(port=args.port, baud=args.baud, command_handler=new_cmd_handler)
    cli = CLI(action_handler=new_action_handler)
    drone = Drone(ser)
    controller = Controller(drone)

    if not args.no_gui:
        gui = GUI((800, 480), drone)
        gui.set_on_quit(on_quit)

        # PyGame needs to run on the main thread...
        gui.main_loop()

    cli.join()
    ser.join()
    controller.join()

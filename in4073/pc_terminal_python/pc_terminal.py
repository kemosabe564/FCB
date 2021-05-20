import argparse

from drone.serial import Serial
from drone.cli import CLI, CLIAction
from drone.drone import Drone
from drone.controller import Controller

ser = None
cli = None
controller = None


def new_cmd_handler(data):
    # cli.to_cli(data)
    pass


def new_action_handler(action, data=None):
    if action == CLIAction.SendCommand:
        ser.send_command(data)

    if action == CLIAction.Exit:
        ser.stop()
        cli.stop()
        controller.stop()

    if action == CLIAction.SetProtocol:
        ser.set_protocol(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('--port', type=str, default='/dev/ttyUSB0', help='COM port of the drone')
    parser.add_argument('--baud', type=int, default=115200, help='Baudrate for drone')

    args = parser.parse_args()

    ser = Serial(port=args.port, baud=args.baud, command_handler=new_cmd_handler)
    cli = CLI(action_handler=new_action_handler)
    drone = Drone(ser)
    controller = Controller(drone)


    cli.join()
    ser.join()
    controller.join()

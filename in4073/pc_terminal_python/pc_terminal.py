import pygame
import argparse

from communication.serial import Serial
from communication.command import Command, CommandType
from communication.cli import CLI, CLIAction

ser = None
cli = None


def new_cmd_handler(data):
    cli.to_cli(data)


def new_action_handler(action, data=None):
    if action == CLIAction.SendCommand:
        ser.send_command(data)

    if action == CLIAction.Exit:
        ser.stop()
        cli.stop()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('--port', type=str, default='/dev/ttyUSB0', help='COM port of the drone')
    parser.add_argument('--baud', type=int, default=115200, help='Baudrate for communication')

    args = parser.parse_args()

    ser = Serial(port=args.port, baud=args.baud, command_handler=new_cmd_handler)
    cli = CLI(action_handler=new_action_handler)

    cli.join()
    ser.join()
